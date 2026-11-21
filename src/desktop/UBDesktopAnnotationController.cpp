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




#include "UBDesktopAnnotationController.h"

#include "gui/UBScreenshot.h"
#include "gui/UBToolOptionsArrows.h"

#include <QLabel>
#include <QScreen>

#include "frameworks/UBPlatformUtils.h"

#include "core/UBApplication.h"
#include "core/UBApplicationController.h"
#include "core/UBDisplayManager.h"
#include "core/UBSettings.h"

#include "web/UBWebController.h"

#include "gui/UBMainWindow.h"

#include "board/UBBoardView.h"
#include "board/UBDrawingController.h"
#include "board/UBBoardController.h"
#include "board/UBBoardPaletteManager.h"

#include "domain/UBGraphicsScene.h"
#include "domain/UBGraphicsPolygonItem.h"

#include "UBCustomCaptureWindow.h"
#include "UBDesktopPalette.h"

#include "gui/UBKeyboardPalette.h"
#include "gui/UBResources.h"
#include "gui/UBToolOptionsPopup.h"
#include "gui/UBAiPanel.h"
#include "gui/UBGifWindow.h"
#include "desktop/UBDesktopFrame.h"

#include "core/memcheck.h"
#include "gui/UBMeasureTool.h"

UBDesktopAnnotationController::UBDesktopAnnotationController(QObject *parent, UBRightPalette* rightPalette)
        : QObject(parent)
        , mTransparentDrawingView(0)
        , mTransparentDrawingScene(0)
        , mDesktopPalette(NULL)
        , mRightPalette(rightPalette)
        , mWindowPositionInitialized(false)
        , mIsFullyTransparent(false)
        , mDesktopToolsPalettePositioned(false)
        , mbArrowClicked(false)
        , mCustomCaptureClicked(false)
        , mBoardStylusTool(UBDrawingController::drawingController()->stylusTool())
        , mDesktopStylusTool(UBDrawingController::drawingController()->stylusTool())
{

    mTransparentDrawingView = new UBBoardView(UBApplication::boardController, static_cast<QWidget*>(0), false, true); // deleted in UBDesktopAnnotationController::destructor
    mTransparentDrawingView->setAttribute(Qt::WA_TranslucentBackground, true);
#ifdef Q_OS_OSX
    // didn't find the equivalent in Qt6
    #if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        mTransparentDrawingView->setAttribute(Qt::WA_MacNoShadow, true);
    #endif
#endif //Q_OS_OSX

    mTransparentDrawingView->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Window | Qt::NoDropShadowWindowHint | Qt::X11BypassWindowManagerHint);
    mTransparentDrawingView->setCacheMode(QGraphicsView::CacheNone);
    mTransparentDrawingView->resize(UBApplication::displayManager->screenSize(ScreenRole::Desktop));

    mTransparentDrawingView->setMouseTracking(true);

    mTransparentDrawingView->setAcceptDrops(true);

    QString backgroundStyle = "QWidget {background-color: rgba(127, 127, 127, 0)}";
    mTransparentDrawingView->setStyleSheet(backgroundStyle);

    mTransparentDrawingScene = std::make_shared<UBGraphicsScene>(nullptr, false);
    updateColors();

    mTransparentDrawingView->setScene(mTransparentDrawingScene.get());
    mTransparentDrawingScene->setDrawingMode(true);

    mDesktopPalette = new UBDesktopPalette(mTransparentDrawingView, rightPalette); 
    // This was not fix, parent reverted
    // FIX #633: The palette must be 'floating' in order to stay on top of the library palette

    if (UBPlatformUtils::hasVirtualKeyboard())
    {
        connect( UBApplication::boardController->paletteManager()->mKeyboardPalette, SIGNAL(keyboardActivated(bool)), 
                 mTransparentDrawingView, SLOT(virtualKeyboardActivated(bool)));

#ifdef UB_REQUIRES_MASK_UPDATE
        connect(UBApplication::boardController->paletteManager()->mKeyboardPalette, SIGNAL(moved(QPoint)), this, SLOT(refreshMask()));
        connect(UBApplication::mainWindow->actionVirtualKeyboard, SIGNAL(triggered(bool)), this, SLOT(refreshMask()));
        connect(mDesktopPalette,SIGNAL(refreshMask()), this, SLOT(refreshMask()));
#endif
    }

    connect(mDesktopPalette, SIGNAL(uniboardClick()), this, SLOT(goToUniboard()));
    connect(mDesktopPalette, SIGNAL(customClick()), this, SLOT(customCapture()));
    connect(mDesktopPalette, SIGNAL(screenClick()), this, SLOT(screenCapture()));
    connect(mDesktopPalette, &UBDesktopPalette::screenshotClick, this, &UBDesktopAnnotationController::saveScreenshot);
    connect(mDesktopPalette, &UBDesktopPalette::clearClick, this, &UBDesktopAnnotationController::eraseDesktopAnnotations);
    connect(UBApplication::mainWindow->actionPointer, SIGNAL(triggered()), this, SLOT(onToolClicked()));
    connect(UBApplication::mainWindow->actionSelector, SIGNAL(triggered()), this, SLOT(onToolClicked()));
    connect(mDesktopPalette, SIGNAL(maximized()), this, SLOT(onDesktopPaletteMaximized()));
    connect(mDesktopPalette, SIGNAL(minimizeStart(eMinimizedLocation)), this, SLOT(onDesktopPaletteMinimize()));
    connect(mDesktopPalette, SIGNAL(mouseEntered()), mTransparentDrawingScene.get(), SLOT(hideTool()));
    connect(mRightPalette, SIGNAL(mouseEntered()), mTransparentDrawingScene.get(), SLOT(hideTool()));
    connect(mRightPalette, SIGNAL(pageSelectionChangedRequired()), this, SLOT(updateBackground()));

    connect(mTransparentDrawingView, SIGNAL(resized(QResizeEvent*)), this, SLOT(onTransparentWidgetResized()));


    connect(UBDrawingController::drawingController(), SIGNAL(stylusToolChanged(int)), this, SLOT(stylusToolChanged(int)));

    mDesktopPalette->setBackgroundBrush(UBSettings::settings()->opaquePaletteColor);

    mFrame = new UBDesktopFrame();

    // The options of a tool open beside its button, in a popup over the desktop.
    mToolOptionsPopup = new UBToolOptionsPopup(mTransparentDrawingView);

    connect(UBApplication::mainWindow->actionEraseDesktopAnnotations, SIGNAL(triggered()), this, SLOT(eraseDesktopAnnotations()));
    connect(UBApplication::boardController, SIGNAL(backgroundChanged()), this, SLOT(updateColors()));
    connect(UBApplication::boardController, SIGNAL(activeSceneChanged()), this, SLOT(updateColors()));
    mHoldTimer.setSingleShot(true);
    connect(&mHoldTimer, &QTimer::timeout, this, &UBDesktopAnnotationController::toolButtonReleased);

