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




#include "UBBoardPaletteManager.h"

#include "board/UBBoardView.h"
#include "gui/UBDocumentScrollBar.h"
#include "gui/UBScreenshot.h"

#include "frameworks/UBPlatformUtils.h"
#include "frameworks/UBFileSystemUtils.h"

#include "core/UBApplication.h"
#include "core/UBApplicationController.h"
#include "core/UBSettings.h"
#include "core/UBSetting.h"
#include "core/UBDisplayManager.h"

#include "gui/UBMainWindow.h"
#include "gui/UBStylusPalette.h"
#include "gui/UBKeyboardPalette.h"
#include "gui/UBToolWidget.h"
#include "gui/UBZoomPalette.h"
#include "gui/UBCanvasRulers.h"
#include "gui/UBWebToolsPalette.h"
#include "gui/UBActionPalette.h"
#include "gui/UBBackgroundPalette.h"
#include "gui/UBFavoriteToolPalette.h"
#include "gui/UBStartupHintsPalette.h"
#include "gui/UBPageNavigationWidget.h"

#include "web/UBWebController.h"

#include "desktop/UBDesktopAnnotationController.h"


#include "network/UBNetworkAccessManager.h"
#include "network/UBServerXMLHttpRequest.h"

#include "domain/UBGraphicsScene.h"
#include "domain/UBGraphicsPixmapItem.h"

#include "document/UBDocumentProxy.h"
#include "podcast/UBPodcastController.h"
#include "board/UBDrawingController.h"

#include "tools/UBToolsManager.h"

#include "UBBoardController.h"

#include "document/UBDocumentController.h"

#include "core/UBPersistenceManager.h"
#include "gui/UBToolOptionsPopup.h"

#include "core/memcheck.h"

inline constexpr int longpress_interval = 350;

UBBoardPaletteManager::UBBoardPaletteManager(QWidget* container, UBBoardController* pBoardController)
    : QObject(container)
    , mKeyboardPalette(0)
    , mWebToolsCurrentPalette(0)
    , mContainer(container)
    , mBoardControler(pBoardController)
    , mStylusPalette(0)
    , mZoomPalette(0)
    , mTipPalette(0)
    , mLeftPalette(NULL)
    , mRightPalette(NULL)
    , mBackgroundsPalette(0)
    , mToolsPalette(0)
    , mAddItemPalette(0)
    , mErasePalette(NULL)
    , mPagePalette(NULL)
    , mPendingPageButtonPressed(false)
    , mPendingZoomButtonPressed(false)
    , mPendingHandButtonPressed(false)
    , mPendingEraseButtonPressed(false)
    , mpPageNavigWidget(NULL)
    , mpCachePropWidget(NULL)
    , mpDownloadWidget(NULL)
    , mDownloadInProgress(false)
{
    setupPalettes();
    connectPalettes();
}


UBBoardPaletteManager::~UBBoardPaletteManager()
{

// mAddedItemPalette is delete automatically because of is parent
// that changes depending on the mode

// mMainWindow->centralWidget is the parent of mStylusPalette
// do not delete this here.
}

void UBBoardPaletteManager::initPalettesPosAtStartup()
{
    mStylusPalette->initPosition();
}

void UBBoardPaletteManager::setupLayout()
{

}

/**
 * \brief Set up the dock palette widgets
 */
void UBBoardPaletteManager::setupDockPaletteWidgets()
{

    //------------------------------------------------//
    // Create the widgets for the dock palettes
    mpPageNavigWidget = new UBPageNavigationWidget();

    mpCachePropWidget = new UBCachePropertiesWidget();

    mpDownloadWidget = new UBDockDownloadWidget();

    // Add the dock palettes
    mLeftPalette = new UBLeftPalette(mContainer);

    // The page navigator is shown in the pages sidebar of the board, not in
    // the left palette.

    mLeftPalette->connectSignals();
    mLeftPalette->showTabWidget(0);

    mRightPalette = new UBRightPalette(mContainer);
    // RIGHT palette widgets
    mpFeaturesWidget = new UBFeaturesWidget();
    mRightPalette->registerWidget(mpFeaturesWidget);
    mRightPalette->addTab(mpFeaturesWidget);

    // The cache widget will be visible only if a cache is put on the page
    mRightPalette->registerWidget(mpCachePropWidget);

    //  The download widget will be part of the right palette but
    //  will become visible only when the first download starts
    mRightPalette->registerWidget(mpDownloadWidget);
    mRightPalette->connectSignals();
    changeMode(eUBDockPaletteWidget_BOARD, true);

    // Hide the tabs that must be hidden
    mRightPalette->removeTab(mpDownloadWidget);
    mRightPalette->removeTab(mpCachePropWidget);

}

