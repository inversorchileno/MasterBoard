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

#include "UBMainMenu.h"
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QDialog>

#include <QActionGroup>
#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QPainter>
#include <QLayout>
#include <QToolBar>
#include <QStandardPaths>
#include <QTimer>
#include <QToolButton>

#include "board/UBDocumentPrinter.h"
#include "board/UBPageExtractor.h"
#include "board/UBBoardController.h"
#include "board/UBBoardSaver.h"
#include "board/UBDrawingController.h"
#include "board/UBDualBoard.h"
#include "gui/UBCanvasRulers.h"
#include "core/UBApplication.h"
#include "core/UBApplicationController.h"
#include "core/UBOpenedFiles.h"
#include "core/UBDocumentManager.h"
#include "core/UBGitHubUpdater.h"
#include "core/UBSettings.h"
#include "core/UBSetting.h"
#include "gui/UBDocumentTabs.h"
#include "gui/UBMainWindow.h"
#include "gui/UBNoteToolIcon.h"
#include "gui/UBShapeToolIcon.h"
#include "frameworks/UBPlatformUtils.h"
#include "gui/UBToolbarChoice.h"
#include "gui/UBReadAloud.h"
#include "gui/UBSpotlight.h"
#include "gui/UBToolOptionsPanel.h"
#include "gui/UBToolColorIcon.h"
#include "gui/UBToolOptionsArrows.h"
#include "gui/UBOcrRunner.h"
#include "gui/UBPageButtons.h"
#include "gui/UBPageIndicator.h"
#include "gui/UBPdfOutlinePanel.h"
#include "gui/UBSearchBar.h"
#include "gui/UBShortcutsDialog.h"
#include "gui/UBToolOptionsBar.h"
#include "gui/UBTheme.h"

UBMainMenu::UBMainMenu(UBMainWindow* mainWindow)
    : QObject(mainWindow)
    , mMainWindow(mainWindow)
{
    createDocumentTabs();
    createMenus();

    // Once the window is shown, the documents left open by a stop can come back.
    QTimer::singleShot(1500, mDocumentTabs, &UBDocumentTabs::offerRecovery);

    // A scanned PDF just opened: offer to read its text.
    connect(UBApplication::applicationController, &UBApplicationController::fileImported, this, [this](auto, const QString& file) {
        if (file.endsWith(QLatin1String(".pdf"), Qt::CaseInsensitive))
        {
            QTimer::singleShot(800, this, [this]() { UBOcrRunner::offerIfScanned(mMainWindow); });
        }
    });
    showColorsOnTools();
    setupDocumentShortcuts();
    arrangeBoardToolBar();
    createCornerButtons();

    // Dark like the menu bar and the tool palettes.
    const QString toolBarStyle = "QToolBar { background: #20242b; border: none; spacing: 2px; }"
                                 "QToolBar QToolButton { color: #e8e8e8; }"
                                 "QToolBar QToolButton:disabled { color: #6b7383; }";
    mMainWindow->boardToolBar->setStyleSheet(UBTheme::style(toolBarStyle));
    mMainWindow->documentToolBar->setStyleSheet(UBTheme::style(toolBarStyle));
    mMainWindow->webToolBar->setStyleSheet(UBTheme::style(toolBarStyle));

    // Once the palettes are made too: a shortcut for each function.
    QTimer::singleShot(0, this, [this]() { UBShortcutsDialog::setup(mMainWindow); });
}

QMenu* UBMainMenu::createApplicationMenu(UBMainWindow* mainWindow, QWidget* parent)
{
    QMenu* menu = new QMenu(parent);

    QActionGroup* pageSizeGroup = new QActionGroup(menu);
    pageSizeGroup->addAction(mainWindow->actionWidePageSize);
    pageSizeGroup->addAction(mainWindow->actionRegularPageSize);
    pageSizeGroup->addAction(mainWindow->actionCustomPageSize);

    QMenu* documentSizeMenu = menu->addMenu(QIcon(":/images/toolbar/pageSize.png"), tr("Page/canvas size"));
    documentSizeMenu->addAction(mainWindow->actionWidePageSize);
    documentSizeMenu->addAction(mainWindow->actionRegularPageSize);
    documentSizeMenu->addAction(mainWindow->actionCustomPageSize);
    menu->addAction(mainWindow->actionCut);
    menu->addAction(mainWindow->actionCopy);
    menu->addAction(mainWindow->actionPaste);
    menu->addAction(mainWindow->actionHideApplication);
    menu->addAction(mainWindow->actionSleep);

    menu->addSeparator();
    menu->addAction(mainWindow->actionHintsAndTips);
    menu->addSeparator();
    menu->addAction(mainWindow->actionPreferences);
    menu->addAction(mainWindow->actionMultiScreen);

    if (!UBSettings::settings()->appHideCheckForSoftwareUpdate->get().toBool())
    {
        menu->addAction(mainWindow->actionCheckUpdate);
    }

    menu->addSeparator();
    menu->addAction(mainWindow->actionPodcast);
    mainWindow->actionPodcast->setText(tr("Podcast"));

    menu->addSeparator();
    menu->addAction(mainWindow->actionQuit);

    return menu;
}

