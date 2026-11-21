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


#include "UBToolbarChoice.h"

#include <QAction>
#include <QDialog>
#include <QDialogButtonBox>
#include <QListWidget>
#include <QPushButton>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QWidgetAction>
#include <QEvent>
#include <QMap>
#include <QMenu>
#include <QMouseEvent>
#include <QPointer>

#include "core/UBSettings.h"
#include "gui/UBActionPalette.h"
#include "gui/UBShortcutsDialog.h"

namespace
{
    struct Registered
    {
        QPointer<UBActionPalette> palette;
        QList<QAction*> choosable;
    };

    QMap<UBToolbarChoice::Palette, Registered>& palettes()
    {
        static QMap<UBToolbarChoice::Palette, Registered> registered;
        return registered;
    }

    // The tools that do two things are named by both.
    QString displayName(QAction* action)
    {
        const QMap<QString, QString> names = {
            { "actionSpotlight", UBToolbarChoice::tr("Spotlight / Magnifier") },
            { "actionAttachment", UBToolbarChoice::tr("Attach file / Hyperlink") },
            { "actionSelector", UBToolbarChoice::tr("Selector: arrow / lasso") },
        };

        return names.value(action->objectName(), action->text());
    }

    UBSetting* orderSetting(UBToolbarChoice::Palette palette)
    {
        return palette == UBToolbarChoice::Palette::Board ? UBSettings::settings()->boardToolOrder
                                                           : UBSettings::settings()->desktopToolOrder;
    }

    UBSetting* hiddenSetting(UBToolbarChoice::Palette palette)
    {
        return palette == UBToolbarChoice::Palette::Board ? UBSettings::settings()->hiddenBoardTools
                                                           : UBSettings::settings()->hiddenDesktopTools;
    }

    /*
     * A line of the Customize menu: the icon of the tool, its name, and at
     * the right the box that shows it in the palette. A click anywhere on
     * the line ticks the box; the menu stays open, to tick several.
     */
    class ToolRow : public QWidget
    {
    public:
        ToolRow(QAction* tool, bool shown, QWidget* parent)
            : QWidget(parent)
        {
            setObjectName("toolRow");
            setAttribute(Qt::WA_StyledBackground);
            setAttribute(Qt::WA_Hover);
            setStyleSheet("QWidget#toolRow { background: transparent; }"
                          "QWidget#toolRow:hover { background: #308cc6; }");

            auto icon = new QLabel(this);
            icon->setPixmap(tool->icon().pixmap(QSize(22, 22)));

            // Dark text on the light menu, white on the line under the mouse.
            mName = new QLabel(displayName(tool), this);
            mName->setStyleSheet("QLabel { color: #1e1e1e; }");

            mBox = new QCheckBox(this);
            QPalette colors = mBox->palette();
            colors.setColor(QPalette::Base, Qt::white);
            colors.setColor(QPalette::Text, QColor("#1e1e1e"));
            colors.setColor(QPalette::WindowText, QColor("#1e1e1e"));
            mBox->setPalette(colors);
            mBox->setChecked(shown);

            auto layout = new QHBoxLayout(this);
            layout->setContentsMargins(8, 3, 10, 3);
            layout->setSpacing(10);
            layout->addWidget(icon);
            layout->addWidget(mName);
            layout->addStretch(1);
            layout->addSpacing(16);
            layout->addWidget(mBox);
        }

        QCheckBox* box() const
        {
            return mBox;
        }

    protected:
        void enterEvent(QEnterEvent*) override
        {
            mName->setStyleSheet("QLabel { color: #ffffff; }");
        }

        void leaveEvent(QEvent*) override
        {
            mName->setStyleSheet("QLabel { color: #1e1e1e; }");
        }

        void mouseReleaseEvent(QMouseEvent* event) override
        {
            // On the name or the icon, as on the box.
            if (event->button() == Qt::LeftButton)
            {
                mBox->toggle();
            }
        }

    private:
        QLabel* mName;
        QCheckBox* mBox;
    };

    // A click on a box ticks it without closing the menu, to tick several.
    class KeepMenuOpen : public QObject
    {
    public:
        using QObject::QObject;

    protected:
        bool eventFilter(QObject* watched, QEvent* event) override
        {
            auto menu = qobject_cast<QMenu*>(watched);

            if (menu && event->type() == QEvent::MouseButtonRelease)
            {
                QAction* action = menu->actionAt(static_cast<QMouseEvent*>(event)->position().toPoint());

                if (action && action->isCheckable() && action->isEnabled())
                {
                    action->trigger();
                    return true;
                }
            }

            return QObject::eventFilter(watched, event);
        }
    };
}