#ifdef UB_REQUIRES_MASK_UPDATE
    connect(mDesktopPalette, SIGNAL(moving()), this, SLOT(refreshMask()));
    connect(UBApplication::boardController->paletteManager()->rightPalette(), SIGNAL(resized()), this, SLOT(refreshMask()));
    connect(UBApplication::boardController->paletteManager()->addItemPalette(), SIGNAL(closed()), this, SLOT(refreshMask()));
#endif
    onDesktopPaletteMaximized();
}

UBDesktopAnnotationController::~UBDesktopAnnotationController()
{
    delete mFrame;
    delete mTransparentDrawingView;
}

void UBDesktopAnnotationController::updateColors(){
    if(UBApplication::boardController->activeScene()->isDarkBackground()){
        mTransparentDrawingScene->setBackground(true, UBPageBackground::plain);
    }else{
        mTransparentDrawingScene->setBackground(false, UBPageBackground::plain);
    }
}

UBDesktopPalette* UBDesktopAnnotationController::desktopPalette()
{
    return mDesktopPalette;
}

QPainterPath UBDesktopAnnotationController::desktopPalettePath() const
{
    QPainterPath result;
    if (mDesktopPalette && mDesktopPalette->isVisible()) {
        result.addRect(mDesktopPalette->geometry());
    }

    return result;
}

