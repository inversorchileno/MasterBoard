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




#include "UBDesktopPalette.h"

#include <QBoxLayout>
#include <QPainter>
#include <QScreen>

#include "gui/UBToolOptionsArrows.h"

#include <QtGui>


#include "core/UBSettings.h"
#include "core/UBSetting.h"
#include "core/UBApplication.h"
#include "core/UBApplicationController.h"

#include "board/UBDrawingController.h"

#include "gui/UBMainWindow.h"
#include "gui/UBToolbarChoice.h"

#include "core/memcheck.h"
#include "gui/UBBarStyle.h"

static const QString sHorizontalKey = QStringLiteral("Desktop/LibertyBoardPaletteHorizontal");
static const QString sEdgeKey = QStringLiteral("Desktop/LibertyBoardPaletteEdge");

// The strip along the outer side of the palette that holds the red triangle.
static constexpr int sTriangleStrip = 12;

// Dropped this close to a border of the screen, the palette lies along it.
static constexpr int sSnapDistance = 120;

// Distance kept from the border; at the margin itself the palette minimizes.
static constexpr int sEdgeGap = 12;

QList<QAction*> UBDesktopPalette::toolActions()
{
    UBMainWindow* mainWindow = UBApplication::mainWindow;

    // Drawing first, then the tools which click on the screen, then the shapes.
    return { mainWindow->actionPen, mainWindow->actionEraser, mainWindow->actionMarker,
             mainWindow->actionPointer, mainWindow->actionSpotlight, mainWindow->actionHighlight, mainWindow->actionSelector,
             mainWindow->actionNote, mainWindow->actionStamp, mainWindow->actionEmoji, mainWindow->actionAttachment,
             mainWindow->actionUnderline, mainWindow->actionText,
             mainWindow->actionArrowShape, mainWindow->actionRectangle, mainWindow->actionAi };
}

UBDesktopPalette::UBDesktopPalette(QWidget *parent, UBRightPalette* _rightPalette)
    : UBActionPalette(Qt::TopLeftCorner, parent)
    , mShowHideAction(NULL)
    , mDisplaySelectAction(NULL)
    , rightPalette(_rightPalette)
{
    QList<QAction*> actions;

    mActionUniboard = new QAction(QIcon(":/images/toolbar/board.png"), tr("Show LibertyBoard"), this);
    connect(mActionUniboard, SIGNAL(triggered()), this, SIGNAL(uniboardClick()));
    actions << mActionUniboard;

    // Saves what is on the screen, with the annotations, in one click.
    mScreenshotAction = new QAction(QIcon(":/images/stylusPalette/screenshot.svg"), tr("Save a screenshot"), this);
    mScreenshotAction->setToolTip(tr("Save a screenshot with the annotations in the Pictures folder"));
    mScreenshotAction->setObjectName("actionDesktopScreenshot");
    connect(mScreenshotAction, &QAction::triggered, this, &UBDesktopPalette::screenshotClick);

    // Erases everything drawn on the desktop in one click.
    mClearAction = new QAction(QIcon(":/images/stylusPalette/trash.svg"), tr("Erase all"), this);
    mClearAction->setToolTip(tr("Erase everything drawn on the screen"));
    mClearAction->setObjectName("actionDesktopClear");
    connect(mClearAction, &QAction::triggered, this, &UBDesktopPalette::clearClick);

    actions << toolActions();
    actions << mScreenshotAction;

    // The screen captures stay available through their shortcuts and the
    // controller, but they are not shown on the palette.
    mActionCustomSelect = new QAction(QIcon(":/images/toolbar/captureArea.png"), tr("Capture Part of the Screen"), this);
    connect(mActionCustomSelect, SIGNAL(triggered()), this, SIGNAL(customClick()));

    mDisplaySelectAction = new QAction(QIcon(":/images/toolbar/captureScreen.png"), tr("Capture the Screen"), this);
    connect(mDisplaySelectAction, SIGNAL(triggered()), this, SIGNAL(screenClick()));

    QIcon showHideIcon;
    showHideIcon.addPixmap(QPixmap(":/images/toolbar/eyeOpened.png"), QIcon::Normal , QIcon::On);
    showHideIcon.addPixmap(QPixmap(":/images/toolbar/eyeClosed.png"), QIcon::Normal , QIcon::Off);
    mShowHideAction = new QAction(showHideIcon, "", this);
    mShowHideAction->setCheckable(true);

    connect(mShowHideAction, SIGNAL(triggered(bool)), this, SLOT(showHideClick(bool)));
    actions << mShowHideAction;
    actions << mClearAction;

    setActions(actions);
    setButtonIconSize(QSize(42, 42));

    // Every button but the one back to the board may be taken out of the
    // palette in the Customize menu.
    QList<QAction*> choosable = toolActions();
    choosable << mScreenshotAction << mClearAction;
    UBToolbarChoice::registerPalette(UBToolbarChoice::Palette::Desktop, this, choosable);

    if (UBSettings::settings()->value(sHorizontalKey, false).toBool())
    {
        setOrientation(Qt::Horizontal);
    }

    adjustSizeAndPosition();

    //  This palette can be minimized
    QIcon maximizeIcon;
    maximizeIcon.addPixmap(QPixmap(":/images/toolbar/stylusTab.png"), QIcon::Normal, QIcon::On);
    mMaximizeAction = new QAction(maximizeIcon, tr("Show the stylus palette"), this);
    connect(mMaximizeAction, SIGNAL(triggered()), this, SLOT(maximizeMe()));
    connect(this, SIGNAL(maximizeStart()), this, SLOT(maximizeMe()));
    connect(this, SIGNAL(minimizeStart(eMinimizedLocation)), this, SLOT(minimizeMe(eMinimizedLocation)));
    setMinimizePermission(true);

    connect(rightPalette, SIGNAL(resized()), this, SLOT(parentResized()));
}


