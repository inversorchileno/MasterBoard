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

#include "UBPageIndicator.h"

#include <QHBoxLayout>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>
#include <QVBoxLayout>

#include "board/UBBoardController.h"
#include "board/UBPageRotation.h"
#include "core/UBApplication.h"
#include "document/UBDocumentContainer.h"
#include "document/UBDocumentProxy.h"
#include "gui/UBTheme.h"

UBPageIndicator::UBPageIndicator(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("pageIndicator");
    setStyleSheet(UBTheme::style("QLineEdit { background: #111317; color: #f0f0f0; border: 1px solid #3a404b; border-radius: 4px; padding: 0px 4px; }"
                                 "QLabel { color: #e8e8e8; background: transparent; }"));

    mPage = new QLineEdit(this);
    mPage->setAlignment(Qt::AlignRight);
    mPage->setFixedWidth(52);
    mPage->setFixedHeight(20);
    mPage->setValidator(new QIntValidator(1, 99999, mPage));
    mPage->setToolTip(tr("Type a page number and press Enter"));

    mTotal = new QLabel(this);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    // The page number on top, the buttons that turn the page under it.
    auto rotateLeft = new QToolButton(this);
    rotateLeft->setIcon(QIcon(":/images/toolbar/rotateLeft.svg"));
    rotateLeft->setToolTip(tr("Rotate the page to the left"));

    auto rotateRight = new QToolButton(this);
    rotateRight->setIcon(QIcon(":/images/toolbar/rotateRight.svg"));
    rotateRight->setToolTip(tr("Rotate the page to the right"));

    for (QToolButton* button : { rotateLeft, rotateRight })
    {
        button->setIconSize(QSize(19, 19));
        button->setFixedSize(24, 21);
        button->setAutoRaise(true);
        button->setStyleSheet(UBTheme::style("QToolButton { border: none; padding: 0px; } QToolButton:hover { background: #3a404b; border-radius: 3px; }"));
    }

    // Written under the arrows like the names under the buttons beside it.
    auto rotateLabel = new QLabel(tr("Rotate"), this);
    rotateLabel->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    rotateLabel->setObjectName("rotateLabel");

    auto pageRow = new QHBoxLayout();
    pageRow->setSpacing(4);
    pageRow->addStretch(1);
    pageRow->addWidget(mPage);
    pageRow->addWidget(mTotal);
    pageRow->addStretch(1);

    auto rotateRow = new QHBoxLayout();
    rotateRow->setSpacing(6);
    rotateRow->addStretch(1);
    rotateRow->addWidget(rotateLeft);
    rotateRow->addWidget(rotateRight);
    rotateRow->addStretch(1);

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(6, 6, 6, 10);
    layout->setSpacing(0);
    layout->addLayout(pageRow);
    layout->addLayout(rotateRow);
    layout->addStretch(1);
    layout->addWidget(rotateLabel);

    connect(rotateLeft, &QToolButton::clicked, this, []() { UBPageRotation::rotateActivePage(-90); });
    connect(rotateRight, &QToolButton::clicked, this, []() { UBPageRotation::rotateActivePage(90); });

    connect(mPage, &QLineEdit::returnPressed, this, &UBPageIndicator::goToTypedPage);
    connect(UBApplication::boardController, &UBBoardController::activeSceneChanged, this, &UBPageIndicator::updatePage);
    connect(UBApplication::boardController, &UBBoardController::pageSelectionChanged, this, &UBPageIndicator::updatePage);

    updatePage();
}

void UBPageIndicator::updatePage()
{
    const auto document = UBApplication::boardController->selectedDocument();
    const int pageCount = document ? document->pageCount() : 0;
    const int page = UBDocumentContainer::pageFromSceneIndex(UBApplication::boardController->activeSceneIndex());

    if (!mPage->hasFocus())
    {
        mPage->setText(QString::number(pageCount > 0 ? page : 0));
    }

    mTotal->setText(QStringLiteral("/ %1").arg(pageCount));
}

void UBPageIndicator::goToTypedPage()
{
    const auto document = UBApplication::boardController->selectedDocument();

    if (!document)
    {
        return;
    }

    const int page = qBound(1, mPage->text().toInt(), document->pageCount());
    UBApplication::boardController->setActiveDocumentScene(UBDocumentContainer::sceneIndexFromPage(page));
    mPage->clearFocus();
    updatePage();
}