void UBBoardPaletteManager::slot_changeMainMode(UBApplicationController::MainMode mainMode)
{
//    Board = 0, Internet, Document, Tutorial, WebDocument

    switch( mainMode )
    {
        case UBApplicationController::Board:
            {
                // call changeMode only when switch NOT from desktop mode
                if(!UBApplication::applicationController->isShowingDesktop())
                    changeMode(eUBDockPaletteWidget_BOARD);
            }
            break;
        case UBApplicationController::Internet:
            changeMode(eUBDockPaletteWidget_WEB);
            break;

        case UBApplicationController::Document:
            changeMode(eUBDockPaletteWidget_DOCUMENT);
            break;

        default:
            {
                if (UBPlatformUtils::hasVirtualKeyboard() && mKeyboardPalette != NULL)
                    mKeyboardPalette->hide();
            }
            break;
    }
}

void UBBoardPaletteManager::slot_changeDesktopMode(bool isDesktop)
{
    UBApplicationController::MainMode currMode = UBApplication::applicationController->displayMode();
    if(!isDesktop)
    {
        switch( currMode )
        {
            case UBApplicationController::Board:
                changeMode(eUBDockPaletteWidget_BOARD);
                break;

            default:
                break;
        }
    }
    else
        changeMode(eUBDockPaletteWidget_DESKTOP);
}

void UBBoardPaletteManager::setupPalettes()
{

    if (UBPlatformUtils::hasVirtualKeyboard())
    {
        mKeyboardPalette = new UBKeyboardPalette(0);
#ifndef Q_OS_WIN
        connect(mKeyboardPalette, SIGNAL(closed()), mKeyboardPalette, SLOT(onDeactivated()));
#endif
    }


    setupDockPaletteWidgets();


    // Add the other palettes
    const int stylusEdge = UBStylusPalette::edge();
    const bool vertical = (stylusEdge == Qt::LeftEdge || stylusEdge == Qt::RightEdge);
    mStylusPalette = new UBStylusPalette(mContainer, vertical ? Qt::Vertical : Qt::Horizontal);
    connect(mStylusPalette, SIGNAL(stylusToolDoubleClicked(int)), UBApplication::boardController, SLOT(stylusToolDoubleClicked(int)));
    connect(mStylusPalette, &UBStylusPalette::edgeChosen, this, &UBBoardPaletteManager::placeStylusPalette);
    connect(mStylusPalette, &UBStylusPalette::screenshotRequested, this, &UBBoardPaletteManager::saveBoardScreenshot);
    mStylusPalette->show(); // always show stylus palette at startup

    // The scroll bar of the document, at the right edge of the board.
    UBBoardView* view = UBApplication::boardController->controlView();
    mDocumentScrollBar = new UBDocumentScrollBar(view, view->parentWidget());
    connect(view, &UBBoardView::resized, mDocumentScrollBar, &UBDocumentScrollBar::placeBesideView);

    mZoomPalette = new UBZoomPalette(mContainer);
    mZoomPalette->show();

    mStylusPalette->stackUnder(mZoomPalette);

    mTipPalette = new UBStartupHintsPalette(mContainer);
    QList<QAction*> backgroundsActions;

    backgroundsActions << UBApplication::mainWindow->actionPlainLightBackground;
    backgroundsActions << UBApplication::mainWindow->actionCrossedLightBackground;
    if(UBSettings::settings()->isSeyesRuledBackground())
        backgroundsActions << UBApplication::mainWindow->actionSeyesRuledLightBackground;
    else
        backgroundsActions << UBApplication::mainWindow->actionRuledLightBackground;
    // The three patterns only: the color of the page, black included, is
    // chosen under them.

    mBackgroundsPalette = new UBBackgroundPalette(backgroundsActions, mContainer);
    mBackgroundsPalette->setButtonIconSize(QSize(128, 128));
    mBackgroundsPalette->groupActions();
    mBackgroundsPalette->setClosable(true);
    mBackgroundsPalette->setAutoClose(false);
    mBackgroundsPalette->adjustSizeAndPosition();
    mBackgroundsPalette->hide();

    QList<QAction*> addItemActions;

    addItemActions << UBApplication::mainWindow->actionAddItemToCurrentPage;
    addItemActions << UBApplication::mainWindow->actionAddItemToNewPage;
    addItemActions << UBApplication::mainWindow->actionAddItemToLibrary;

    mAddItemPalette = new UBActionPalette(addItemActions, Qt::Horizontal, mContainer);
    mAddItemPalette->setButtonIconSize(QSize(128, 128));
    mAddItemPalette->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    mAddItemPalette->groupActions();
    mAddItemPalette->setClosable(true);
    mAddItemPalette->adjustSizeAndPosition();
    mAddItemPalette->hide();

    QList<QAction*> eraseActions;

    eraseActions << UBApplication::mainWindow->actionEraseAnnotations;
    eraseActions << UBApplication::mainWindow->actionEraseItems;
    eraseActions << UBApplication::mainWindow->actionClearPage;
    eraseActions << UBApplication::mainWindow->actionEraseBackground;

    mErasePalette = new UBActionPalette(eraseActions, Qt::Horizontal , mContainer);
    mErasePalette->setButtonIconSize(QSize(128, 128));
    mErasePalette->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    mErasePalette->groupActions();
    mErasePalette->setClosable(true);
    mErasePalette->adjustSizeAndPosition();
    mErasePalette->hide();

    QList<QAction*> pageActions;

    pageActions << UBApplication::mainWindow->actionNewPage;
    pageActions << UBApplication::mainWindow->actionDuplicatePage;
    pageActions << UBApplication::mainWindow->actionImportPage;

    mPagePalette = new UBActionPalette(pageActions, Qt::Horizontal , mContainer);
    mPagePalette->setButtonIconSize(QSize(128, 128));
    mPagePalette->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    mPagePalette->groupActions();
    mPagePalette->setClosable(true);
    mPagePalette->adjustSizeAndPosition();
    mPagePalette->hide();

    connect(UBSettings::settings()->appToolBarOrientationVertical, SIGNAL(changed(QVariant)), this, SLOT(changeStylusPaletteOrientation(QVariant)));
}

