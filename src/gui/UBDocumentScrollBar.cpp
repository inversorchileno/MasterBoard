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


#include "UBDocumentScrollBar.h"

#include <QCursor>
#include <QTimer>

#include "board/UBBoardController.h"
#include "board/UBBoardView.h"
#include "core/UBApplication.h"
#include "document/UBDocumentProxy.h"

// A position of the document is a number of pages, given to the bar in thousandths.
static constexpr int sUnitsPerPage = 1000;

UBDocumentScrollBar::UBDocumentScrollBar(UBBoardView* view, QWidget* parent)
    : QScrollBar(Qt::Vertical, parent)
    , mView(view)
{
    setStyleSheet(
        "QScrollBar:vertical { background: rgba(40, 44, 52, 150); width: 14px; margin: 0px; border-radius: 7px; }"
        "QScrollBar::handle:vertical { background: rgba(200, 205, 215, 220); min-height: 40px; border-radius: 6px; margin: 2px; }"
        "QScrollBar::handle:vertical:hover, QScrollBar::handle:vertical:pressed { background: #f6d77a; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }"
        "QScrollBar:horizontal { background: rgba(40, 44, 52, 150); height: 14px; margin: 0px; border-radius: 7px; }"
        "QScrollBar::handle:horizontal { background: rgba(200, 205, 215, 220); min-width: 40px; border-radius: 6px; margin: 2px; }"
        "QScrollBar::handle:horizontal:hover, QScrollBar::handle:horizontal:pressed { background: #f6d77a; }"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0px; }"
        "QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal { background: transparent; }");
    setCursor(Qt::ArrowCursor);
    hide();

    connect(mView, &UBBoardView::documentPositionChanged, this, &UBDocumentScrollBar::updateFromView);
    connect(UBApplication::boardController, &UBBoardController::activeSceneChanged, this, &UBDocumentScrollBar::updateFromView);
    connect(this, &QScrollBar::valueChanged, this, &UBDocumentScrollBar::showPosition);
    connect(mView, &UBBoardView::pageFlowChanged, this, [this]() {
        placeBesideView();
        updateFromView();
    });

    mShowTimer = new QTimer(this);
    mShowTimer->setSingleShot(true);
    mShowTimer->setInterval(30);
    connect(mShowTimer, &QTimer::timeout, this, &UBDocumentScrollBar::showPendingPosition);

    // The pointer is followed, including over the palettes, without the
    // events of the board, which the drawing tools take.
    mPointerTimer = new QTimer(this);
    mPointerTimer->setInterval(150);
    connect(mPointerTimer, &QTimer::timeout, this, &UBDocumentScrollBar::checkPointer);
    mPointerTimer->start();

    mHideTimer = new QTimer(this);
    mHideTimer->setSingleShot(true);
    mHideTimer->setInterval(900);
    connect(mHideTimer, &QTimer::timeout, this, [this]() {
        if (!isSliderDown())
        {
            hide();
        }
    });
}

int UBDocumentScrollBar::barWidth()
{
    return 14;
}

void UBDocumentScrollBar::placeBesideView()
{
    // The view may be one side of the double board, inside its splitter.
    // Along the right edge, or along the bottom when the pages are beside each other.
    const QRect view(mView->mapTo(parentWidget(), QPoint(0, 0)), mView->size());

    if (mView->pageFlow() != UBBoardView::VerticalFlow)
    {
        setOrientation(Qt::Horizontal);
        setGeometry(view.left(), view.bottom() - barWidth() + 1, view.width(), barWidth());
    }
    else
    {
        setOrientation(Qt::Vertical);
        setGeometry(view.right() - barWidth() + 1, view.top(), barWidth(), view.height());
    }

    raise();
}

bool UBDocumentScrollBar::isUseful() const
{
    std::shared_ptr<UBDocumentProxy> document = UBApplication::boardController->selectedDocument();
    return document && document->pageCount() >= 2 && mView->isVisible();
}

void UBDocumentScrollBar::updateFromView()
{
    std::shared_ptr<UBDocumentProxy> document = UBApplication::boardController->selectedDocument();

    if (!document || mUpdating || isSliderDown())
    {
        return;
    }

    mUpdating = true;
    const int pageStep = qMax(1, int(mView->visiblePageFraction() * sUnitsPerPage));
    setPageStep(pageStep);
    setSingleStep(sUnitsPerPage / 20);
    setRange(0, qMax(0, document->pageCount() * sUnitsPerPage - pageStep));
    setValue(int(mView->documentPosition() * sUnitsPerPage));
    mUpdating = false;

    if (!isUseful())
    {
        hide();
    }
}

void UBDocumentScrollBar::showPosition(int value)
{
    if (mUpdating)
    {
        return;
    }

    // A drag gives many values in a row: the board follows the last one,
    // a moment later, rather than changing of page for each of them.
    mPendingValue = value;

    if (!mShowTimer->isActive())
    {
        mShowTimer->start();
    }
}

void UBDocumentScrollBar::showPendingPosition()
{
    mUpdating = true;
    mView->showDocumentPosition(qreal(mPendingValue) / sUnitsPerPage);
    mUpdating = false;
}

void UBDocumentScrollBar::checkPointer()
{
    if (!isUseful())
    {
        hide();
        return;
    }

    // Close to the right edge of the board, or to its bottom edge when the
    // pages are beside each other, or on the bar.
    const QRect view(mView->mapToGlobal(QPoint(0, 0)), mView->size());
    const QRect zone = mView->pageFlow() != UBBoardView::VerticalFlow
                           ? QRect(view.left(), view.bottom() - 3 * barWidth(), view.width(), 3 * barWidth() + 1)
                           : QRect(view.right() - 3 * barWidth(), view.top(), 3 * barWidth() + 1, view.height());

    if (zone.contains(QCursor::pos()) || isSliderDown())
    {
        mHideTimer->stop();

        if (isHidden())
        {
            updateFromView();
            placeBesideView();
            show();
        }
    }
    else if (isVisible() && !mHideTimer->isActive())
    {
        mHideTimer->start();
    }
}
