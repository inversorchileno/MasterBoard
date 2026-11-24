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

#include <QString>

/**
 * @brief The diagnostic log, for the person who tests Master Board: off by
 * default, chosen in Preferences > General.
 *
 * When it is on, Master Board writes in a folder of its own what is done
 * (functions used, pages, tools, modes) and its messages; when it closes
 * unexpectedly, where it stopped (the call stack). Started again after
 * such a close, it offers to open the folder, to send the files.
 * When it is off, nothing is written and no folder is made.
 */
class UBDiagnostics
{
public:
    static bool isEnabled();

    /** @brief Turns the log on or off, at once. */
    static void setEnabled(bool on);

    /** @brief At the start of the program: starts the log when it is on. */
    static void start();

    /** @brief Follows the functions, pages, tools and modes, once the windows exist. */
    static void followActions();

    /** @brief At a normal end of the program. */
    static void stop();

    /** @brief Writes a line in the log, when it is on. */
    static void note(const QString& text);

    /** @brief Master Board closed unexpectedly the last time, while the log was on. */
    static bool closedUnexpectedly();

    /** @brief Says so and offers to open the folder of the log. */
    static void offerReport();

    /** @brief The folder of the log. */
    static QString folder();

    static void openFolder();
};