UBDesktopPalette::~UBDesktopPalette()
{

}


QList<QAction*> UBDesktopPalette::fullActions() const
{
    QList<QAction*> actions;

    actions << mActionUniboard;
    actions << toolActions();
    actions << mScreenshotAction;
    actions << mShowHideAction;
    actions << mClearAction;

    return actions;
}

void UBDesktopPalette::adjustPosition()
{
    // All the tools are seen, whatever the size of the screen.
    if (const QScreen* paletteScreen = screen())
    {
        const QRect area = paletteScreen->availableGeometry();
        fitInLength((mOrientation == Qt::Vertical ? area.height() : area.width()) - 40, mOrientation, 42);
    }

    QPoint pos = this->pos();
    if (mOrientation == Qt::Vertical && this->pos().y() < 30)
    {
        pos.setY(30);
        moveInsideParent(pos);
    }
}

void UBDesktopPalette::setOrientation(Qt::Orientation orientation)
{
    mOrientation = orientation;

    if (auto box = qobject_cast<QBoxLayout*>(layout()))
    {
        box->setDirection(orientation == Qt::Horizontal ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom);
    }

    UBSettings::settings()->setValue(sHorizontalKey, orientation == Qt::Horizontal);

    // The buttons take their preferred size again before fitting the new length.
    setButtonIconSize(QSize(42, 42));
    updateMargins();
    adjustSizeAndPosition();
    adjustPosition();
}

void UBDesktopPalette::updateMargins(bool resizeToFit)
{
    if (!layout())
    {
        return;
    }

    const int margin = sLayoutContentMargin / 2 + border();
    QMargins margins(margin, margin, margin, margin);

    // Room for the red triangle on the side facing the border of the screen.
    if (!mMinimized)
    {
        switch (screenEdge())
        {
            case Qt::LeftEdge:
                margins.setLeft(margin + sTriangleStrip);
                break;
            case Qt::RightEdge:
                margins.setRight(margin + sTriangleStrip);
                break;
            case Qt::TopEdge:
                margins.setTop(margin + sTriangleStrip);
                break;
            case Qt::BottomEdge:
                margins.setBottom(margin + sTriangleStrip);
                break;
        }
    }

    if (layout()->contentsMargins() != margins)
    {
        layout()->setContentsMargins(margins);

        if (resizeToFit)
        {
            adjustSizeAndPosition();
        }
    }

    update();
}

