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


#include "UBShortcutsDialog.h"

#include <QAction>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMap>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPainter>
#include <QPointer>
#include <QPushButton>
#include <QRegularExpression>
#include <QSet>
#include <QToolButton>
#include <QTreeWidget>
#include <QVBoxLayout>

#include "core/UBSettings.h"
#include "core/UBShortcutManager.h"
#include "gui/UBToolbarChoice.h"

namespace
{
    const char* const sDefaultProperty = "lbDefaultShortcuts";
    const char* const sAdoptedProperty = "lbShortcutAdopted";
    const QString sSettingsGroup = QStringLiteral("LibertyBoardShortcuts/");
    // Saved for a function that has its first shortcut again.
    const QString sDefaultValue = QStringLiteral("default");

    QPointer<QMainWindow>& mainWindowPointer()
    {
        static QPointer<QMainWindow> mainWindow;
        return mainWindow;
    }

    /*
     * The shortcuts given by Master Board to the functions that have none.
     * Ctrl + Shift and the F keys: Ctrl + Alt is Alt Gr on many keyboards,
     * and Alt + Shift changes the language of the keyboard on Windows.
     */
    const QMap<QString, QString>& inventedShortcuts()
    {
        static const QMap<QString, QString> shortcuts = {
            // Home
            { "actionLbExtract", "Ctrl+F2" },
            { "actionLbOcr", "Ctrl+Shift+Y" },
            { "actionMultiScreen", "Ctrl+F3" },
            { "actionPodcast", "Ctrl+F6" },
            // Mode
            { "actionLbSingleBoard", "Ctrl+1" },
            { "actionLbDualBoard", "Ctrl+2" },
            // Edit
            { "actionSleep", "Ctrl+F12" },
            { "actionWidePageSize", "Ctrl+F8" },
            { "actionRegularPageSize", "Ctrl+F9" },
            { "actionCustomPageSize", "Ctrl+F10" },
            { "actionLbDictate", "F8" },
            // About
            { "actionLbShortcuts", "Ctrl+F1" },
            { "actionLbCreator", "Ctrl+Shift+F2" },
            { "actionHintsAndTips", "F1" },
            { "actionLbGitHub", "Alt+F1" },
            { "actionCheckUpdate", "Ctrl+Shift+F1" },
            // Beside the menus
            { "actionLbOutline", "F7" },
            { "actionLbLock", "Ctrl+Shift+B" },
            // Tools
            { "actionPen", "Ctrl+Shift+P" },
            { "actionSpotlight", "Ctrl+Shift+F" },
            { "actionHighlight", "Ctrl+Shift+I" },
            { "actionSelector", "Ctrl+Shift+E" },
            { "actionHand", "Ctrl+Shift+M" },
            { "actionNote", "Ctrl+Shift+O" },
            { "actionStamp", "Ctrl+Shift+T" },
            { "actionEmoji", "Ctrl+Shift+G" },
            { "actionComment", "Ctrl+Shift+Q" },
            { "actionAttachment", "Ctrl+Shift+J" },
            { "actionSound", "Ctrl+Shift+K" },
            { "actionUnderline", "Ctrl+Shift+U" },
            { "actionVoice", "Ctrl+Shift+V" },
            { "actionArrowShape", "Ctrl+Shift+A" },
            { "actionRectangle", "Ctrl+Shift+R" },
            { "actionBoardScreenshot", "Ctrl+Shift+C" },
            { "actionAi", "F9" },
            { "actionMeasure", "Ctrl+Shift+D" },
            { "actionLbTranscribe", "Ctrl+F11" },
            { "actionDesktopScreenshot", "Ctrl+Shift+X" },
            { "actionDesktopClear", "Ctrl+Shift+Backspace" },
            // Pages and board
            { "actionNewPage", "Ctrl+Return" },
            { "actionDuplicatePage", "Ctrl+Shift+Return" },
            { "actionClearPage", "Ctrl+Shift+Del" },
            { "actionEraseItems", "Ctrl+Shift+F10" },
            { "actionEraseAnnotations", "Ctrl+Shift+F11" },
            { "actionEraseBackground", "Ctrl+Shift+F12" },
            { "actionBackgrounds", "Ctrl+Shift+W" },
        };

        return shortcuts;
    }

