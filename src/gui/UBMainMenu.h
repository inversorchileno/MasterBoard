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

#pragma once

#include <QObject>

#include "core/UBApplicationController.h"

class QMenu;
class QToolBar;
class QAction;
class QLabel;
class QToolButton;
class UBDocumentTabs;
class UBGitHubUpdater;
class UBMainWindow;

/**
 * @brief The menu bar of the main window, like most desktop programs: Home
 * (new, open, save, settings, quit), Mode, Edit and About, and the tabs of
 * the open documents above the toolbar of the board.
 *
 * The toolbars keep only the actions used most often; every other action
 * of a mode is in the menus, with its keyboard shortcut.
 */
class UBMainMenu : public QObject
{
    Q_OBJECT

public:
    explicit UBMainMenu(UBMainWindow* mainWindow);

    /** @brief The application menu: page size, clipboard, help, preferences and quit. */
    static QMenu* createApplicationMenu(UBMainWindow* mainWindow, QWidget* parent);

    void onMainModeChanged(UBApplicationController::MainMode mode);

private:
    void createMenus();
    void setupDocumentShortcuts();

    /** @brief About, Creator: who made Master Board, and how. */
    void showCreator();
    void arrangeBoardToolBar();
    void createCornerButtons();
    void createDocumentTabs();
    void showColorsOnTools();
    void openFiles();
    void showSavedNotice();
    /** @brief Turns off the tools that would change a locked canvas, or back on. */
        void updateLockedTools(bool locked);
        void updateDocumentTabsVisibility();
    void setFullScreen(bool fullScreen);
    void setToolBarsHidden(bool hidden);
    QToolBar* currentToolBar() const;

    UBMainWindow* mMainWindow{nullptr};
    UBDocumentTabs* mDocumentTabs{nullptr};
    QToolBar* mDocumentTabsToolBar{nullptr};
    QLabel* mSavedNotice{nullptr};
    QAction* mFullScreenAction{nullptr};
    QAction* mExitFullScreenAction{nullptr};
    bool mWindowWasMaximized{true};
    UBGitHubUpdater* mUpdater{nullptr};
    QToolButton* mToggleToolBarButton{nullptr};
    UBApplicationController::MainMode mMode{UBApplicationController::Board};
    bool mToolBarsHidden{false};
};