QRect UBDesktopPalette::minimizeTriangleArea() const
{
    // In the strip outside the bar, on the side of the border of the screen.
    const int length = 2 * sTriangleStrip;

    switch (screenEdge())
    {
        case Qt::LeftEdge:
            return QRect(0, height() / 2 - length / 2, sTriangleStrip, length);
        case Qt::RightEdge:
            return QRect(width() - sTriangleStrip, height() / 2 - length / 2, sTriangleStrip, length);
        case Qt::TopEdge:
            return QRect(width() / 2 - length / 2, 0, length, sTriangleStrip);
        default:
            return QRect(width() / 2 - length / 2, height() - sTriangleStrip, length, sTriangleStrip);
    }
}

void UBDesktopPalette::paintEvent(QPaintEvent* event)
{
    if (mMinimized)
    {
        UBActionPalette::paintEvent(event);
        return;
    }

    // The bar keeps its width: the strip of the red triangle is outside it,
    // left transparent.
    QRect bar = rect();

    switch (screenEdge())
    {
        case Qt::LeftEdge:
            bar.setLeft(sTriangleStrip);
            break;
        case Qt::RightEdge:
            bar.setRight(width() - 1 - sTriangleStrip);
            break;
        case Qt::TopEdge:
            bar.setTop(sTriangleStrip);
            break;
        default:
            bar.setBottom(height() - 1 - sTriangleStrip);
            break;
    }

    {
        QPainter painter(this);
        UBBarStyle::paint(painter, bar, border());
    }

    // A small red triangle pointing to the border: a click minimizes the palette there.
    const QRectF area = minimizeTriangleArea();
    const qreal inset = 2;
    QPolygonF triangle;

    switch (screenEdge())
    {
        case Qt::LeftEdge:
            triangle << QPointF(area.left() + inset, area.center().y()) << QPointF(area.right() - inset, area.top() + inset)
                     << QPointF(area.right() - inset, area.bottom() - inset);
            break;
        case Qt::RightEdge:
            triangle << QPointF(area.right() - inset, area.center().y()) << QPointF(area.left() + inset, area.top() + inset)
                     << QPointF(area.left() + inset, area.bottom() - inset);
            break;
        case Qt::TopEdge:
            triangle << QPointF(area.center().x(), area.top() + inset) << QPointF(area.left() + inset, area.bottom() - inset)
                     << QPointF(area.right() - inset, area.bottom() - inset);
            break;
        default:
            triangle << QPointF(area.center().x(), area.bottom() - inset) << QPointF(area.left() + inset, area.top() + inset)
                     << QPointF(area.right() - inset, area.top() + inset);
            break;
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(QColor("#7a0f0f"), 1));
    painter.setBrush(QColor("#e53935"));
    painter.drawPolygon(triangle);
}

void UBDesktopPalette::mousePressEvent(QMouseEvent* event)
{
    if (!mMinimized && event->button() == Qt::LeftButton
        && minimizeTriangleArea().adjusted(-4, -4, 4, 4).contains(event->pos()))
    {
        event->accept();
        minimizeToEdge();
        return;
    }

    UBActionPalette::mousePressEvent(event);
}

void UBDesktopPalette::minimizeToEdge()
{
    QWidget* parent = parentWidget();

    if (!parent)
    {
        return;
    }

    // Against the border, where pushing the palette also minimizes it.
    const int margin = UBSettings::boardMargin - border();
    QPoint position = pos();
    eMinimizedLocation location = eMinimizedLocation_Left;

    switch (screenEdge())
    {
        case Qt::LeftEdge:
            position.setX(margin);
            break;
        case Qt::RightEdge:
            position.setX(parent->width() - getParentRightOffset() - width() - margin);
            location = eMinimizedLocation_Right;
            break;
        case Qt::TopEdge:
            position.setY(margin);
            location = eMinimizedLocation_Top;
            break;
        default:
            position.setY(parent->height() - height() - margin);
            location = eMinimizedLocation_Bottom;
            break;
    }

    move(position);
    minimizeAt(location);
}

