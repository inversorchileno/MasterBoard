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

#include <QColor>
#include <QIcon>
#include <QString>

/**
 * @brief The colors of the bars of Master Board: the menu bar, the options
 * of the tools at the top, the document tabs and the zoom.
 *
 * The style sheets are written with the colors of the dark theme; style()
 * and color() give the same colors in the theme chosen in the preferences:
 * dark, white, grey, or the one of the system (dark or white). The theme is
 * read once: a new choice is used the next time Master Board starts.
 */
class UBTheme
{
public:
    enum class Kind
    {
        Dark,
        White,
        Grey
    };

    /** @brief Notes whether the system uses a dark theme, before Master Board sets its own. */
    static void noteSystemScheme();

    /** @brief The theme in use. */
    static Kind kind();

    /** @brief A style sheet written with the dark colors, in the colors of the theme. */
    static QString style(const QString& darkStyle);

    /** @brief A color of the dark theme, in the theme. */
    static QColor color(const QString& darkColor);

    /** @brief An icon drawn in the light grey of the dark theme, drawn in the color of the text of the theme. */
    static QIcon icon(const QString& path);

private:
    static bool sSystemIsDark;
};
