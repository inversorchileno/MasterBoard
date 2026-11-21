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


#include "UBGitHubUpdater.h"

#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QProgressDialog>
#include <QStandardPaths>
#include <QTemporaryFile>

#include "core/UBApplication.h"
#include "gui/UBMainWindow.h"

#ifndef LIBERTYBOARD_BUILD
#define LIBERTYBOARD_BUILD 0
#endif

static const char* sLatestReleaseUrl = "https://api.github.com/repos/inversorchileno/LibertyBoard/releases/latest";
static const char* sReleasesPage = "https://github.com/inversorchileno/LibertyBoard/releases/latest";

static QNetworkRequest gitHubRequest(const QUrl& url)
{
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("LibertyBoard"));
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    return request;
}

UBGitHubUpdater::UBGitHubUpdater(QWidget* dialogParent)
    : QObject(dialogParent)
    , mDialogParent(dialogParent)
    , mNetwork(new QNetworkAccessManager(this))
    , mReleasePage(QString::fromLatin1(sReleasesPage))
{
}

int UBGitHubUpdater::currentBuild()
{
    return LIBERTYBOARD_BUILD;
}

QString UBGitHubUpdater::currentVersionText()
{
    // 2.0.0 is shown as 2.0.
    QString version = QCoreApplication::applicationVersion();

    if (version.count('.') == 2 && version.endsWith(".0"))
    {
        version.chop(2);
    }

    if (currentBuild() > 0)
    {
        return tr("%1 (build %2)").arg(version).arg(currentBuild());
    }

    return version;
}

/** @brief The first line of the messages of the update: the version in use. */
static QString inUse()
{
    return UBGitHubUpdater::tr("You are using LibertyBoard %1.").arg(UBGitHubUpdater::currentVersionText()) + "\n\n";
}

void UBGitHubUpdater::checkForUpdate()
{
    if (mBusy)
    {
        return;
    }

    mBusy = true;
    UBApplication::showMessage(tr("Looking for a new version..."), true);

    QNetworkReply* reply = mNetwork->get(gitHubRequest(QUrl(QString::fromLatin1(sLatestReleaseUrl))));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() { onReleaseReceived(reply); });
}