void UBMainMenu::createMenus()
{
    QMenuBar* menuBar = mMainWindow->menuBar();
    menuBar->setNativeMenuBar(false);

    mUpdater = new UBGitHubUpdater(mMainWindow);

    // Home: new, open, save, settings and quit.
    QMenu* homeMenu = menuBar->addMenu(tr("&Home"));

    auto newAction = new QAction(mMainWindow->actionNewDocument->icon(), tr("New board"), mMainWindow);
    newAction->setObjectName("actionLbNew");
    newAction->setShortcut(QKeySequence::New);
    mMainWindow->addAction(newAction);
    connect(newAction, &QAction::triggered, mDocumentTabs, &UBDocumentTabs::newDocument);

    auto openAction = new QAction(QIcon(":/images/toolbar/documents.png"), tr("Open/Import"), mMainWindow);
    openAction->setObjectName("actionLbOpen");
    openAction->setShortcut(QKeySequence::Open);
    mMainWindow->addAction(openAction);
    connect(openAction, &QAction::triggered, this, &UBMainMenu::openFiles);

    auto saveAction = new QAction(QIcon(":/images/toolbar/export.png"), tr("Save"), mMainWindow);
    saveAction->setObjectName("actionLbSave");
    saveAction->setShortcut(QKeySequence::Save);
    saveAction->setToolTip(tr("Save the annotations in the PDF, or the page as a picture"));
    mMainWindow->addAction(saveAction);
    connect(saveAction, &QAction::triggered, mDocumentTabs, &UBDocumentTabs::saveCurrent);

    auto saveAsAction = new QAction(QIcon(":/images/toolbar/saveAs.svg"), tr("Save as..."), mMainWindow);
    saveAsAction->setObjectName("actionLbSaveAs");
    saveAsAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_S));
    saveAsAction->setToolTip(tr("Save a copy as a PDF, PNG or JPEG file"));
    mMainWindow->addAction(saveAsAction);
    connect(saveAsAction, &QAction::triggered, mDocumentTabs, &UBDocumentTabs::saveCurrentAs);

    homeMenu->addAction(newAction);
    homeMenu->addAction(openAction);

    // The files opened last, to open again in one click.
    QMenu* recentMenu = homeMenu->addMenu(QIcon(":/images/toolbar/recent.svg"), tr("Recent files"));
    connect(recentMenu, &QMenu::aboutToShow, this, [this, recentMenu]() {
        recentMenu->clear();
        const QStringList files = UBOpenedFiles::recentFiles();

        for (const QString& file : files)
        {
            QAction* action = recentMenu->addAction(QFileInfo(file).fileName());
            action->setToolTip(QDir::toNativeSeparators(file));
            connect(action, &QAction::triggered, this, [this, file]() { mDocumentTabs->openFile(file); });
        }

        if (files.isEmpty())
        {
            recentMenu->addAction(tr("No recent files"))->setEnabled(false);
        }
    });
    homeMenu->addAction(saveAction);
    homeMenu->addAction(saveAsAction);

    auto printAction = new QAction(QIcon(":/images/toolbar/print.svg"), tr("Print..."), mMainWindow);
    printAction->setObjectName("actionLbPrint");
    printAction->setShortcut(QKeySequence::Print);
    // Ctrl+P prints, as in other programs, instead of choosing the pen.
    mMainWindow->actionPen->setShortcut(QKeySequence());
    mMainWindow->addAction(printAction);
    connect(printAction, &QAction::triggered, this, [this]() { UBDocumentPrinter::print(mMainWindow); });
    homeMenu->addAction(printAction);

    auto extractAction = new QAction(QIcon(":/images/toolbar/extractPages.svg"), tr("Extract pages..."), mMainWindow);
    extractAction->setObjectName("actionLbExtract");
    connect(extractAction, &QAction::triggered, this, [this]() { UBPageExtractor::extract(mMainWindow); });
    homeMenu->addAction(extractAction);

    auto ocrAction = new QAction(QIcon(":/images/toolbar/ocr.svg"), tr("Recognize text (OCR)..."), mMainWindow);
    ocrAction->setObjectName("actionLbOcr");
    ocrAction->setToolTip(tr("Reads the text of the scanned pages, so that it can be searched and copied"));
    connect(ocrAction, &QAction::triggered, this, [this]() { UBOcrRunner::run(mMainWindow); });
    homeMenu->addAction(ocrAction);
    homeMenu->addSeparator();
    homeMenu->addAction(mMainWindow->actionPreferences);
    homeMenu->addSeparator();
    // The podcast and the screens are in the Tools menu and at the top right.
    mMainWindow->actionPodcast->setText(tr("Podcast"));
    mMainWindow->actionMultiScreen->setText(tr("Multiscreen"));

    auto closeAction = new QAction(QIcon(":/images/toolbar/closeDocument.svg"), tr("Close document"), mMainWindow);
    closeAction->setObjectName("actionLbClose");
    closeAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_F4));
    mMainWindow->addAction(closeAction);
    connect(closeAction, &QAction::triggered, mDocumentTabs, &UBDocumentTabs::closeCurrent);
    homeMenu->addAction(closeAction);
    homeMenu->addAction(mMainWindow->actionQuit);

    QMenu* modeMenu = menuBar->addMenu(tr("&Mode"));
    // Two documents side by side on the board.
    auto dualBoard = new UBDualBoard(mDocumentTabs, this);

    // The board, single: also the way back from the double board, which
    // leaves both documents in their tabs.
    auto singleBoardAction = new QAction(mMainWindow->actionBoard->icon(), mMainWindow->actionBoard->text(), mMainWindow);
    singleBoardAction->setObjectName("actionLbSingleBoard");
    connect(singleBoardAction, &QAction::triggered, this, [dualBoard]() {
        UBApplication::applicationController->showBoard();
        dualBoard->setEnabled(false);
    });
    modeMenu->addAction(singleBoardAction);
    auto dualBoardAction = new QAction(QIcon(":/images/toolbar/dualBoard.svg"), tr("Double board"), mMainWindow);
    dualBoardAction->setObjectName("actionLbDualBoard");
    dualBoardAction->setCheckable(true);
    dualBoardAction->setToolTip(tr("Two documents side by side: click a side to work on it"));
    connect(dualBoardAction, &QAction::toggled, this, [dualBoard](bool enabled) {
        UBApplication::applicationController->showBoard();
        dualBoard->setEnabled(enabled);
    });
    modeMenu->addAction(dualBoardAction);
    connect(dualBoard, &UBDualBoard::enabledChanged, dualBoardAction, [dualBoardAction](bool enabled) {
        const QSignalBlocker blocker(dualBoardAction);
        dualBoardAction->setChecked(enabled);
    });
    modeMenu->addAction(mMainWindow->actionWeb);
    modeMenu->addAction(mMainWindow->actionDesktop);

    QMenu* editMenu = menuBar->addMenu(tr("&Edit"));
    QActionGroup* pageSizeGroup = new QActionGroup(editMenu);
    pageSizeGroup->addAction(mMainWindow->actionWidePageSize);
    pageSizeGroup->addAction(mMainWindow->actionRegularPageSize);
    pageSizeGroup->addAction(mMainWindow->actionCustomPageSize);

    QMenu* pageSizeMenu = editMenu->addMenu(QIcon(":/images/toolbar/pageSize.png"), tr("Page/canvas size"));
    pageSizeMenu->addAction(mMainWindow->actionWidePageSize);
    pageSizeMenu->addAction(mMainWindow->actionRegularPageSize);
    pageSizeMenu->addAction(mMainWindow->actionCustomPageSize);

    // Any size, always: also a bigger canvas to go on drawing.
    mMainWindow->actionCustomPageSize->setEnabled(true);
    mMainWindow->actionCustomPageSize->setText(tr("Custom size..."));
    // Rulers in pixels at the top and at the left of the page.
    auto rulersAction = new QAction(QIcon(":/images/stylusPalette/measure.svg"), tr("Canvas measurements"), mMainWindow);
    rulersAction->setObjectName("actionLbCanvasRulers");
    rulersAction->setCheckable(true);
    rulersAction->setToolTip(tr("Rulers in pixels at the top and at the left of the page, from 0 at its top left corner"));
    connect(rulersAction, &QAction::toggled, this, [](bool shown) { UBCanvasRulers::setShown(shown); });
    editMenu->addAction(rulersAction);
    editMenu->addSeparator();
    QTimer::singleShot(0, rulersAction, [rulersAction]() { rulersAction->setChecked(UBCanvasRulers::isShown()); });

    editMenu->addAction(mMainWindow->actionCut);
    editMenu->addAction(mMainWindow->actionCopy);
    editMenu->addAction(mMainWindow->actionPaste);
    // Reading aloud keeps its shortcut, without a menu: the tool Read aloud,
    // and Dictate and Transcribe in the options of Text, do the rest.
    UBReadAloud* readAloud = UBReadAloud::instance();
    auto readAction = new QAction(tr("Read aloud"), mMainWindow);
    readAction->setObjectName("actionLbRead");
    readAction->setShortcut(QKeySequence("Ctrl+Shift+L"));
    mMainWindow->addAction(readAction);
    connect(readAction, &QAction::triggered, readAloud, &UBReadAloud::readOrStop);

    editMenu->addAction(mMainWindow->actionHideApplication);
    editMenu->addAction(mMainWindow->actionSleep);
    editMenu->addSeparator();

    // The whole screen for the board: no menu and no taskbar of the system.
    mFullScreenAction = new QAction(QIcon(":/images/toolbar/fullScreen.svg"), tr("Full screen"), mMainWindow);
    mFullScreenAction->setObjectName("actionLbFullScreen");
    mFullScreenAction->setCheckable(true);
    mFullScreenAction->setShortcut(QKeySequence(Qt::Key_F11));
    mMainWindow->addAction(mFullScreenAction);
    connect(mFullScreenAction, &QAction::toggled, this, &UBMainMenu::setFullScreen);
    editMenu->addAction(mFullScreenAction);

    // Every tool, also those taken out of the palettes; and the choice of
    // the buttons of each palette.
    QMenu* toolsMenu = menuBar->addMenu(tr("&Tools"));
    toolsMenu->setProperty("lbDynamicMenu", true);
    connect(toolsMenu, &QMenu::aboutToShow, this, [toolsMenu]() { UBToolbarChoice::fillToolsMenu(toolsMenu); });
    QMenu* customizeMenu = menuBar->addMenu(tr("&Customize"));
    customizeMenu->setProperty("lbDynamicMenu", true);
    connect(customizeMenu, &QMenu::aboutToShow, this, [customizeMenu]() { UBToolbarChoice::fillCustomizeMenu(customizeMenu); });

    QMenu* aboutMenu = menuBar->addMenu(tr("&About"));

    // The guide of the tools, with pictures.
    aboutMenu->addAction(mMainWindow->actionHintsAndTips);

    // The list of the keyboard shortcuts, which can be changed.
    auto shortcutsAction = new QAction(QIcon(":/images/toolbar/keyboard.svg"), tr("Shortcuts"), mMainWindow);
    shortcutsAction->setObjectName("actionLbShortcuts");
    connect(shortcutsAction, &QAction::triggered, this, [this]() { UBShortcutsDialog::open(mMainWindow); });
    aboutMenu->addAction(shortcutsAction);

    aboutMenu->addSeparator();

    auto gitHubAction = new QAction(QIcon(":/images/toolbar/github.svg"), tr("Master Board on GitHub"), mMainWindow);
    gitHubAction->setObjectName("actionLbGitHub");
    connect(gitHubAction, &QAction::triggered, this, []() {
        QDesktopServices::openUrl(QUrl("https://github.com/inversorchileno/MasterBoard"));
    });
    aboutMenu->addAction(gitHubAction);

    // Replaces the update check of OpenBoard by the one of Master Board.
    mMainWindow->actionCheckUpdate->disconnect();
    mMainWindow->actionCheckUpdate->setText(tr("Update"));
    mMainWindow->actionCheckUpdate->setToolTip(tr("Download and install the latest version from GitHub"));
    mMainWindow->actionCheckUpdate->setVisible(true);
    connect(mMainWindow->actionCheckUpdate, &QAction::triggered, mUpdater, &UBGitHubUpdater::checkForUpdate);

    aboutMenu->addAction(mMainWindow->actionCheckUpdate);

    // Who made Master Board, and how: the last of the menu.
    aboutMenu->addSeparator();
    auto creatorAction = new QAction(QIcon(":/images/OpenBoard.png"), tr("About Master Board"), mMainWindow);
    creatorAction->setObjectName("actionLbCreator");
    connect(creatorAction, &QAction::triggered, this, &UBMainMenu::showCreator);
    aboutMenu->addAction(creatorAction);

    // The same dark gray as the tool palettes.
    menuBar->setStyleSheet(UBTheme::style(
        "QMenuBar { background: #323232; color: #f0f0f0; }"
        "QMenuBar::item { padding: 4px 10px 2px 10px; background: transparent; }"
        "QMenuBar::item:selected { background: #4a4a4a; }"));
}