void UBToolbarChoice::registerPalette(Palette palette, UBActionPalette* actionPalette, const QList<QAction*>& choosable)
{
    palettes()[palette] = Registered{ actionPalette, choosable };
    apply(palette);

    // Their shortcuts work also on the window of the palette, the desktop.
    actionPalette->window()->addActions(choosable);
    UBShortcutsDialog::adopt(choosable);
}

QList<QAction*> UBToolbarChoice::choosableActions(Palette palette)
{
    return palettes().value(palette).choosable;
}

bool UBToolbarChoice::isShown(Palette palette, QAction* action)
{
    return !hiddenSetting(palette)->get().toStringList().contains(action->objectName());
}

void UBToolbarChoice::setShown(Palette palette, QAction* action, bool shown)
{
    QStringList hidden = hiddenSetting(palette)->get().toStringList();
    hidden.removeAll(action->objectName());

    if (!shown)
    {
        hidden << action->objectName();
    }

    hiddenSetting(palette)->set(hidden);
    UBSettings::settings()->save();
    apply(palette);
}

void UBToolbarChoice::apply(Palette palette)
{
    const Registered registered = palettes().value(palette);

    if (!registered.palette)
    {
        return;
    }

    QList<QAction*> hidden;

    for (QAction* action : registered.choosable)
    {
        if (!isShown(palette, action))
        {
            hidden << action;
        }
    }

    registered.palette->setHiddenActions(hidden);
    registered.palette->setOrder(orderedActions(palette));
}

QList<QAction*> UBToolbarChoice::orderedActions(Palette palette)
{
    const QList<QAction*> choosable = palettes().value(palette).choosable;
    const QStringList order = orderSetting(palette)->get().toStringList();
    QList<QAction*> ordered;

    for (const QString& name : order)
    {
        for (QAction* action : choosable)
        {
            if (action->objectName() == name && !ordered.contains(action))
            {
                ordered << action;
            }
        }
    }

    // The buttons not yet in the order, as new ones, where they come.
    for (int i = 0; i < choosable.size(); ++i)
    {
        if (!ordered.contains(choosable.at(i)))
        {
            ordered.insert(qMin(i, ordered.size()), choosable.at(i));
        }
    }

    return ordered;
}

void UBToolbarChoice::setShownOrder(Palette palette, const QList<QAction*>& shownOrder)
{
    // The places of the buttons shown take the new order; the hidden
    // buttons stay where they were, for when they are shown again.
    QList<QAction*> all = orderedActions(palette);
    int next = 0;

    for (int i = 0; i < all.size() && next < shownOrder.size(); ++i)
    {
        if (shownOrder.contains(all.at(i)))
        {
            all[i] = shownOrder.at(next++);
        }
    }

    QStringList names;

    for (QAction* action : std::as_const(all))
    {
        names << action->objectName();
    }

    orderSetting(palette)->set(names);
    UBSettings::settings()->save();
    apply(palette);
}