void UBGitHubUpdater::onReleaseReceived(QNetworkReply* reply)
{
    reply->deleteLater();
    UBApplication::showMessage(QString());

    // No release published yet: nothing newer than this build.
    const bool noRelease = (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 404);

    if (reply->error() != QNetworkReply::NoError && !noRelease)
    {
        mBusy = false;
        QMessageBox::warning(mDialogParent, tr("Update"),
                             inUse() + tr("LibertyBoard could not reach GitHub to look for a new version. Check the internet connection and try again."));
        return;
    }

    const QJsonObject release = QJsonDocument::fromJson(reply->readAll()).object();
    const QString tag = release.value("tag_name").toString();
    const int build = tag.section('-', -1).toInt();

    if (release.contains("html_url"))
    {
        mReleasePage = QUrl(release.value("html_url").toString());
    }

    QUrl zipUrl;

    for (const QJsonValue& asset : release.value("assets").toArray())
    {
        const QJsonObject object = asset.toObject();

        // The zip of Windows; a release also has the files of Linux and macOS.
        const QString name = object.value("name").toString();

        if (name.endsWith(".zip", Qt::CaseInsensitive) && (!zipUrl.isValid() || name.contains("windows", Qt::CaseInsensitive)))
        {
            zipUrl = QUrl(object.value("browser_download_url").toString());
        }
    }

#ifndef Q_OS_WIN
    // On Linux and macOS the new version is downloaded from its page.
    if (build > currentBuild())
    {
        mBusy = false;
        const auto open = QMessageBox::question(mDialogParent, tr("Update"),
            inUse() + tr("A new version of LibertyBoard is available (build %1).\n\nOpen its page to download it?").arg(build),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

        if (open == QMessageBox::Yes)
        {
            QDesktopServices::openUrl(mReleasePage);
        }

        return;
    }
#endif

    if (build <= currentBuild() || !zipUrl.isValid())
    {
        mBusy = false;
        QMessageBox::information(mDialogParent, tr("Update"),
                                 inUse() + tr("You have the latest version of LibertyBoard."));
        return;
    }

    const auto answer = QMessageBox::question(mDialogParent, tr("Update"),
        inUse() + tr("A new version of LibertyBoard is available (build %1).\n\n"
           "Download and install it now? LibertyBoard saves your documents, closes and opens again.").arg(build),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

    if (answer != QMessageBox::Yes)
    {
        mBusy = false;
        return;
    }

    download(zipUrl, build);
}

void UBGitHubUpdater::download(const QUrl& url, int build)
{
    Q_UNUSED(build)

    mProgress = new QProgressDialog(tr("Downloading the new version..."), tr("Cancel"), 0, 100, mDialogParent);
    mProgress->setWindowTitle(tr("Update"));
    mProgress->setWindowModality(Qt::WindowModal);
    mProgress->setMinimumDuration(0);
    mProgress->setValue(0);

    QNetworkReply* reply = mNetwork->get(gitHubRequest(url));

    connect(reply, &QNetworkReply::downloadProgress, mProgress, [this](qint64 received, qint64 total) {
        if (total > 0)
        {
            mProgress->setValue(int(received * 100 / total));
        }
    });
    connect(mProgress, &QProgressDialog::canceled, reply, &QNetworkReply::abort);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() { onDownloaded(reply); });
}

void UBGitHubUpdater::onDownloaded(QNetworkReply* reply)
{
    reply->deleteLater();
    mProgress->deleteLater();
    mProgress = nullptr;
    mBusy = false;

    if (reply->error() == QNetworkReply::OperationCanceledError)
    {
        return;
    }

    if (reply->error() != QNetworkReply::NoError)
    {
        QMessageBox::warning(mDialogParent, tr("Update"),
                             tr("The new version could not be downloaded. Try again later."));
        return;
    }

    const QString folder = QDir::tempPath() + "/LibertyBoardUpdate";
    QDir(folder).removeRecursively();
    QDir().mkpath(folder);

    QFile zip(folder + "/LibertyBoard-windows.zip");

    if (!zip.open(QIODevice::WriteOnly) || zip.write(reply->readAll()) < 0)
    {
        QMessageBox::warning(mDialogParent, tr("Update"),
                             tr("The new version could not be saved on this computer."));
        return;
    }

    zip.close();
    install(zip.fileName());
}

void UBGitHubUpdater::install(const QString& zipFile)
{
#ifdef Q_OS_WIN
    const QString programFolder = QDir::toNativeSeparators(QCoreApplication::applicationDirPath());
    const QString executable = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());

    // The program folder must accept new files, which is not the case of
    // Program Files without administrator rights.
    QTemporaryFile probe(QCoreApplication::applicationDirPath() + "/write-test-XXXXXX");

    if (!probe.open())
    {
        QMessageBox::information(mDialogParent, tr("Update"),
            tr("LibertyBoard cannot replace its own files in this folder. The download page opens: unzip the new version over the current one."));
        QDesktopServices::openUrl(mReleasePage);
        return;
    }

    probe.close();

    const QString folder = QFileInfo(zipFile).absolutePath();
    const QString script = folder + "/install.ps1";
    QFile file(script);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return;
    }

    auto quoted = [](const QString& text) {
        return "'" + QString(text).replace("'", "''") + "'";
    };

    const QString content = QString(
        "$ErrorActionPreference = 'Continue'\n"
        "try { Wait-Process -Id %1 -Timeout 120 -ErrorAction SilentlyContinue } catch {}\n"
        "Start-Sleep -Seconds 1\n"
        "$zip = %2\n"
        "$extract = Join-Path %3 'extract'\n"
        "Remove-Item $extract -Recurse -Force -ErrorAction SilentlyContinue\n"
        "Expand-Archive -Path $zip -DestinationPath $extract -Force\n"
        "$source = $extract\n"
        "$items = @(Get-ChildItem $extract)\n"
        "if ($items.Count -eq 1 -and $items[0].PSIsContainer) { $source = $items[0].FullName }\n"
        "robocopy $source %4 /E /R:5 /W:1 /NFL /NDL /NJH /NJS | Out-Null\n"
        "Start-Process -FilePath %5\n")
        .arg(QCoreApplication::applicationPid())
        .arg(quoted(QDir::toNativeSeparators(zipFile)), quoted(QDir::toNativeSeparators(folder)),
             quoted(programFolder), quoted(executable));

    file.write(content.toUtf8());
    file.close();

    const bool started = QProcess::startDetached("powershell.exe",
        { "-NoProfile", "-ExecutionPolicy", "Bypass", "-WindowStyle", "Hidden", "-File", QDir::toNativeSeparators(script) });

    if (!started)
    {
        QMessageBox::warning(mDialogParent, tr("Update"), tr("The installation could not be started."));
        return;
    }

    // Closes as the Quit menu does, saving the documents first.
    UBApplication::mainWindow->actionQuit->trigger();
#else
    Q_UNUSED(zipFile)
    QDesktopServices::openUrl(mReleasePage);
#endif
}
