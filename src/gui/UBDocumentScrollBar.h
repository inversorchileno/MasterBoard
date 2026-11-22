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

#include <QScrollBar>

class QTimer;
class UBBoardView;

/**
 * @brief A scroll bar at the right of the board for the whole document,
 * page after page: dragging it goes quickly anywhere in the document. It is
 * at the bottom when the pages are beside each other.
 *
 * It shows only for a document of two pages or more, and only while the
 * pointer is close to the right edge of the board.
 */
class UBDocumentScrollBar : public QScrollBar
{
    Q_OBJECT

public:
    UBDocumentScrollBar(UBBoardView* view, QWidget* parent);

    /** @brief The width of the bar, which the palette at the right leaves free. */
    static int barWidth();

    /** @brief Places the bar along the right edge of the board view. */
    void placeBesideView();

private:
    void updateFromView();
    void showPosition(int value);
    void showPendingPosition();
    void checkPointer();
    bool isUseful() const;

    UBBoardView* mView{nullptr};
    QTimer* mPointerTimer{nullptr};
    QTimer* mHideTimer{nullptr};
    QTimer* mShowTimer{nullptr};
    int mPendingValue{0};
    bool mUpdating{false};
};
