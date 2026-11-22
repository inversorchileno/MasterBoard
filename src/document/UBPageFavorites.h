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
#include <QObject>
#include <memory>

class UBDocumentProxy;

/**
 * @brief The favorite pages of a document, marked with the star of their
 * thumbnail. They are kept in the metadata of the document as the numbers
 * of the pages, which follow the pages when pages are added, deleted,
 * duplicated or moved.
 */
class UBPageFavorites : public QObject
{
    Q_OBJECT

public:
    static UBPageFavorites* instance();

    static QList<int> favorites(std::shared_ptr<UBDocumentProxy> proxy);
    static bool isFavorite(std::shared_ptr<UBDocumentProxy> proxy, int index);
    static void setFavorite(std::shared_ptr<UBDocumentProxy> proxy, int index, bool favorite);

    /** @brief A page was put at the index: the favorites after it move one page down. */
    static void pageInserted(std::shared_ptr<UBDocumentProxy> proxy, int index);

    /** @brief These pages were deleted: they are no favorites any more, the next ones move up. */
    static void pagesDeleted(std::shared_ptr<UBDocumentProxy> proxy, QList<int> indexes);

    /** @brief A page was moved from one place to another. */
    static void pageMoved(std::shared_ptr<UBDocumentProxy> proxy, int from, int to);

signals:
    void changed(std::shared_ptr<UBDocumentProxy> proxy);

private:
    static void save(std::shared_ptr<UBDocumentProxy> proxy, QList<int> pages);
};
