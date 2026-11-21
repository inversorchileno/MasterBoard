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


#include "UBToolOptionsArrows.h"

#include <QAction>
#include <QIcon>
#include <QIconEngine>
#include <QMap>
#include <QPainter>
#include <QPixmap>
#include <QPolygonF>
#include <QTransform>

#include "core/UBApplication.h"
#include "gui/UBMainWindow.h"

namespace
{
    // The icons as they are without a triangle, kept the first time a tool is seen.
    QMap<QAction*, QIcon>& plainIcons()
    {
        static QMap<QAction*, QIcon> icons;
        return icons;
    }

    // The edge of the palette showing the triangles, while they are shown.
    bool sApplied = false;
    Qt::Edge sEdge = Qt::LeftEdge;

    QList<QAction*> toolsWithOptions()
    {
        UBMainWindow* w = UBApplication::mainWindow;
        return { w->actionPen, w->actionEraser, w->actionMarker, w->actionPointer, w->actionSpotlight, w->actionHighlight,
                 w->actionNote, w->actionStamp, w->actionEmoji, w->actionAttachment, w->actionUnderline, w->actionText, w->actionLine, w->actionArrowShape,
                 w->actionRectangle, w->actionEllipse, w->actionAi, w->actionMeasure };
    }

    QIcon plainIcon(QAction* action)
    {
        if (!plainIcons().contains(action))
        {
            plainIcons().insert(action, action->icon());
        }

        return plainIcons().value(action);
    }

    /** The triangle in a 42 x 42 icon. */
    QPolygonF triangle(Qt::Edge paletteEdge)
    {
        switch (paletteEdge)
        {
            case Qt::RightEdge: // pointing left, bottom left
                return QPolygonF({ QPointF(7, 30), QPointF(1, 35), QPointF(7, 40) });
            case Qt::TopEdge: // pointing down, bottom center
                return QPolygonF({ QPointF(16, 35), QPointF(26, 35), QPointF(21, 41) });
            case Qt::BottomEdge: // pointing up, top center
                return QPolygonF({ QPointF(16, 7), QPointF(26, 7), QPointF(21, 1) });
            default: // left: pointing right, bottom right
                return QPolygonF({ QPointF(35, 30), QPointF(41, 35), QPointF(35, 40) });
        }
    }

    /**
     * Paints the plain icon and its triangle at the size and resolution of
     * the screen, so that the vector icons stay sharp on scaled displays.
     */
    class TriangleIconEngine : public QIconEngine
    {
    public:
        TriangleIconEngine(const QIcon& icon, Qt::Edge paletteEdge)
            : mIcon(icon)
            , mPaletteEdge(paletteEdge)
        {
        }

        void paint(QPainter* painter, const QRect& rect, QIcon::Mode mode, QIcon::State state) override
        {
            mIcon.paint(painter, rect, Qt::AlignCenter, mode, state);

            const qreal scale = qMin(rect.width(), rect.height()) / 42.0;
            const QPointF origin = QRectF(rect).center() - QPointF(21 * scale, 21 * scale);

            painter->save();
            painter->setRenderHint(QPainter::Antialiasing);
            painter->translate(origin);
            painter->scale(scale, scale);
            painter->setPen(QPen(QColor(235, 235, 235), 1.2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            painter->setBrush(Qt::black);
            painter->drawPolygon(triangle(mPaletteEdge));
            painter->restore();
        }

        QPixmap pixmap(const QSize& size, QIcon::Mode mode, QIcon::State state) override
        {
            return scaledPixmap(size, mode, state, 1.0);
        }

        QPixmap scaledPixmap(const QSize& size, QIcon::Mode mode, QIcon::State state, qreal scale) override
        {
            QPixmap pixmap(size * scale);
            pixmap.fill(Qt::transparent);

            QPainter painter(&pixmap);
            paint(&painter, QRect(QPoint(0, 0), pixmap.size()), mode, state);
            painter.end();

            pixmap.setDevicePixelRatio(scale);
            return pixmap;
        }

        QSize actualSize(const QSize& size, QIcon::Mode, QIcon::State) override
        {
            return size;
        }

        QIconEngine* clone() const override
        {
            return new TriangleIconEngine(mIcon, mPaletteEdge);
        }

    private:
        QIcon mIcon;
        Qt::Edge mPaletteEdge;
    };
}

void UBToolOptionsArrows::apply(Qt::Edge paletteEdge)
{
    if (!UBApplication::mainWindow)
    {
        return;
    }

    for (QAction* action : toolsWithOptions())
    {
        action->setIcon(QIcon(new TriangleIconEngine(plainIcon(action), paletteEdge)));
    }

    sApplied = true;
    sEdge = paletteEdge;
}

void UBToolOptionsArrows::setIcon(QAction* action, const QIcon& icon)
{
    plainIcons().insert(action, icon);
    action->setIcon(sApplied && toolsWithOptions().contains(action) ? QIcon(new TriangleIconEngine(icon, sEdge)) : icon);
}

void UBToolOptionsArrows::remove()
{
    sApplied = false;

    for (auto it = plainIcons().constBegin(); it != plainIcons().constEnd(); ++it)
    {
        it.key()->setIcon(it.value());
    }
}

bool UBToolOptionsArrows::isOnTriangle(Qt::Edge paletteEdge, const QPoint& position, const QSize& buttonSize, const QSize& iconSize)
{
    // The icon is centered in the button and the triangle is scaled with it;
    // a margin around the triangle makes it easy to hit with a pen.
    const qreal scale = qMin(iconSize.width(), iconSize.height()) / 42.0;
    const QPointF offset((buttonSize.width() - 42 * scale) / 2.0, (buttonSize.height() - 42 * scale) / 2.0);
    const QRectF triangleArea = QTransform::fromScale(scale, scale).mapRect(triangle(paletteEdge).boundingRect());
    return triangleArea.translated(offset).adjusted(-5, -5, 5, 5).contains(position);
}

Qt::Edge UBToolOptionsArrows::openingSide(Qt::Edge paletteEdge)
{
    switch (paletteEdge)
    {
        case Qt::RightEdge:
            return Qt::LeftEdge;
        case Qt::TopEdge:
            return Qt::BottomEdge;
        case Qt::BottomEdge:
            return Qt::TopEdge;
        default:
            return Qt::RightEdge;
    }
}