void UBBoardPaletteManager::pagePaletteButtonPressed()
{
    mPageButtonPressedTime = QTime::currentTime();

    mPendingPageButtonPressed = true;
    QTimer::singleShot(longpress_interval, this, SLOT(pagePaletteButtonReleased()));
}


void UBBoardPaletteManager::pagePaletteButtonReleased()
{
    if (mPendingPageButtonPressed)
    {
        if( mPageButtonPressedTime.msecsTo(QTime::currentTime()) > longpress_interval-100)
        {
            // The palette is reinstanciated because the duplication depends on the current scene
            delete(mPagePalette);
            mPagePalette = 0;
            QList<QAction*>pageActions;
            pageActions << UBApplication::mainWindow->actionNewPage;
            pageActions << UBApplication::mainWindow->actionDuplicatePage;
            pageActions << UBApplication::mainWindow->actionImportPage;

            mPagePalette = new UBActionPalette(pageActions, Qt::Horizontal , mContainer);
            mPagePalette->setButtonIconSize(QSize(128, 128));
            mPagePalette->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
            mPagePalette->groupActions();
            mPagePalette->setClosable(true);

            // As we recreate the pagePalette every time, we must reconnect the slots
            connect(UBApplication::mainWindow->actionNewPage, SIGNAL(triggered()), mPagePalette, SLOT(close()));
            connect(UBApplication::mainWindow->actionDuplicatePage, SIGNAL(triggered()), mPagePalette, SLOT(close()));
            connect(UBApplication::mainWindow->actionImportPage, SIGNAL(triggered()), mPagePalette, SLOT(close()));
            connect(mPagePalette, SIGNAL(closed()), this, SLOT(pagePaletteClosed()));

            togglePagePalette(true);
        }
        else
        {
            UBApplication::mainWindow->actionNewPage->trigger();
        }

        mPendingPageButtonPressed = false;
    }
}

void UBBoardPaletteManager::erasePaletteButtonPressed()
{
    mEraseButtonPressedTime = QTime::currentTime();

    mPendingEraseButtonPressed = true;
    QTimer::singleShot(longpress_interval, this, SLOT(erasePaletteButtonReleased()));
}


void UBBoardPaletteManager::erasePaletteButtonReleased()
{
    if (mPendingEraseButtonPressed)
    {
        if( mEraseButtonPressedTime.msecsTo(QTime::currentTime()) > longpress_interval - 100)
        {
            toggleErasePalette(true);
        }
        else
        {
            UBApplication::mainWindow->actionClearPage->trigger();
        }

        mPendingEraseButtonPressed = false;
    }
}



void UBBoardPaletteManager::linkClicked(const QUrl& url)
{
      UBApplication::applicationController->showInternet();
      UBApplication::webController->loadUrl(url);
}


void UBBoardPaletteManager::purchaseLinkActivated(const QString& link)
{
    UBApplication::applicationController->showInternet();
    UBApplication::webController->loadUrl(QUrl(link));
}