void UBDesktopAnnotationController::eraseDesktopAnnotations()
{
    if (mTransparentDrawingScene)
    {
        mTransparentDrawingScene->clearContent(UBGraphicsScene::clearAnnotations);
    }
}


UBBoardView* UBDesktopAnnotationController::drawingView()
{
    return mTransparentDrawingView;
}


void UBDesktopAnnotationController::showWindow()
{
    mDesktopPalette->setDisplaySelectButtonVisible(true);

    connect(UBApplication::applicationController, SIGNAL(desktopMode(bool))
            , mDesktopPalette, SLOT(setDisplaySelectButtonVisible(bool)));

    mDesktopPalette->show();
    mFrame->showOn(UBApplication::displayManager->screen(ScreenRole::Desktop));

    bool showDisplay = UBSettings::settings()->webShowPageImmediatelyOnMirroredScreen->get().toBool();

    mDesktopPalette->showHideClick(showDisplay);
    mDesktopPalette->updateShowHideState(showDisplay);

    if (!mWindowPositionInitialized)
    {
        // Placed first away from the border, where it would minimize itself,
        // then again once it has its full size.
        mWindowPositionInitialized = true;
        mDesktopPalette->placeAtSavedEdge();
        mDesktopPalette->maximizeMe();
        mDesktopPalette->placeAtSavedEdge();
    }

    updateBackground();

    mBoardStylusTool = UBDrawingController::drawingController()->stylusTool();

    UBDrawingController::drawingController()->setStylusTool(mDesktopStylusTool);

#ifndef Q_OS_LINUX
    UBPlatformUtils::showFullScreen(mTransparentDrawingView);
#else
    // this is necessary to avoid hiding the panels on Unity and Cinnamon
    // if finer control is necessary, use qgetenv("XDG_CURRENT_DESKTOP")
    mTransparentDrawingView->show();
#endif
    UBPlatformUtils::hideMenuBarAndDock();

    mDesktopPalette->appear();

#ifdef UB_REQUIRES_MASK_UPDATE
    updateMask(true);
#endif // UB_REQUIRES_MASK_UPDATE
}


void UBDesktopAnnotationController::close()
{
    if (mTransparentDrawingView)
        mTransparentDrawingView->hide();

    mDesktopPalette->hide();
    mToolOptionsPopup->hide();
    mFrame->hide();
}


void UBDesktopAnnotationController::stylusToolChanged(int tool)
{
    Q_UNUSED(tool);
#ifdef Q_OS_OSX
    /* no longer needed
     if (UBDrawingController::drawingController()->isInDesktopMode())
     {
         UBStylusTool::Enum eTool = (UBStylusTool::Enum)tool;
         if(eTool == UBStylusTool::Selector)
         {
             UBPlatformUtils::toggleFinder(true);
         }
         else
         {
             UBPlatformUtils::toggleFinder(false);
         }
     }*/
#endif

    updateBackground();
}


void UBDesktopAnnotationController::updateBackground()
{
    QBrush newBrush;

    if (mIsFullyTransparent
            || UBDrawingController::drawingController()->stylusTool() == UBStylusTool::Selector)
    {
        newBrush = QBrush(Qt::transparent);
#ifdef UB_REQUIRES_MASK_UPDATE
        updateMask(true);
#endif //UB_REQUIRES_MASK_UPDATE
    }
    else
    {
#if defined(Q_OS_OSX)
        newBrush = QBrush(QColor(127, 127, 127, 15));
#else
        newBrush = QBrush(QColor(127, 127, 127, 1));
#endif
#ifdef UB_REQUIRES_MASK_UPDATE
        updateMask(false);
#endif //UB_REQUIRES_MASK_UPDATE
    }

    if (mTransparentDrawingScene && mTransparentDrawingScene->backgroundBrush() != newBrush)
        mTransparentDrawingScene->setBackgroundBrush(newBrush);
}


void UBDesktopAnnotationController::hideWindow()
{
    if (mTransparentDrawingView)
        mTransparentDrawingView->hide();

    mDesktopPalette->hide();
    mFrame->hide();
    mToolOptionsPopup->hide();

    mDesktopStylusTool = UBDrawingController::drawingController()->stylusTool();
    UBDrawingController::drawingController()->setStylusTool(mBoardStylusTool);
}


