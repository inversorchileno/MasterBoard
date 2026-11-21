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

#include "UBSpotlight.h"

#include <QApplication>
#include <QCursor>
#include <QGuiApplication>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>

#include "core/UBSetting.h"
#include "core/UBSettings.h"
#include "frameworks/UBPlatformUtils.h"

UBSpotlight::UBSpotlight(QWidget* parent)
    : QWidget(parent, Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint
                      | Qt::WindowTransparentForInput | Qt::WindowDoesNotAcceptFocus | Qt::X11BypassWindowManagerHint)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_TransparentForMouseEvents);

    mTimer.setInterval(16);
    connect(&mTimer, &QTimer::timeout, this, &UBSpotlight::followPointer);
}

void UBSpotlight::setActive(bool active)
{
    if (!active)
    {
        mTimer.stop();
        hide();
        return;
    }

    QScreen* screen = QGuiApplication::screenAt(QCursor::pos());

    if (!screen)
    {
        screen = QGuiApplication::primaryScreen();
    }

    if (screen)
    {
        setGeometry(screen->geometry());
    }

    mPointer = QCursor::pos() - geometry().topLeft();
    show();
    raise();

    // The magnifier shows a picture of the screen, which must not hold it.
    mHiddenFromCapture = UBPlatformUtils::hideFromScreenCapture(this);
    mTimer.start();
}

void UBSpotlight::followPointer()
{
    const QPoint global = QCursor::pos();

    // To the screen of the pointer.
    if (!geometry().contains(global))
    {
        if (QScreen* screen = QGuiApplication::screenAt(global))
        {
            setGeometry(screen->geometry());
        }
    }

    const QPoint pointer = global - geometry().topLeft();
    const bool magnifier = UBSettings::settings()->spotlightMode->get().toInt() == 1;

    // The magnifier follows what moves under it, the spotlight only the pointer.
    if (pointer != mPointer || magnifier)
    {
        mPointer = pointer;
        update();
    }

    raise();
}

QPixmap UBSpotlight::pictureUnder(const QRect& area) const
{
    if (mHiddenFromCapture)
    {
        QScreen* screen = QGuiApplication::screenAt(geometry().center());
        return screen ? screen->grabWindow(0, area.x(), area.y(), area.width(), area.height()) : QPixmap();
    }

    // Where the system cannot leave this window out of the picture, the
    // windows of Master Board under it are taken.
    const QRect global = area.translated(geometry().topLeft());

    for (QWidget* window : QApplication::topLevelWidgets())
    {
        if (window != this && window->isVisible() && window->geometry().contains(global.center())
            && !(window->windowFlags() & Qt::WindowTransparentForInput))
        {
            return window->grab(QRect(global.topLeft() - window->geometry().topLeft(), global.size()));
        }
    }

    return QPixmap();
}

void UBSpotlight::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

    UBSettings* settings = UBSettings::settings();
    const int radius = qBound(20, settings->spotlightRadius->get().toInt(), 600);
    const QRectF circle(mPointer.x() - radius, mPointer.y() - radius, 2 * radius, 2 * radius);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (settings->spotlightMode->get().toInt() == 1)
    {
        // The magnifier: what is under a smaller circle, at the size of the circle.
        const qreal zoom = qBound(1.2, settings->magnifierZoom->get().toDouble(), 8.0);
        const int sourceRadius = qMax(4, qRound(radius / zoom));
        const QRect source(mPointer.x() - sourceRadius, mPointer.y() - sourceRadius, 2 * sourceRadius, 2 * sourceRadius);
        const QPixmap picture = pictureUnder(source);

        QPainterPath lens;
        lens.addEllipse(circle);
        painter.setClipPath(lens);
        painter.fillRect(circle, QColor("#202328"));

        if (!picture.isNull())
        {
            painter.setRenderHint(QPainter::SmoothPixmapTransform);
            painter.drawPixmap(circle.toRect(), picture);
        }

        painter.setClipping(false);
        painter.setPen(QPen(QColor("#202328"), 4));
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(circle);
        painter.setPen(QPen(Qt::white, 1.5));
        painter.drawEllipse(circle.adjusted(2, 2, -2, -2));
        return;
    }

    // The spotlight: dark everywhere but in the circle, with a soft edge.
    const qreal darkness = qBound(0.0, settings->spotlightDarkness->get().toDouble(), 1.0);
    QPainterPath shade;
    shade.addRect(rect());
    shade.addEllipse(circle);
    painter.fillPath(shade, QColor(0, 0, 0, qRound(255 * darkness)));

    QRadialGradient edge(mPointer, radius);
    edge.setColorAt(0.85, Qt::transparent);
    edge.setColorAt(1.0, QColor(0, 0, 0, qRound(255 * darkness)));
    painter.setPen(Qt::NoPen);
    painter.setBrush(edge);
    painter.drawEllipse(circle);
}