void UBMainMenu::showCreator()
{
    QDialog dialog(mMainWindow);
    dialog.setWindowTitle(tr("About Master Board"));
    dialog.setMinimumWidth(560);

    auto logo = new QLabel(&dialog);
    logo->setPixmap(QIcon(":/images/OpenBoard.png").pixmap(96, 96));
    logo->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    const QStringList paragraphs = {
        tr("Master Board was created by Luis, known on YouTube as «Inversor Chileno»."),
        tr("It was born from a simple idea: to have in a single program what used to be spread over many different tools. "
           "Taking OpenBoard, a free digital whiteboard, as a base, I added the reading and annotation of PDFs, templates, "
           "shapes, stamps, emojis and GIFs, reading aloud and dictation, artificial intelligence and much more."),
        tr("It is for students, teachers, workers, content creators and, in general, for everyone who wants a multifunctional "
           "PDF and whiteboard tool."),
        tr("The curious thing is that I know nothing about writing code. I built Master Board with the help of Claude, an "
           "artificial intelligence: I brought the ideas and Claude turned them into a program. If your ideas are clear and you "
           "know how to write them in a good prompt, you too can create what you imagine."),
    };

    QString html = "<h2>Master Board</h2>";

    for (const QString& paragraph : paragraphs)
    {
        html += "<p>" + paragraph.toHtmlEscaped() + "</p>";
    }

    auto text = new QLabel(html, &dialog);
    text->setWordWrap(true);
    text->setTextFormat(Qt::RichText);
    text->setMinimumWidth(440);

    auto buttons = new QDialogButtonBox(QDialogButtonBox::Ok, &dialog);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);

    auto row = new QHBoxLayout();
    row->addWidget(logo);
    row->addSpacing(12);
    row->addWidget(text, 1);

    auto layout = new QVBoxLayout(&dialog);
    layout->addLayout(row);
    layout->addWidget(buttons);
    dialog.exec();
}