void UBToolbarChoice::openOrderWindow(QWidget* parent)
{
    QDialog dialog(parent);
    dialog.setWindowTitle(tr("Move items"));
    dialog.resize(440, 640);

    auto layout = new QVBoxLayout(&dialog);
    auto help = new QLabel(tr("Drag the tools up or down, or use the buttons. Only the tools shown in the palette are listed."), &dialog);
    help->setWordWrap(true);
    layout->addWidget(help);

    auto tabs = new QTabWidget(&dialog);
    layout->addWidget(tabs, 1);

    QMap<Palette, QListWidget*> lists;

    for (const auto& section : { qMakePair(Palette::Board, tr("Board / PDF")), qMakePair(Palette::Desktop, tr("Desktop")) })
    {
        const Palette palette = section.first;
        auto page = new QWidget(tabs);
        auto pageLayout = new QVBoxLayout(page);

        // On the desktop, the button back to the board stays at the top.
        if (palette == Palette::Desktop)
        {
            auto fixed = new QLabel(tr("Back to the board: always first"), page);
            fixed->setStyleSheet("QLabel { color: #6b7383; font-style: italic; padding: 4px 6px; border: 1px dashed #b8bec8; }");
            pageLayout->addWidget(fixed);
        }

        auto list = new QListWidget(page);
        list->setDragDropMode(QAbstractItemView::InternalMove);
        list->setDefaultDropAction(Qt::MoveAction);
        list->setIconSize(QSize(28, 28));
        list->setSpacing(1);

        for (QAction* action : orderedActions(palette))
        {
            if (isShown(palette, action))
            {
                auto item = new QListWidgetItem(action->icon(), displayName(action), list);
                item->setData(Qt::UserRole, QVariant::fromValue(static_cast<QObject*>(action)));
            }
        }

        pageLayout->addWidget(list, 1);

        auto buttons = new QHBoxLayout();
        auto up = new QPushButton(tr("Up"), page);
        auto down = new QPushButton(tr("Down"), page);
        auto original = new QPushButton(tr("Original order"), page);
        buttons->addWidget(up);
        buttons->addWidget(down);
        buttons->addStretch(1);
        buttons->addWidget(original);
        pageLayout->addLayout(buttons);

        auto move = [list](int step) {
            const int row = list->currentRow();

            if (row < 0 || row + step < 0 || row + step >= list->count())
            {
                return;
            }

            QListWidgetItem* item = list->takeItem(row);
            list->insertItem(row + step, item);
            list->setCurrentRow(row + step);
        };

        QObject::connect(up, &QPushButton::clicked, list, [move]() { move(-1); });
        QObject::connect(down, &QPushButton::clicked, list, [move]() { move(1); });
        QObject::connect(original, &QPushButton::clicked, list, [list, palette]() {
            // The order of the palette as it comes, for the tools shown.
            QList<QListWidgetItem*> items;

            while (list->count())
            {
                items << list->takeItem(0);
            }

            for (QAction* action : palettes().value(palette).choosable)
            {
                for (QListWidgetItem* item : std::as_const(items))
                {
                    if (item->data(Qt::UserRole).value<QObject*>() == action)
                    {
                        list->addItem(item);
                    }
                }
            }
        });

        tabs->addTab(page, section.second);
        lists[palette] = list;
    }

    // The order when the window opens, given back by Cancel after Apply.
    QMap<Palette, QList<QAction*>> before;

    for (auto it = lists.cbegin(); it != lists.cend(); ++it)
    {
        for (QAction* action : orderedActions(it.key()))
        {
            if (isShown(it.key(), action))
            {
                before[it.key()] << action;
            }
        }
    }

    auto applyLists = [&lists]() {
        for (auto it = lists.cbegin(); it != lists.cend(); ++it)
        {
            QList<QAction*> order;

            for (int i = 0; i < it.value()->count(); ++i)
            {
                order << qobject_cast<QAction*>(it.value()->item(i)->data(Qt::UserRole).value<QObject*>());
            }

            setShownOrder(it.key(), order);
        }
    };

    bool applied = false;
    auto buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Apply | QDialogButtonBox::Cancel, &dialog);
    layout->addWidget(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    QObject::connect(buttons->button(QDialogButtonBox::Apply), &QPushButton::clicked, &dialog, [&applyLists, &applied]() {
        // The palettes take the order at once; the window stays open.
        applyLists();
        applied = true;
    });

    if (dialog.exec() == QDialog::Accepted)
    {
        applyLists();
    }
    else if (applied)
    {
        for (auto it = before.cbegin(); it != before.cend(); ++it)
        {
            setShownOrder(it.key(), it.value());
        }
    }
}

void UBToolbarChoice::fillToolsMenu(QMenu* menu)
{
    // The tools of the board, then those only on the desktop: one entry
    // each, whatever the palettes show.
    menu->clear();
    QList<QAction*> listed;

    for (Palette palette : { Palette::Board, Palette::Desktop })
    {
        for (QAction* action : palettes().value(palette).choosable)
        {
            if (!listed.contains(action) && action->actionGroup())
            {
                menu->addAction(action);
                listed << action;
            }
        }
    }
}

void UBToolbarChoice::fillCustomizeMenu(QMenu* menu)
{
    menu->clear();

    const QList<QPair<Palette, QString>> sections = {
        { Palette::Desktop, tr("Desktop") },
        { Palette::Board, tr("Board / PDF") },
    };

    for (const auto& section : sections)
    {
        const Palette palette = section.first;
        QMenu* sub = menu->addMenu(section.second);
        sub->installEventFilter(new KeepMenuOpen(sub));

        for (QAction* tool : orderedActions(palette))
        {
            // The box at the left shows whether the button is shown, its
            // icon at the right which button it is.
            auto row = new ToolRow(tool, isShown(palette, tool), sub);
            QObject::connect(row->box(), &QCheckBox::toggled, sub, [palette, tool](bool shown) { setShown(palette, tool, shown); });

            auto item = new QWidgetAction(sub);
            item->setDefaultWidget(row);
            sub->addAction(item);
        }
    }

    // The order of the buttons, in a window.
    menu->addSeparator();
    QAction* move = menu->addAction(tr("Move items..."));
    QObject::connect(move, &QAction::triggered, menu, [menu]() { openOrderWindow(menu->parentWidget() ? menu->parentWidget()->window() : nullptr); });
}
