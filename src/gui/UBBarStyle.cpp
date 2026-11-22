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



#include "UBBarStyle.h"

#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QCursor>
#include <QDateTime>
#include <QGraphicsOpacityEffect>
#include <QMessageBox>
#include <QPointer>
#include <QPropertyAnimation>
#include <QRadioButton>
#include <QTimer>
#include <QDialog>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QSlider>
#include <QToolButton>
#include <QVBoxLayout>

#include "board/UBBoardController.h"
#include "board/UBBoardPaletteManager.h"
#include "core/UBApplication.h"
#include "core/UBApplicationController.h"
#include "core/UBSettings.h"
#include "desktop/UBDesktopAnnotationController.h"
#include "desktop/UBDesktopPalette.h"
#include "gui/UBFloatingPalette.h"
#include "gui/UBStylusPalette.h"

static const QString sAutoHideKey = QStringLiteral("App/LibertyBoardBarAutoHide");
static const QString sHideEffectKey = QStringLiteral("App/LibertyBoardBarHideEffect");

void UBBarStyle::paint(QPainter& painter, const QRectF& bar, int edge)
{
    UBSettings* settings = UBSettings::settings();
    const qreal opacity = qBound(10, settings->barOpacity->get().toInt(), 100) / 100.0;

    QColor inner(settings->barInnerColor->get().toString());
    QColor outer(settings->barOuterColor->get().toString());
    inner.setAlphaF(opacity);
    outer.setAlphaF(opacity);

    // From square corners at 0 to a round end at 100.
    const qreal roundness = qBound(0, settings->barRoundness->get().toInt(), 100) / 100.0;
    const qreal radius = roundness * qMin(bar.width(), bar.height()) / 2;
    const QRectF inside = bar.adjusted(edge, edge, -edge, -edge);
    const qreal insideRadius = qMax<qreal>(0, radius - edge);

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);

    // The edge as a ring, so that the inside is not painted over it.
    QPainterPath ring;
    ring.addRoundedRect(bar, radius, radius);
    ring.addRoundedRect(inside, insideRadius, insideRadius);
    painter.setBrush(outer);
    painter.drawPath(ring);

    painter.setBrush(inner);
    painter.drawRoundedRect(inside, insideRadius, insideRadius);
    painter.restore();
}

void UBBarStyle::repaintPalettes()
{
    for (QWidget* widget : QApplication::allWidgets())
    {
        if (qobject_cast<UBFloatingPalette*>(widget))
        {
            widget->update();
        }
    }
}

int UBBarStyle::edge()
{
    return UBStylusPalette::edge();
}

void UBBarStyle::placeBars(int edge)
{
    if (UBApplication::boardController && UBApplication::boardController->paletteManager())
    {
        UBApplication::boardController->paletteManager()->placeStylusPalette(edge);
    }

    if (UBApplication::applicationController && UBApplication::applicationController->uninotesController())
    {
        if (UBDesktopPalette* desktop = UBApplication::applicationController->uninotesController()->desktopPalette())
        {
            desktop->placeAtEdge(edge);
        }
    }
}

bool UBBarStyle::autoHide()
{
    return UBSettings::settings()->value(sAutoHideKey, false).toBool();
}

void UBBarStyle::setAutoHide(bool on)
{
    UBSettings::settings()->setValue(sAutoHideKey, on);
}

UBBarStyle::HideEffect UBBarStyle::hideEffect()
{
    return UBSettings::settings()->value(sHideEffectKey, 0).toInt() == 1 ? HideEffect::Fade : HideEffect::Slide;
}

void UBBarStyle::setHideEffect(HideEffect effect)
{
    UBSettings::settings()->setValue(sHideEffectKey, int(effect));
}