    bool isTypingKey(int key)
    {
        static const QSet<int> keys = {
            Qt::Key_Space, Qt::Key_Return, Qt::Key_Enter, Qt::Key_Tab, Qt::Key_Backtab, Qt::Key_Backspace,
            Qt::Key_Delete, Qt::Key_Escape, Qt::Key_Left, Qt::Key_Right, Qt::Key_Up, Qt::Key_Down,
            Qt::Key_Home, Qt::Key_End, Qt::Key_PageUp, Qt::Key_PageDown, Qt::Key_Insert
        };

        return key < Qt::Key_Escape || keys.contains(key);
    }

    bool isModifierKey(int key)
    {
        return key == Qt::Key_Shift || key == Qt::Key_Control || key == Qt::Key_Alt || key == Qt::Key_Meta
                || key == Qt::Key_AltGr || key == Qt::Key_Super_L || key == Qt::Key_Super_R
                || key == Qt::Key_unknown || key == 0;
    }
}

UBShortcutsDialog::UBShortcutsDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Shortcuts"));
    resize(640, 720);

    auto layout = new QVBoxLayout(this);

    auto help = new QLabel(tr("Choose a function, press the new keys in the box below and click Assign."), this);
    help->setWordWrap(true);
    layout->addWidget(help);

    mSearch = new QLineEdit(this);
    mSearch->setPlaceholderText(tr("Search a function..."));
    mSearch->setClearButtonEnabled(true);
    layout->addWidget(mSearch);

    mTree = new QTreeWidget(this);
    mTree->setColumnCount(2);
    mTree->setHeaderLabels({ tr("Function"), tr("Shortcut") });
    mTree->setRootIsDecorated(false);
    mTree->setUniformRowHeights(true);
    mTree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    mTree->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    mTree->header()->setStretchLastSection(false);
    layout->addWidget(mTree, 1);

    mFunction = new QLabel(this);
    QFont bold = mFunction->font();
    bold.setBold(true);
    mFunction->setFont(bold);
    layout->addWidget(mFunction);

    auto editRow = new QHBoxLayout;
    editRow->addWidget(new QLabel(tr("New shortcut:"), this));
    mCatcher = new UBKeyCatcher(this);
    editRow->addWidget(mCatcher, 1);
    mAssign = new QPushButton(tr("Assign"), this);
    mRemove = new QPushButton(tr("Remove"), this);
    mDefault = new QPushButton(tr("Default"), this);
    mDefault->setToolTip(tr("Gives back the shortcut that this function had at first"));
    editRow->addWidget(mAssign);
    editRow->addWidget(mRemove);
    editRow->addWidget(mDefault);
    layout->addLayout(editRow);

    auto buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    QPushButton* restoreAll = buttons->addButton(tr("Restore all"), QDialogButtonBox::ResetRole);
    restoreAll->setToolTip(tr("Gives back the first shortcuts to all the functions"));
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(restoreAll, &QPushButton::clicked, this, &UBShortcutsDialog::restoreAll);
    connect(mSearch, &QLineEdit::textChanged, this, &UBShortcutsDialog::filter);
    connect(mTree, &QTreeWidget::currentItemChanged, this, &UBShortcutsDialog::showCurrent);
    connect(mTree, &QTreeWidget::itemActivated, mCatcher, [this]() { mCatcher->setFocus(); });
    connect(mAssign, &QPushButton::clicked, this, &UBShortcutsDialog::assign);
    connect(mRemove, &QPushButton::clicked, this, &UBShortcutsDialog::remove);
    connect(mDefault, &QPushButton::clicked, this, &UBShortcutsDialog::restoreCurrent);
    connect(mCatcher, &UBKeyCatcher::caught, this, [this]() { mAssign->setEnabled(currentAction()); });

    fill();
    showCurrent();
}

