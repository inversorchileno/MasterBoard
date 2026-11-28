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


#include "UBSearchBar.h"

#include <QApplication>
#include <QEvent>
#include <QGraphicsPathItem>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenuBar>
#include <QMessageBox>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

#include "board/UBBoardController.h"
#include "board/UBBoardPaletteManager.h"
#include "board/UBBoardView.h"
#include "core/UBApplication.h"
#include "core/UBPersistenceManager.h"
#include "document/UBDocumentProxy.h"
#include "domain/UBGraphicsPDFItem.h"
#include "domain/UBGraphicsScene.h"
#include "gui/UBMainWindow.h"
#include "gui/UBStylusPalette.h"
#include "pdf/UBPdfTextSearch.h"

UBSearchBar::UBSearchBar(QWidget* parent)
    : QFrame(parent)
{
    setObjectName("searchBar");
    setStyleSheet(
        "#searchBar { background: #2b2f36; border: 1px solid #4a505b; border-radius: 6px; }"
        "QLineEdit { background: #111317; color: #f0f0f0; border: 1px solid #3a404b; border-radius: 4px; padding: 3px 6px; }"
        "QLabel { color: #c8ccd2; }"
        "QToolButton { color: #f0f0f0; background: transparent; border: none; font-size: 15px; padding: 2px 6px; }"
        "QToolButton:hover { background: #4a505b; border-radius: 3px; }"
        "QListWidget { background: #1d2026; color: #e8e8e8; border: 1px solid #3a404b; border-radius: 4px; outline: none; }"
        "QListWidget::item { padding: 5px 4px; border-bottom: 1px solid #2f343c; }"
        "QListWidget::item:selected { background: #3d5a80; color: #ffffff; }"
        "QListWidget::item:hover:!selected { background: #2b3038; }");

    mText = new QLineEdit(this);
    mText->setPlaceholderText(tr("Search..."));
    mText->setMinimumWidth(240);
    mText->installEventFilter(this);

    auto previous = new QToolButton(this);
    previous->setText(QStringLiteral("\u2039"));
    previous->setToolTip(tr("Previous result"));

    auto next = new QToolButton(this);
    next->setText(QStringLiteral("\u203A"));
    next->setToolTip(tr("Next result"));

    auto close = new QToolButton(this);
    close->setText(QStringLiteral("\u2715"));
    close->setToolTip(tr("Close"));

    mCount = new QLabel(this);
    mCount->setMinimumWidth(90);

    auto row = new QHBoxLayout();
    row->setSpacing(4);
    row->addWidget(mText, 1);
    row->addWidget(previous);
    row->addWidget(next);
    row->addWidget(close);

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 6, 6, 6);
    layout->setSpacing(3);
    layout->addLayout(row);
    layout->addWidget(mCount);

    // Every place found, to go to any of them.
    mList = new QListWidget(this);
    mList->setWordWrap(true);
    mList->setTextElideMode(Qt::ElideNone);
    mList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    mList->hide();
    layout->addWidget(mList, 1);

    connect(mList, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) { showPlace(mList->row(item)); });

    // The list follows the text while it is written.
    mTyping = new QTimer(this);
    mTyping->setSingleShot(true);
    mTyping->setInterval(400);
    connect(mText, &QLineEdit::textEdited, mTyping, qOverload<>(&QTimer::start));
    connect(mTyping, &QTimer::timeout, this, [this]() {
        const QString text = mText->text().trimmed();

        if (text != mSearchedText && (text.size() >= 2 || text.isEmpty()))
        {
            mSearchedText = text;
            search();
        }
    });

    connect(mText, &QLineEdit::returnPressed, this, [this]() { step(+1); });
    connect(previous, &QToolButton::clicked, this, [this]() { step(-1); });
    connect(next, &QToolButton::clicked, this, [this]() { step(+1); });
    connect(close, &QToolButton::clicked, this, &QWidget::hide);

    parent->installEventFilter(this);
    hide();
}

void UBSearchBar::open()
{
    UBBoardController* board = UBApplication::boardController;
    std::shared_ptr<UBGraphicsScene> scene = board ? board->activeScene() : nullptr;

    if (!scene || !pdfItemOf(scene.get()))
    {
        QMessageBox::information(window(), tr("Search"), tr("The search only works in a PDF with text."));
        return;
    }

    UBApplication::applicationController->showBoard();
    placeInWindow();
    show();
    raise();
    mText->setFocus();
    mText->selectAll();
}