bool UBBarStyle::askHideEffect(QWidget* parent)
{
    QMessageBox box(parent);
    box.setIcon(QMessageBox::Question);
    box.setWindowTitle(QObject::tr("Hide automatically"));
    box.setText(QObject::tr("How should the bar hide itself? It comes back the same way when the pointer goes near its side."));
    QPushButton* slide = box.addButton(QObject::tr("Slide toward its side"), QMessageBox::AcceptRole);
    QPushButton* fade = box.addButton(QObject::tr("Fade out"), QMessageBox::AcceptRole);
    box.addButton(QMessageBox::Cancel);
    box.setDefaultButton(slide);
    box.exec();

    if (box.clickedButton() == slide)
    {
        setHideEffect(HideEffect::Slide);
        return true;
    }

    if (box.clickedButton() == fade)
    {
        setHideEffect(HideEffect::Fade);
        return true;
    }

    return false;
}

namespace
{
    // A palette that hides itself: moved out of its parent, past its edge,
    // so that nothing else needs to know (it stays shown for the program).
    struct Watched
    {
        QPointer<QWidget> palette;
        bool hidden{false};
        int edge{Qt::RightEdge};
        QPoint home;
        QPoint away;
        qint64 outsideSince{0};
        QPointer<QAbstractAnimation> animation;
    };

    QList<Watched>& watchedPalettes()
    {
        static QList<Watched> palettes;
        return palettes;
    }

    QRect globalRect(QWidget* widget)
    {
        return QRect(widget->mapToGlobal(QPoint(0, 0)), widget->size());
    }

    // The edge of its parent the palette stands at: the one chosen for the
    // palette of the board, else the closest one.
    int nearestEdge(QWidget* palette)
    {
        if (qobject_cast<UBStylusPalette*>(palette))
        {
            return UBStylusPalette::edge();
        }

        // A tall palette stands at the left or the right, a wide one at the
        // top or the bottom: the closer of the two.
        const QRect area = palette->parentWidget()->rect();
        const QRect bar = palette->geometry();

        if (bar.height() >= bar.width())
        {
            return (bar.left() - area.left() <= area.right() - bar.right()) ? Qt::LeftEdge : Qt::RightEdge;
        }

        return (bar.top() - area.top() <= area.bottom() - bar.bottom()) ? Qt::TopEdge : Qt::BottomEdge;
    }

    // The pointer near the side of the palette: within its gap to the edge, and a little more.
    bool nearSide(const Watched& watched, const QPoint& pointer)
    {
        QWidget* parent = watched.palette->parentWidget();
        const QRect area = globalRect(parent);

        if (!area.adjusted(-2, -2, 2, 2).contains(pointer))
        {
            return false;
        }

        const QRect home(parent->mapToGlobal(watched.home), watched.palette->size());
        const int reach = 14;

        switch (watched.edge)
        {
            case Qt::LeftEdge:
                return pointer.x() <= qMax(area.left() + reach, home.left() + 6);
            case Qt::TopEdge:
                return pointer.y() <= qMax(area.top() + reach, home.top() + 6);
            case Qt::BottomEdge:
                return pointer.y() >= qMin(area.bottom() - reach, home.bottom() - 6);
            default:
                return pointer.x() >= qMin(area.right() - reach, home.right() - 6);
        }
    }

    QPoint awayFrom(QWidget* palette, int edge, const QPoint& home)
    {
        QWidget* parent = palette->parentWidget();

        switch (edge)
        {
            case Qt::LeftEdge:
                return QPoint(-palette->width() - 4, home.y());
            case Qt::TopEdge:
                return QPoint(home.x(), -palette->height() - 4);
            case Qt::BottomEdge:
                return QPoint(home.x(), parent->height() + 4);
            default:
                return QPoint(parent->width() + 4, home.y());
        }
    }

    void refreshMask(QWidget* palette)
    {
        // The desktop palette: its transparent window follows it.
        QMetaObject::invokeMethod(palette, "refreshMask");
    }

