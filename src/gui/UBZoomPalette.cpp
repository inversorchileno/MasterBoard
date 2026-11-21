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


#include "UBZoomPalette.h"

#include <QHBoxLayout>
#include <QIntValidator>
#include <QLineEdit>
#include <QMouseEvent>
#include <QScrollBar>
#include <QToolButton>

#include "board/UBBoardController.h"
#include "board/UBBoardView.h"
#include "core/UB.h"
#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "gui/UBTheme.h"

static const int sMinimumPercent = 10;

UBZoomPalette::UBZoomPalette(QWidget* parent)
    : UBFloatingPalette(Qt::BottomRightCorner, parent)
    , mBoardController(UBApplication::boardController)
{
    setBackgroundBrush(QBrush(UBTheme::color(UBSettings::opaquePaletteColor.name())));
    setStyleSheet(UBTheme::style("QToolButton { color: #f0f0f0; background: transparent; border: none; border-radius: 4px;"
                  " font-size: 18px; font-weight: bold; min-width: 28px; min-height: 28px; }"
                  "QToolButton:hover { background: #4a4a4a; }"
                  "QLineEdit { color: #f0f0f0; background: #1e1e1e; border: 1px solid #555555; border-radius: 4px;"
                  " font-size: 13px; padding: 2px 4px; }"));

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(border() + 6, border() + 4, border() + 6, border() + 4);
    layout->setSpacing(4);

    mZoomOutButton = new QToolButton(this);
    mZoomOutButton->setText(QStringLiteral("\u2212"));
    mZoomOutButton->setToolTip(tr("Zoom Out"));
    mZoomOutButton->setFocusPolicy(Qt::NoFocus);
    mZoomOutButton->setAutoRepeat(true);
    layout->addWidget(mZoomOutButton);

    mPercent = new QLineEdit(this);
    mPercent->setAlignment(Qt::AlignCenter);
    mPercent->setFixedWidth(64);
    mPercent->setFocusPolicy(Qt::ClickFocus);
    mPercent->setToolTip(tr("Zoom in percent: type a value and press Enter"));
    layout->addWidget(mPercent);

    mZoomInButton = new QToolButton(this);
    mZoomInButton->setText(QStringLiteral("+"));
    mZoomInButton->setToolTip(tr("Zoom In"));
    mZoomInButton->setFocusPolicy(Qt::NoFocus);
    mZoomInButton->setAutoRepeat(true);
    layout->addWidget(mZoomInButton);

    connect(mZoomOutButton, &QToolButton::clicked, this, [this]() { stepZoom(false); });
    connect(mZoomInButton, &QToolButton::clicked, this, [this]() { stepZoom(true); });
    connect(mPercent, &QLineEdit::editingFinished, this, &UBZoomPalette::onPercentEdited);
    connect(mBoardController, &UBBoardController::zoomChanged, this, &UBZoomPalette::refreshPalette);
    connect(mBoardController, &UBBoardController::activeSceneChanged, this, &UBZoomPalette::refreshPalette);

    resize(sizeHint());
    refreshPalette();
}

void UBZoomPalette::placeIn(const QRect& area, const QRect& avoid)
{
    resize(sizeHint());

    const int margin = 8;
    QRect box(QPoint(area.right() - margin - width(), area.bottom() - margin - height()), size());

    if (avoid.isValid() && box.intersects(avoid))
    {
        // Beside a vertical palette, above a horizontal one.
        if (avoid.height() > avoid.width())
        {
            box.moveRight(avoid.left() - margin);
        }
        else
        {
            box.moveBottom(avoid.top() - margin);
        }
    }

    move(box.topLeft());
    raise();
}

void UBZoomPalette::refreshPalette()
{
    if (!mPercent->hasFocus())
    {
        mPercent->setText(QString("%1 %").arg(qRound(mBoardController->currentZoom() * 100)));
    }
}

void UBZoomPalette::mousePressEvent(QMouseEvent* event)
{
    event->accept();
}

void UBZoomPalette::mouseMoveEvent(QMouseEvent* event)
{
    event->accept();
}

void UBZoomPalette::mouseReleaseEvent(QMouseEvent* event)
{
    event->accept();
}

void UBZoomPalette::zoomTo(qreal zoom)
{
    UBBoardView* view = mBoardController->controlView();
    const qreal current = mBoardController->currentZoom();

    if (!view || current <= 0)
    {
        return;
    }

    zoom = qBound(sMinimumPercent / 100.0, zoom, qreal(UB_MAX_ZOOM));

    // Around the middle of what the user sees.
    const QPointF center = view->mapToScene(view->viewport()->rect().center());
    mBoardController->zoom(zoom / current, center);
}

void UBZoomPalette::stepZoom(bool in)
{
    // Steps of 10 %, like most PDF readers.
    const int percent = qRound(mBoardController->currentZoom() * 100);
    const int next = in ? (percent / 10 + 1) * 10 : ((percent + 9) / 10 - 1) * 10;
    zoomTo(qMax(sMinimumPercent, next) / 100.0);
}

void UBZoomPalette::onPercentEdited()
{
    QString text = mPercent->text();
    text.remove('%');

    bool ok = false;
    const int percent = text.trimmed().toInt(&ok);

    if (ok && percent > 0)
    {
        zoomTo(percent / 100.0);
    }

    mPercent->clearFocus();
    refreshPalette();
}