void UBMainMenu::setupDocumentShortcuts()
{
    const QList<QPair<QAction*, QKeySequence>> shortcuts = {
        { mMainWindow->actionNewFolder, QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_N) },
        { mMainWindow->actionImport, QKeySequence(Qt::CTRL | Qt::Key_I) },
        { mMainWindow->actionRename, QKeySequence(Qt::Key_F2) },
        { mMainWindow->actionDuplicate, QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_D) },
    };

    for (const auto& shortcut : shortcuts)
    {
        // Only in the Documents mode, where these actions apply.
        shortcut.first->setShortcut(shortcut.second);
        shortcut.first->setShortcutContext(Qt::WidgetWithChildrenShortcut);

        if (QWidget* documents = mMainWindow->documentsWidget())
        {
            documents->addAction(shortcut.first);
        }
    }
}

void UBMainMenu::arrangeBoardToolBar()
{
    // Undo and redo, then the options of the selected tool, then the pages,
    // backgrounds and erase on the right.
    QToolBar* toolBar = mMainWindow->boardToolBar;
    toolBar->insertWidget(mMainWindow->actionPages, new UBToolOptionsBar(toolBar));
    // The same line as after undo and redo, between the options and the pages.
    toolBar->insertSeparator(mMainWindow->actionPages);

    // Pages and Delete, one above the other, where the Pages button was.
    toolBar->insertWidget(mMainWindow->actionPages, new UBPageButtons(toolBar));
    toolBar->removeAction(mMainWindow->actionPages);
    toolBar->insertWidget(mMainWindow->actionForward, new UBPageIndicator(toolBar));
}

