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




#include "UBStrokeStylePanel.h"

#include <QApplication>
#include <QGraphicsView>
#include <QHBoxLayout>
#include <QPainterPath>
#include <QPainterPathStroker>
#include <QTimer>

#include <functional>

#include "board/UBBoardController.h"
#include "board/UBDrawingController.h"
#include "core/UB.h"
#include "core/UBApplication.h"
#include "domain/UBGraphicsItemUndoCommand.h"
#include "domain/UBGraphicsPolygonItem.h"
#include "domain/UBGraphicsScene.h"
#include "domain/UBGraphicsStroke.h"
#include "domain/UBGraphicsStrokesGroup.h"
#include "gui/UBToolOptionsWidgets.h"

// The polygons of a group of lines, in the coordinates of the group.
static QList<UBGraphicsPolygonItem*> polygonsOf(UBGraphicsStrokesGroup* group)
{
    QList<UBGraphicsPolygonItem*> polygons;

    for (QGraphicsItem* child : group->childItems())
    {
        if (auto polygon = dynamic_cast<UBGraphicsPolygonItem*>(child))
        {
            polygons << polygon;
        }
    }

    return polygons;
}

static qreal signedArea(const QPolygonF& polygon)
{
    qreal area = 0;

    for (int i = 0; i < polygon.size(); ++i)
    {
        const QPointF& a = polygon.at(i);
        const QPointF& b = polygon.at((i + 1) % polygon.size());
        area += a.x() * b.y() - b.x() * a.y();
    }

    return area / 2;
}

static qreal length(const QPolygonF& polygon)
{
    qreal total = 0;

    for (int i = 0; i + 1 < polygon.size(); ++i)
    {
        total += QLineF(polygon.at(i), polygon.at(i + 1)).length();
    }

    return total;
}

// The thickness of the lines of a group: for a long thin shape, twice its
// area divided by its outline.
static qreal estimatedWidth(UBGraphicsStrokesGroup* group)
{
    // A pen stroke drawn in this session knows the width of its points.
    qreal widths = 0;
    int count = 0;

    for (UBGraphicsPolygonItem* polygon : polygonsOf(group))
    {
        if (UBGraphicsStroke* stroke = polygon->stroke())
        {
            for (const auto& point : stroke->points())
            {
                widths += point.second;
                ++count;
            }

            if (count > 0)
            {
                return widths / count;
            }
        }
    }

    QPainterPath path;
    path.setFillRule(Qt::WindingFill);

    for (UBGraphicsPolygonItem* polygon : polygonsOf(group))
    {
        path.addPolygon(polygon->mapToParent(polygon->polygon()));
    }

    path = path.simplified();
    qreal area = 0;
    qreal outline = 0;

    for (const QPolygonF& part : path.toSubpathPolygons())
    {
        area += signedArea(part);
        outline += length(part);
    }

    return outline > 0 ? qAbs(2 * qAbs(area) / outline) : 0;
}

// Fewer points along the same outline (Douglas-Peucker): the points closer
// than the tolerance to the line between their neighbours are dropped.
static void keepPoints(const QPolygonF& polygon, int first, int last, qreal tolerance, QVector<bool>& kept)
{
    qreal farthest = 0;
    int index = -1;
    const QLineF chord(polygon.at(first), polygon.at(last));

    for (int i = first + 1; i < last; ++i)
    {
        const QPointF p = polygon.at(i);
        qreal distance;

        if (chord.length() < 1e-9)
        {
            distance = QLineF(chord.p1(), p).length();
        }
        else
        {
            const QPointF d = chord.p2() - chord.p1();
            distance = qAbs(d.x() * (chord.p1().y() - p.y()) - d.y() * (chord.p1().x() - p.x())) / chord.length();
        }

        if (distance > farthest)
        {
            farthest = distance;
            index = i;
        }
    }

    if (index >= 0 && farthest > tolerance)
    {
        kept[index] = true;
        keepPoints(polygon, first, index, tolerance, kept);
        keepPoints(polygon, index, last, tolerance, kept);
    }
}

static QPolygonF simplified(const QPolygonF& polygon, qreal tolerance)
{
    if (polygon.size() < 8)
    {
        return polygon;
    }

    // An outline: split at its middle so that both halves are open lines.
    const int middle = polygon.size() / 2;
    QVector<bool> kept(polygon.size(), false);
    kept[0] = kept[middle] = kept[polygon.size() - 1] = true;
    keepPoints(polygon, 0, middle, tolerance, kept);
    keepPoints(polygon, middle, polygon.size() - 1, tolerance, kept);

    QPolygonF result;

    for (int i = 0; i < polygon.size(); ++i)
    {
        if (kept.at(i))
        {
            result << polygon.at(i);
        }
    }

    return result;
}

