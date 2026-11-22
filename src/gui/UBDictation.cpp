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


#include "UBDictation.h"

#include <QGraphicsTextItem>
#include <QGraphicsView>
#include <QPointer>
#include <QTextCursor>
#include <QTimer>

#include "board/UBBoardController.h"
#include "board/UBDrawingController.h"
#include "board/UBBoardView.h"
#include "core/UBApplication.h"
#include "domain/UBGraphicsScene.h"
#include "domain/UBGraphicsTextItem.h"
#include "frameworks/UBPlatformUtils.h"
#include "gui/UBMainWindow.h"
#include "gui/UBTextBoxes.h"

bool UBDictation::isAvailable()
{
#ifdef Q_OS_WIN
    return true;
#else
    return false;
#endif
}

QString UBDictation::onlyOnWindows()
{
    return tr("Only on Windows.");
}

void UBDictation::start(QGraphicsTextItem* textBox)
{
    if (!textBox)
    {
        textBox = UBTextBoxes::current();
    }

    if (!textBox)
    {
        textBox = UBTextBoxes::create();
    }

    if (!textBox)
    {
        return;
    }

    // The cursor at the end of the text, in the view, so that Windows writes there.
    textBox->setTextInteractionFlags(Qt::TextEditorInteraction);
    QTextCursor cursor = textBox->textCursor();
    cursor.movePosition(QTextCursor::End);
    textBox->setTextCursor(cursor);

    if (textBox->scene() && !textBox->scene()->views().isEmpty())
    {
        QGraphicsView* view = textBox->scene()->views().first();
        view->activateWindow();
        view->setFocus();
    }

    textBox->setFocus();

    // After the click, once the text box has the cursor.
    QPointer<QGraphicsTextItem> box(textBox);
    QTimer::singleShot(200, UBApplication::mainWindow, [box]() {
        if (box)
        {
            box->setFocus();
        }

        if (!UBPlatformUtils::startDictation())
        {
            UBApplication::mainWindow->information(tr("Dictate"), tr("The dictation of the system is not available here. "
                                                                    "On Windows, press Windows + H in a text."));
        }
    });
}
