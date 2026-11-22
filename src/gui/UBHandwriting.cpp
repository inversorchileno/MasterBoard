/*
 * Copyright (C) 2015-2026 Département de l'Instruction Publique (DIP-SEM)
 *
 * This file is part of OpenBoard.
 *
 * OpenBoard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * OpenBoard is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenBoard. If not, see <http://www.gnu.org/licenses/>.
 */




#ifdef _WIN32
// The handwriting recognition of Windows, through its runtime (C++/WinRT).
// _WIN32 rather than Q_OS_WIN: no header of Qt is read yet here.
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.UI.Input.Inking.h>
#endif

#include "UBHandwriting.h"

#include <QApplication>
#include <QLocale>
#include <QPointer>
#include <thread>

#include "board/UBBoardController.h"
#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "domain/UBGraphicsItemUndoCommand.h"
#include "domain/UBGraphicsPolygonItem.h"
#include "domain/UBGraphicsScene.h"
#include "domain/UBGraphicsStroke.h"
#include "domain/UBGraphicsStrokesGroup.h"
#include "domain/UBGraphicsTextItem.h"
#include "frameworks/UBPlatformUtils.h"

static const QString sEnabledKey = QStringLiteral("Board/LibertyBoardHandwriting");

UBHandwriting* UBHandwriting::instance()
{
    static UBHandwriting handwriting;
    return &handwriting;
}

UBHandwriting::UBHandwriting()
{
    // The words are read once the pen has rested a moment.
    mRest.setSingleShot(true);
    mRest.setInterval(1200);
    connect(&mRest, &QTimer::timeout, this, &UBHandwriting::recognizePending);
}

bool UBHandwriting::isAvailable()
{
#ifdef Q_OS_WIN
    return true;
#else
    // For the tests on other systems: a fixed text instead of the recognition.
    return !qEnvironmentVariableIsEmpty("MASTERBOARD_FAKE_HANDWRITING");
#endif
}

bool UBHandwriting::isEnabled()
{
    return isAvailable() && UBSettings::settings()->value(sEnabledKey, false).toBool();
}

void UBHandwriting::setEnabled(bool enabled)
{
    UBSettings::settings()->setValue(sEnabledKey, enabled);
}

void UBHandwriting::addWritten(std::shared_ptr<UBGraphicsScene> scene, const QList<UBGraphicsStrokesGroup*>& groups)
{
    if (!isEnabled() || groups.isEmpty())
    {
        return;
    }

    // Another page: what was waiting on the previous one is read now.
    if (mScene.lock() != scene && !mPending.isEmpty())
    {
        recognizePending();
    }

    mScene = scene;
    mPending << groups;
    mRest.start();
}

void UBHandwriting::recognizePending()
{
    mRest.stop();
    std::shared_ptr<UBGraphicsScene> scene = mScene.lock();
    const QList<UBGraphicsStrokesGroup*> groups = mPending;
    mPending.clear();

    if (!scene || groups.isEmpty())
    {
        return;
    }

    // The points of the pen, as it went: the recognizer reads the movement.
    const QList<QGraphicsItem*> onPage = scene->items();
    QList<QPolygonF> strokes;

    for (UBGraphicsStrokesGroup* group : groups)
    {
        if (!onPage.contains(group))
        {
            continue;
        }

        QList<UBGraphicsStroke*> seen;

        for (QGraphicsItem* child : group->childItems())
        {
            auto polygon = dynamic_cast<UBGraphicsPolygonItem*>(child);
            UBGraphicsStroke* stroke = polygon ? polygon->stroke() : nullptr;

            if (!stroke || seen.contains(stroke))
            {
                continue;
            }

            seen << stroke;
            QPolygonF points;

            for (const auto& point : stroke->points())
            {
                points << group->mapToScene(point.first);
            }

            if (points.size() >= 2)
            {
                strokes << points;
            }
        }
    }

    if (strokes.isEmpty())
    {
        return;
    }

    QString language = UBSettings::settings()->appPreferredLanguage->get().toString();

    if (language.isEmpty())
    {
        language = UBPlatformUtils::systemLanguage();
    }

    // Read away from the window, which goes on meanwhile.
    std::weak_ptr<UBGraphicsScene> weakScene = scene;
    QPointer<UBHandwriting> self(this);

    std::thread([self, weakScene, groups, strokes, language]() {
        const QString text = recognize(strokes, language);

        QMetaObject::invokeMethod(qApp, [self, weakScene, groups, text]() {
            if (self)
            {
                self->replaceByText(weakScene, groups, text);
            }
        });
    }).detach();
}

