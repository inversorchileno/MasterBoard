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

#include <QCoreApplication>

class QGraphicsTextItem;

/**
 * @brief Dictation into a text box of the board: what is said is written in
 * it, by the dictation of the system (Windows + H on Windows).
 *
 * The dictation of Windows writes where the cursor of text is, so the text
 * box is given the cursor first.
 */
class UBDictation
{
    Q_DECLARE_TR_FUNCTIONS(UBDictation)

public:
    /**
     * @brief Starts the dictation into this text box, or else into the text
     * box being written on the board, or else into a new text box in the
     * middle of the board shown.
     */
    static void start(QGraphicsTextItem* textBox = nullptr);

    /**
     * @brief Whether the system has a dictation that Master Board can start:
     * only Windows. Elsewhere the buttons of dictation are disabled.
     */
    static bool isAvailable();

    /**
     * @brief The tooltip of the buttons of dictation where it is not available.
     */
    static QString onlyOnWindows();
};