void UBMainMenu::createCornerButtons()
{
    auto corner = new QWidget(mMainWindow->menuBar());
    auto layout = new QHBoxLayout(corner);
    layout->setContentsMargins(0, 0, 4, 0);
    layout->setSpacing(4);

    const QString buttonStyle = UBTheme::style("QToolButton { color: #f0f0f0; background: transparent; border: none; padding: 2px 8px; }"
                                               "QToolButton:hover { background: #4a4a4a; }");

    // "Saved", for a few seconds after each save.
    // Its place is kept, so that the buttons beside it do not move.
    mSavedNotice = new QLabel(corner);
    mSavedNotice->setStyleSheet(UBTheme::style("QLabel { color: #8fe3a0; font-weight: bold; padding: 0 10px; }"));
    mSavedNotice->setAlignment(Qt::AlignCenter);
    QFont bold = mSavedNotice->font();
    bold.setBold(true);
    mSavedNotice->setMinimumWidth(QFontMetrics(bold).horizontalAdvance(tr("Saved")) + 24);
    layout->addWidget(mSavedNotice);
    connect(mDocumentTabs, &UBDocumentTabs::saved, this, &UBMainMenu::showSavedNotice);

    // Finds a text in the PDF shown on the board.
    auto searchBar = new UBSearchBar(mMainWindow);
    auto searchAction = new QAction(UBTheme::icon(":/images/toolbar/search.svg"), tr("Search..."), mMainWindow);
    searchAction->setObjectName("actionLbSearch");
    searchAction->setShortcut(QKeySequence::Find);
    // Ctrl+F is for the search, as in other programs, not for the selector.
    mMainWindow->actionSelector->setShortcut(QKeySequence());
    searchAction->setToolTip(tr("Find words in the PDF"));
    mMainWindow->addAction(searchAction);
    // A second press closes the search.
    connect(searchAction, &QAction::triggered, searchBar, [searchBar]() {
        if (searchBar->isVisible())
        {
            searchBar->hide();
        }
        else
        {
            searchBar->open();
        }
    });

    auto searchButton = new QToolButton(corner);
    searchButton->setDefaultAction(searchAction);
    searchButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    searchButton->setIconSize(QSize(18, 18));
    searchButton->setStyleSheet(buttonStyle);
    layout->addWidget(searchButton);

    // The index of the PDF, beside the search; one panel at a time.
    auto outlinePanel = new UBPdfOutlinePanel(mMainWindow);
    auto outlineAction = new QAction(QIcon(":/images/toolbar/outline.svg"), tr("Index"), mMainWindow);
    outlineAction->setObjectName("actionLbOutline");
    outlineAction->setToolTip(tr("The index of the PDF: a click on a title goes to its page"));
    connect(outlineAction, &QAction::triggered, outlinePanel, [outlinePanel, searchBar]() {
        // A second press closes the index.
        if (outlinePanel->isVisible())
        {
            outlinePanel->hide();
            return;
        }

        searchBar->hide();
        outlinePanel->open();
    });
    connect(searchAction, &QAction::triggered, outlinePanel, &QWidget::hide);

    auto outlineButton = new QToolButton(corner);
    outlineButton->setDefaultAction(outlineAction);
    outlineButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    outlineButton->setIconSize(QSize(18, 18));
    outlineButton->setStyleSheet(buttonStyle);
    layout->insertWidget(layout->indexOf(searchButton), outlineButton);

    // The spotlight and the magnifier follow the pointer while their tool is chosen and shown.
    auto spotlight = new UBSpotlight();
    auto updateSpotlight = [spotlight]() {
        UBDrawingController* drawingController = UBDrawingController::drawingController();
        spotlight->setActive(drawingController->stylusTool() == UBStylusTool::Spotlight
                             && drawingController->isSpotlightShown());
    };
    connect(UBDrawingController::drawingController(), &UBDrawingController::stylusToolChanged, spotlight, updateSpotlight);
    connect(UBDrawingController::drawingController(), &UBDrawingController::spotlightShownChanged, spotlight, updateSpotlight);

    // Locks the canvas against changes, left of the index.
    auto lockAction = new QAction(QIcon(":/images/toolbar/lockOpen.svg"), tr("Lock canvas"), mMainWindow);
    lockAction->setObjectName("actionLbLock");
    lockAction->setCheckable(true);
    lockAction->setToolTip(tr("Lock the canvas: nothing can be drawn, written, highlighted, selected, moved or deleted; "
                              "the laser pointer and the hand still work"));
    connect(lockAction, &QAction::toggled, UBDrawingController::drawingController(), &UBDrawingController::setCanvasLocked);

    // On a page of a PDF, the lock says that it locks the PDF.
    auto updateLockText = [lockAction]() {
        const bool locked = UBDrawingController::drawingController()->isCanvasLocked();

        if (UBBoardSaver::isFromPdf())
        {
            lockAction->setText(locked ? tr("Unlock PDF") : tr("Lock PDF"));
            lockAction->setToolTip(tr("Lock the PDF: nothing can be drawn, written, highlighted, selected, moved or deleted; "
                                      "the laser pointer and the hand still work"));
        }
        else
        {
            lockAction->setText(locked ? tr("Unlock canvas") : tr("Lock canvas"));
            lockAction->setToolTip(tr("Lock the canvas: nothing can be drawn, written, highlighted, selected, moved or deleted; "
                                      "the laser pointer and the hand still work"));
        }
    };

    connect(UBDrawingController::drawingController(), &UBDrawingController::canvasLockedChanged, this,
            [this, lockAction, updateLockText](bool locked) {
                lockAction->setChecked(locked);
                lockAction->setIcon(QIcon(locked ? ":/images/toolbar/lockClosed.svg" : ":/images/toolbar/lockOpen.svg"));
                updateLockText();
                updateLockedTools(locked);
            });
    connect(UBApplication::boardController, &UBBoardController::activeSceneChanged, this, updateLockText);

    // The lock is for the board: the desktop keeps all its tools. The
    // attachment tool puts files on the board, hyperlinks on the desktop.
    connect(UBApplication::applicationController, &UBApplicationController::desktopMode, this, [this](bool desktop) {
        updateLockedTools(UBDrawingController::drawingController()->isCanvasLocked() && !desktop);
        UBAttachmentOptionsPanel::updateToolIcon(desktop);
    });
    UBAttachmentOptionsPanel::updateToolIcon(false);
    UBSpotlightOptionsPanel::updateToolIcon();

    auto lockButton = new QToolButton(corner);
    lockButton->setDefaultAction(lockAction);
    lockButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    lockButton->setIconSize(QSize(18, 18));
    lockButton->setStyleSheet(buttonStyle);
    layout->insertWidget(layout->indexOf(outlineButton), lockButton);

    // Room for the longest of its names.
    int widest = 0;

    for (const QString& name : { tr("Unlock canvas"), tr("Lock canvas"), tr("Unlock PDF"), tr("Lock PDF") })
    {
        lockAction->setText(name);
        widest = qMax(widest, lockButton->sizeHint().width());
    }

    lockButton->setMinimumWidth(widest);
    updateLockText();

    // The podcast and the screens, left of the lock: not in the palettes.
    for (QAction* action : { mMainWindow->actionMultiScreen, mMainWindow->actionPodcast })
    {
        auto button = new QToolButton(corner);
        button->setDefaultAction(action);
        button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        button->setIconSize(QSize(18, 18));
        button->setStyleSheet(buttonStyle);
        layout->insertWidget(layout->indexOf(lockButton), button);
    }

    // Goes to the desktop mode, like the board button of the desktop palette goes back.
    auto desktopButton = new QToolButton(corner);
    desktopButton->setDefaultAction(mMainWindow->actionDesktop);
    desktopButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    desktopButton->setIconSize(QSize(18, 18));
    desktopButton->setStyleSheet(buttonStyle);
    layout->addWidget(desktopButton);

    mToggleToolBarButton = new QToolButton(corner);
    mToggleToolBarButton->setStyleSheet(buttonStyle);
    mToggleToolBarButton->setToolTip(tr("Hide or show the toolbar"));
    layout->addWidget(mToggleToolBarButton);

    connect(mToggleToolBarButton, &QToolButton::clicked, this, [this]() {
        setToolBarsHidden(!mToolBarsHidden);
    });

    setToolBarsHidden(false);
    mMainWindow->menuBar()->setCornerWidget(corner, Qt::TopRightCorner);
}