// Replaces each group by a changed copy, in one step of Undo.
static void changeGroups(const QList<UBGraphicsStrokesGroup*>& groups, const std::function<void(UBGraphicsStrokesGroup*)>& change)
{
    auto scene = UBApplication::boardController->activeScene();

    if (!scene || groups.isEmpty())
    {
        return;
    }

    QSet<QGraphicsItem*> removed;
    QSet<QGraphicsItem*> added;

    for (UBGraphicsStrokesGroup* group : groups)
    {
        auto copy = dynamic_cast<UBGraphicsStrokesGroup*>(group->deepCopy());

        if (!copy)
        {
            continue;
        }

        change(copy);
        copy->setZValue(group->zValue());
        scene->removeItem(group);
        scene->addItem(copy);
        copy->setSelected(true);
        removed << group;
        added << copy;
    }

    UBApplication::undoStack->push(new UBGraphicsItemUndoCommand(scene, removed, added));
    scene->setModified(true);
}

void UBStrokeStylePanel::install(QGraphicsView* view)
{
    new UBStrokeStylePanel(view);
}

UBStrokeStylePanel::UBStrokeStylePanel(QGraphicsView* view)
    : QFrame(view->viewport())
    , mView(view)
{
    setObjectName("strokeStylePanel");
    setStyleSheet("#strokeStylePanel { background: #2b2f36; border: 1px solid #555b66; border-radius: 8px; }"
                  "QLabel { color: #f0f0f0; }");

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 6, 10, 6);
    layout->setSpacing(12);

    mColors = new UBColorSwatches(UBColorSwatches::standardColors(), false, this);
    mColors->setThreeRows();
    layout->addWidget(mColors);

    // The size as a share of the size of the lines when they were selected.
    mWidth = new UBValueSlider({ 25, 50, 75, 100, 125, 150, 200, 250, 300, 400 }, "%", 0, this);
    mWidth->setStacked(tr("Size"));
    layout->addWidget(mWidth);

    mOpacity = new UBValueSlider({ 10, 20, 30, 40, 50, 60, 70, 80, 90, 100 }, "%", 0, this);
    mOpacity->setStacked(tr("Opacity"));
    layout->addWidget(mOpacity);

    // A drag of a slider gives many values: the lines change once it rests.
    mWidthTimer = new QTimer(this);
    mWidthTimer->setSingleShot(true);
    mWidthTimer->setInterval(350);
    connect(mWidthTimer, &QTimer::timeout, this, &UBStrokeStylePanel::applyWidth);

    mOpacityTimer = new QTimer(this);
    mOpacityTimer->setSingleShot(true);
    mOpacityTimer->setInterval(350);
    connect(mOpacityTimer, &QTimer::timeout, this, &UBStrokeStylePanel::applyOpacity);

    connect(mColors, &UBColorSwatches::colorChosen, this, &UBStrokeStylePanel::applyColor);
    connect(mWidth, &UBValueSlider::valueChanged, this, [this]() {
        if (!mUpdating)
        {
            mWidthTimer->start();
        }
    });
    connect(mOpacity, &UBValueSlider::valueChanged, this, [this]() {
        if (!mUpdating)
        {
            mOpacityTimer->start();
        }
    });

    // The selection is followed, whatever the page shown.
    auto timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &UBStrokeStylePanel::follow);
    timer->start(150);
    hide();
}

QList<UBGraphicsStrokesGroup*> UBStrokeStylePanel::selectedLines() const
{
    QList<UBGraphicsStrokesGroup*> groups;
    auto scene = UBApplication::boardController->activeScene();

    if (!scene || !mView || mView->scene() != scene.get())
    {
        return groups;
    }

    for (QGraphicsItem* item : scene->selectedItems())
    {
        auto group = dynamic_cast<UBGraphicsStrokesGroup*>(item);

        if (!group)
        {
            if (auto polygon = dynamic_cast<UBGraphicsPolygonItem*>(item))
            {
                group = polygon->strokesGroup();
            }
        }

        // Something else is selected too: its own frame has its own options.
        if (!group)
        {
            return {};
        }

        if (!groups.contains(group))
        {
            groups << group;
        }
    }

    return groups;
}