void UBShortcutsDialog::setup(QMainWindow* mainWindow)
{
    mainWindowPointer() = mainWindow;

    for (const Group& group : groups())
    {
        adopt(group.second);
    }
}

void UBShortcutsDialog::adopt(const QList<QAction*>& actions)
{
    // Before the setup, the menus may still change the first shortcuts.
    if (!mainWindowPointer())
    {
        return;
    }

    for (QAction* action : actions)
    {
        if (!action || action->objectName().isEmpty() || action->property(sAdoptedProperty).toBool())
        {
            continue;
        }

        action->setProperty(sAdoptedProperty, true);

        // A function without shortcut gets one of Master Board, if it is free.
        const QKeySequence invented(inventedShortcuts().value(action->objectName()), QKeySequence::PortableText);

        if (action->shortcuts().isEmpty() && !invented.isEmpty() && !userOf(invented, action))
        {
            action->setShortcut(invented);
        }

        action->setProperty(sDefaultProperty, QKeySequence::listToString(action->shortcuts(), QKeySequence::PortableText));

        // The shortcut chosen by the user, maybe none.
        const QString key = sSettingsGroup + action->objectName();

        const QVariant saved = UBSettings::settings()->value(key);

        if (saved.isValid() && saved.toString() != sDefaultValue)
        {
            action->setShortcut(QKeySequence(saved.toString(), QKeySequence::PortableText));
        }

        // Also when the menu bar is hidden, in full screen.
        if (QMainWindow* mainWindow = mainWindowPointer())
        {
            mainWindow->addAction(action);
        }
    }
}

void UBShortcutsDialog::open(QWidget* parent)
{
    UBShortcutsDialog dialog(parent);
    dialog.exec();
}

QList<UBShortcutsDialog::Group> UBShortcutsDialog::groups()
{
    QList<Group> result;
    QSet<QAction*> listed;

    auto addGroup = [&result, &listed](const QString& name, const QList<QAction*>& actions) {
        QList<QAction*> shown;

        for (QAction* action : actions)
        {
            if (action && !action->isSeparator() && !action->menu() && !action->objectName().isEmpty()
                    && !action->property("builtIn").toBool() && !nameOf(action).isEmpty() && !listed.contains(action))
            {
                listed << action;
                shown << action;
            }
        }

        if (!shown.isEmpty())
        {
            result << Group(name, shown);
        }
    };

    QMainWindow* mainWindow = mainWindowPointer();

    if (mainWindow)
    {
        // The menus, each with its submenus; not the lists made when opened.
        const QList<QAction*> menuActions = mainWindow->menuBar()->actions();

        for (QAction* menuAction : menuActions)
        {
            QMenu* menu = menuAction->menu();

            if (!menu || menu->property("lbDynamicMenu").toBool())
            {
                continue;
            }

            QList<QAction*> actions;
            QList<QMenu*> menus = { menu };

            while (!menus.isEmpty())
            {
                const QList<QAction*> children = menus.takeFirst()->actions();

                for (QAction* action : children)
                {
                    if (action->menu())
                    {
                        menus << action->menu();
                    }
                    else
                    {
                        actions << action;
                    }
                }
            }

            addGroup(nameOf(menuAction), actions);
        }

        // The buttons beside the menus: search, index, lock...
        if (QWidget* corner = mainWindow->menuBar()->cornerWidget(Qt::TopRightCorner))
        {
            QList<QAction*> actions;
            const QList<QToolButton*> buttons = corner->findChildren<QToolButton*>();

            for (QToolButton* button : buttons)
            {
                actions << button->defaultAction();
            }

            addGroup(tr("Buttons beside the menus"), actions);
        }
    }

    addGroup(tr("Tools of the board / PDF"), UBToolbarChoice::choosableActions(UBToolbarChoice::Palette::Board));
    addGroup(tr("Tools of the desktop"), UBToolbarChoice::choosableActions(UBToolbarChoice::Palette::Desktop));

    // The other functions of the board: pages, erasing, backgrounds.
    if (mainWindow)
    {
        QList<QAction*> actions;

        for (const char* name : { "actionUndo", "actionRedo", "actionNewPage", "actionDuplicatePage", "actionClearPage",
                                  "actionEraseItems", "actionEraseAnnotations", "actionEraseBackground", "actionBackgrounds" })
        {
            actions << mainWindow->findChild<QAction*>(name);
        }

        addGroup(tr("Pages and board"), actions);
    }

    return result;
}