void UBBoardPaletteManager::connectPalettes()
{
    connect(UBApplication::mainWindow->actionStylus, SIGNAL(toggled(bool)), this, SLOT(toggleStylusPalette(bool)));

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    foreach(QObject *widget, UBApplication::mainWindow->actionZoomIn->associatedObjects())
#else
    foreach(QWidget *widget, UBApplication::mainWindow->actionZoomIn->associatedWidgets())
#endif
    {
        QAbstractButton *button = qobject_cast<QAbstractButton*>(widget);
        if (button)
        {
            connect(button, SIGNAL(pressed()), this, SLOT(zoomButtonPressed()));
            connect(button, SIGNAL(released()), this, SLOT(zoomButtonReleased()));
        }
    }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    foreach(QObject *widget, UBApplication::mainWindow->actionZoomOut->associatedObjects())
#else
    foreach(QWidget *widget, UBApplication::mainWindow->actionZoomOut->associatedWidgets())
#endif
    {
        QAbstractButton *button = qobject_cast<QAbstractButton*>(widget);
        if (button)
        {
            connect(button, SIGNAL(pressed()), this, SLOT(zoomButtonPressed()));
            connect(button, SIGNAL(released()), this, SLOT(zoomButtonReleased()));
        }
    }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    foreach(QObject *widget, UBApplication::mainWindow->actionHand->associatedObjects())
#else
    foreach(QWidget *widget, UBApplication::mainWindow->actionHand->associatedWidgets())
#endif
    {
        QAbstractButton *button = qobject_cast<QAbstractButton*>(widget);
        if (button)
        {
            connect(button, SIGNAL(pressed()), this, SLOT(handButtonPressed()));
            connect(button, SIGNAL(released()), this, SLOT(handButtonReleased()));
        }
    }

    connect(UBApplication::mainWindow->actionBackgrounds, SIGNAL(toggled(bool)), this, SLOT(toggleBackgroundPalette(bool)));
    connect(mBackgroundsPalette, SIGNAL(closed()), this, SLOT(backgroundPaletteClosed()));

    connect(UBApplication::mainWindow->actionPlainLightBackground, SIGNAL(triggered()), this, SLOT(changeBackground()));
    connect(UBApplication::mainWindow->actionCrossedLightBackground, SIGNAL(triggered()), this, SLOT(changeBackground()));
    connect(UBApplication::mainWindow->actionRuledLightBackground, SIGNAL(triggered()), this, SLOT(changeBackground()));
    connect(UBApplication::mainWindow->actionSeyesRuledLightBackground, SIGNAL(triggered()), this, SLOT(changeBackground()));
    connect(UBApplication::mainWindow->actionPodcast, SIGNAL(triggered(bool)), this, SLOT(tooglePodcastPalette(bool)));

    connect(UBApplication::mainWindow->actionAddItemToCurrentPage, SIGNAL(triggered()), this, SLOT(addItemToCurrentPage()));
    connect(UBApplication::mainWindow->actionAddItemToNewPage, SIGNAL(triggered()), this, SLOT(addItemToNewPage()));
    connect(UBApplication::mainWindow->actionAddItemToLibrary, SIGNAL(triggered()), this, SLOT(addItemToLibrary()));

    connect(UBApplication::mainWindow->actionEraseItems, SIGNAL(triggered()), mErasePalette, SLOT(close()));
    connect(UBApplication::mainWindow->actionEraseAnnotations, SIGNAL(triggered()), mErasePalette, SLOT(close()));
    connect(UBApplication::mainWindow->actionClearPage, SIGNAL(triggered()), mErasePalette, SLOT(close()));
    connect(UBApplication::mainWindow->actionEraseBackground,SIGNAL(triggered()),mErasePalette,SLOT(close()));
    connect(mErasePalette, SIGNAL(closed()), this, SLOT(erasePaletteClosed()));

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    foreach(QObject *widget, UBApplication::mainWindow->actionErase->associatedObjects())
#else
    foreach(QWidget *widget, UBApplication::mainWindow->actionErase->associatedWidgets())
#endif
    {
        QAbstractButton *button = qobject_cast<QAbstractButton*>(widget);
        if (button)
        {
            connect(button, SIGNAL(pressed()), this, SLOT(erasePaletteButtonPressed()));
            connect(button, SIGNAL(released()), this, SLOT(erasePaletteButtonReleased()));
        }
    }

    connect(UBApplication::mainWindow->actionNewPage, SIGNAL(triggered()), mPagePalette, SLOT(close()));
    connect(UBApplication::mainWindow->actionDuplicatePage, SIGNAL(triggered()), mPagePalette, SLOT(close()));
    connect(UBApplication::mainWindow->actionImportPage, SIGNAL(triggered()), mPagePalette, SLOT(close()));
    connect(mPagePalette, SIGNAL(closed()), this, SLOT(pagePaletteClosed()));

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    foreach(QObject *widget, UBApplication::mainWindow->actionPages->associatedObjects())
#else
    foreach(QWidget *widget, UBApplication::mainWindow->actionPages->associatedWidgets())
#endif
    {
        QAbstractButton *button = qobject_cast<QAbstractButton*>(widget);
        if (button)
        {
            connect(button, SIGNAL(pressed()), this, SLOT(pagePaletteButtonPressed()));
            connect(button, SIGNAL(released()), this, SLOT(pagePaletteButtonReleased()));
        }
    }

}


bool isFirstResized = true;
void UBBoardPaletteManager::containerResized()
{
    int innerMargin = UBSettings::boardMargin;

    int userLeft = innerMargin;
    int userWidth = mContainer->width() - (2 * innerMargin);
    int userTop = innerMargin;
    int userHeight = mContainer->height() - (2 * innerMargin);

    if(mStylusPalette)
    {
        mStylusPalette->move(userLeft, userTop);
        mStylusPalette->adjustSizeAndPosition();
        mStylusPalette->initPosition();
    }

    placeZoomPalette();

    if (isFirstResized && mKeyboardPalette && mKeyboardPalette->parent() == UBApplication::boardController->controlContainer())
    {
        isFirstResized = false;
        mKeyboardPalette->move(userLeft + (userWidth - mKeyboardPalette->width())/2,
                               userTop + (userHeight - mKeyboardPalette->height())/2);
        mKeyboardPalette->adjustSizeAndPosition();
    }
}


