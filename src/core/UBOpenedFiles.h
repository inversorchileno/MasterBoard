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

#include <memory>

#include <QString>
#include <QStringList>

class UBDocumentProxy;

/**
 * @brief The files opened in Master Board and the documents they became:
 * opening a PDF again shows the document worked on, with its annotations,
 * as long as the file was not changed by another program. Also the list
 * of the recent files of the Home menu.
 */
namespace UBOpenedFiles
{
    /** @brief Remembers that the file, as it is now, is shown by this document. */
    void remember(const QString& filePath, const std::shared_ptr<UBDocumentProxy>& document);

    /** @brief The document of the file, if it was opened before and did not change since; nullptr otherwise. */
    std::shared_ptr<UBDocumentProxy> documentOf(const QString& filePath);

    /** @brief The files opened last, newest first, those still on the disk. */
    QStringList recentFiles();
}