QString UBShortcutsDialog::nameOf(const QAction* action)
{
    QString name = action->text();

    if (name.isEmpty())
    {
        name = action->toolTip();
    }

    name.remove(QRegularExpression("&(?!&)"));

    if (name.endsWith(QLatin1String("...")))
    {
        name.chop(3);
    }

    return name.trimmed();
}

QKeySequence UBShortcutsDialog::defaultOf(const QAction* action)
{
    const QList<QKeySequence> shortcuts = QKeySequence::listFromString(action->property(sDefaultProperty).toString(),
                                                                        QKeySequence::PortableText);
    return shortcuts.value(0);
}

QAction* UBShortcutsDialog::userOf(const QKeySequence& shortcut, QAction* except)
{
    if (shortcut.isEmpty())
    {
        return nullptr;
    }

    QList<QAction*> actions;

    if (QMainWindow* mainWindow = mainWindowPointer())
    {
        actions = mainWindow->findChildren<QAction*>();
    }

    for (const auto& group : UBShortcutManager::shortcutManager()->actionGroups())
    {
        actions << group.second;
    }

    actions << UBToolbarChoice::choosableActions(UBToolbarChoice::Palette::Desktop);

    for (QAction* action : actions)
    {
        if (action != except && action->shortcuts().contains(shortcut))
        {
            return action;
        }
    }

    return nullptr;
}