void UBDesktopAnnotationController::goToUniboard()
{
    UBApplication::applicationController->showBoard();
}


void UBDesktopAnnotationController::customCapture()
{
    if (!mCustomCaptureClicked)
    {
        mCustomCaptureClicked = true;
        onToolClicked();
        mIsFullyTransparent = true;
        updateBackground();

        mDesktopPalette->disappearForCapture();
        UBCustomCaptureWindow customCaptureWindow(mDesktopPalette);
        // need to show the window before execute it to avoid some glitch on windows.

    #ifndef Q_OS_WIN // Working only without this call on win32 desktop mode
        UBPlatformUtils::showFullScreen(&customCaptureWindow);
    #endif

        if (customCaptureWindow.execute(getScreenPixmap()) == QDialog::Accepted)
        {
            QPixmap selectedPixmap = customCaptureWindow.getSelectedPixmap();
            emit imageCaptured(selectedPixmap, false);
        }


        mDesktopPalette->appear();

        mCustomCaptureClicked = false;
        mIsFullyTransparent = false;
        updateBackground();
    }
}


void UBDesktopAnnotationController::screenCapture()
{
    onToolClicked();
    mIsFullyTransparent = true;
    updateBackground();

    mDesktopPalette->disappearForCapture();

    QPixmap originalPixmap = getScreenPixmap();

    mDesktopPalette->appear();

    emit imageCaptured(originalPixmap, false);

    mIsFullyTransparent = false;

    updateBackground();
}


void UBDesktopAnnotationController::saveScreenshot()
{
    // Without the palette and the border, with the annotations. The palette
    // is a child of the drawing window, so it is hidden rather than made
    // transparent.
    mDesktopPalette->hide();
    mFrame->hide();

    if (mToolOptionsPopup)
    {
        mToolOptionsPopup->hide();
    }

    qApp->processEvents();

    QTimer::singleShot(150, this, [this]() {
        // The whole screen except the taskbar of the system.
        QScreen* screen = UBApplication::displayManager->screen(ScreenRole::Control);
        const QRect area = screen ? screen->availableGeometry() : QRect();
        const QPixmap screenshot = screen ? UBApplication::displayManager->grab(ScreenRole::Control,
                                                area.translated(-screen->geometry().topLeft()))
                                          : QPixmap();
        mDesktopPalette->show();
        mFrame->showOn(UBApplication::displayManager->screen(ScreenRole::Desktop));

        if (screen)
        {
            UBScreenshot::flash(area);
        }

        showNotice(UBScreenshot::save(screenshot));
    });
}