void UBMainMenu::showColorsOnTools()
{
    UBSettings* settings = UBSettings::settings();

    // The note tool shows the ribbon of the next note, in its color.
    auto updateNote = [this, settings]() {
        const QColor color(settings->noteColor->get().toString());
        UBToolOptionsArrows::setIcon(mMainWindow->actionNote, UBNoteToolIcon::create(color));
    };

    // The pen, marker, laser and highlight show the color they draw with.
    auto iconOf = [](const QString& off, const QString& on) {
        QIcon icon;
        icon.addFile(off, QSize(), QIcon::Normal, QIcon::Off);
        icon.addFile(on, QSize(), QIcon::Normal, QIcon::On);
        return icon;
    };

    const QIcon pen = iconOf(":/images/stylusPalette/pen.svg", ":/images/stylusPalette/penOn.svg");
    const QIcon marker = iconOf(":/images/stylusPalette/marker.svg", ":/images/stylusPalette/markerOn.svg");
    const QIcon laser = iconOf(":/images/stylusPalette/laser.png", ":/images/stylusPalette/laserOn.png");
    const QIcon highlight = iconOf(":/images/stylusPalette/highlight.svg", ":/images/stylusPalette/highlightOn.svg");

    auto opaque = [](QColor color) {
        color.setAlpha(255);
        return color;
    };

    auto updatePenAndMarker = [this, settings, pen, marker, opaque]() {
        UBToolOptionsArrows::setIcon(mMainWindow->actionPen, UBToolColorIcon::create(pen, opaque(settings->currentPenColor())));
        UBToolOptionsArrows::setIcon(mMainWindow->actionMarker, UBToolColorIcon::create(marker, opaque(settings->currentMarkerColor())));
    };

    auto updateLaser = [this, settings, laser]() {
        UBToolOptionsArrows::setIcon(mMainWindow->actionPointer, UBToolColorIcon::create(laser, QColor(settings->laserColor->get().toString())));
    };

    auto updateHighlight = [this, settings, highlight]() {
        const QColor color(settings->highlightSettings.strokeColor->get().toString());
        UBToolOptionsArrows::setIcon(mMainWindow->actionHighlight, UBToolColorIcon::create(highlight, color));
    };

    updateNote();
    updatePenAndMarker();
    updateLaser();
    updateHighlight();

    connect(settings->noteColor, &UBSetting::changed, this, updateNote);
    connect(settings->laserColor, &UBSetting::changed, this, updateLaser);
    connect(settings->highlightSettings.strokeColor, &UBSetting::changed, this, updateHighlight);

    // The arrow, rectangle and ellipse show the border and the filling of the next shape.
    const QList<QPair<QAction*, UBStylusTool::Enum>> shapes = {
        { mMainWindow->actionArrowShape, UBStylusTool::Arrow },
        { mMainWindow->actionRectangle, UBStylusTool::Rectangle },
        { mMainWindow->actionEllipse, UBStylusTool::Ellipse },
    };

    for (const auto& shape : shapes)
    {
        UBSettings::ShapeSettings* shapeSettings = settings->shapeSettings(shape.second);
        QAction* action = shape.first;
        const int tool = shape.second;

        auto updateShape = [action, tool, shapeSettings, settings]() {
            const QColor stroke(shapeSettings->strokeColor->get().toString());
            const QString fillName = shapeSettings->fillColor ? shapeSettings->fillColor->get().toString() : QString();
            UBToolOptionsArrows::setIcon(action, UBShapeToolIcon::create(tool, stroke, fillName.isEmpty() ? QColor() : QColor(fillName),
                                                                         settings->shapeKind->get().toInt()));
        };

        updateShape();
        connect(shapeSettings->strokeColor, &UBSetting::changed, this, updateShape);

        // The shapes tool shows the shape chosen.
        if (tool == UBStylusTool::Rectangle)
        {
            connect(settings->shapeKind, &UBSetting::changed, this, updateShape);
        }

        if (shapeSettings->fillColor)
        {
            connect(shapeSettings->fillColor, &UBSetting::changed, this, updateShape);
        }
    }

    UBDrawingController* drawing = UBDrawingController::drawingController();
    connect(drawing, &UBDrawingController::colorPaletteChanged, this, updatePenAndMarker);

    // The selector shows a lasso while it selects with one.
    const QIcon arrowIcon = mMainWindow->actionSelector->icon();
    QIcon lassoIcon;
    {
        const QSize size(84, 84);
        QPixmap off(size);
        off.fill(Qt::transparent);
        QPixmap on(size);
        on.fill(Qt::transparent);

        {
            QPainter painter(&on);
            painter.setRenderHint(QPainter::Antialiasing);
            // The light disc of the chosen tool, as on the arrow.
            painter.setPen(QPen(QColor(150, 150, 150, 200), 2));
            painter.setBrush(QColor(255, 255, 255, 215));
            painter.drawEllipse(QRectF(3, 3, 78, 78));
        }

        for (QPixmap* pixmap : { &off, &on })
        {
            QPainter painter(pixmap);
            painter.setRenderHint(QPainter::Antialiasing);
            QIcon(":/images/toolbar/lasso.svg").paint(&painter, QRect(14, 14, 56, 56));
        }

        lassoIcon.addPixmap(off, QIcon::Normal, QIcon::Off);
        lassoIcon.addPixmap(on, QIcon::Normal, QIcon::On);
    }

    auto updateSelector = [this, arrowIcon, lassoIcon](bool lasso) {
        UBToolOptionsArrows::setIcon(mMainWindow->actionSelector, lasso ? lassoIcon : arrowIcon);
    };

    updateSelector(drawing->isLassoSelection());
    connect(drawing, &UBDrawingController::lassoSelectionChanged, this, updateSelector);
    connect(drawing, &UBDrawingController::colorIndexChanged, this, updatePenAndMarker);
}