void UBBoardPaletteManager::changeBackground()
{
    // A pattern keeps the color of the page.
    const bool dark = UBApplication::boardController->activeScene()->isDarkBackground();

    if (UBApplication::mainWindow->actionCrossedLightBackground->isChecked())
        UBApplication::boardController->changeBackground(dark, UBPageBackground::crossed);

    else if (UBApplication::mainWindow->actionRuledLightBackground->isChecked() ||
             UBApplication::mainWindow->actionSeyesRuledLightBackground->isChecked())
        UBApplication::boardController->changeBackground(dark, UBPageBackground::ruled);

    else
        UBApplication::boardController->changeBackground(dark, UBPageBackground::plain);

    mBackgroundsPalette->backgroundChanged();
}


void UBBoardPaletteManager::activeSceneChanged()
{
    std::shared_ptr<UBGraphicsScene> activeScene =  UBApplication::boardController->activeScene();
    int pageIndex = UBApplication::boardController->activeSceneIndex();

    if (mStylusPalette)
        connect(mStylusPalette, SIGNAL(mouseEntered()), activeScene.get(), SLOT(hideTool()));

    if (mpPageNavigWidget)
    {
        mpPageNavigWidget->setPageNumber(UBDocumentContainer::pageFromSceneIndex(pageIndex), activeScene->document()->pageCount());
    }

    if (mZoomPalette)
        connect(mZoomPalette, SIGNAL(mouseEntered()), activeScene.get(), SLOT(hideTool()));

    if (mBackgroundsPalette) {
        connect(mBackgroundsPalette, SIGNAL(mouseEntered()), activeScene.get(), SLOT(hideTool()));
        mBackgroundsPalette->refresh();
    }
}


void UBBoardPaletteManager::toggleBackgroundPalette(bool checked)
{
    mBackgroundsPalette->setVisible(checked);

    if (checked)
    {
        UBApplication::mainWindow->actionErase->setChecked(false);
        UBApplication::mainWindow->actionNewPage->setChecked(false);

        mBackgroundsPalette->adjustSizeAndPosition();
        mBackgroundsPalette->move((mContainer->width() - mBackgroundsPalette->width()) / 2,
            (mContainer->height() - mBackgroundsPalette->height()) / 5);
    }
}


void UBBoardPaletteManager::backgroundPaletteClosed()
{
    UBApplication::mainWindow->actionBackgrounds->setChecked(false);
}


void UBBoardPaletteManager::toggleStylusPalette(bool checked)
{
    mStylusPalette->setVisible(checked);
}


void UBBoardPaletteManager::toggleErasePalette(bool checked)
{
    mErasePalette->setVisible(checked);
    if (checked)
    {
        UBApplication::mainWindow->actionBackgrounds->setChecked(false);
        UBApplication::mainWindow->actionNewPage->setChecked(false);

        mErasePalette->adjustSizeAndPosition();
        mErasePalette->move((mContainer->width() - mErasePalette->width()) / 2,
            (mContainer->height() - mErasePalette->height()) / 5);
    }
}


void UBBoardPaletteManager::erasePaletteClosed()
{
    UBApplication::mainWindow->actionErase->setChecked(false);
}


void UBBoardPaletteManager::togglePagePalette(bool checked)
{
    mPagePalette->setVisible(checked);
    if (checked)
    {
        UBApplication::mainWindow->actionBackgrounds->setChecked(false);
        UBApplication::mainWindow->actionErase->setChecked(false);

        mPagePalette->adjustSizeAndPosition();
        mPagePalette->move((mContainer->width() - mPagePalette->width()) / 2,
            (mContainer->height() - mPagePalette->height()) / 5);
    }
}


void UBBoardPaletteManager::pagePaletteClosed()
{
    UBApplication::mainWindow->actionPages->setChecked(false);
}


void UBBoardPaletteManager::tooglePodcastPalette(bool checked)
{
    UBPodcastController::instance()->toggleRecordingPalette(checked);
}


void UBBoardPaletteManager::addItem(const QUrl& pUrl)
{
    mItemUrl = pUrl;
    mPixmap = QPixmap();
    mPos = QPointF(0, 0);
    mScaleFactor = 1.;

    mAddItemPalette->show();
    mAddItemPalette->adjustSizeAndPosition();

    mAddItemPalette->move((mContainer->width() - mAddItemPalette->width()) / 2,
        (mContainer->height() - mAddItemPalette->height()) / 5);
}

