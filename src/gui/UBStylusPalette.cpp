/*
 * Copyright (C) 2015-2022 Département de l'Instruction Publique (DIP-SEM)
 *
 * Copyright (C) 2013 Open Education Foundation
 *
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour
 * l'Education Numérique en Afrique (GIP ENA)
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




#include "UBStylusPalette.h"

#include <QtGui>

#include "UBMainWindow.h"

#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "core/UBApplicationController.h"
#include "core/UBShortcutManager.h"
#include "gui/UBAiPanel.h"


#include "board/UBBoardController.h"
#include "board/UBBoardView.h"
#include "gui/UBDocumentScrollBar.h"
#include "board/UBDrawingController.h"
#include "gui/UBToolbarChoice.h"
#include "gui/UBBarStyle.h"


#include "core/memcheck.h"

UBStylusPalette::UBStylusPalette(QWidget *parent, Qt::Orientation orient)
    : UBActionPalette(Qt::TopLeftCorner, parent, orient)
    , mLastSelectedId(-1)
{
    QList<QAction*> actions;

    // The order of the desktop palette, with the line among the shapes and
    // the hand just above the note. Zooming is done in the zoom box. The tool that
    // interacts with the items is left out: the selector does the same.
    actions << UBApplication::mainWindow->actionPen;
    actions << UBApplication::mainWindow->actionEraser;
    actions << UBApplication::mainWindow->actionMarker;
    actions << UBApplication::mainWindow->actionPointer;
    actions << UBApplication::mainWindow->actionSpotlight;
    actions << UBApplication::mainWindow->actionHighlight;
    actions << UBApplication::mainWindow->actionSelector;
    actions << UBApplication::mainWindow->actionHand;
    actions << UBApplication::mainWindow->actionNote;
    actions << UBApplication::mainWindow->actionStamp;
    actions << UBApplication::mainWindow->actionEmoji;
    // Only on the board: they put notes, files and sounds in the document.
    // On the desktop the attachment tool puts hyperlinks.
    actions << UBApplication::mainWindow->actionComment;
    actions << UBApplication::mainWindow->actionAttachment;
    actions << UBApplication::mainWindow->actionSound;
    actions << UBApplication::mainWindow->actionUnderline;
    actions << UBApplication::mainWindow->actionText;
    // Only on the board: reading and dictation.
    actions << UBApplication::mainWindow->actionVoice;
    // The line is one of the shapes: no tool of its own in the toolbar.
    actions << UBApplication::mainWindow->actionArrowShape;
    // The shapes tool draws the ellipse too, with all the other shapes.
    actions << UBApplication::mainWindow->actionRectangle;

    // The AI: its options are what to give to an AI on the web.
    actions << UBApplication::mainWindow->actionAi;
    actions << UBApplication::mainWindow->actionMeasure;

    // Saves the board as it is seen, without the palettes, in one click.
    mScreenshotAction = new QAction(QIcon(":/images/stylusPalette/screenshot.svg"), tr("Save a screenshot"), this);
    mScreenshotAction->setToolTip(tr("Save a screenshot of the board in the Pictures folder"));
    mScreenshotAction->setObjectName("actionBoardScreenshot");
    mScreenshotAction->setProperty("ungrouped", true);
    connect(mScreenshotAction, &QAction::triggered, this, &UBStylusPalette::screenshotRequested);
    actions << mScreenshotAction;

    setActions(actions);
    setButtonIconSize(QSize(42, 42));
    setBackgroundBrush(QBrush(UBSettings::opaquePaletteColor));
    groupActions();

    // Every button may be taken out of the palette in the Customize menu.
    UBToolbarChoice::registerPalette(UBToolbarChoice::Palette::Board, this, actions);

    UBShortcutManager::shortcutManager()->addActionGroup(mActionGroup);

    adjustSizeAndPosition();

    initPosition();


    // Hidden until the pointer goes near its side, when chosen in Bar style.
    UBBarStyle::watch(this);
    foreach(const UBActionPaletteButton* button, mButtons)
    {
        if (button->defaultAction() != mScreenshotAction)
        {
            connect(button, SIGNAL(doubleClicked()), this, SLOT(stylusToolDoubleClicked()));
        }
    }

}

void UBStylusPalette::initPosition()
{
    QWidget* pParentW = parentWidget();
    if(!pParentW) return ;

    mCustomPosition = true;

    const QRect area = boardArea();

    // All the tools are seen, whatever the size of the screen.
    const bool vertical = (edge() == Qt::LeftEdge || edge() == Qt::RightEdge);
    fitInLength((vertical ? area.height() : area.width()) - 2 * border(), vertical ? Qt::Vertical : Qt::Horizontal, 42);
    QPoint pos;

    switch (edge())
    {
        case Qt::LeftEdge:
            pos = QPoint(area.left() + border(), area.center().y() - height() / 2);
            break;
        case Qt::TopEdge:
            pos = QPoint(area.center().x() - width() / 2, area.top() + border());
            break;
        case Qt::BottomEdge:
            pos = QPoint(area.center().x() - width() / 2, area.bottom() - border() - height());
            break;
        default:
            // Left of the scroll bar of the document.
            pos = QPoint(area.right() - border() - UBDocumentScrollBar::barWidth() - width(), area.center().y() - height() / 2);
            break;
    }

    moveInsideParent(pos);
}

QRect UBStylusPalette::boardArea() const
{
    // The board view, beside the pages sidebar, or the whole parent.
    UBBoardView* view = UBApplication::boardController ? UBApplication::boardController->controlView() : nullptr;

    if (view && view->parentWidget() == parentWidget())
    {
        return view->geometry();
    }

    // Both sides of the double board, in their splitter.
    if (view && view->parentWidget() && view->parentWidget()->parentWidget() == parentWidget())
    {
        return view->parentWidget()->geometry();
    }

    return parentWidget()->rect();
}

int UBStylusPalette::edge()
{
    return UBSettings::settings()->value("Board/LibertyBoardStylusPaletteEdge", int(Qt::RightEdge)).toInt();
}

void UBStylusPalette::setEdge(int edge)
{
    UBSettings::settings()->setValue("Board/LibertyBoardStylusPaletteEdge", edge);
}

void UBStylusPalette::mouseReleaseEvent(QMouseEvent* event)
{
    const bool wasMoving = mIsMoving;

    UBActionPalette::mouseReleaseEvent(event);

    QWidget* parent = parentWidget();

    if (!wasMoving || !parent)
    {
        return;
    }

    // Dropped close to a border: the palette is placed along that border,
    // horizontal at the top and bottom, vertical at the left and right. The
    // pointer tells which border, as a long palette may touch several.
    const int snapDistance = 120;
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    const QPoint pointer = parent->mapFromGlobal(event->globalPosition().toPoint());
#else
    const QPoint pointer = parent->mapFromGlobal(event->globalPos());
#endif
    const QRect area = boardArea();
    const QList<QPair<int, int>> distances = {
        { pointer.x() - area.left(), Qt::LeftEdge },
        { area.right() - pointer.x(), Qt::RightEdge },
        { pointer.y() - area.top(), Qt::TopEdge },
        { area.bottom() - pointer.y(), Qt::BottomEdge },
    };

    int chosenEdge = 0;
    int shortestDistance = snapDistance;

    for (const auto& distance : distances)
    {
        if (distance.first < shortestDistance)
        {
            shortestDistance = distance.first;
            chosenEdge = distance.second;
        }
    }

    if (chosenEdge)
    {
        // Queued: the palette may be replaced by one of the other orientation.
        QMetaObject::invokeMethod(this, [this, chosenEdge]() { emit edgeChosen(chosenEdge); }, Qt::QueuedConnection);
    }
}

UBStylusPalette::~UBStylusPalette()
{
    if (mActionGroup)
    {
        UBShortcutManager::shortcutManager()->removeActionGroup(mActionGroup);
    }
}

void UBStylusPalette::stylusToolDoubleClicked()
{
    // The first click of the double click selected the tool.
    emit stylusToolDoubleClicked(UBDrawingController::drawingController()->stylusTool());
}