void UBDesktopAnnotationController::showNotice(const QString& text)
{
    // A short notice beside the palette, which disappears by itself.
    auto notice = new QLabel(text, nullptr, Qt::ToolTip | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    notice->setAttribute(Qt::WA_DeleteOnClose);
    notice->setStyleSheet("QLabel { background: #323232; color: #f0f0f0; border: 1px solid #555555;"
                          " border-radius: 6px; padding: 8px 12px; font-size: 13px; }");
    notice->adjustSize();

    const QRect palette(mDesktopPalette->mapToGlobal(QPoint(0, 0)), mDesktopPalette->size());
    const QRect screen = mDesktopPalette->screen()->availableGeometry();
    QPoint position(palette.right() + 12, palette.bottom() - notice->height());

    if (position.x() + notice->width() > screen.right())
    {
        position.setX(palette.left() - 12 - notice->width());
    }

    notice->move(position);
    notice->show();
    QTimer::singleShot(3000, notice, &QLabel::close);
}

QPixmap UBDesktopAnnotationController::getScreenPixmap()
{
    return UBApplication::displayManager->grab(ScreenRole::Control);
}


void UBDesktopAnnotationController::updateShowHideState(bool pEnabled)
{
    mDesktopPalette->updateShowHideState(pEnabled);
}


void UBDesktopAnnotationController::screenLayoutChanged()
{
    if (UBApplication::applicationController &&
            UBApplication::displayManager &&
            UBApplication::displayManager->hasDisplay())
    {
        mDesktopPalette->setShowHideButtonVisible(true);
    }
    else
    {
        mDesktopPalette->setShowHideButtonVisible(false);
    }
}

QList<QAction*> UBDesktopAnnotationController::toolActions() const
{
    UBMainWindow* mainWindow = UBApplication::mainWindow;

    Q_UNUSED(mainWindow)
    return UBDesktopPalette::toolActions();
}

int UBDesktopAnnotationController::toolOf(QAction* action) const
{
    UBMainWindow* mainWindow = UBApplication::mainWindow;

    const QMap<QAction*, int> tools = {
        { mainWindow->actionPen, UBStylusTool::Pen },
        { mainWindow->actionEraser, UBStylusTool::Eraser },
        { mainWindow->actionMarker, UBStylusTool::Marker },
        { mainWindow->actionPointer, UBStylusTool::Pointer },
        { mainWindow->actionSelector, UBStylusTool::Selector },
        { mainWindow->actionArrowShape, UBStylusTool::Arrow },
        { mainWindow->actionRectangle, UBStylusTool::Rectangle },
        { mainWindow->actionEllipse, UBStylusTool::Ellipse },
        { mainWindow->actionUnderline, UBStylusTool::Underline },
        { mainWindow->actionText, UBStylusTool::Text },
        { mainWindow->actionHighlight, UBStylusTool::Highlight },
        { mainWindow->actionNote, UBStylusTool::Note },
        { mainWindow->actionStamp, UBStylusTool::Stamp },
        { mainWindow->actionEmoji, UBStylusTool::Emoji },
        { mainWindow->actionSpotlight, UBStylusTool::Spotlight },
        { mainWindow->actionAttachment, UBStylusTool::Attachment },
        { mainWindow->actionAi, UBStylusTool::Ai },
        { mainWindow->actionMeasure, UBStylusTool::Measure },
    };

    return tools.value(action, -1);
}

/**
 * \brief A tool button was pressed: the tool is selected, and its options are
 * shown when the little arrow of the button is clicked or the button is held.
 */
void UBDesktopAnnotationController::toolButtonPressed()
{
    auto button = qobject_cast<UBActionPaletteButton*>(sender());

    if (!button)
    {
        return;
    }

    QAction* action = button->defaultAction();
    const int tool = toolOf(action);

    mToolOptionsPopup->hide();
    UBDrawingController::drawingController()->setStylusTool(tool);
    mPressedAction = action;
    mPressTime = QTime::currentTime();
    mbArrowClicked = false;

    // The selector lets the clicks through on the desktop: its lasso is for the board.
    if (!UBToolOptionsPopup::hasOptions(tool) || tool == UBStylusTool::Selector)
    {
        return;
    }

    // The AI and the measurements do nothing by themselves: their options open at once.
    if (tool == UBStylusTool::Ai || tool == UBStylusTool::Measure)
    {
        mbArrowClicked = true;
        toolButtonReleased();
        return;
    }

    // Check if the mouse cursor is on the little arrow
    const QPoint cursorPos = button->mapFromGlobal(QCursor::pos());

    if (UBToolOptionsArrows::isOnTriangle(mDesktopPalette->screenEdge(), cursorPos, button->size(), button->iconSize()))
    {
        mbArrowClicked = true;
        toolButtonReleased();
    }
    else
    {
        mHoldTimer.start(PROPERTY_PALETTE_TIMER);
    }
}

/**
 * \brief The tool button was released, or held long enough to show its options.
 */
void UBDesktopAnnotationController::toolButtonReleased()
{
    mHoldTimer.stop();

    QAction* action = mPressedAction;
    mPressedAction = nullptr;

    if (!action)
    {
        return;
    }

    const int tool = toolOf(action);
    const bool held = mPressTime.msecsTo(QTime::currentTime()) > PROPERTY_PALETTE_TIMER - 100;

    if (UBToolOptionsPopup::hasOptions(tool) && tool != UBStylusTool::Selector && (mbArrowClicked || held))
    {
        showToolOptions(action);
    }
    else
    {
        action->trigger();
    }

    action->setChecked(true);
    switchCursor(tool);
}

void UBDesktopAnnotationController::showToolOptions(QAction* action)
{
    UBActionPaletteButton* button = mDesktopPalette->getButtonFromAction(action);

    if (!button)
    {
        return;
    }

    const QRect anchor(button->mapToGlobal(QPoint(0, 0)), button->size());
    mToolOptionsPopup->showForTool(toolOf(action), anchor, UBToolOptionsArrows::openingSide(mDesktopPalette->screenEdge()));
}


void UBDesktopAnnotationController::switchCursor(const int tool)
{
    mTransparentDrawingScene->setToolCursor(tool);
    mTransparentDrawingView->setToolCursor(tool);
}

/**
 * \brief Reconnect the pressed & released signals of the property palettes
 */
void UBDesktopAnnotationController::onDesktopPaletteMaximized()
{
    for (QAction* action : toolActions())
    {
        UBActionPaletteButton* button = mDesktopPalette->getButtonFromAction(action);

        if (button)
        {
            connect(button, &UBActionPaletteButton::pressed, this, &UBDesktopAnnotationController::toolButtonPressed, Qt::UniqueConnection);
            connect(button, &UBActionPaletteButton::released, this, &UBDesktopAnnotationController::toolButtonReleased, Qt::UniqueConnection);
        }
    }
}

/**
 * \brief Disconnect the pressed & release signals of the property palettes
 * This is done to prevent memory leaks
 */
void UBDesktopAnnotationController::onDesktopPaletteMinimize()
{
    for (QAction* action : toolActions())
    {
        UBActionPaletteButton* button = mDesktopPalette->getButtonFromAction(action);

        if (button)
        {
            disconnect(button, &UBActionPaletteButton::pressed, this, &UBDesktopAnnotationController::toolButtonPressed);
            disconnect(button, &UBActionPaletteButton::released, this, &UBDesktopAnnotationController::toolButtonReleased);
        }
    }
}

void UBDesktopAnnotationController::TransparentWidgetResized()
{
    onTransparentWidgetResized();
}

/**
 * \brief Resize the library palette.
 */
void UBDesktopAnnotationController::onTransparentWidgetResized()
{
    int rW = UBApplication::boardController->paletteManager()->rightPalette()->width();
    int lW = UBApplication::boardController->paletteManager()->leftPalette()->width();

    int rH = mTransparentDrawingView->height();

    UBApplication::boardController->paletteManager()->rightPalette()->resize(rW+1, rH);
    UBApplication::boardController->paletteManager()->rightPalette()->resize(rW, rH);

    UBApplication::boardController->paletteManager()->leftPalette()->resize(lW+1, rH);
    UBApplication::boardController->paletteManager()->leftPalette()->resize(lW, rH);
}

void UBDesktopAnnotationController::updateMask(bool bTransparent)
{
    if(bTransparent)
    {
        // Here we have to generate a new mask This method is certainly resource
        // consuming but for the moment this is the only solution that I found.
        mMask = QPixmap(mTransparentDrawingView->width(), mTransparentDrawingView->height());
        mMask.fill(Qt::transparent);

        QPainter p;

        p.begin(&mMask);

        p.setPen(Qt::red);
        p.setBrush(QBrush(Qt::red));

        // Here we draw the widget mask
        if(mDesktopPalette->isVisible())
        {
            p.drawRect(mDesktopPalette->geometry().x(), mDesktopPalette->geometry().y(), mDesktopPalette->width(), mDesktopPalette->height());
        }
        if(UBApplication::boardController->paletteManager()->mKeyboardPalette->isVisible())
        {
            p.drawRect(UBApplication::boardController->paletteManager()->mKeyboardPalette->geometry().x(), UBApplication::boardController->paletteManager()->mKeyboardPalette->geometry().y(),
                       UBApplication::boardController->paletteManager()->mKeyboardPalette->width(), UBApplication::boardController->paletteManager()->mKeyboardPalette->height());
        }

        if(UBApplication::boardController->paletteManager()->leftPalette()->isVisible())
        {
            QRect leftPalette(UBApplication::boardController->paletteManager()->leftPalette()->geometry().x(),
                        UBApplication::boardController->paletteManager()->leftPalette()->geometry().y(),
                        UBApplication::boardController->paletteManager()->leftPalette()->width(),
                        UBApplication::boardController->paletteManager()->leftPalette()->height());

            QRect tabsPalette(UBApplication::boardController->paletteManager()->leftPalette()->getTabPaletteRect());

            p.drawRect(leftPalette);
            p.drawRect(tabsPalette);
        }

        if(UBApplication::boardController->paletteManager()->rightPalette()->isVisible())
        {
            QRect rightPalette(UBApplication::boardController->paletteManager()->rightPalette()->geometry().x(),
                        UBApplication::boardController->paletteManager()->rightPalette()->geometry().y(),
                        UBApplication::boardController->paletteManager()->rightPalette()->width(),
                        UBApplication::boardController->paletteManager()->rightPalette()->height());

            QRect tabsPalette(UBApplication::boardController->paletteManager()->rightPalette()->getTabPaletteRect());

            p.drawRect(rightPalette);
            p.drawRect(tabsPalette);
        }

        //Rquiered only for compiz wm
        //TODO. Window manager detection screen

        if (UBApplication::boardController->paletteManager()->addItemPalette()->isVisible()) {
            p.drawRect(UBApplication::boardController->paletteManager()->addItemPalette()->geometry());
        }

        // The panel of the AI, inside this layer, keeps its clicks.
        // So does the window of the GIFs.
        for (const QRect& side : { UBAiPanel::desktopArea(), UBGifWindow::desktopArea(), UBMeasureTool::desktopArea() })
        {
            if (!side.isNull())
            {
                p.drawRect(side);
            }
        }


        p.end();

        // Then we add the annotations. We create another painter because we need to
        // apply transformations on it for coordinates matching
        QPainter annotationPainter;

        QTransform trans;
        trans.translate(mTransparentDrawingView->width()/2, mTransparentDrawingView->height()/2);

        annotationPainter.begin(&mMask);
        annotationPainter.setPen(Qt::red);
        annotationPainter.setBrush(Qt::red);

        annotationPainter.setTransform(trans);

        QList<QGraphicsItem*> allItems = mTransparentDrawingScene->items();

        for(int i = 0; i < allItems.size(); i++)
        {
            QGraphicsItem* pCrntItem = allItems.at(i);

            if(pCrntItem->isVisible() && pCrntItem->type() == UBGraphicsPolygonItem::Type)
            {
                QPainterPath crntPath = pCrntItem->shape();
                QRectF rect = crntPath.boundingRect();

                annotationPainter.drawRect(rect);
            }
            else if (pCrntItem->isVisible() && !pCrntItem->parentItem() && dynamic_cast<UBGraphicsItem*>(pCrntItem))
            {
                // Text boxes, stamps and other items take the clicks, with
                // room around them for the buttons of their frame.
                annotationPainter.drawRect(pCrntItem->sceneBoundingRect().adjusted(-40, -40, 40, 40));
            }
        }

        annotationPainter.end();

        mTransparentDrawingView->setMask(mMask.mask());
    }
    else
    {
        mMask = QPixmap(mTransparentDrawingView->width(), mTransparentDrawingView->height());
        mMask.fill(Qt::transparent);

        QPainter p;

        p.begin(&mMask);

        p.setPen(Qt::red);
        p.setBrush(QBrush(Qt::red));

        p.drawRect(0, 0, mTransparentDrawingView->width(), mTransparentDrawingView->height());
        p.end();

        mTransparentDrawingView->setMask(mMask.mask());
    }
}

void UBDesktopAnnotationController::refreshMask()
{
    if (mTransparentDrawingScene && mTransparentDrawingView->isVisible()) {
        if(mIsFullyTransparent
                || UBDrawingController::drawingController()->stylusTool() == UBStylusTool::Selector
                //Needed to work correctly when another actions on stylus are checked
                || UBDrawingController::drawingController()->stylusTool() == UBStylusTool::Eraser
                || UBDrawingController::drawingController()->stylusTool() == UBStylusTool::Pointer
                || UBDrawingController::drawingController()->stylusTool() == UBStylusTool::Pen
                || UBDrawingController::drawingController()->stylusTool() == UBStylusTool::Marker)
        {
            updateMask(true);
        }
    }
}

void UBDesktopAnnotationController::onToolClicked()
{
    mToolOptionsPopup->hide();
}
