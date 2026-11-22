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



#include "UBCanvasRulers.h"

#include <QGraphicsView>
#include <QPainter>
#include <QTimer>
#include <QtMath>

#include "board/UBBoardController.h"
#include "board/UBBoardView.h"
#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "domain/UBGraphicsPDFItem.h"
#include "domain/UBGraphicsScene.h"

static const QString sShownKey = QStringLiteral("Board/LibertyBoardCanvasRulers");
static constexpr int sThickness = 24;

static QPointer<UBCanvasRulers> sTop;
static QPointer<UBCanvasRulers> sLeft;

void UBCanvasRulers::setShown(bool shown)
{
    UBSettings::settings()->setValue(sShownKey, shown);
    UBBoardView* view = UBApplication::boardController->controlView();

    if (shown && view && !sTop)
    {
        sTop = new UBCanvasRulers(Qt::Horizontal, view);
        sLeft = new UBCanvasRulers(Qt::Vertical, view);
    }

    for (UBCanvasRulers* ruler : { sTop.data(), sLeft.data() })
    {
        if (ruler)
        {
            ruler->setVisible(shown);
        }
    }
}

bool UBCanvasRulers::isShown()
{
    return UBSettings::settings()->value(sShownKey, false).toBool();
}

UBCanvasRulers::UBCanvasRulers(Qt::Orientation orientation, QGraphicsView* view)
    : QWidget(view->viewport())
    , mOrientation(orientation)
    , mView(view)
{
    setAttribute(Qt::WA_TransparentForMouseEvents);

    // The page scrolls and zooms under the rulers; the board changes size.
    auto timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        place();
        update();
    });
    timer->start(60);
    place();
    show();
    raise();
}

void UBCanvasRulers::place()
{
    QWidget* area = parentWidget();

    if (mOrientation == Qt::Horizontal)
    {
        setGeometry(0, 0, area->width(), sThickness);
    }
    else
    {
        setGeometry(0, 0, sThickness, area->height());
    }

    raise();
}

QPointF UBCanvasRulers::origin() const
{
    const auto scene = UBApplication::boardController->activeScene();

    if (!scene)
    {
        return QPointF();
    }

    if (auto pdf = qgraphicsitem_cast<UBGraphicsPDFItem*>(scene->backgroundObject()))
    {
        return pdf->sceneBoundingRect().topLeft();
    }

    const QSizeF size = scene->nominalSize();
    return QPointF(-size.width() / 2, -size.height() / 2);
}

void UBCanvasRulers::paintEvent(QPaintEvent*)
{
    if (!mView)
    {
        return;
    }

    QPainter painter(this);
    painter.fillRect(rect(), QColor(236, 238, 242, 235));
    painter.setPen(QColor(150, 156, 166));

    const bool horizontal = mOrientation == Qt::Horizontal;

    if (horizontal)
    {
        painter.drawLine(0, height() - 1, width(), height() - 1);
    }
    else
    {
        painter.drawLine(width() - 1, 0, width() - 1, height());
    }

    // Pixels of the screen for one pixel of the page.
    const qreal scale = horizontal ? mView->transform().m11() : mView->transform().m22();

    if (scale <= 0)
    {
        return;
    }

    // A labelled step of at least 50 pixels on the screen, at least 5 of the page.
    static const QList<int> steps = { 5, 10, 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000 };
    int step = steps.last();

    for (int candidate : steps)
    {
        if (candidate * scale >= 50)
        {
            step = candidate;
            break;
        }
    }

    const int minor = step % 5 == 0 ? step / 5 : step / 2;
    const QPointF zero = origin();
    const QPointF first = mView->mapToScene(QPoint(0, 0)) - zero;
    const QPointF last = mView->mapToScene(QPoint(mView->viewport()->width(), mView->viewport()->height())) - zero;
    const qreal from = horizontal ? first.x() : first.y();
    const qreal to = horizontal ? last.x() : last.y();

    QFont font = painter.font();
    font.setPixelSize(10);
    painter.setFont(font);

    for (qint64 value = qint64(qFloor(from / minor)) * minor; value <= to; value += minor)
    {
        const QPointF scenePoint = zero + (horizontal ? QPointF(value, 0) : QPointF(0, value));
        const QPointF onScreen = mView->mapFromScene(scenePoint);
        const int position = horizontal ? onScreen.x() : onScreen.y();
        const bool major = value % step == 0;
        const int length = major ? sThickness / 2 : sThickness / 5;

        painter.setPen(QColor(90, 96, 106));

        if (horizontal)
        {
            painter.drawLine(position, height() - length, position, height() - 1);

            if (major)
            {
                painter.drawText(QRect(position + 2, 1, 60, 12), Qt::AlignLeft | Qt::AlignTop, QString::number(value));
            }
        }
        else
        {
            painter.drawLine(width() - length, position, width() - 1, position);

            if (major)
            {
                // Written upward, as on the rulers of the PDF readers.
                painter.save();
                painter.translate(11, position - 2);
                painter.rotate(-90);
                painter.drawText(QRect(0, -10, 60, 12), Qt::AlignLeft | Qt::AlignTop, QString::number(value));
                painter.restore();
            }
        }
    }
}