void UBHandwriting::replaceByText(std::weak_ptr<UBGraphicsScene> weakScene, QList<UBGraphicsStrokesGroup*> groups, const QString& text)
{
    std::shared_ptr<UBGraphicsScene> scene = weakScene.lock();

    if (!scene || text.trimmed().isEmpty())
    {
        return;
    }

    // Only the lines still on the page, in case some were erased meanwhile.
    const QList<QGraphicsItem*> onPage = scene->items();
    QSet<QGraphicsItem*> removed;
    QRectF bounds;
    QColor color;

    for (UBGraphicsStrokesGroup* group : groups)
    {
        if (!onPage.contains(group))
        {
            continue;
        }

        bounds |= group->sceneBoundingRect();
        removed << group;

        if (!color.isValid())
        {
            for (QGraphicsItem* child : group->childItems())
            {
                if (auto polygon = dynamic_cast<UBGraphicsPolygonItem*>(child))
                {
                    color = polygon->color();
                    color.setAlpha(255);
                    break;
                }
            }
        }
    }

    if (removed.isEmpty())
    {
        return;
    }

    UBApplication::undoStack->beginMacro(tr("Convert handwriting"));

    for (QGraphicsItem* item : removed)
    {
        scene->removeItem(item);
    }

    UBApplication::undoStack->push(new UBGraphicsItemUndoCommand(scene, removed, QSet<QGraphicsItem*>()));

    // The text of about the height of the writing, at its place.
    const int lines = qMax(1, int(text.count('\n')) + 1);
    const int pointSize = qBound(8, qRound(bounds.height() / lines * 0.55), 200);
    UBGraphicsTextItem* textItem = scene->addTextWithFont(text.trimmed(), bounds.topLeft(), pointSize);

    if (textItem && color.isValid())
    {
        textItem->setDefaultTextColor(color);
        textItem->setColorOnLightBackground(color);
        textItem->setColorOnDarkBackground(color);
    }

    UBApplication::undoStack->endMacro();
    scene->setModified(true);
}

QString UBHandwriting::recognize(const QList<QPolygonF>& strokes, const QString& language)
{
#ifdef Q_OS_WIN
    QString text;

    try
    {
        winrt::init_apartment(winrt::apartment_type::multi_threaded);

        using namespace winrt::Windows::UI::Input::Inking;
        using winrt::Windows::Foundation::Point;

        InkStrokeBuilder builder;
        InkStrokeContainer container;

        for (const QPolygonF& stroke : strokes)
        {
            std::vector<Point> points;

            for (const QPointF& point : stroke)
            {
                points.push_back(Point(float(point.x()), float(point.y())));
            }

            container.AddStroke(builder.CreateStroke(points));
        }

        // The recognizer of the language of Master Board, when Windows has it;
        // else the one of Windows.
        InkRecognizerContainer recognizers;
        const QLocale locale(language);
        const QString nativeName = locale.nativeLanguageName();
        const QString englishName = QLocale::languageToString(locale.language());

        for (const InkRecognizer& recognizer : recognizers.GetRecognizers())
        {
            const QString name = QString::fromWCharArray(recognizer.Name().c_str());

            if ((!nativeName.isEmpty() && name.contains(nativeName, Qt::CaseInsensitive))
                || (!englishName.isEmpty() && name.contains(englishName, Qt::CaseInsensitive)))
            {
                recognizers.SetDefaultRecognizer(recognizer);
                break;
            }
        }

        const auto results = recognizers.RecognizeAsync(container, InkRecognitionTarget::All).get();
        qreal lastBottom = -1;

        for (const InkRecognitionResult& result : results)
        {
            const auto candidates = result.GetTextCandidates();

            if (candidates.Size() == 0)
            {
                continue;
            }

            // A word below the previous ones starts a new line.
            const auto box = result.BoundingRect();

            if (!text.isEmpty())
            {
                text += (lastBottom >= 0 && box.Y > lastBottom) ? QStringLiteral("\n") : QStringLiteral(" ");
            }

            text += QString::fromWCharArray(candidates.GetAt(0).c_str());
            lastBottom = qMax(lastBottom, qreal(box.Y + box.Height));
        }
    }
    catch (...)
    {
        text.clear();
    }

    winrt::uninit_apartment();
    return text;
#else
    Q_UNUSED(strokes);
    Q_UNUSED(language);
    return qEnvironmentVariable("MASTERBOARD_FAKE_HANDWRITING");
#endif
}