    void hidePalette(Watched& watched)
    {
        QWidget* palette = watched.palette;
        watched.home = palette->pos();
        watched.edge = nearestEdge(palette);
        watched.away = awayFrom(palette, watched.edge, watched.home);
        watched.hidden = true;
        watched.outsideSince = 0;
        const QPoint away = watched.away;

        if (UBBarStyle::hideEffect() == UBBarStyle::HideEffect::Fade)
        {
            auto effect = new QGraphicsOpacityEffect(palette);
            effect->setOpacity(1.0);
            palette->setGraphicsEffect(effect);
            auto animation = new QPropertyAnimation(effect, "opacity", palette);
            animation->setDuration(300);
            animation->setStartValue(1.0);
            animation->setEndValue(0.0);
            QObject::connect(animation, &QPropertyAnimation::finished, palette, [palette, away]() {
                palette->move(away);
                palette->setGraphicsEffect(nullptr);
                refreshMask(palette);
            });
            watched.animation = animation;
            animation->start(QAbstractAnimation::DeleteWhenStopped);
        }
        else
        {
            auto animation = new QPropertyAnimation(palette, "pos", palette);
            animation->setDuration(250);
            animation->setEasingCurve(QEasingCurve::InCubic);
            animation->setStartValue(watched.home);
            animation->setEndValue(away);
            QObject::connect(animation, &QPropertyAnimation::finished, palette, [palette]() { refreshMask(palette); });
            watched.animation = animation;
            animation->start(QAbstractAnimation::DeleteWhenStopped);
        }
    }

    void showPalette(Watched& watched)
    {
        QWidget* palette = watched.palette;
        watched.hidden = false;
        watched.outsideSince = 0;

        if (UBBarStyle::hideEffect() == UBBarStyle::HideEffect::Fade)
        {
            auto effect = new QGraphicsOpacityEffect(palette);
            effect->setOpacity(0.0);
            palette->setGraphicsEffect(effect);
            palette->move(watched.home);
            refreshMask(palette);
            auto animation = new QPropertyAnimation(effect, "opacity", palette);
            animation->setDuration(300);
            animation->setStartValue(0.0);
            animation->setEndValue(1.0);
            QObject::connect(animation, &QPropertyAnimation::finished, palette, [palette]() { palette->setGraphicsEffect(nullptr); });
            watched.animation = animation;
            animation->start(QAbstractAnimation::DeleteWhenStopped);
        }
        else
        {
            auto animation = new QPropertyAnimation(palette, "pos", palette);
            animation->setDuration(250);
            animation->setEasingCurve(QEasingCurve::OutCubic);
            animation->setStartValue(palette->pos());
            animation->setEndValue(watched.home);
            QObject::connect(animation, &QPropertyAnimation::finished, palette, [palette]() { refreshMask(palette); });
            watched.animation = animation;
            animation->start(QAbstractAnimation::DeleteWhenStopped);
        }
    }

    void checkPalettes()
    {
        QList<Watched>& palettes = watchedPalettes();
        const QPoint pointer = QCursor::pos();
        const qint64 now = QDateTime::currentMSecsSinceEpoch();
        const bool on = UBBarStyle::autoHide();

        for (int i = palettes.size() - 1; i >= 0; --i)
        {
            Watched& watched = palettes[i];

            if (!watched.palette)
            {
                palettes.removeAt(i);
                continue;
            }

            QWidget* palette = watched.palette;

            if (watched.animation || !palette->isVisible() || !palette->parentWidget())
            {
                continue;
            }

            // Placed again by the program while hidden: it is shown there.
            if (watched.hidden && palette->pos() != watched.away)
            {
                watched.hidden = false;
                watched.outsideSince = 0;
                continue;
            }

            if (watched.hidden)
            {
                if (!on || nearSide(watched, pointer))
                {
                    showPalette(watched);
                }

                continue;
            }

            if (!on)
            {
                continue;
            }

            watched.home = palette->pos();
            watched.edge = nearestEdge(palette);

            // Kept while it is used: the pointer on it or near its side, a
            // button held, a menu or a window of its options open.
            const bool busy = globalRect(palette).adjusted(-24, -24, 24, 24).contains(pointer)
                              || nearSide(watched, pointer)
                              || QApplication::mouseButtons() != Qt::NoButton
                              || QApplication::activePopupWidget()
                              || QApplication::activeModalWidget();

            if (busy)
            {
                watched.outsideSince = 0;
            }
            else if (!watched.outsideSince)
            {
                watched.outsideSince = now;
            }
            else if (now - watched.outsideSince > 900)
            {
                hidePalette(watched);
            }
        }
    }
}

