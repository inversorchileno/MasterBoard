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

#include <QPointer>
#include <QWidget>

class QGraphicsView;

/**
 * @brief The rulers of Edit > Canvas measurements: at the top and at the
 * left of the board, in pixels of the page or of the PDF, from 0 at its
 * top left corner. Their steps follow the zoom, from 5 to 1000 pixels.
 */
class UBCanvasRulers : public QWidget
{
    Q_OBJECT

public:
    /** @brief Shows or hides the rulers over the board, and remembers it. */
    static void setShown(bool shown);
    static bool isShown();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    UBCanvasRulers(Qt::Orientation orientation, QGraphicsView* view);

    /** @brief The top left corner of the page or of the PDF, in the coordinates of the scene. */
    QPointF origin() const;
    void place();

    Qt::Orientation mOrientation;
    QPointer<QGraphicsView> mView;
};
