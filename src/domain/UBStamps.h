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

#include <QColor>
#include <QImage>
#include <QList>
#include <QSize>
#include <QString>

class QGraphicsItem;

/**
 * @brief The stamps put on a page with the stamp tool: standard ones, like
 * "Approved" or "Draft", and a custom one made from a picture of the user.
 *
 * The standard stamps are in English, and in Spanish when Master Board is
 * in Spanish. A stamp is a picture with a transparent background, so that
 * the page shows around its frame and its letters.
 */
class UBStamps
{
public:
    /** @brief The size of a stamp on the page, in points of the page. */
    static QSize pageSize();

    /** @brief The number of stamps, the custom one included, which is the first. */
    static int count();

    /** @brief True for the first stamp, made from a picture of the user. */
    static bool isCustom(int index);

    /** @brief The name of the stamp, as shown under its picture. */
    static QString name(int index);

    /** @brief The picture of the stamp, at twice its size on the page. */
    static QImage image(int index);

    /** @brief The stamp the next click puts on the page. */
    static int current();
    static void setCurrent(int index);

    /** @brief True while a click with the stamp tool takes a stamp away instead of putting one. */
    static bool isRemoving();
    static void setRemoving(bool removing);

    /** @brief Marks an item of the page as a stamp, which Remove can take away. */
    static void markAsStamp(QGraphicsItem* item);
    static bool isStamp(const QGraphicsItem* item);

    /**
     * @brief The picture file of the custom stamp, empty until one is
     * chosen: it is chosen again each time Master Board starts.
     */
    static QString customFile();
    static void setCustomFile(const QString& file);

private:
    struct Standard
    {
        const char* english;
        const char* spanish;
        QColor color;
    };

    static QList<Standard> standards();
    static bool isSpanish();
    static QImage drawStandard(const Standard& stamp);
    static QImage drawCustom();
};