bool UBSearchBar::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == parent() && (event->type() == QEvent::Resize || event->type() == QEvent::Show))
    {
        placeInWindow();
    }
    else if (watched == mText && event->type() == QEvent::KeyPress)
    {
        auto key = static_cast<QKeyEvent*>(event);

        if (key->key() == Qt::Key_Escape)
        {
            hide();
            return true;
        }

        if ((key->key() == Qt::Key_Return || key->key() == Qt::Key_Enter) && (key->modifiers() & Qt::ShiftModifier))
        {
            step(-1);
            return true;
        }
    }

    return QFrame::eventFilter(watched, event);
}

void UBSearchBar::hideEvent(QHideEvent* event)
{
    clearMark();
    QFrame::hideEvent(event);
}

QRect UBSearchBar::panelRect(QWidget* window, int width, bool besideTools)
{
    auto mainWindow = qobject_cast<QMainWindow*>(window);
    int top = mainWindow && mainWindow->menuBar() ? mainWindow->menuBar()->geometry().bottom() + 6 : 6;
    UBBoardController* board = UBApplication::boardController;

    // Under the toolbar, on the board, so that no button is hidden.
    if (board && board->controlView() && board->controlView()->isVisible())
    {
        top = qMax(top, board->controlView()->mapTo(window, QPoint()).y() + 8);
    }

    int right = window->width() - 12;

    // Beside the tools when they stand on the right of the board, not over them.
    QWidget* tools = board && board->paletteManager() ? board->paletteManager()->stylusPalette() : nullptr;

    if (besideTools && tools && tools->isVisible())
    {
        const QRect toolsRect(tools->mapTo(window, QPoint()), tools->size());

        if (toolsRect.left() > window->width() / 2)
        {
            right = toolsRect.left() - 8;
        }
    }

    // Down to the bottom of the board.
    const int height = qMax(220, window->height() - top - 60);
    return QRect(right - width, top, width, height);
}

void UBSearchBar::placeInWindow()
{
    if (mList->isVisible())
    {
        const QRect rect = panelRect(parentWidget(), 360, true);
        setFixedSize(rect.size());
        move(rect.topLeft());
        return;
    }

    setMinimumSize(0, 0);
    setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    adjustSize();
    const QRect rect = panelRect(parentWidget(), width(), false);
    move(rect.topLeft());
}

void UBSearchBar::step(int direction)
{
    const QString text = mText->text().trimmed();
    UBBoardController* board = UBApplication::boardController;

    if (text.isEmpty() || !board)
    {
        return;
    }

    mTyping->stop();

    // A new text, or another document: search again.
    if (text != mSearchedText || board->selectedDocument() != mDocument)
    {
        mSearchedText = text;
        search();
    }

    if (mPlaces.isEmpty())
    {
        return;
    }

    if (mCurrent < 0)
    {

        // The first place from the page shown on.
        std::shared_ptr<UBGraphicsScene> scene = board->activeScene();
        UBGraphicsPDFItem* item = scene ? pdfItemOf(scene.get()) : nullptr;
        const int shownPage = item ? item->pageNumber() : 1;
        int first = 0;

        while (first < mPlaces.size() && mPlaces.at(first).pdfPage < shownPage)
        {
            ++first;
        }

        showPlace(first < mPlaces.size() ? first : 0);
        return;
    }

    showPlace((mCurrent + direction + mPlaces.size()) % mPlaces.size());
}

void UBSearchBar::search()
{
    clearMark();
    mPlaces.clear();
    mCurrent = -1;

    UBBoardController* board = UBApplication::boardController;
    mDocument = board->selectedDocument();
    std::shared_ptr<UBGraphicsScene> scene = board->activeScene();
    UBGraphicsPDFItem* item = scene ? pdfItemOf(scene.get()) : nullptr;

    if (!mDocument || !item || mSearchedText.isEmpty())
    {
        updateCount();
        fillList();
        return;
    }

    mPdfFile = mDocument->persistencePath() + "/" + UBPersistenceManager::objectDirectory + "/"
               + item->fileUuid().toString() + ".pdf";

    QApplication::setOverrideCursor(Qt::WaitCursor);
    const UBPdfTextSearch pdf(mPdfFile);

    for (int page = 1; page <= pdf.pageCount(); ++page)
    {
        for (const UBPdfTextSearch::Found& found : pdf.findWithLines(page, mSearchedText))
        {
            mPlaces << Place{ page, found.rect, found.line };
        }
    }

    QApplication::restoreOverrideCursor();
    updateCount();
    fillList();
}

void UBSearchBar::fillList()
{
    mList->clear();

    for (const Place& place : std::as_const(mPlaces))
    {
        auto item = new QListWidgetItem(tr("Page %1").arg(place.pdfPage) + "\n" + shownLine(place.line));
        item->setToolTip(place.line);
        mList->addItem(item);
    }

    mList->setVisible(!mPlaces.isEmpty());
    placeInWindow();
}

