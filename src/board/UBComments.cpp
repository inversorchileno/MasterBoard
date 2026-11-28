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


#include "UBComments.h"

#include <algorithm>

#include <QDialog>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QLocale>
#include <QMessageBox>
#include <QPainter>
#include <QSvgRenderer>
#include <QTextOption>
#include <QTreeWidget>
#include <QVBoxLayout>

#include "board/UBBoardController.h"
#include "board/UBBoardView.h"
#include "core/UBApplication.h"
#include "core/UBPersistenceManager.h"
#include "document/UBDocumentContainer.h"
#include "document/UBDocumentProxy.h"
#include "domain/UBGraphicsCommentItem.h"
#include "domain/UBGraphicsPixmapItem.h"
#include "domain/UBGraphicsScene.h"

namespace
{
    QPointer<UBGraphicsCommentItem> sCurrent;

    std::shared_ptr<UBGraphicsScene> pageScene(int index)
    {
        UBBoardController* board = UBApplication::boardController;

        if (index == board->activeSceneIndex())
        {
            return board->activeScene();
        }

        return UBPersistenceManager::persistenceManager()->loadDocumentScene(board->selectedDocument(), index);
    }

    /** The window of the note as a picture, at twice its size on the page. */
    QImage pictureOf(const UBGraphicsCommentItem* comment)
    {
        const int width = 2 * 240;
        QFont textFont;
        textFont.setPixelSize(2 * 11);
        QFont titleFont = textFont;
        titleFont.setBold(true);

        const QRect textArea(2 * 8, 2 * 30, width - 2 * 16, 4000);
        const QFontMetrics metrics(textFont);
        const QRect needed = metrics.boundingRect(textArea, Qt::TextWordWrap, comment->text());
        const int height = needed.bottom() + 2 * 10;

        QImage image(width, height, QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::transparent);

        QPainter painter(&image);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
        painter.setPen(QPen(QColor("#c9a800"), 2));
        painter.setBrush(QColor("#fff8b8"));
        painter.drawRoundedRect(QRectF(image.rect()).adjusted(1, 1, -1, -1), 8, 8);

        QSvgRenderer icon(QStringLiteral(":/images/marks/comment.svg"));
        icon.render(&painter, QRectF(2 * 6, 2 * 5, 2 * 18, 2 * 18));

        painter.setPen(QColor("#3a3200"));
        painter.setFont(titleFont);
        painter.drawText(QRect(2 * 30, 2 * 6, width - 2 * 36, 2 * 18), Qt::AlignVCenter | Qt::AlignLeft,
                         QLocale().toString(comment->date(), QLocale::ShortFormat));

        painter.setPen(QColor("#1e1e1e"));
        painter.setFont(textFont);
        painter.drawText(textArea, Qt::TextWordWrap, comment->text());

        return image;
    }
}

QList<UBComments::Entry> UBComments::all()
{
    QList<Entry> entries;
    UBBoardController* board = UBApplication::boardController;
    std::shared_ptr<UBDocumentProxy> document = board ? board->selectedDocument() : nullptr;

    if (!document)
    {
        return entries;
    }

    for (int index = 0; index < document->pageCount(); ++index)
    {
        std::shared_ptr<UBGraphicsScene> scene = pageScene(index);

        if (!scene)
        {
            continue;
        }

        QList<UBGraphicsCommentItem*> comments;

        for (QGraphicsItem* item : scene->items())
        {
            if (auto comment = qgraphicsitem_cast<UBGraphicsCommentItem*>(item))
            {
                comments << comment;
            }
        }

        std::sort(comments.begin(), comments.end(), [](UBGraphicsCommentItem* a, UBGraphicsCommentItem* b) {
            return a->scenePos().y() != b->scenePos().y() ? a->scenePos().y() < b->scenePos().y()
                                                          : a->scenePos().x() < b->scenePos().x();
        });

        for (UBGraphicsCommentItem* comment : std::as_const(comments))
        {
            entries << Entry{ index, comment };
        }
    }

    return entries;
}

void UBComments::step(int direction, QWidget* parent)
{
    const QList<Entry> entries = all();

    if (entries.isEmpty())
    {
        QMessageBox::information(parent, QObject::tr("Quick notes"), QObject::tr("The document has no quick notes."));
        return;
    }

    int current = -1;

    for (int i = 0; i < entries.size(); ++i)
    {
        if (entries.at(i).comment == sCurrent)
        {
            current = i;
        }
    }

    int next = 0;

    if (current >= 0)
    {
        next = (current + direction + entries.size()) % entries.size();
    }
    else
    {
        // From the page shown: the first note on it or after it, or before it going back.
        const int page = UBApplication::boardController->activeSceneIndex();
        next = direction > 0 ? 0 : entries.size() - 1;

        for (int i = 0; i < entries.size(); ++i)
        {
            const int candidate = direction > 0 ? i : entries.size() - 1 - i;

            if (direction > 0 ? entries.at(candidate).page >= page : entries.at(candidate).page <= page)
            {
                next = candidate;
                break;
            }
        }
    }

    show(entries.at(next));
}

