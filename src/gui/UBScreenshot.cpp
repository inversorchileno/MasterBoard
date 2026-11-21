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


#include "UBScreenshot.h"

#include <QDateTime>
#include <QDir>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QStandardPaths>
#include <QWidget>

QString UBScreenshot::save(const QPixmap& screenshot)
{
    const QString folder = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) + "/Master Board";
    QDir().mkpath(folder);
    const QString filename = folder + "/" + tr("Screenshot") + " "
                             + QDateTime::currentDateTime().toString("yyyy-MM-dd HH-mm-ss") + ".png";

    const bool saved = !screenshot.isNull() && screenshot.save(filename, "PNG");
    return saved ? tr("Screenshot saved in %1").arg(QDir::toNativeSeparators(folder))
                 : tr("The screenshot could not be saved.");
}

void UBScreenshot::flash(const QRect& area)
{
    auto shade = new QWidget(nullptr, Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint
                                          | Qt::WindowTransparentForInput | Qt::WindowDoesNotAcceptFocus);
    shade->setAttribute(Qt::WA_DeleteOnClose);
    shade->setAttribute(Qt::WA_ShowWithoutActivating);
    shade->setAttribute(Qt::WA_TransparentForMouseEvents);
    shade->setStyleSheet("background: black;");
    shade->setGeometry(area);
    shade->setWindowOpacity(0.45);
    shade->show();

    auto fade = new QPropertyAnimation(shade, "windowOpacity", shade);
    fade->setDuration(450);
    fade->setStartValue(0.45);
    fade->setEndValue(0.0);
    fade->setEasingCurve(QEasingCurve::OutCubic);
    connect(fade, &QPropertyAnimation::finished, shade, &QWidget::close);
    fade->start();
}

void UBScreenshot::flash(QWidget* widget)
{
    QWidget* window = widget ? widget->window() : nullptr;

    if (!window)
    {
        return;
    }

    auto shade = new QWidget(window);
    shade->setAttribute(Qt::WA_TransparentForMouseEvents);
    shade->setAttribute(Qt::WA_StyledBackground);
    shade->setStyleSheet("background: black;");
    shade->setGeometry(QRect(widget->mapTo(window, QPoint(0, 0)), widget->size()));

    auto opacity = new QGraphicsOpacityEffect(shade);
    opacity->setOpacity(0.45);
    shade->setGraphicsEffect(opacity);
    shade->show();
    shade->raise();

    auto fade = new QPropertyAnimation(opacity, "opacity", shade);
    fade->setDuration(450);
    fade->setStartValue(0.45);
    fade->setEndValue(0.0);
    fade->setEasingCurve(QEasingCurve::OutCubic);
    connect(fade, &QPropertyAnimation::finished, shade, &QObject::deleteLater);
    fade->start();
}