void UBMainMenu::createDocumentTabs()
{
    // A row of its own under the toolbar of the board.
    mDocumentTabs = new UBDocumentTabs(mMainWindow);
    mDocumentTabsToolBar = new QToolBar(mMainWindow);
    mDocumentTabsToolBar->setObjectName("documentTabsToolBar");
    mDocumentTabsToolBar->setMovable(false);
    mDocumentTabsToolBar->setFloatable(false);
    mDocumentTabsToolBar->toggleViewAction()->setVisible(false);
    mDocumentTabsToolBar->setContextMenuPolicy(Qt::PreventContextMenu);
    // A thin white line under the tabs parts them from the options of the tools.
    mDocumentTabsToolBar->setStyleSheet(UBTheme::style("QToolBar { background: #20242b; border: none; border-bottom: 1px solid #e8e8e8;"
                                                       " padding: 0px; margin: 0px; spacing: 0px; }"));
    mDocumentTabsToolBar->addWidget(mDocumentTabs);

    // In full screen the menu is hidden: the way back is at the right of the tabs.
    auto spacer = new QWidget(mDocumentTabsToolBar);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    mDocumentTabsToolBar->addWidget(spacer);

    auto exitFullScreen = new QToolButton(mDocumentTabsToolBar);
    exitFullScreen->setText(tr("Exit full screen"));
    exitFullScreen->setToolTip(tr("Back to the window, with the menu (F11)"));
    exitFullScreen->setStyleSheet(UBTheme::style("QToolButton { color: #f0f0f0; background: #3a404b; border: none; border-radius: 3px; padding: 2px 10px; margin: 1px 4px; }"
                                                 "QToolButton:hover { background: #4a5260; }"));
    exitFullScreen->setFixedHeight(22);
    exitFullScreen->setToolButtonStyle(Qt::ToolButtonTextOnly);
    mExitFullScreenAction = mDocumentTabsToolBar->addWidget(exitFullScreen);
    mExitFullScreenAction->setVisible(false);
    connect(exitFullScreen, &QToolButton::clicked, this, [this]() { mFullScreenAction->setChecked(false); });

    // As thin as the tabs: no room for icons or a handle around them.
    mDocumentTabsToolBar->setIconSize(QSize(16, 16));
    mDocumentTabsToolBar->layout()->setContentsMargins(0, 0, 0, 0);
    mDocumentTabsToolBar->layout()->setSpacing(0);
    mDocumentTabsToolBar->setFixedHeight(26);

    mMainWindow->insertToolBar(mMainWindow->boardToolBar, mDocumentTabsToolBar);
    mMainWindow->insertToolBarBreak(mMainWindow->boardToolBar);

    connect(mDocumentTabs, &UBDocumentTabs::countChanged, this, &UBMainMenu::updateDocumentTabsVisibility);
    updateDocumentTabsVisibility();
}

