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




#include "UBPageFavorites.h"

#include <algorithm>

#include "core/UBPersistenceManager.h"
#include "document/UBDocumentProxy.h"

static const QString sKey = QStringLiteral("LibertyBoardFavoritePages");

UBPageFavorites* UBPageFavorites::instance()
{
    static UBPageFavorites favorites;
    return &favorites;
}

QList<int> UBPageFavorites::favorites(std::shared_ptr<UBDocumentProxy> proxy)
{
    QList<int> pages;

    if (!proxy)
    {
        return pages;
    }

    for (const QString& number : proxy->metaData(sKey).toString().split(',', Qt::SkipEmptyParts))
    {
        bool ok = false;
        const int page = number.toInt(&ok);

        if (ok && page >= 0)
        {
            pages << page;
        }
    }

    return pages;
}

bool UBPageFavorites::isFavorite(std::shared_ptr<UBDocumentProxy> proxy, int index)
{
    return favorites(proxy).contains(index);
}

void UBPageFavorites::setFavorite(std::shared_ptr<UBDocumentProxy> proxy, int index, bool favorite)
{
    QList<int> pages = favorites(proxy);
    pages.removeAll(index);

    if (favorite)
    {
        pages << index;
    }

    save(proxy, pages);
}

void UBPageFavorites::pageInserted(std::shared_ptr<UBDocumentProxy> proxy, int index)
{
    QList<int> pages = favorites(proxy);

    if (pages.isEmpty())
    {
        return;
    }

    for (int& page : pages)
    {
        if (page >= index)
        {
            ++page;
        }
    }

    save(proxy, pages);
}

void UBPageFavorites::pagesDeleted(std::shared_ptr<UBDocumentProxy> proxy, QList<int> indexes)
{
    QList<int> pages = favorites(proxy);

    if (pages.isEmpty())
    {
        return;
    }

    QList<int> kept;

    for (int page : pages)
    {
        if (indexes.contains(page))
        {
            continue;
        }

        const int before = int(std::count_if(indexes.begin(), indexes.end(), [page](int deleted) { return deleted < page; }));
        kept << page - before;
    }

    save(proxy, kept);
}

void UBPageFavorites::pageMoved(std::shared_ptr<UBDocumentProxy> proxy, int from, int to)
{
    QList<int> pages = favorites(proxy);

    if (pages.isEmpty() || from == to)
    {
        return;
    }

    for (int& page : pages)
    {
        if (page == from)
        {
            page = to;
        }
        else if (from < to && page > from && page <= to)
        {
            --page;
        }
        else if (from > to && page >= to && page < from)
        {
            ++page;
        }
    }

    save(proxy, pages);
}

void UBPageFavorites::save(std::shared_ptr<UBDocumentProxy> proxy, QList<int> pages)
{
    if (!proxy)
    {
        return;
    }

    std::sort(pages.begin(), pages.end());
    pages.erase(std::unique(pages.begin(), pages.end()), pages.end());

    QStringList numbers;

    for (int page : pages)
    {
        numbers << QString::number(page);
    }

    proxy->setMetaData(sKey, numbers.join(','));
    UBPersistenceManager::persistenceManager()->persistDocumentMetadata(proxy, true);
    emit instance()->changed(proxy);
}