QString UBShortcutsDialog::refusal(QAction* action, const QKeySequence& shortcut)
{
    if (shortcut.isEmpty())
    {
        return tr("no keys were pressed.");
    }

    const QKeyCombination combination = shortcut[0];
    const Qt::KeyboardModifiers modifiers = combination.keyboardModifiers() & ~Qt::KeypadModifier;
    const int key = combination.key();

    if (modifiers & Qt::MetaModifier)
    {
        return tr("the Windows key is kept by Windows for its own shortcuts.");
    }

    // Shortcuts of Windows itself, which never reach Master Board.
    const QList<QPair<QKeySequence, QString>> windowsShortcuts = {
        { QKeySequence(Qt::ALT | Qt::Key_Tab), tr("Windows uses it to change between the open windows.") },
        { QKeySequence(Qt::ALT | Qt::SHIFT | Qt::Key_Tab), tr("Windows uses it to change between the open windows.") },
        { QKeySequence(Qt::ALT | Qt::SHIFT | Qt::Key_Backtab), tr("Windows uses it to change between the open windows.") },
        { QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_Tab), tr("Windows uses it to change between the open windows.") },
        { QKeySequence(Qt::ALT | Qt::Key_Escape), tr("Windows uses it to change between the open windows.") },
        { QKeySequence(Qt::ALT | Qt::Key_F4), tr("Windows uses it to close the window.") },
        { QKeySequence(Qt::ALT | Qt::Key_Space), tr("Windows uses it to open the menu of the window.") },
        { QKeySequence(Qt::CTRL | Qt::Key_Escape), tr("Windows uses it to open the Start menu.") },
        { QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Escape), tr("Windows uses it to open the Task Manager.") },
        { QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_Delete), tr("Windows uses it to open its security screen.") },
        { QKeySequence(Qt::Key_Print), tr("Windows uses it to take screenshots.") },
        { QKeySequence(Qt::ALT | Qt::Key_Print), tr("Windows uses it to take screenshots.") },
        { QKeySequence(Qt::CTRL | Qt::Key_Print), tr("Windows uses it to take screenshots.") },
        { QKeySequence(Qt::SHIFT | Qt::Key_Print), tr("Windows uses it to take screenshots.") },
        { QKeySequence(Qt::SHIFT | Qt::Key_F10), tr("Windows uses it to open the menu of the right click.") },
        { QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_Up), tr("on many computers it turns the screen.") },
        { QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_Down), tr("on many computers it turns the screen.") },
        { QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_Left), tr("on many computers it turns the screen.") },
        { QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_Right), tr("on many computers it turns the screen.") },
    };

    for (const auto& windowsShortcut : windowsShortcuts)
    {
        // Quit keeps Alt + F4, which closes the window as in Windows.
        if (windowsShortcut.first == shortcut && !action->shortcuts().contains(shortcut))
        {
            return windowsShortcut.second;
        }
    }

    if ((modifiers & ~Qt::ShiftModifier) == Qt::NoModifier && isTypingKey(key))
    {
        return tr("this key is used to write, to move or to delete. Add Ctrl or Alt, or choose a key from F1 to F12.");
    }

    if ((modifiers & Qt::ControlModifier) && (modifiers & Qt::AltModifier) && key < Qt::Key_Escape && key != Qt::Key_Space)
    {
        return tr("Ctrl + Alt works as Alt Gr, which writes signs such as @, # or €.");
    }

    if (shortcut == QKeySequence(Qt::CTRL | Qt::Key_A))
    {
        return tr("Master Board uses it to select all the text of a text box.");
    }

    // Alt and a letter opens a menu.
    if (modifiers == Qt::AltModifier)
    {
        if (QMainWindow* mainWindow = mainWindowPointer())
        {
            const QList<QAction*> menuActions = mainWindow->menuBar()->actions();

            for (QAction* menuAction : menuActions)
            {
                if (QKeySequence::mnemonic(menuAction->text()) == shortcut)
                {
                    return tr("Master Board uses it to open the menu «%1».").arg(nameOf(menuAction));
                }
            }
        }
    }

    if (QAction* user = userOf(shortcut, action))
    {
        return tr("Master Board already uses it for «%1». Remove it there first.").arg(nameOf(user));
    }

    return QString();
}

void UBShortcutsDialog::setShortcut(QAction* action, const QKeySequence& shortcut)
{
    action->setShortcut(shortcut);
    UBSettings::settings()->setValue(sSettingsGroup + action->objectName(), shortcut.toString(QKeySequence::PortableText));
}

void UBShortcutsDialog::restoreDefault(QAction* action)
{
    UBSettings::settings()->setValue(sSettingsGroup + action->objectName(), sDefaultValue);
    action->setShortcuts(QKeySequence::listFromString(action->property(sDefaultProperty).toString(), QKeySequence::PortableText));
}

