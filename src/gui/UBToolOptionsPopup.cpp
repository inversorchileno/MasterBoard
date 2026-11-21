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

#include "UBToolOptionsPopup.h"

#include <QGuiApplication>
#include <QScreen>
#include <QVBoxLayout>

#include "core/UB.h"
#include "gui/UBToolOptionsPanel.h"
#include "gui/UBTheme.h"

static constexpr int sGap = 8;

UBToolOptionsPopup::UBToolOptionsPopup(QWidget* parent)
    : QFrame(parent, Qt::Popup | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::NoDropShadowWindowHint)
{
    setObjectName("toolOptionsPopup");
    setStyleSheet(UBTheme::style("QFrame#toolOptionsPopup { background: #20242b; border: 1px solid #3a404b; }"));

    mLayout = new QVBoxLayout(this);
    mLayout->setContentsMargins(1, 1, 1, 1);
    mLayout->setSizeConstraint(QLayout::SetFixedSize);
}

bool UBToolOptionsPopup::hasOptions(int tool)
{
    switch (tool)
    {
        case UBStylusTool::Pen:
        case UBStylusTool::Eraser:
        case UBStylusTool::Marker:
        case UBStylusTool::Pointer:
        case UBStylusTool::Line:
        case UBStylusTool::Text:
        case UBStylusTool::Arrow:
        case UBStylusTool::Rectangle:
        case UBStylusTool::Ellipse:
        case UBStylusTool::Underline:
        case UBStylusTool::Highlight:
        case UBStylusTool::Note:
        case UBStylusTool::Stamp:
        case UBStylusTool::Spotlight:
        case UBStylusTool::Attachment:
        case UBStylusTool::Voice:
        case UBStylusTool::Emoji:
        case UBStylusTool::Ai:
        case UBStylusTool::Comment:
        case UBStylusTool::Selector:
            return true;
        default:
            return false;
    }
}

void UBToolOptionsPopup::showForTool(int tool, const QRect& anchor, Qt::Edge side)
{
    UBToolOptionsPanel* panel = panelFor(tool);

    if (!panel)
    {
        return;
    }

    // Only the panel of this tool is laid out, so that the popup takes its size.
    for (UBToolOptionsPanel* other : std::as_const(mPanels))
    {
        if (other != panel)
        {
            mLayout->removeWidget(other);
            other->hide();
        }
    }

    panel->refresh();
    mLayout->addWidget(panel);
    panel->show();
    adjustSize();

    const QScreen* screen = QGuiApplication::screenAt(anchor.center());
    const QRect available = screen ? screen->availableGeometry() : QRect(0, 0, 1920, 1080);

    // Beside the button, on the side the triangle points to, or the other
    // side when there is no room.
    QPoint position;

    switch (side)
    {
        case Qt::LeftEdge:
            position = QPoint(anchor.left() - sGap - width(), anchor.top());
            if (position.x() < available.left())
            {
                position.setX(anchor.right() + sGap);
            }
            break;
        case Qt::BottomEdge:
            position = QPoint(anchor.center().x() - width() / 2, anchor.bottom() + sGap);
            if (position.y() + height() > available.bottom())
            {
                position.setY(anchor.top() - sGap - height());
            }
            break;
        case Qt::TopEdge:
            position = QPoint(anchor.center().x() - width() / 2, anchor.top() - sGap - height());
            if (position.y() < available.top())
            {
                position.setY(anchor.bottom() + sGap);
            }
            break;
        default:
            position = QPoint(anchor.right() + sGap, anchor.top());
            if (position.x() + width() > available.right())
            {
                position.setX(anchor.left() - sGap - width());
            }
            break;
    }

    position.setY(qBound(available.top(), position.y(), available.bottom() - height()));
    position.setX(qBound(available.left(), position.x(), available.right() - width()));

    move(position);
    show();
    raise();
    activateWindow();
}

UBToolOptionsPanel* UBToolOptionsPopup::panelFor(int tool)
{
    // The line tool shares the color and width of the pen.
    const int panelTool = (tool == UBStylusTool::Line) ? int(UBStylusTool::Pen) : tool;

    if (!mPanels.contains(panelTool))
    {
        UBToolOptionsPanel* panel = UBToolOptionsPanel::create(panelTool, this);

        if (!panel)
        {
            return nullptr;
        }

        panel->hide();
        mPanels.insert(panelTool, panel);
    }

    return mPanels.value(panelTool);
}
