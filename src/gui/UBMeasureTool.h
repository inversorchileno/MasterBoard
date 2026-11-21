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
#include <QRect>

class QWidget;

/**
 * @brief The measurements: distance, perimeter and area, on the board and on
 * the desktop.
 *
 * The first time, it asks for the unit and for a line of one unit drawn on
 * the screen; then the lines and the shapes drawn are measured with that
 * reference. A window that can be moved shows the measure, with Restart,
 * which asks for the unit and the reference again, and a cross that ends
 * the measurements. On the board the reference is in the coordinates of
 * the page, so that the zoom does not change the measures.
 */
class UBMeasureTool : public QObject
{
    Q_OBJECT

public:
    enum class Mode
    {
        None,
        Distance,
        Perimeter,
        Area
    };

    static UBMeasureTool* instance();

    /** @brief Measures in this mode; the mode in use chosen again ends the measurements. */
    void choose(Mode mode);

    /** @brief Ends the measurements: the window and the lines go. */
    void close();

    Mode mode() const { return mMode; }

    /** @brief Where the window of the measure is in the layer over the desktop, or nothing. */
    static QRect desktopArea();

signals:
    void modeChanged();

private:
    explicit UBMeasureTool(QObject* parent);

    void toolChanged(int tool);
    void showLayer();
    void restart();
    void askUnit();

    Mode mMode{Mode::None};
    bool mDesktop{false};
};