void UBComments::show(const Entry& entry)
{
    UBBoardController* board = UBApplication::boardController;

    if (!entry.comment)
    {
        return;
    }

    if (entry.page != board->activeSceneIndex())
    {
        board->persistCurrentScene(false, true);
        board->setActiveDocumentScene(entry.page);
    }

    // Hidden notes are shown again to be seen.
    if (!UBGraphicsCommentItem::areShown())
    {
        setShown(true);
    }

    sCurrent = entry.comment;
    board->controlView()->centerOn(entry.comment->scenePos());
    entry.comment->open();
}

void UBComments::showList(QWidget* parent)
{
    const QList<Entry> entries = all();

    QDialog dialog(parent);
    dialog.setWindowTitle(QObject::tr("Quick notes"));
    dialog.resize(560, 360);

    auto list = new QTreeWidget(&dialog);
    list->setHeaderLabels({ QObject::tr("Page"), QObject::tr("Date"), QObject::tr("Text") });
    list->setRootIsDecorated(false);
    list->header()->setStretchLastSection(true);

    for (int i = 0; i < entries.size(); ++i)
    {
        const Entry& entry = entries.at(i);
        auto row = new QTreeWidgetItem(list, { QString::number(UBDocumentContainer::pageFromSceneIndex(entry.page)),
                                               QLocale().toString(entry.comment->date(), QLocale::ShortFormat),
                                               entry.comment->text().simplified() });
        row->setData(0, Qt::UserRole, i);
    }

    list->resizeColumnToContents(0);
    list->resizeColumnToContents(1);

    auto buttons = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
    auto layout = new QVBoxLayout(&dialog);
    layout->addWidget(list);
    layout->addWidget(buttons);

    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    QObject::connect(list, &QTreeWidget::itemActivated, &dialog, [&dialog](QTreeWidgetItem*) { dialog.accept(); });

    if (dialog.exec() == QDialog::Accepted && list->currentItem())
    {
        show(entries.at(list->currentItem()->data(0, Qt::UserRole).toInt()));
    }
}

void UBComments::setShown(bool shown)
{
    UBGraphicsCommentItem::setShown(shown);

    for (const Entry& entry : all())
    {
        if (entry.comment)
        {
            entry.comment->setVisible(shown);
        }
    }
}

void UBComments::merge(QWidget* parent)
{
    const QList<Entry> entries = all();

    if (entries.isEmpty())
    {
        QMessageBox::information(parent, QObject::tr("Quick notes"), QObject::tr("The document has no quick notes."));
        return;
    }

    const auto answer = QMessageBox::question(parent, QObject::tr("Merge quick notes"),
        QObject::tr("Each quick note becomes a picture of the note on its page, with its date and text. "
                    "The notes can no longer be edited. Merge the %1 quick notes?").arg(entries.size()));

    if (answer != QMessageBox::Yes)
    {
        return;
    }

    UBBoardController* board = UBApplication::boardController;
    std::shared_ptr<UBDocumentProxy> document = board->selectedDocument();
    QList<int> changedPages;

    for (const Entry& entry : entries)
    {
        UBGraphicsCommentItem* comment = entry.comment;
        std::shared_ptr<UBGraphicsScene> scene = pageScene(entry.page);

        if (!comment || !scene || comment->text().trimmed().isEmpty())
        {
            continue;
        }

        // The picture keeps the size it has on a page seen whole.
        const QImage picture = pictureOf(comment);
        auto item = new UBGraphicsPixmapItem();
        item->setPixmap(QPixmap::fromImage(picture));
        item->setFlag(QGraphicsItem::ItemIsMovable, true);
        item->setFlag(QGraphicsItem::ItemIsSelectable, true);
        item->setTransform(QTransform::fromScale(0.5, 0.5));
        item->setPos(comment->scenePos());
        scene->addItem(item);

        scene->removeItem(comment);
        scene->removeItemFromDeletion(comment);
        delete comment;
        scene->setModified(true);

        if (!changedPages.contains(entry.page))
        {
            changedPages << entry.page;
        }
    }

    for (int page : std::as_const(changedPages))
    {
        if (page != board->activeSceneIndex())
        {
            UBPersistenceManager::persistenceManager()->persistDocumentScene(document, pageScene(page), page, false, true);
        }
    }

    board->persistCurrentScene(false, true);
}