void UBBoardPaletteManager::changeMode(eUBDockPaletteWidgetMode newMode, bool isInit)
{
    mRightPalette->switchMode(newMode);
    mLeftPalette->switchMode(newMode);

    // Master Board shows neither the page thumbnails nor the library on the
    // sides: the tool options take the right side of the board.
    const bool rightPaletteVisible = false;
    const bool leftPaletteVisible = false;

    switch( newMode )
    {
        case eUBDockPaletteWidget_BOARD:
            {
                // On Application start up the mAddItemPalette isn't initialized yet
                if(mAddItemPalette){
                    mAddItemPalette->setParent(UBApplication::boardController->controlContainer());
                }
                mLeftPalette->assignParent(mContainer);
                mRightPalette->assignParent(mContainer);
                mRightPalette->stackUnder(mStylusPalette);
                mLeftPalette->stackUnder(mStylusPalette);
                if (UBPlatformUtils::hasVirtualKeyboard()
                    && mKeyboardPalette != NULL
                    && UBSettings::settings()->useSystemOnScreenKeyboard->get().toBool() == false)
                {
                    if(mKeyboardPalette->m_isVisible) {
                        mKeyboardPalette->hide();
                        mKeyboardPalette->setParent(UBApplication::boardController->controlContainer());
                        mKeyboardPalette->show();
                    }
                    else
                        mKeyboardPalette->setParent(UBApplication::boardController->controlContainer());
                }

                mLeftPalette->setVisible(leftPaletteVisible);
                mRightPalette->setVisible(rightPaletteVisible);
#ifdef Q_OS_WIN
                if (rightPaletteVisible)
                    mRightPalette->setAdditionalVOffset(0);
#endif

                if( !isInit )
                    containerResized();
                if (mWebToolsCurrentPalette)
                    mWebToolsCurrentPalette->hide();
            }
            break;

        case eUBDockPaletteWidget_DESKTOP:
            {
                mAddItemPalette->setParent((QWidget*)UBApplication::applicationController->uninotesController()->drawingView());
                mLeftPalette->assignParent((QWidget*)UBApplication::applicationController->uninotesController()->drawingView());
                mRightPalette->assignParent((QWidget*)UBApplication::applicationController->uninotesController()->drawingView());
                mStylusPalette->raise();

                if (UBPlatformUtils::hasVirtualKeyboard()
                    && mKeyboardPalette != NULL
                    && UBSettings::settings()->useSystemOnScreenKeyboard->get().toBool() == false)
                {

                    if(mKeyboardPalette->m_isVisible)
                    {
                        mKeyboardPalette->hide();
#ifndef Q_OS_LINUX
                        mKeyboardPalette->setParent((QWidget*)UBApplication::applicationController->uninotesController()->drawingView());
#else
                        mKeyboardPalette->setParent(0);
#endif
#ifdef Q_OS_OSX
                        mKeyboardPalette->setWindowFlags(Qt::Dialog | Qt::Popup | Qt::FramelessWindowHint);
#endif
                        mKeyboardPalette->show();
                    }
                    else
// In linux keyboard in desktop mode have to allways be with null parent
#ifdef Q_OS_LINUX
                        mKeyboardPalette->setParent(0);
#else
                        mKeyboardPalette->setParent((QWidget*)UBApplication::applicationController->uninotesController()->drawingView());
#endif //Q_OS_LINUX
#ifdef Q_OS_OSX
                        mKeyboardPalette->setWindowFlags(Qt::Dialog | Qt::Popup | Qt::FramelessWindowHint);
#endif

                }

                mLeftPalette->setVisible(leftPaletteVisible);
                mRightPalette->setVisible(rightPaletteVisible);
#ifdef Q_OS_WIN
                if (rightPaletteVisible && UBSettings::settings()->appToolBarPositionedAtTop->get().toBool())
                    mRightPalette->setAdditionalVOffset(30);
#endif

                if(!isInit)
                    UBApplication::applicationController->uninotesController()->TransparentWidgetResized();

                if (mWebToolsCurrentPalette)
                    mWebToolsCurrentPalette->hide();
            }
            break;

        case eUBDockPaletteWidget_WEB:
            {
                mAddItemPalette->setParent(UBApplication::mainWindow);
                if (UBPlatformUtils::hasVirtualKeyboard()
                    && mKeyboardPalette != NULL
                    && UBSettings::settings()->useSystemOnScreenKeyboard->get().toBool() == false)
                {
//                    tmp variable?
//                    WBBrowserWindow* brWnd = UBApplication::webController->GetCurrentWebBrowser();

                    if(mKeyboardPalette->m_isVisible)
                    {
                        mKeyboardPalette->hide();
                        mKeyboardPalette->setParent(UBApplication::mainWindow);
                        mKeyboardPalette->show();
                    }
                    else
                        mKeyboardPalette->setParent(UBApplication::mainWindow);
                }

            }
            break;

        case eUBDockPaletteWidget_DOCUMENT:
            {
                mLeftPalette->setVisible(leftPaletteVisible);
                mRightPalette->setVisible(rightPaletteVisible);
                mLeftPalette->assignParent(UBApplication::documentController->controlView());
                mRightPalette->assignParent(UBApplication::documentController->controlView());
                if (UBPlatformUtils::hasVirtualKeyboard()
                    && mKeyboardPalette != NULL
                    && UBSettings::settings()->useSystemOnScreenKeyboard->get().toBool() == false)
                {

                    if(mKeyboardPalette->m_isVisible)
                    {
                        mKeyboardPalette->hide();
                        mKeyboardPalette->setParent(UBApplication::documentController->controlView());
                        mKeyboardPalette->show();
                    }
                    else
                        mKeyboardPalette->setParent(UBApplication::documentController->controlView());
                }
                if (mWebToolsCurrentPalette)
                    mWebToolsCurrentPalette->hide();
            }
            break;

        default:
            {
                mLeftPalette->setVisible(leftPaletteVisible);
                mRightPalette->setVisible(rightPaletteVisible);
                mLeftPalette->assignParent(0);
                mRightPalette->assignParent(0);
                if (UBPlatformUtils::hasVirtualKeyboard()
                    && mKeyboardPalette != NULL
                    && UBSettings::settings()->useSystemOnScreenKeyboard->get().toBool() == false)
                {

                    if(mKeyboardPalette->m_isVisible)
                    {
                        mKeyboardPalette->hide();
                        mKeyboardPalette->setParent(0);
                        mKeyboardPalette->show();
                    }
                    else
                        mKeyboardPalette->setParent(0);
                }
            }
            break;
    }

    if( !isInit )
        UBApplication::boardController->notifyPageChanged();
}