void UBShortcutsDialog::fill()
{
    QAction* current = currentAction();
    mTree->clear();

    for (const Group& group : groups())
    {
        auto groupItem = new QTreeWidgetItem(mTree, { group.first });
        groupItem->setFlags(Qt::ItemIsEnabled);
        groupItem->setFirstColumnSpanned(true);
        QFont bold = groupItem->font(0);
        bold.setBold(true);
        groupItem->setFont(0, bold);
        groupItem->setBackground(0, palette().alternateBase());

        for (QAction* action : group.second)
        {
            auto item = new QTreeWidgetItem(groupItem, { nameOf(action), action->shortcut().toString(QKeySequence::NativeText) });
            item->setIcon(0, action->icon());
            item->setToolTip(0, action->toolTip());
            item->setData(0, Qt::UserRole, QVariant::fromValue(static_cast<QObject*>(action)));

            if (action == current)
            {
                mTree->setCurrentItem(item);
            }
        }
    }

    // The keys of OpenBoard that can not be changed: pages, zoom, scroll.
    auto fixedItem = new QTreeWidgetItem(mTree, { tr("Fixed keys (can not be changed)") });
    fixedItem->setFlags(Qt::ItemIsEnabled);
    fixedItem->setFirstColumnSpanned(true);
    QFont bold = fixedItem->font(0);
    bold.setBold(true);
    fixedItem->setFont(0, bold);
    fixedItem->setBackground(0, palette().alternateBase());

    for (const auto& group : UBShortcutManager::shortcutManager()->actionGroups())
    {
        for (QAction* action : group.second)
        {
            if (action->property("builtIn").toBool())
            {
                QStringList keys;

                for (const QKeySequence& shortcut : action->shortcuts())
                {
                    keys << shortcut.toString(QKeySequence::NativeText);
                }

                auto item = new QTreeWidgetItem(fixedItem, { nameOf(action), keys.join(QLatin1String(", ")) });
                item->setFlags(Qt::ItemIsEnabled);
            }
        }
    }

    mTree->expandAll();
    filter(mSearch->text());
}

void UBShortcutsDialog::filter(const QString& text)
{
    for (int i = 0; i < mTree->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem* groupItem = mTree->topLevelItem(i);
        bool anyShown = false;

        for (int j = 0; j < groupItem->childCount(); ++j)
        {
            QTreeWidgetItem* item = groupItem->child(j);
            const bool shown = text.isEmpty() || item->text(0).contains(text, Qt::CaseInsensitive)
                    || item->text(1).contains(text, Qt::CaseInsensitive);
            item->setHidden(!shown);
            anyShown |= shown;
        }

        groupItem->setHidden(!anyShown);
    }
}

QAction* UBShortcutsDialog::currentAction() const
{
    QTreeWidgetItem* item = mTree->currentItem();
    return item ? qobject_cast<QAction*>(item->data(0, Qt::UserRole).value<QObject*>()) : nullptr;
}

void UBShortcutsDialog::showCurrent()
{
    QAction* action = currentAction();
    mCatcher->clear();
    mCatcher->setEnabled(action);
    mAssign->setEnabled(false);
    mRemove->setEnabled(action && !action->shortcut().isEmpty());
    mDefault->setEnabled(action && action->shortcut() != defaultOf(action));

    if (!action)
    {
        mFunction->setText(tr("Choose a function in the list."));
        return;
    }

    const QString shortcut = action->shortcut().toString(QKeySequence::NativeText);
    mFunction->setText(shortcut.isEmpty() ? tr("%1: no shortcut").arg(nameOf(action))
                                          : tr("%1: %2").arg(nameOf(action), shortcut));
}

void UBShortcutsDialog::assign()
{
    QAction* action = currentAction();
    const QKeySequence shortcut = mCatcher->shortcut();

    if (!action)
    {
        return;
    }

    const QString reason = refusal(action, shortcut);

    if (!reason.isEmpty())
    {
        QMessageBox::warning(this, tr("Shortcut not possible"),
                             tr("%1 can not be used: %2").arg(shortcut.toString(QKeySequence::NativeText), reason));
        mCatcher->clear();
        mCatcher->setFocus();
        mAssign->setEnabled(false);
        return;
    }

    setShortcut(action, shortcut);
    fill();
    showCurrent();
}

void UBShortcutsDialog::remove()
{
    if (QAction* action = currentAction())
    {
        setShortcut(action, QKeySequence());
        fill();
        showCurrent();
    }
}