void UBBarStyle::watch(QWidget* palette)
{
    static QTimer* timer = nullptr;

    if (!timer)
    {
        timer = new QTimer(qApp);
        timer->setInterval(80);
        QObject::connect(timer, &QTimer::timeout, qApp, &checkPalettes);
        timer->start();
    }

    Watched watched;
    watched.palette = palette;
    watchedPalettes() << watched;
}

void UBBarStyle::openWindow(QWidget* parent)
{
    UBSettings* settings = UBSettings::settings();

    // Given back by Cancel.
    const QVariant innerBefore = settings->barInnerColor->get();
    const QVariant outerBefore = settings->barOuterColor->get();
    const QVariant opacityBefore = settings->barOpacity->get();
    const QVariant roundnessBefore = settings->barRoundness->get();
    const int edgeBefore = edge();
    const bool autoHideBefore = autoHide();
    const HideEffect effectBefore = hideEffect();

    QDialog dialog(parent);
    dialog.setWindowTitle(QObject::tr("Bar style"));
    auto layout = new QVBoxLayout(&dialog);

    auto help = new QLabel(QObject::tr("The style of the tool palettes, on the board and on the desktop. "
                                       "The palettes change at once."), &dialog);
    help->setWordWrap(true);
    layout->addWidget(help);

    auto grid = new QGridLayout();
    layout->addLayout(grid);

    auto colorButton = [&dialog](UBSetting* setting) {
        auto button = new QToolButton(&dialog);
        button->setFixedSize(64, 26);

        auto show = [button](const QColor& color) {
            button->setStyleSheet(QString("QToolButton { background: %1; border: 1px solid #555555; border-radius: 4px; }").arg(color.name()));
        };
        show(QColor(setting->get().toString()));

        QObject::connect(button, &QToolButton::clicked, &dialog, [&dialog, setting, show]() {
            const QColor color = QColorDialog::getColor(QColor(setting->get().toString()), &dialog);

            if (color.isValid())
            {
                setting->set(color.name());
                show(color);
                repaintPalettes();
            }
        });

        // Restore gives the default back: the button follows.
        QObject::connect(setting, &UBSetting::changed, button, [show](const QVariant& value) { show(QColor(value.toString())); });
        return button;
    };

    grid->addWidget(new QLabel(QObject::tr("Color inside"), &dialog), 0, 0);
    grid->addWidget(colorButton(settings->barInnerColor), 0, 1, Qt::AlignLeft);
    grid->addWidget(new QLabel(QObject::tr("Color of the edge"), &dialog), 1, 0);
    grid->addWidget(colorButton(settings->barOuterColor), 1, 1, Qt::AlignLeft);

    auto slider = [&dialog](UBSetting* setting, int minimum, const QString& left, const QString& right) {
        auto row = new QWidget(&dialog);
        auto rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(0, 0, 0, 0);
        auto value = new QSlider(Qt::Horizontal, row);
        value->setRange(minimum, 100);
        value->setValue(setting->get().toInt());
        value->setMinimumWidth(240);
        auto number = new QLabel(QString("%1 %").arg(value->value()), row);
        number->setMinimumWidth(44);

        if (!left.isEmpty())
        {
            rowLayout->addWidget(new QLabel(left, row));
        }

        rowLayout->addWidget(value, 1);

        if (!right.isEmpty())
        {
            rowLayout->addWidget(new QLabel(right, row));
        }

        rowLayout->addWidget(number);

        QObject::connect(value, &QSlider::valueChanged, row, [setting, number](int chosen) {
            setting->set(chosen);
            number->setText(QString("%1 %").arg(chosen));
            repaintPalettes();
        });
        QObject::connect(setting, &UBSetting::changed, value, [value](const QVariant& chosen) {
            const QSignalBlocker blocker(value);
            value->setValue(chosen.toInt());
        });
        return row;
    };

    grid->addWidget(new QLabel(QObject::tr("Opacity"), &dialog), 2, 0);
    grid->addWidget(slider(settings->barOpacity, 10, QString(), QString()), 2, 1);
    grid->addWidget(new QLabel(QObject::tr("Corners"), &dialog), 3, 0);
    grid->addWidget(slider(settings->barRoundness, 0, QObject::tr("Square"), QObject::tr("Round")), 3, 1);

    // Where the bars stand: as when they are dropped near that edge.
    auto location = new QWidget(&dialog);
    auto locationLayout = new QHBoxLayout(location);
    locationLayout->setContentsMargins(0, 0, 0, 0);
    auto edges = new QButtonGroup(location);
    const QList<QPair<QString, int>> edgeNames = { { QObject::tr("Top"), Qt::TopEdge }, { QObject::tr("Bottom"), Qt::BottomEdge },
                                                   { QObject::tr("Left"), Qt::LeftEdge }, { QObject::tr("Right"), Qt::RightEdge } };

    for (const auto& name : edgeNames)
    {
        auto choice = new QRadioButton(name.first, location);
        choice->setChecked(name.second == edgeBefore);
        edges->addButton(choice, name.second);
        locationLayout->addWidget(choice);
    }

    locationLayout->addStretch(1);
    QObject::connect(edges, &QButtonGroup::idClicked, &dialog, [](int chosen) { placeBars(chosen); });
    grid->addWidget(new QLabel(QObject::tr("Location"), &dialog), 4, 0);
    grid->addWidget(location, 4, 1);

    // Hidden until the pointer goes near their side.
    auto hiding = new QWidget(&dialog);
    auto hidingLayout = new QHBoxLayout(hiding);
    hidingLayout->setContentsMargins(0, 0, 0, 0);
    auto hide = new QCheckBox(QObject::tr("Hide automatically"), hiding);
    hide->setChecked(autoHideBefore);
    auto effect = new QComboBox(hiding);
    effect->addItem(QObject::tr("Slide toward its side"), int(HideEffect::Slide));
    effect->addItem(QObject::tr("Fade out"), int(HideEffect::Fade));
    effect->setCurrentIndex(int(effectBefore));
    effect->setEnabled(autoHideBefore);
    hidingLayout->addWidget(hide);
    hidingLayout->addWidget(effect);
    hidingLayout->addStretch(1);
    grid->addWidget(hiding, 5, 1);

    QObject::connect(hide, &QCheckBox::clicked, &dialog, [&dialog, hide, effect](bool on) {
        if (on && !askHideEffect(&dialog))
        {
            hide->setChecked(false);
            return;
        }

        effect->setCurrentIndex(int(hideEffect()));
        effect->setEnabled(on);
        setAutoHide(on);
    });
    QObject::connect(effect, qOverload<int>(&QComboBox::currentIndexChanged), &dialog, [](int index) {
        setHideEffect(index == 1 ? HideEffect::Fade : HideEffect::Slide);
    });

    auto buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::RestoreDefaults, &dialog);
    layout->addWidget(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    QObject::connect(buttons->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked, &dialog, [settings]() {
        settings->barInnerColor->reset();
        settings->barOuterColor->reset();
        settings->barOpacity->reset();
        settings->barRoundness->reset();
        repaintPalettes();
    });

    if (dialog.exec() == QDialog::Accepted)
    {
        settings->save();
        return;
    }

    settings->barInnerColor->set(innerBefore);
    settings->barOuterColor->set(outerBefore);
    settings->barOpacity->set(opacityBefore);
    settings->barRoundness->set(roundnessBefore);
    setAutoHide(autoHideBefore);
    setHideEffect(effectBefore);

    if (edge() != edgeBefore)
    {
        placeBars(edgeBefore);
    }

    repaintPalettes();
}