void UBMainMenu::updateLockedTools(bool locked)
{
    // On a locked canvas, the tools that would change the page are off.
    const QList<QAction*> changing = {
        mMainWindow->actionPen, mMainWindow->actionEraser, mMainWindow->actionMarker, mMainWindow->actionHighlight,
        mMainWindow->actionNote, mMainWindow->actionStamp, mMainWindow->actionEmoji, mMainWindow->actionComment, mMainWindow->actionAttachment,
        mMainWindow->actionSound, mMainWindow->actionUnderline, mMainWindow->actionText, mMainWindow->actionLine,
        mMainWindow->actionArrowShape, mMainWindow->actionRectangle, mMainWindow->actionEllipse, mMainWindow->actionErase,
        mMainWindow->actionCut, mMainWindow->actionPaste,
    };

    for (QAction* action : changing)
    {
        action->setEnabled(!locked);
    }
}

void UBMainMenu::updateDocumentTabsVisibility()
{
    // Only useful to switch between documents, hidden with a single one,
    // except in full screen, where it holds the way back.
    const bool fullScreen = mFullScreenAction && mFullScreenAction->isChecked();
    mDocumentTabsToolBar->setVisible(mMode == UBApplicationController::Board && (mDocumentTabs->count() > 1 || fullScreen));
}

void UBMainMenu::setFullScreen(bool fullScreen)
{
    if (fullScreen)
    {
        mWindowWasMaximized = mMainWindow->isMaximized();
        mMainWindow->menuBar()->hide();
        mMainWindow->showFullScreen();
    }
    else
    {
        mMainWindow->menuBar()->show();

        if (mWindowWasMaximized)
        {
            mMainWindow->showMaximized();
        }
        else
        {
            mMainWindow->showNormal();
        }
    }

    mExitFullScreenAction->setVisible(fullScreen);
    updateDocumentTabsVisibility();
}

void UBMainMenu::openFiles()
{
    static const QString lastFolderKey = QStringLiteral("App/LibertyBoardLastOpenFolder");
    const QString folder = UBSettings::settings()->value(lastFolderKey,
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();

    const QStringList files = QFileDialog::getOpenFileNames(mMainWindow, tr("Open"), folder,
        UBDocumentManager::documentManager()->importFileFilter(true));

    if (files.isEmpty())
    {
        return;
    }

    UBSettings::settings()->setValue(lastFolderKey, QFileInfo(files.first()).absolutePath());

    for (const QString& file : files)
    {
        mDocumentTabs->openFile(file);
    }
}

void UBMainMenu::showSavedNotice()
{
    mSavedNotice->setText(tr("Saved"));
    QTimer::singleShot(3000, mSavedNotice, &QLabel::clear);
}

QToolBar* UBMainMenu::currentToolBar() const
{
    switch (mMode)
    {
        case UBApplicationController::Document:
            return mMainWindow->documentToolBar;
        case UBApplicationController::Internet:
            return mMainWindow->webToolBar;
        default:
            return mMainWindow->boardToolBar;
    }
}

void UBMainMenu::setToolBarsHidden(bool hidden)
{
    mToolBarsHidden = hidden;
    mToggleToolBarButton->setText(hidden ? QStringLiteral("\u25BC") : QStringLiteral("\u25B2"));

    if (mMode == UBApplicationController::Board
            || mMode == UBApplicationController::Document
            || mMode == UBApplicationController::Internet)
    {
        currentToolBar()->setVisible(!hidden);
    }
}

void UBMainMenu::onMainModeChanged(UBApplicationController::MainMode mode)
{
    mMode = mode;
    updateDocumentTabsVisibility();

    // The application controller shows the toolbar of each mode: keep it
    // hidden if the user hid it.
    if (mToolBarsHidden)
    {
        QMetaObject::invokeMethod(this, [this]() { setToolBarsHidden(true); }, Qt::QueuedConnection);
    }
}