void UBBoardPaletteManager::addItem(const QPixmap& pPixmap, const QPointF& pos,  qreal scaleFactor, const QUrl& sourceUrl)
{
    mItemUrl = sourceUrl;
    mPixmap = pPixmap;
    mPos = pos;
    mScaleFactor = scaleFactor;

    mAddItemPalette->show();
    mAddItemPalette->adjustSizeAndPosition();

    mAddItemPalette->move((mContainer->width() - mAddItemPalette->width()) / 2,
        (mContainer->height() - mAddItemPalette->height()) / 5);
}


void UBBoardPaletteManager::addItemToCurrentPage()
{
    UBApplication::applicationController->showBoard();
    mAddItemPalette->hide();
    if(mPixmap.isNull())
        UBApplication::boardController->downloadURL(mItemUrl);
    else
    {
        UBGraphicsPixmapItem* item = UBApplication::boardController->activeScene()->addPixmap(mPixmap, NULL, mPos, mScaleFactor);

        QString documentPath = UBApplication::boardController->selectedDocument()->persistencePath();
        QString fileName = UBPersistenceManager::imageDirectory + "/" + item->uuid().toString() + ".png";
        QString path = documentPath + "/" + fileName;

        item->setSourceUrl(QUrl(path));
        item->setSelected(true);

        UBDrawingController::drawingController()->setStylusTool(UBStylusTool::Selector);
    }
}


void UBBoardPaletteManager::addItemToNewPage()
{
    UBApplication::boardController->addScene();
    addItemToCurrentPage();
}


