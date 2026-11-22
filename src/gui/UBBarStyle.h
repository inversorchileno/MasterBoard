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

#include <QList>
#include <QRectF>
#include <QString>

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

    /** @brief The edge of the board where the palettes stand. */
    static int edge();

    /** @brief Places the palette of the board and the one of the desktop along this edge. */
    static void placeBars(int edge);

    /** @brief The palettes hide themselves when the pointer leaves them, and come back near their edge. */
    static bool autoHide();
    static void setAutoHide(bool on);

    /** @brief How they hide: sliding out toward their edge, or fading out. */
    enum class HideEffect { Slide = 0, Fade = 1 };
    static HideEffect hideEffect();
    static void setHideEffect(HideEffect effect);

    /** @brief How long the pointer stays away before a bar hides, in milliseconds: 500 to 10000. */
    static int hideDelay();
    static void setHideDelay(int milliseconds);

    /** @brief The choices of that time: half a second, then 1 to 10 seconds. */
    static QList<int> hideDelays();
    static QString hideDelayName(int milliseconds);

    /**
     * @brief Asks how the palettes should hide, when the automatic hiding
     * is turned on. False if the person cancels.
     */
    static bool askHideEffect(QWidget* parent);

    /** @brief Watches this palette, to hide it and show it again when the automatic hiding is on. */
    static void watch(QWidget* palette);
};