void UBDesktopPalette::placeAtSavedEdge()
{
    QWidget* parent = parentWidget();

    if (!parent)
    {
        return;
    }

    setCustomPosition(true);

    const int edge = UBSettings::settings()->value(sEdgeKey, int(Qt::LeftEdge)).toInt();

    if (mOrientation == Qt::Horizontal)
    {
        const int x = (parent->width() - width()) / 2;
        move(x, edge == Qt::BottomEdge ? parent->height() - height() - sEdgeGap : sEdgeGap);
    }
    else
    {
        // Away from the border, where the palette would minimize itself.
        move(edge == Qt::RightEdge ? parent->width() - width() - 20 : 20, 150);
    }

    updateMargins();
}

void UBDesktopPalette::snapToEdge(const QPoint& pointer)
{
    QWidget* parent = parentWidget();

    if (!parent || mMinimized)
    {
        return;
    }

    // The pointer tells which border: a long palette may touch several.
    const QRect area = parent->rect();
    const QList<QPair<int, Qt::Edge>> distances = {
        { pointer.x() - area.left(), Qt::LeftEdge },
        { area.right() - pointer.x(), Qt::RightEdge },
        { pointer.y() - area.top(), Qt::TopEdge },
        { area.bottom() - pointer.y(), Qt::BottomEdge },
    };

    int shortestDistance = sSnapDistance;
    int chosenEdge = 0;

    for (const auto& distance : distances)
    {
        if (distance.first < shortestDistance)
        {
            shortestDistance = distance.first;
            chosenEdge = distance.second;
        }
    }

    if (!chosenEdge)
    {
        return;
    }

    const bool horizontal = (chosenEdge == Qt::TopEdge || chosenEdge == Qt::BottomEdge);
    const Qt::Orientation orientation = horizontal ? Qt::Horizontal : Qt::Vertical;

    if (orientation != mOrientation)
    {
        setOrientation(orientation);

        // Along the chosen border, centered on the pointer.
        QPoint position;

        switch (chosenEdge)
        {
            case Qt::LeftEdge:
                position = QPoint(sEdgeGap, pointer.y() - height() / 2);
                break;
            case Qt::RightEdge:
                position = QPoint(area.width() - getParentRightOffset() - width() - sEdgeGap, pointer.y() - height() / 2);
                break;
            case Qt::TopEdge:
                position = QPoint(pointer.x() - width() / 2, sEdgeGap);
                break;
            default:
                position = QPoint(pointer.x() - width() / 2, area.height() - height() - sEdgeGap);
                break;
        }

        // Kept inside the screen, without touching the border.
        position.setX(qBound(sEdgeGap, position.x(), qMax(sEdgeGap, area.width() - width() - sEdgeGap)));
        position.setY(qBound(sEdgeGap, position.y(), qMax(sEdgeGap, area.height() - height() - sEdgeGap)));
        move(position);
    }

    UBSettings::settings()->setValue(sEdgeKey, chosenEdge);
}

void UBDesktopPalette::disappearForCapture()
{
    setWindowOpacity(0.0);
    qApp->processEvents();
}


void UBDesktopPalette::appear()
{
    setWindowOpacity(1.0);
}


void UBDesktopPalette::showHideClick(bool checked)
{
    UBApplication::applicationController->mirroringEnabled(checked);
}


void UBDesktopPalette::updateShowHideState(bool pShowEnabled)
{
    mShowHideAction->setChecked(pShowEnabled);

    if (mShowHideAction->isChecked())
        mShowHideAction->setToolTip(tr("Show Board on Secondary Screen"));
    else
        mShowHideAction->setToolTip(tr("Show Desktop on Secondary Screen"));

    if (pShowEnabled)
        raise();
}


