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



#include "UBTheme.h"

#include <QGuiApplication>
#include <QFile>
#include <QHash>
#include <QPainter>
#include <QPixmap>
#include <QSvgRenderer>
#include <QSettings>
#include <QStyleHints>

#include "core/UBSettings.h"

bool UBTheme::sSystemIsDark = false;

void UBTheme::noteSystemScheme()
{
#ifdef Q_OS_WIN
    // The choice of Windows for the applications: Settings > Personalization > Colors.
    QSettings personalize("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                          QSettings::NativeFormat);
    sSystemIsDark = personalize.value("AppsUseLightTheme", 1).toInt() == 0;
#elif (QT_VERSION >= QT_VERSION_CHECK(6, 5, 0))
    sSystemIsDark = QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark;
#endif
}

QString UBTheme::chosenAtStart()
{
    static const QString chosen = UBSettings::settings()->appTheme->get().toString();
    return chosen;
}

UBTheme::Kind UBTheme::kind()
{
    // Read once: the bars keep their colors until Master Board starts again.
    static const Kind theme = []() {
        const QString chosen = chosenAtStart();

        if (chosen == "white")
        {
            return Kind::White;
        }

        if (chosen == "grey")
        {
            return Kind::Grey;
        }

        if (chosen == "system")
        {
            return sSystemIsDark ? Kind::Dark : Kind::White;
        }

        return Kind::Dark;
    }();

    return theme;
}

/** @brief The colors of the dark theme, and those that take their place in the white and the grey ones. */
static const QHash<QString, QString>& replacements(UBTheme::Kind kind)
{
    // Dark text on the white bars; on the grey ones, a light grey with black
    // text, which keeps more contrast than a dark grey with white text.
    static const QHash<QString, QString> white = {
        { "#20242b", "#f4f5f7" }, // bars
        { "#323232", "#ffffff" }, // menu bar
        { "#2c313a", "#ffffff" }, // buttons
        { "#3a404b", "#c9ced6" }, // borders
        { "#4a4a4a", "#e3e7ee" }, // hovered items
        { "#4a5260", "#d5dbe4" },
        { "#6b7383", "#8a93a3" }, // hovered borders, disabled text
        { "#3d4452", "#dce4f0" }, // checked buttons
        { "#e8e8e8", "#1d2026" }, // text and drawn icons
        { "#f0f0f0", "#1d2026" },
        { "#9aa3b2", "#5b6472" }, // titles of sections
        { "#111317", "#ffffff" }, // fields
        { "#1e1e1e", "#ffffff" },
        { "#1d2026", "#ffffff" },
        { "#555555", "#b5bcc7" },
        { "#4a5160", "#c9ced6" }, // grooves of the sliders
        { "#ffffff", "#2f5fa8" }, // handles of the sliders
        { "#8fe3a0", "#1f7a33" }, // "Saved"
        { "#323842", "#e3e7ee" }, // document tabs
        { "#d8d8d8", "#1d2026" },
        { "#444b57", "#d5dbe4" },
        { "rgba(255, 255, 255, 60)", "rgba(0, 0, 0, 35)" },
    };
    static const QHash<QString, QString> grey = {
        { "#20242b", "#c9cdd3" },
        { "#323232", "#bcc1c8" },
        { "#2c313a", "#dadde2" },
        { "#3a404b", "#9ba2ac" },
        { "#4a4a4a", "#aeb4bd" },
        { "#4a5260", "#b3b9c2" },
        { "#6b7383", "#5d6573" },
        { "#3d4452", "#eef0f3" },
        { "#e8e8e8", "#111317" },
        { "#f0f0f0", "#111317" },
        { "#9aa3b2", "#3d4452" },
        { "#111317", "#eef0f3" },
        { "#1e1e1e", "#eef0f3" },
        { "#1d2026", "#eef0f3" },
        { "#555555", "#8a919c" },
        { "#4a5160", "#8a919c" },
        { "#ffffff", "#1d2026" },
        { "#8fe3a0", "#14532d" },
        { "#323842", "#b3b9c2" },
        { "#d8d8d8", "#111317" },
        { "#444b57", "#a5acb6" },
        { "rgba(255, 255, 255, 60)", "rgba(0, 0, 0, 40)" },
    };
    static const QHash<QString, QString> none;

    switch (kind)
    {
    case UBTheme::Kind::White:
        return white;
    case UBTheme::Kind::Grey:
        return grey;
    default:
        return none;
    }
}

QString UBTheme::style(const QString& darkStyle)
{
    const QHash<QString, QString>& colors = replacements(kind());

    if (colors.isEmpty())
    {
        return darkStyle;
    }

    // One pass, so that a color put in is not replaced again.
    QString result;
    result.reserve(darkStyle.size());
    int i = 0;

    while (i < darkStyle.size())
    {
        bool replaced = false;

        if (darkStyle.at(i) == '#' || darkStyle.at(i) == 'r')
        {
            for (auto it = colors.cbegin(); it != colors.cend(); ++it)
            {
                if (darkStyle.mid(i, it.key().size()).compare(it.key(), Qt::CaseInsensitive) == 0
                    && (it.key().startsWith('r') || i + it.key().size() >= darkStyle.size()
                        || !QChar(darkStyle.at(i + it.key().size())).isLetterOrNumber()))
                {
                    result += it.value();
                    i += it.key().size();
                    replaced = true;
                    break;
                }
            }
        }

        if (!replaced)
        {
            result += darkStyle.at(i);
            ++i;
        }
    }

    return result;
}

QIcon UBTheme::icon(const QString& path)
{
    if (kind() == Kind::Dark)
    {
        return QIcon(path);
    }

    QFile file(path);

    if (!file.open(QIODevice::ReadOnly))
    {
        return QIcon(path);
    }

    const QByteArray svg = style(QString::fromUtf8(file.readAll())).toUtf8();
    QPixmap pixmap(64, 64);
    pixmap.fill(Qt::transparent);
    QSvgRenderer renderer(svg);
    QPainter painter(&pixmap);
    renderer.render(&painter);
    painter.end();
    return QIcon(pixmap);
}

QColor UBTheme::color(const QString& darkColor)
{
    return QColor(replacements(kind()).value(darkColor.toLower(), darkColor));
}