void UBStrokeStylePanel::follow()
{
    const QList<UBGraphicsStrokesGroup*> groups = selectedLines();

    // Not while the selection is dragged.
    if (groups.isEmpty() || QApplication::mouseButtons() != Qt::NoButton)
    {
        if (groups.isEmpty())
        {
            hide();
            mShownFor.clear();
        }

        return;
    }

    if (groups != mShownFor)
    {
        // The values of the first line selected.
        mShownFor = groups;
        mUpdating = true;
        mShownWidth = estimatedWidth(groups.first());
        mAppliedRatio = 1;
        mWidth->setValue(100);

        const QList<UBGraphicsPolygonItem*> polygons = polygonsOf(groups.first());
        const QColor color = polygons.isEmpty() ? QColor(Qt::black) : polygons.first()->color();
        mColors->setCurrentColor(QColor(color.red(), color.green(), color.blue()));
        mOpacity->setValue(qRound(color.alphaF() * 100));
        mUpdating = false;
    }

    // Under the selection, or above it when there is no room below.
    QRectF selection;

    for (UBGraphicsStrokesGroup* group : groups)
    {
        selection |= group->sceneBoundingRect();
    }

    const QRect onView = mView->mapFromScene(selection).boundingRect();
    adjustSize();
    QPoint place(onView.center().x() - width() / 2, onView.bottom() + 40);

    if (place.y() + height() > parentWidget()->height() - 10)
    {
        place.setY(onView.top() - height() - 40);
    }

    place.setX(qBound(10, place.x(), parentWidget()->width() - width() - 10));
    place.setY(qBound(10, place.y(), parentWidget()->height() - height() - 10));
    move(place);
    show();
    raise();
}

void UBStrokeStylePanel::applyColor(const QColor& color)
{
    if (mUpdating || !color.isValid())
    {
        return;
    }

    changeGroups(selectedLines(), [color](UBGraphicsStrokesGroup* group) {
        for (UBGraphicsPolygonItem* polygon : polygonsOf(group))
        {
            QColor changed = color;
            changed.setAlpha(polygon->color().alpha());
            polygon->setColor(changed);
            polygon->setColorOnDarkBackground(changed);
            polygon->setColorOnLightBackground(changed);
        }
    });

    mShownFor = selectedLines();
}

void UBStrokeStylePanel::applyOpacity()
{
    const int alpha = qBound(25, qRound(mOpacity->value() * 255 / 100), 255);

    changeGroups(selectedLines(), [alpha](UBGraphicsStrokesGroup* group) {
        for (UBGraphicsPolygonItem* polygon : polygonsOf(group))
        {
            QColor changed = polygon->color();
            changed.setAlpha(alpha);
            polygon->setColor(changed);
            polygon->setColorOnDarkBackground(changed);
            polygon->setColorOnLightBackground(changed);
        }
    });

    mShownFor = selectedLines();
}

void UBStrokeStylePanel::applyWidth()
{
    const qreal ratio = mWidth->value() / 100;
    const qreal delta = mShownWidth * (ratio - mAppliedRatio) / 2;

    if (qAbs(delta) < 0.05)
    {
        return;
    }

    changeGroups(selectedLines(), [delta](UBGraphicsStrokesGroup* group) {
        // Each piece of the lines grows or thins all around by half the
        // difference of width: the pieces of a pen stroke are rounded at
        // their ends, so that they stay joined.
        QPainterPathStroker stroker;
        stroker.setWidth(2 * qAbs(delta));
        stroker.setJoinStyle(Qt::RoundJoin);
        stroker.setCapStyle(Qt::RoundCap);

        for (UBGraphicsPolygonItem* polygon : polygonsOf(group))
        {
            QPainterPath path;
            path.setFillRule(polygon->fillRule());
            // The rough edges of the marker have thousands of points: a
            // lighter outline keeps the change quick.
            path.addPolygon(simplified(polygon->polygon(), qMax(0.5, qAbs(delta) / 4)));
            path.closeSubpath();

            const QPainterPath edge = stroker.createStroke(path);
            const QPainterPath result = delta > 0 ? path.united(edge) : path.subtracted(edge);
            polygon->setFillRule(Qt::OddEvenFill);
            polygon->setPolygon(result.toFillPolygon());
        }

        // A group keeps the outline of its parts from when they were added.
        for (UBGraphicsPolygonItem* polygon : polygonsOf(group))
        {
            group->removeFromGroup(polygon);
            group->addToGroup(polygon);
        }
    });

    // The next change starts from the size just chosen.
    mAppliedRatio = ratio;
    mShownFor = selectedLines();
}