QString UBSearchBar::shownLine(const QString& line) const
{
    // Some words before and after the text, enough to recognise the place.
    const int around = 45;
    const int at = line.indexOf(mSearchedText, 0, Qt::CaseInsensitive);

    if (line.isEmpty() || at < 0)
    {
        return line.isEmpty() ? QStringLiteral("\u2026") : line;
    }

    const int start = qMax(0, at - around);
    const int end = qMin(line.size(), at + mSearchedText.size() + around);
    QString shown = line.mid(start, end - start);

    if (start > 0)
    {
        shown.prepend(QStringLiteral("\u2026"));
    }

    if (end < line.size())
    {
        shown.append(QStringLiteral("\u2026"));
    }

    return shown;
}

void UBSearchBar::showPlace(int index)
{
    UBBoardController* board = UBApplication::boardController;
    const Place place = mPlaces.at(index);
    const int sceneIndex = sceneIndexOf(place.pdfPage);

    if (sceneIndex < 0)
    {
        return;
    }

    mCurrent = index;
    updateCount();
    clearMark();

    {
        const QSignalBlocker blocker(mList);
        mList->setCurrentRow(index);
        mList->scrollToItem(mList->currentItem());
    }

    if (sceneIndex != board->activeSceneIndex())
    {
        board->persistCurrentScene(false, true);
        board->setActiveDocumentScene(sceneIndex);
    }

    std::shared_ptr<UBGraphicsScene> scene = board->activeScene();
    UBGraphicsPDFItem* item = scene ? pdfItemOf(scene.get()) : nullptr;

    if (!item)
    {
        return;
    }

    // From points of the page to the page item, then to the board.
    const QRectF bounds = item->boundingRect();
    const QSizeF points = item->pageSize();
    const qreal sx = bounds.width() / points.width();
    const qreal sy = bounds.height() / points.height();
    const QRectF onItem(bounds.left() + place.rect.left() * sx, bounds.top() + place.rect.top() * sy,
                        place.rect.width() * sx, place.rect.height() * sy);
    const QPolygonF onBoard = item->mapToScene(onItem.adjusted(-2, -2, 2, 2));

    QPainterPath path;
    path.addPolygon(onBoard);
    path.closeSubpath();

    // A mark of the screen only: it is not part of the page and is not saved.
    mMark = new QGraphicsPathItem(path);
    mMark->setBrush(QColor(255, 214, 0, 110));
    mMark->setPen(QPen(QColor(255, 140, 0), 0));
    mMark->setZValue(1e9);
    mMark->setAcceptedMouseButtons(Qt::NoButton);
    // The page does not own it, so it is never in the page's list of things
    // to delete: a page that is gone takes it with it, and clearMark() then
    // only forgets it.
    scene->QGraphicsScene::addItem(mMark);
    mMarkScene = scene;

    board->controlView()->centerOn(onBoard.boundingRect().center());
}

void UBSearchBar::clearMark()
{
    std::shared_ptr<UBGraphicsScene> scene = mMarkScene.lock();

    if (mMark && scene)
    {
        if (mMark->scene() == scene.get())
        {
            scene->QGraphicsScene::removeItem(mMark);
        }

        delete mMark;
    }

    mMark = nullptr;
    mMarkScene.reset();
}

void UBSearchBar::updateCount()
{
    if (mSearchedText.isEmpty())
    {
        mCount->clear();
    }
    else if (mPlaces.isEmpty())
    {
        mCount->setText(tr("Not found"));
    }
    else
    {
        mCount->setText(tr("%1 of %2").arg(mCurrent + 1).arg(mPlaces.size()));
    }

    adjustSize();
}

int UBSearchBar::sceneIndexOf(int pdfPage) const
{
    if (!mDocument)
    {
        return -1;
    }

    // An imported PDF has one page of the board for each page of the PDF,
    // in the same order: that page is tried first.
    QList<int> candidates{ pdfPage - 1 };

    for (int i = 0; i < mDocument->pageCount(); ++i)
    {
        if (i != pdfPage - 1)
        {
            candidates << i;
        }
    }

    for (int index : std::as_const(candidates))
    {
        if (index < 0 || index >= mDocument->pageCount())
        {
            continue;
        }

        std::shared_ptr<UBGraphicsScene> scene = UBPersistenceManager::persistenceManager()->loadDocumentScene(mDocument, index);
        UBGraphicsPDFItem* item = scene ? pdfItemOf(scene.get()) : nullptr;

        if (item && item->pageNumber() == pdfPage
                && mPdfFile.endsWith(item->fileUuid().toString() + ".pdf"))
        {
            return index;
        }
    }

    return -1;
}

UBGraphicsPDFItem* UBSearchBar::pdfItemOf(UBGraphicsScene* scene)
{
    return qgraphicsitem_cast<UBGraphicsPDFItem*>(scene->backgroundObject());
}
