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

#include <QRectF>

class QPainter;
class QWidget;

/**
 * @brief The style of the tool palettes, on the board and on the desktop:
 * the color inside, the color of the edge, the opacity, and the corners,
 * from square to round. Chosen in Customize > Bar style.
 */
class UBBarStyle
{
public:
    /** @brief Paints a bar in the rectangle, with an edge of this width. */
    static void paint(QPainter& painter, const QRectF& bar, int edge);

    /** @brief The window to choose the style; the palettes follow each change. */
    static void openWindow(QWidget* parent);

    /** @brief Repaints the palettes with the style of the settings. */
    static void repaintPalettes();
};
