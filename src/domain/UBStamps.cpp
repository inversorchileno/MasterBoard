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


#include "UBStamps.h"

#include <QFont>
#include <QGraphicsItem>
#include <QFontMetricsF>
#include <QImageReader>
#include <QPainter>
#include <QPainterPath>

#include "core/UBSettings.h"
#include "frameworks/UBPlatformUtils.h"

static const QString sCurrentKey = QStringLiteral("Board/LibertyBoardStamp");

// The picture of the custom stamp, for this session only.
static QString sCustomFile;

// Marks the stamps among the pictures of a page, while Master Board runs.
static constexpr int sStampDataKey = 0x5354414d;
static bool sRemoving = false;

// The picture is drawn at twice the size of the stamp on the page, sharp when zoomed.
static constexpr int sResolution = 2;

QSize UBStamps::pageSize()
{
    return QSize(240, 80);
}

QList<UBStamps::Standard> UBStamps::standards()
{
    const QColor green("#3aa845");
    const QColor red("#e8452c");
    const QColor blue("#1f6fc5");

    return {
        { "Approved", "Aprobado", green },
        { "As Is", "Tal cual", red },
        { "Confidential", "Confidencial", red },
        { "Departmental", "Departamental", blue },
        { "Draft", "Borrador", red },
        { "Experimental", "Experimental", blue },
        { "Expired", "Vencido", red },
        { "Final", "Final", red },
        { "For Comment", "Para comentarios", green },
        { "For Public Release", "Para publicación", green },
        { "Not Approved", "No aprobado", red },
        { "Not For Public Release", "No publicable", red },
        { "Sold", "Vendido", blue },
        { "Top Secret", "Alto secreto", red },
    };
}

bool UBStamps::isSpanish()
{
    // The language of Master Board: the one chosen in the preferences, or the one of the system.
    QString language = UBSettings::settings()->appPreferredLanguage->get().toString();

    if (language.isEmpty())
    {
        language = UBPlatformUtils::systemLanguage();
    }

    return language.startsWith("es");
}

int UBStamps::count()
{
    return standards().size() + 1;
}

bool UBStamps::isCustom(int index)
{
    return index == 0;
}

QString UBStamps::name(int index)
{
    if (isCustom(index))
    {
        return isSpanish() ? QStringLiteral("Tu PNG") : QStringLiteral("Your PNG");
    }

    const Standard stamp = standards().value(index - 1);
    return QString::fromUtf8(isSpanish() ? stamp.spanish : stamp.english);
}

QImage UBStamps::image(int index)
{
    if (isCustom(index))
    {
        return drawCustom();
    }

    if (index < 1 || index > standards().size())
    {
        return QImage();
    }

    return drawStandard(standards().at(index - 1));
}

int UBStamps::current()
{
    // The custom stamp only once its picture is chosen in this session.
    const int index = qBound(0, UBSettings::settings()->value(sCurrentKey, 1).toInt(), count() - 1);
    return isCustom(index) && sCustomFile.isEmpty() ? 1 : index;
}

void UBStamps::setCurrent(int index)
{
    UBSettings::settings()->setValue(sCurrentKey, index);
}

bool UBStamps::isRemoving()
{
    return sRemoving;
}

void UBStamps::setRemoving(bool removing)
{
    sRemoving = removing;
}

void UBStamps::markAsStamp(QGraphicsItem* item)
{
    item->setData(sStampDataKey, true);
}

bool UBStamps::isStamp(const QGraphicsItem* item)
{
    return item && item->data(sStampDataKey).toBool();
}

QString UBStamps::customFile()
{
    return sCustomFile;
}

void UBStamps::setCustomFile(const QString& file)
{
    sCustomFile = file;
}

QImage UBStamps::drawStandard(const Standard& stamp)
{
    const QSize size = pageSize() * sResolution;
    QImage image(size, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    // A rounded frame, as left by a rubber stamp.
    const qreal border = 7 * sResolution / 2.0;
    const QRectF frame = QRectF(image.rect()).adjusted(border, border, -border, -border);
    painter.setPen(QPen(stamp.color, border * 2 * 0.75, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(frame, 12 * sResolution, 12 * sResolution);

    // The name in capitals, on two lines when it is long, as large as the frame allows.
    QString text = QString::fromUtf8(isSpanish() ? stamp.spanish : stamp.english).toUpper();
    QStringList lines{ text };

    if (text.length() > 14 && text.contains(' '))
    {
        // Cut at the space closest to the middle.
        int cut = -1;

        for (int i = 0; i < text.length(); ++i)
        {
            if (text.at(i) == ' ' && (cut < 0 || qAbs(i - text.length() / 2) < qAbs(cut - text.length() / 2)))
            {
                cut = i;
            }
        }

        lines = QStringList{ text.left(cut), text.mid(cut + 1) };
    }

    QFont font;
    font.setFamilies({ "Rockwell Extra Bold", "Cooper Black", "Arial Black", "DejaVu Serif" });
    font.setWeight(QFont::Black);
    font.setStretch(QFont::SemiCondensed);

    const QRectF textArea = frame.adjusted(border * 2.5, border * 1.8, -border * 2.5, -border * 1.8);
    qreal pixelSize = textArea.height() / lines.size();

    for (; pixelSize > 6; pixelSize -= 1)
    {
        font.setPixelSize(int(pixelSize));
        const QFontMetricsF metrics(font);
        qreal widest = 0;

        for (const QString& line : std::as_const(lines))
        {
            widest = qMax(widest, metrics.horizontalAdvance(line));
        }

        if (widest <= textArea.width() && metrics.height() * lines.size() <= textArea.height() * 1.15)
        {
            break;
        }
    }

    painter.setFont(font);
    painter.setPen(stamp.color);
    painter.drawText(textArea, Qt::AlignCenter, lines.join('\n'));

    return image;
}

QImage UBStamps::drawCustom()
{
    const QString file = customFile();

    if (file.isEmpty())
    {
        return QImage();
    }

    QImageReader reader(file);
    reader.setAutoTransform(true);
    const QImage picture = reader.read();

    if (picture.isNull())
    {
        return QImage();
    }

    // The picture fits in the frame of a standard stamp, keeping its
    // proportions: a square or round picture keeps a margin above and below.
    const QSize size = pageSize() * sResolution;
    const QSize fitted = picture.size().scaled(size * 0.92, Qt::KeepAspectRatio);

    QImage image(size, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    const QRect target(QPoint((size.width() - fitted.width()) / 2, (size.height() - fitted.height()) / 2), fitted);
    painter.drawImage(target, picture);

    return image;
}
