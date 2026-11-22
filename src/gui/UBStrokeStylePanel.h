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

#include <QFrame>
#include <QPointer>

class QGraphicsView;
class QTimer;
class UBColorSwatches;
class UBGraphicsStrokesGroup;
class UBValueSlider;

/**
 * @brief A small panel under what is selected on the board, when it is
 * only lines: pen strokes, shapes or arrows. It changes their color, their
 * size (the thickness of the lines) and their opacity, each change undone
 * with Undo.
 */
class UBStrokeStylePanel : public QFrame
{
    Q_OBJECT

public:
    /** @brief Puts the panel on the view of the board. */
    static void install(QGraphicsView* view);

private:
    explicit UBStrokeStylePanel(QGraphicsView* view);

    /** @brief The lines selected on the page, or none when something else is selected too. */
    QList<UBGraphicsStrokesGroup*> selectedLines() const;

    /** @brief Shows the panel under the selection, with its values, or hides it. */
    void follow();

    void applyColor(const QColor& color);
    void applyOpacity();
    void applyWidth();

    QPointer<QGraphicsView> mView;
    UBColorSwatches* mColors{nullptr};
    UBValueSlider* mWidth{nullptr};
    UBValueSlider* mOpacity{nullptr};
    QTimer* mWidthTimer{nullptr};
    QTimer* mOpacityTimer{nullptr};
    QList<UBGraphicsStrokesGroup*> mShownFor;
    qreal mShownWidth{0};
    qreal mAppliedRatio{1};
    bool mUpdating{false};
};
