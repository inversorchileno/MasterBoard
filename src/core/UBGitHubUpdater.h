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
#include <QUrl>

class QNetworkAccessManager;
class QNetworkReply;
class QProgressDialog;
class QWidget;

/**
 * @brief Looks for a newer Master Board on GitHub, downloads it and installs
 * it over the current one.
 *
 * Every Windows build on GitHub is published as a release named
 * "build-<number>" with a zip of the program. The number of the running
 * build is compiled in as LIBERTYBOARD_BUILD. Installing is done by a small
 * script which waits until Master Board is closed, copies the new files over
 * the program folder and starts Master Board again.
 */
class UBGitHubUpdater : public QObject
{
    Q_OBJECT

public:
    explicit UBGitHubUpdater(QWidget* dialogParent);

    /** @brief The number of the running build, 0 when built outside of GitHub. */
    static int currentBuild();

    /** @brief The version in use, as "V2.31": the major version and the number of the build. */
    static QString currentVersionText();

    /** @brief The version of a build, as "V2.32". */
    static QString versionText(int build);

public slots:
    void checkForUpdate();

private:
    void onReleaseReceived(QNetworkReply* reply);
    void download(const QUrl& url, int build);
    void onDownloaded(QNetworkReply* reply);
    void install(const QString& zipFile);

    QWidget* mDialogParent{nullptr};
    QNetworkAccessManager* mNetwork{nullptr};
    QProgressDialog* mProgress{nullptr};
    QUrl mReleasePage;
    bool mBusy{false};
};
