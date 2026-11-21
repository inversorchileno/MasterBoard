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

#include <QDialog>
#include <QKeySequence>
#include <QList>
#include <QPair>

class QAction;
class QLabel;
class QLineEdit;
class QMainWindow;
class QPushButton;
class QTreeWidget;
class QTreeWidgetItem;
class UBKeyCatcher;

/**
 * @brief The list of the keyboard shortcuts of Master Board, opened from
 * About, Shortcuts: every function of the menus and of the tool palettes,
 * with its shortcut, which can be changed. A shortcut that Windows or
 * Master Board already uses is refused with a message.
 */
class UBShortcutsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UBShortcutsDialog(QWidget* parent = nullptr);

    /**
     * @brief Gives a shortcut to the functions that have none and puts back
     * the shortcuts chosen by the user. Called once the menus and the
     * palettes are made.
     */
    static void setup(QMainWindow* mainWindow);

    /** @brief Same as setup, for actions made later, like those of the desktop. */
    static void adopt(const QList<QAction*>& actions);

    /** @brief Opens the list. */
    static void open(QWidget* parent);

private:
    using Group = QPair<QString, QList<QAction*>>;

    /** @brief The functions shown in the list, by group. */
    static QList<Group> groups();

    /** @brief Why a shortcut can not be given to an action, or empty. */
    static QString refusal(QAction* action, const QKeySequence& shortcut);

    /** @brief The action, other than the given one, that already has the shortcut. */
    static QAction* userOf(const QKeySequence& shortcut, QAction* except);

    static QString nameOf(const QAction* action);
    static QKeySequence defaultOf(const QAction* action);
    static void setShortcut(QAction* action, const QKeySequence& shortcut);
    static void restoreDefault(QAction* action);

    void fill();
    void filter(const QString& text);
    void showCurrent();
    void assign();
    void remove();
    void restoreCurrent();
    void restoreAll();
    QAction* currentAction() const;

private:
    QLineEdit* mSearch;
    QTreeWidget* mTree;
    QLabel* mFunction;
    UBKeyCatcher* mCatcher;
    QPushButton* mAssign;
    QPushButton* mRemove;
    QPushButton* mDefault;
};

/** @brief A field that shows the keys pressed in it, instead of writing them. */
class UBKeyCatcher : public QWidget
{
    Q_OBJECT

public:
    explicit UBKeyCatcher(QWidget* parent = nullptr);

    QKeySequence shortcut() const;
    void clear();

signals:
    void caught(const QKeySequence& shortcut);

protected:
    bool event(QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    QSize sizeHint() const override;

private:
    QKeySequence mShortcut;
    QString mPressed;
};