void UBShortcutsDialog::restoreCurrent()
{
    QAction* action = currentAction();

    if (!action)
    {
        return;
    }

    // Its first shortcut may have been given to another function meanwhile.
    QAction* user = userOf(defaultOf(action), action);

    if (user)
    {
        QMessageBox::warning(this, tr("Shortcut not possible"),
                             tr("%1 can not be used: %2").arg(defaultOf(action).toString(QKeySequence::NativeText),
                                                            tr("Master Board already uses it for «%1». Remove it there first.")
                                                            .arg(nameOf(user))));
        return;
    }

    restoreDefault(action);
    fill();
    showCurrent();
}

void UBShortcutsDialog::restoreAll()
{
    const QMessageBox::StandardButton answer = QMessageBox::question(
        this, tr("Restore all"), tr("Give back the first shortcuts to all the functions?"));

    if (answer != QMessageBox::Yes)
    {
        return;
    }

    for (const Group& group : groups())
    {
        for (QAction* action : group.second)
        {
            restoreDefault(action);
        }
    }

    fill();
    showCurrent();
}

UBKeyCatcher::UBKeyCatcher(QWidget* parent)
    : QWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_InputMethodEnabled, false);
    setToolTip(tr("Click here and press the keys of the new shortcut"));
}

QKeySequence UBKeyCatcher::shortcut() const
{
    return mShortcut;
}

void UBKeyCatcher::clear()
{
    mShortcut = QKeySequence();
    mPressed.clear();
    update();
}

bool UBKeyCatcher::event(QEvent* event)
{
    // Escape alone closes the window, as everywhere.
    if (event->type() == QEvent::ShortcutOverride || event->type() == QEvent::KeyPress)
    {
        auto keyEvent = static_cast<QKeyEvent*>(event);

        if (keyEvent->key() == Qt::Key_Escape && keyEvent->modifiers() == Qt::NoModifier)
        {
            return QWidget::event(event);
        }
    }

    switch (event->type())
    {
    case QEvent::ShortcutOverride:
        // The keys are for this box, not for the shortcuts of the program.
        event->accept();
        return true;

    case QEvent::KeyPress:
    {
        auto keyEvent = static_cast<QKeyEvent*>(event);
        const int key = keyEvent->key();
        const Qt::KeyboardModifiers modifiers = keyEvent->modifiers() & ~Qt::KeypadModifier;

        if (isModifierKey(key))
        {
            // Ctrl, Alt or Shift alone: shown while the rest is pressed.
            mPressed = QKeySequence(modifiers.toInt()).toString(QKeySequence::NativeText);
        }
        else
        {
            mShortcut = QKeySequence(QKeyCombination(modifiers, static_cast<Qt::Key>(key)));
            mPressed.clear();
            emit caught(mShortcut);
        }

        update();
        return true;
    }

    case QEvent::KeyRelease:
        mPressed.clear();
        update();
        return true;

    case QEvent::FocusIn:
    case QEvent::FocusOut:
        update();
        break;

    default:
        break;
    }

    return QWidget::event(event);
}

void UBKeyCatcher::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    const QRect box = rect().adjusted(0, 0, -1, -1);
    painter.setPen(hasFocus() ? palette().highlight().color() : palette().mid().color());
    painter.setBrush(isEnabled() ? palette().base() : palette().window());
    painter.drawRoundedRect(box, 3, 3);

    QString text = mShortcut.toString(QKeySequence::NativeText);
    QColor color = palette().text().color();

    if (!mPressed.isEmpty())
    {
        text = mPressed;
    }
    else if (text.isEmpty())
    {
        text = hasFocus() ? tr("Press the keys...") : tr("Click here and press the keys");
        color = palette().placeholderText().color();
    }

    painter.setPen(color);
    painter.drawText(box.adjusted(8, 0, -8, 0), Qt::AlignVCenter | Qt::AlignLeft, text);
}

QSize UBKeyCatcher::sizeHint() const
{
    return QSize(220, fontMetrics().height() + 14);
}
