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


#pragma once

#include <QObject>
#include <QPixmap>
#include <QRect>
#include <QString>

class QWidget;

/**
 * @brief Keeps a screenshot in the Pictures folder and shows, like Windows,
 * that it was taken: the captured area dims for a moment.
 */
class UBScreenshot : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Saves the picture in Pictures/Master Board with the date and
     * time in its name. Returns the message to show to the user.
     */
    static QString save(const QPixmap& screenshot);

    /** @brief Dims the area, in global coordinates, and fades back. */
    static void flash(const QRect& area);

    /**
     * @brief Dims a widget of a window and fades back, with a layer inside
     * the window, which stays above it whatever the window system does.
     */
    static void flash(QWidget* widget);
};
