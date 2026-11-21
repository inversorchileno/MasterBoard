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


#include "UBPageButtons.h"

#include <QToolButton>
#include <QVBoxLayout>

#include "board/UBBoardController.h"
#include "core/UBApplication.h"
#include "document/UBDocumentProxy.h"
#include "gui/UBMainWindow.h"
#include "gui/UBTheme.h"

UBPageButtons::UBPageButtons(QWidget* parent)
    : QWidget(parent)
{
    auto makeButton = [this](const QIcon& icon, const QString& text, const QString& tip) {
        auto button = new QToolButton(this);
        button->setIcon(icon);
        button->setText(text);
        button->setToolTip(tip);
        button->setIconSize(QSize(18, 18));
        button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        button->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        button->setFixedHeight(24);
        button->setStyleSheet(UBTheme::style("QToolButton { color: #e8e8e8; background: transparent; border: none; padding: 1px 4px; text-align: left; }"
                                             "QToolButton:hover { background: #3a404b; border-radius: 3px; }"
                                             "QToolButton:disabled { color: #6b7383; }"));
        return button;
    };

    UBMainWindow* window = UBApplication::mainWindow;
    auto add = makeButton(window->actionPages->icon(), tr("Add page"), window->actionNewPage->toolTip());
    auto remove = makeButton(QIcon(":/images/toolbar/deletePage.svg"), tr("Delete page"), tr("Deletes the page shown, with all it holds"));

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 0, 4, 0);
    layout->setSpacing(2);
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    layout->addWidget(add);
    layout->addWidget(remove);

    connect(add, &QToolButton::clicked, window->actionNewPage, &QAction::trigger);
    connect(remove, &QToolButton::clicked, this, &UBPageButtons::deletePage);

    // A document keeps at least one page.
    auto updateRemove = [remove]() {
        auto document = UBApplication::boardController->selectedDocument();
        remove->setEnabled(document && document->pageCount() > 1);
    };

    connect(UBApplication::boardController, &UBBoardController::activeSceneChanged, this, updateRemove);
    connect(UBApplication::boardController, &UBBoardController::pageSelectionChanged, this, updateRemove);
    updateRemove();
}

void UBPageButtons::deletePage()
{
    UBBoardController* board = UBApplication::boardController;
    auto document = board->selectedDocument();

    if (!document || document->pageCount() < 2)
    {
        return;
    }

    // Deleting asks first, as deleting from the page list does.
    board->deleteScene(board->activeSceneIndex());

}