void UBBoardPaletteManager::addItemToLibrary()
{
    QByteArray data;

    if(mPixmap.isNull())
    {
        QFile file(mItemUrl.toLocalFile());

        if (file.open(QFile::ReadOnly))
        {
            data = file.readAll();
            file.close();
        }
    }
    else
    {
        if(mScaleFactor != 1.)
        {
             mPixmap = mPixmap.scaled(mScaleFactor * mPixmap.width(), mScaleFactor* mPixmap.height()
                     , Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }

        QBuffer buffer(&data);
        mPixmap.save(&buffer, "png");
    }

    if(!data.isEmpty())
    {
        QDateTime now = QDateTime::currentDateTime();
        QString capturedName  = tr("CapturedImage") + "-" + now.toString("dd-MM-yyyy hh-mm-ss") + ".png";
        mpFeaturesWidget->importImage(data, capturedName);
    }
    else
    {
        UBApplication::showMessage(tr("Error Adding Image to Library"));
    }

    mAddItemPalette->hide();
}

void UBBoardPaletteManager::zoomButtonPressed()
{
    mZoomButtonPressedTime = QTime::currentTime();

    mPendingZoomButtonPressed = true;
    QTimer::singleShot(longpress_interval, this, SLOT(zoomButtonReleased()));
}


void UBBoardPaletteManager::zoomButtonReleased()
{
    if (mPendingZoomButtonPressed)
    {
        if(mZoomButtonPressedTime.msecsTo(QTime::currentTime()) > longpress_interval - 100)
        {
            mBoardControler->zoomRestore();
        }

        mPendingZoomButtonPressed = false;
    }
}

void UBBoardPaletteManager::handButtonPressed()
{
    mHandButtonPressedTime = QTime::currentTime();

    mPendingHandButtonPressed = true;
    QTimer::singleShot(longpress_interval, this, SLOT(handButtonReleased()));
}


void UBBoardPaletteManager::handButtonReleased()
{
    if (mPendingHandButtonPressed)
    {
        if(mHandButtonPressedTime.msecsTo(QTime::currentTime()) > longpress_interval - 100)
        {
            mBoardControler->centerRestore();
        }

        mPendingHandButtonPressed = false;
    }
}

void UBBoardPaletteManager::showVirtualKeyboard(bool show)
{
    if (mKeyboardPalette)
        mKeyboardPalette->setVisible(show);
}

void UBBoardPaletteManager::placeStylusPalette(int edge)
{
    UBStylusPalette::setEdge(edge);

    const bool vertical = (edge == Qt::LeftEdge || edge == Qt::RightEdge);

    if (vertical != UBSettings::settings()->appToolBarOrientationVertical->get().toBool())
    {
        // Replaces the palette by one of the other orientation.
        UBSettings::settings()->appToolBarOrientationVertical->set(vertical);
    }
    else
    {
        mStylusPalette->initPosition();
    }

    placeZoomPalette();
}

void UBBoardPaletteManager::saveBoardScreenshot()
{
    // Only the board, as it is seen: the palettes are widgets above it and
    // are not part of what the view renders.
    UBBoardView* view = UBApplication::boardController->controlView();
    QWidget* viewport = view->viewport();
    const qreal ratio = viewport->devicePixelRatioF();

    QPixmap screenshot(viewport->size() * ratio);
    screenshot.setDevicePixelRatio(ratio);
    screenshot.fill(Qt::white);

    {
        QPainter painter(&screenshot);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        view->render(&painter, QRectF(QPointF(0, 0), viewport->size()), viewport->rect());
    }

    UBScreenshot::flash(viewport);
    UBApplication::showMessage(UBScreenshot::save(screenshot));
}

void UBBoardPaletteManager::placeZoomPalette()
{
    if (!mZoomPalette)
    {
        return;
    }

    QRect area = mStylusPalette ? mStylusPalette->boardArea() : mContainer->rect();
    const QRect avoid = (mStylusPalette && mStylusPalette->isVisible()) ? mStylusPalette->geometry() : QRect();

    // The rulers of the canvas go at the right or at the bottom when the
    // toolbar is at the left or at the top: the box stays clear of them.
    if (UBCanvasRulers::isShown() && mStylusPalette && mStylusPalette->isVisible())
    {
        if (UBStylusPalette::edge() == Qt::LeftEdge)
        {
            area.setRight(area.right() - UBCanvasRulers::thickness());
        }
        else if (UBStylusPalette::edge() == Qt::TopEdge)
        {
            area.setBottom(area.bottom() - UBCanvasRulers::thickness());
        }
    }

    mZoomPalette->placeIn(area, avoid);
    mZoomPalette->refreshPalette();
    mZoomPalette->show();
}

void UBBoardPaletteManager::changeStylusPaletteOrientation(QVariant var)
{
    bool bVertical = var.toBool();
    const int edge = UBStylusPalette::edge();

    // When the orientation is chosen in the preferences, the palette moves
    // to a border that suits it.
    if (bVertical && edge != Qt::LeftEdge && edge != Qt::RightEdge)
    {
        UBStylusPalette::setEdge(Qt::RightEdge);
    }
    else if (!bVertical && edge != Qt::TopEdge && edge != Qt::BottomEdge)
    {
        UBStylusPalette::setEdge(Qt::BottomEdge);
    }

    bool bVisible = mStylusPalette->isVisible();

    // Clean the old palette
    if(NULL != mStylusPalette)
    {
        delete mStylusPalette;
        mStylusPalette = NULL;
    }

    // Create the new palette
    if(bVertical)
    {
        mStylusPalette = new UBStylusPalette(mContainer, Qt::Vertical);
    }
    else
    {
        mStylusPalette = new UBStylusPalette(mContainer, Qt::Horizontal);
    }

    connect(mStylusPalette, SIGNAL(stylusToolDoubleClicked(int)), UBApplication::boardController, SLOT(stylusToolDoubleClicked(int)));
    connect(mStylusPalette, &UBStylusPalette::edgeChosen, this, &UBBoardPaletteManager::placeStylusPalette);
    mStylusPalette->setVisible(bVisible); // always show stylus palette at startup
    placeZoomPalette();
}


void UBBoardPaletteManager::refreshPalettes()
{
    mRightPalette->update();
    mLeftPalette->update();
}

void UBBoardPaletteManager::startDownloads()
{
    if(!mDownloadInProgress)
    {
        mDownloadInProgress = true;
        mpDownloadWidget->setVisibleState(true);
        mRightPalette->addTab(mpDownloadWidget);
    }
}

void UBBoardPaletteManager::stopDownloads()
{
    if(mDownloadInProgress)
    {
        mDownloadInProgress = false;
        mpDownloadWidget->setVisibleState(false);
        mRightPalette->removeTab(mpDownloadWidget);
    }
}

void UBBoardPaletteManager::showToolOptions(int tool)
{
    if (!mStylusPalette || !UBToolOptionsPopup::hasOptions(tool))
    {
        return;
    }

    if (!mToolOptionsPopup)
    {
        mToolOptionsPopup = new UBToolOptionsPopup(mContainer);
    }

    QRect anchor = mStylusPalette->geometry();
    anchor.moveTopLeft(mStylusPalette->mapToGlobal(QPoint(0, 0)));

    QAction* checkedAction = nullptr;

    for (QAction* action : mStylusPalette->actions())
    {
        if (action->isChecked())
        {
            checkedAction = action;
            break;
        }
    }

    if (checkedAction)
    {
        if (UBActionPaletteButton* button = mStylusPalette->getButtonFromAction(checkedAction))
        {
            anchor = QRect(button->mapToGlobal(QPoint(0, 0)), button->size());
        }
    }

    mToolOptionsPopup->showForTool(tool, anchor);
}