void UBDesktopPalette::setShowHideButtonVisible(bool visible)
{
    mShowHideAction->setVisible(visible);
}


void UBDesktopPalette::setDisplaySelectButtonVisible(bool visible)
{
    mDisplaySelectAction->setVisible(visible);
}

//  Called when the palette is near the border and must be minimized
void UBDesktopPalette::minimizeMe(eMinimizedLocation location)
{
    QList<QAction*> actions;
    clearLayout();

    actions << mMaximizeAction;
    setActions(actions);

    mMinimized = true;
    updateMargins(false);

    // The single button stays against the border where the palette was:
    // away from it, the palette would open again.
    if (QWidget* parent = parentWidget())
    {
        const int margin = UBSettings::boardMargin - border();
        resize(preferredSize());

        if (location == eMinimizedLocation_Right)
        {
            move(parent->width() - getParentRightOffset() - width() - margin, pos().y());
        }
        else if (location == eMinimizedLocation_Bottom)
        {
            move(pos().x(), parent->height() - height() - margin);
        }
    }

    adjustSizeAndPosition();

#ifdef UB_REQUIRES_MASK_UPDATE
        emit refreshMask();
#endif
}

//  Called when the user wants to maximize the palette
void UBDesktopPalette::maximizeMe()
{
    clearLayout();
    setActions(fullActions());

    mMinimized = false;
    updateMargins();
    adjustSizeAndPosition();
    adjustPosition();

    // Notify that the maximization has been done
    emit maximized();
#ifdef UB_REQUIRES_MASK_UPDATE
        emit refreshMask();
#endif
}

Qt::Edge UBDesktopPalette::screenEdge() const
{
    // Vertical: on the left or the right half of its screen; horizontal: on
    // the top or the bottom half.
    const QScreen* paletteScreen = screen();
    const QRect area = paletteScreen ? paletteScreen->geometry() : QRect();
    const QPoint center = mapToGlobal(rect().center());

    if (mOrientation == Qt::Horizontal)
    {
        return (area.isValid() && center.y() > area.center().y()) ? Qt::BottomEdge : Qt::TopEdge;
    }

    return (area.isValid() && center.x() > area.center().x()) ? Qt::RightEdge : Qt::LeftEdge;
}

void UBDesktopPalette::updateOptionArrows()
{
    UBToolOptionsArrows::apply(screenEdge());
}

void UBDesktopPalette::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);

    updateOptionArrows();
    adjustPosition();
    updateMargins();
}

void UBDesktopPalette::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event);

    // Back to the board, whose tools have no triangle.
    UBToolOptionsArrows::remove();
}

void UBDesktopPalette::mouseReleaseEvent(QMouseEvent* event)
{
    const bool wasMoving = mIsMoving;

    UBActionPalette::mouseReleaseEvent(event);

    if (wasMoving && parentWidget())
    {
        // Dropped close to a border: horizontal at the top and the bottom,
        // vertical at the left and the right.
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        snapToEdge(parentWidget()->mapFromGlobal(event->globalPosition().toPoint()));
#else
        snapToEdge(parentWidget()->mapFromGlobal(event->globalPos()));
#endif
    }

    // Moved to the other half of the screen: the triangles turn, and the
    // red triangle goes to the other side.
    updateOptionArrows();
    updateMargins();
    emit refreshMask();
}

QPoint UBDesktopPalette::buttonPos(QAction *action)
{
    QPoint p;

    UBActionPaletteButton* pB = mMapActionToButton[action];
    if(NULL != pB)
    {
        p = pB->pos();
    }

    return p;
}


int UBDesktopPalette::getParentRightOffset()
{
    return rightPalette->width();
}

void UBDesktopPalette::parentResized()
{
    QPoint p = pos();
    if (minimizedLocation() == eMinimizedLocation_Right)
    {
        p.setX(parentWidget()->width() - getParentRightOffset() -width());
    }

    moveInsideParent(p);
}
