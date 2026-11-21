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


#include "UBDocumentTabs.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QTabBar>
#include <QToolButton>
#include <QUndoStack>

#include "board/UBBoardController.h"
#include "board/UBBoardSaver.h"
#include "core/UBApplication.h"
#include "core/UBApplicationController.h"
#include "core/UBOpenedFiles.h"
#include "core/UBPersistenceManager.h"
#include "core/UBSettings.h"
#include "document/UBDocumentController.h"
#include "document/UBDocumentProxy.h"
#include "domain/UBGraphicsScene.h"
#include "gui/UBTheme.h"

static const QString sRunningKey = QStringLiteral("App/LibertyBoardRunning");
static const QString sSessionKey = QStringLiteral("App/LibertyBoardSession");

UBDocumentTabs::UBDocumentTabs(QWidget* parent)
    : QWidget(parent)
    , mSaver(new UBBoardSaver(this))
{
    setObjectName("documentTabs");
    setStyleSheet(UBTheme::style(
        "#documentTabs { background: #20242b; }"
        "QTabBar::tab { background: #323842; color: #d8d8d8; padding: 3px 10px 5px 10px; margin-right: 2px; margin-top: 2px; margin-bottom: 0px;"
        " border-top-left-radius: 4px; border-top-right-radius: 4px; min-width: 120px; max-width: 320px; }"
        "QTabBar::tab:selected { background: #f6d77a; color: #111111; }"
        "QTabBar::tab:hover:!selected { background: #444b57; }"
        "QToolButton { color: #f0f0f0; background: transparent; border: none; font-size: 15px; padding: 0 8px; }"
        "QToolButton:hover { background: #444b57; }"));

    mTabBar = new QTabBar(this);
    mTabBar->setTabsClosable(true);
    mTabBar->setExpanding(false);
    mTabBar->setElideMode(Qt::ElideMiddle);
    mTabBar->setDocumentMode(true);
    mTabBar->setDrawBase(false);
    // A tab is dragged to put the documents in another order.
    mTabBar->setMovable(true);
    // As high as the tabs, with room under their names and little above them.
    setFixedHeight(25);

    auto newButton = new QToolButton(this);
    newButton->setText("+");
    newButton->setToolTip(tr("New board"));
    newButton->setFixedSize(26, 22);

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(4, 0, 4, 0);
    layout->setSpacing(2);
    layout->addWidget(mTabBar, 0, Qt::AlignTop);
    layout->addWidget(newButton);
    layout->addStretch(1);

    connect(mTabBar, &QTabBar::tabBarClicked, this, &UBDocumentTabs::onTabClicked);
    connect(mTabBar, &QTabBar::tabCloseRequested, this, &UBDocumentTabs::closeTab);
    connect(mTabBar, &QTabBar::tabMoved, this, &UBDocumentTabs::onTabMoved);
    connect(newButton, &QToolButton::clicked, this, &UBDocumentTabs::newDocument);
    connect(UBApplication::boardController, &UBBoardController::activeSceneChanged, this, &UBDocumentTabs::onActiveSceneChanged);
    connect(UBApplication::applicationController, &UBApplicationController::fileImported, this, &UBDocumentTabs::onFileImported);

    if (UBApplication::undoStack)
    {
        connect(UBApplication::undoStack.data(), &QUndoStack::indexChanged, this, &UBDocumentTabs::onUndoStackChanged);
    }

    // Set while LibertyBoard runs: still set at the next start, it stopped without closing.
    UBSettings* settings = UBSettings::settings();
    mStoppedUnexpectedly = settings->value(sRunningKey, false).toBool();
    settings->setValue(sRunningKey, true);
    settings->save();
    connect(qApp, &QCoreApplication::aboutToQuit, this, []() {
        UBSettings::settings()->setValue(sRunningKey, false);
        UBSettings::settings()->save();
    });
    connect(this, &UBDocumentTabs::countChanged, this, &UBDocumentTabs::saveSession);

    onActiveSceneChanged();
}

int UBDocumentTabs::count() const
{
    return mDocuments.size();
}

QList<std::shared_ptr<UBDocumentProxy>> UBDocumentTabs::documents() const
{
    QList<std::shared_ptr<UBDocumentProxy>> list;

    for (const OpenDocument& open : mDocuments)
    {
        list << open.document;
    }

    return list;
}

QString UBDocumentTabs::nameOf(const std::shared_ptr<UBDocumentProxy>& document) const
{
    const int index = indexOf(document);
    return index >= 0 ? mTabBar->tabText(index) : QString();
}

int UBDocumentTabs::pageOf(const std::shared_ptr<UBDocumentProxy>& document) const
{
    const int index = indexOf(document);
    return index >= 0 ? mDocuments.at(index).page : 0;
}

void UBDocumentTabs::addDocument(const std::shared_ptr<UBDocumentProxy>& document, int page)
{
    if (!document || indexOf(document) >= 0)
    {
        return;
    }

    OpenDocument open{ document };
    open.page = page;
    mDocuments.append(open);
    mTabBar->addTab(QString());
    updateTabText(mDocuments.size() - 1);
    mTabBar->setCurrentIndex(currentIndex());
    emit countChanged(count());
}

bool UBDocumentTabs::closeDocument(const std::shared_ptr<UBDocumentProxy>& document)
{
    const int index = indexOf(document);

    if (index < 0)
    {
        return true;
    }

    closeTab(index);
    return indexOf(document) < 0;
}

void UBDocumentTabs::setShowHandler(std::function<bool(const std::shared_ptr<UBDocumentProxy>&)> handler)
{
    mShowHandler = handler;
}

void UBDocumentTabs::openFile(const QString& filePath)
{
    const QString absolute = QFileInfo(filePath).absoluteFilePath();

    for (int i = 0; i < mDocuments.size(); ++i)
    {
        if (QFileInfo(mDocuments.at(i).pdfFile) == QFileInfo(absolute))
        {
            showDocument(i);
            return;
        }
    }

    UBApplication::applicationController->importFile(absolute);
}

bool UBDocumentTabs::saveCurrent()
{
    const QString filename = fileOfCurrentPage();

    if (filename.isEmpty())
    {
        return saveCurrentAs();
    }

    if (!mSaver->saveTo(filename))
    {
        QMessageBox::warning(window(), tr("Save"),
                             tr("%1 could not be saved. Check that it is not open in another program.")
                                 .arg(QDir::toNativeSeparators(filename)));
        return false;
    }

    rememberSavedFile(filename);
    return true;
}

bool UBDocumentTabs::saveCurrentAs()
{
    const QString filename = mSaver->saveAs(window(), fileOfCurrentPage());

    if (filename.isEmpty())
    {
        return false;
    }

    rememberSavedFile(filename);
    return true;
}

void UBDocumentTabs::newDocument()
{
    std::shared_ptr<UBDocumentProxy> blank = UBPersistenceManager::persistenceManager()->createNewDocument();

    if (!blank)
    {
        return;
    }

    UBApplication::boardController->persistCurrentScene(false, true);
    UBApplication::boardController->setActiveDocumentScene(blank);
    UBApplication::applicationController->showBoard();
}

void UBDocumentTabs::closeCurrent()
{
    closeTab(currentIndex());
}

int UBDocumentTabs::indexOf(const std::shared_ptr<UBDocumentProxy>& document) const
{
    for (int i = 0; i < mDocuments.size(); ++i)
    {
        if (mDocuments.at(i).document == document)
        {
            return i;
        }
    }

    return -1;
}

int UBDocumentTabs::currentIndex() const
{
    return indexOf(UBApplication::boardController->selectedDocument());
}

QString UBDocumentTabs::fileOfCurrentPage() const
{
    const int index = currentIndex();

    if (index < 0)
    {
        return QString();
    }

    const OpenDocument& open = mDocuments.at(index);

    // A PDF is saved whole; a board page by page, as pictures.
    if (!open.pdfFile.isEmpty())
    {
        return open.pdfFile;
    }

    return open.pageFiles.value(UBApplication::boardController->activeSceneIndex());
}

void UBDocumentTabs::rememberSavedFile(const QString& filename)
{
    const int index = currentIndex();

    if (index < 0)
    {
        return;
    }

    OpenDocument& open = mDocuments[index];

    if (QFileInfo(filename).suffix().compare("pdf", Qt::CaseInsensitive) == 0)
    {
        open.pdfFile = filename;

        // Opening the saved PDF again shows this document, with its annotations to edit.
        UBOpenedFiles::remember(filename, open.document);
    }
    else
    {
        open.pageFiles.insert(UBApplication::boardController->activeSceneIndex(), filename);
    }

    open.modified = false;
    updateTabText(index);
    saveSession();
    emit saved();
}

void UBDocumentTabs::showDocument(int index)
{
    if (index < 0 || index >= mDocuments.size())
    {
        return;
    }

    const OpenDocument& open = mDocuments.at(index);
    UBBoardController* board = UBApplication::boardController;

    if (open.document == board->selectedDocument())
    {
        mTabBar->setCurrentIndex(index);
        return;
    }

    // Removed meanwhile, for example replaced by a new import of the same name.
    if (!QDir(open.document->persistencePath()).exists())
    {
        mDocuments.removeAt(index);
        mTabBar->removeTab(index);
        mTabBar->setCurrentIndex(currentIndex());
        emit countChanged(count());
        return;
    }

    mSwitching = true;
    board->persistCurrentScene(false, true);
    board->setActiveDocumentScene(open.document, qBound(0, open.page, open.document->pageCount() - 1));
    mSwitching = false;
    mTabBar->setCurrentIndex(index);
}

void UBDocumentTabs::closeTab(int index)
{
    if (index < 0 || index >= mDocuments.size())
    {
        return;
    }

    emit documentClosing(mDocuments.at(index).document);

    if (index >= mDocuments.size() || !confirmClose(index))
    {
        return;
    }

    const bool closingCurrent = index == currentIndex();

    if (mDocuments.size() == 1)
    {
        // The board is never empty: a new blank board takes the place of the last tab.
        newDocument();
        index = indexOf(mDocuments.at(0).document);
    }
    else if (closingCurrent)
    {
        showDocument(index + 1 < mDocuments.size() ? index + 1 : index - 1);
    }

    if (index >= 0 && index < mDocuments.size())
    {
        UBApplication::boardController->persistCurrentScene(false, true);
        mDocuments.removeAt(index);
        mTabBar->removeTab(index);
    }

    mTabBar->setCurrentIndex(currentIndex());
    emit countChanged(count());
}

bool UBDocumentTabs::confirmClose(int index)
{
    if (!mDocuments.at(index).modified)
    {
        return true;
    }

    // The question is about the document of the tab, shown while asking.
    showDocument(index);

    const QString question = UBBoardSaver::isFromPdf() ? tr("Save the changes to the PDF before closing?")
                                                       : tr("Save the board as a picture before closing?");
    const auto answer = QMessageBox::question(window(), tr("Close document"), question,
                                              QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
                                              QMessageBox::Save);

    if (answer == QMessageBox::Cancel)
    {
        return false;
    }

    return answer == QMessageBox::Discard || saveCurrent();
}

void UBDocumentTabs::updateTabText(int index)
{
    const OpenDocument& open = mDocuments.at(index);
    QString name = open.pdfFile.isEmpty() ? open.document->name() : QFileInfo(open.pdfFile).completeBaseName();

    mTabBar->setTabText(index, open.modified ? name + QStringLiteral(" \u2022") : name);
    mTabBar->setTabToolTip(index, open.pdfFile.isEmpty() ? name : QDir::toNativeSeparators(open.pdfFile));
}

void UBDocumentTabs::onActiveSceneChanged()
{
    UBBoardController* board = UBApplication::boardController;
    std::shared_ptr<UBDocumentProxy> document = board ? board->selectedDocument() : nullptr;

    if (!document)
    {
        return;
    }

    int index = indexOf(document);

    if (index < 0)
    {
        mDocuments.append(OpenDocument{ document });
        index = mDocuments.size() - 1;
        mTabBar->addTab(QString());
        updateTabText(index);
        emit countChanged(count());
    }

    mDocuments[index].page = board->activeSceneIndex();
    mTabBar->setCurrentIndex(index);
    saveSession();
}

void UBDocumentTabs::onFileImported(std::shared_ptr<UBDocumentProxy> document, const QString& filePath)
{
    const int index = indexOf(document);

    if (index >= 0 && QFileInfo(filePath).suffix().compare("pdf", Qt::CaseInsensitive) == 0)
    {
        mDocuments[index].pdfFile = filePath;
        updateTabText(index);
    }

    // The blank board shown before the file was opened has no use any more.
    for (int i = mDocuments.size() - 1; i >= 0; --i)
    {
        if (mDocuments.at(i).document != document && isBlank(mDocuments.at(i)))
        {
            mDocuments.removeAt(i);
            mTabBar->removeTab(i);
        }
    }

    mTabBar->setCurrentIndex(currentIndex());
    emit countChanged(count());
}

bool UBDocumentTabs::isBlank(const OpenDocument& open) const
{
    if (open.modified || !open.pdfFile.isEmpty() || !open.pageFiles.isEmpty() || open.document->pageCount() != 1)
    {
        return false;
    }

    std::shared_ptr<UBGraphicsScene> page = UBPersistenceManager::persistenceManager()->loadDocumentScene(open.document, 0);
    return page && page->isEmpty();
}

void UBDocumentTabs::onTabMoved(int from, int to)
{
    // The documents keep the order of their tabs.
    mDocuments.move(from, to);
    saveSession();
}

void UBDocumentTabs::onTabClicked(int index)
{
    if (index >= 0 && index < mDocuments.size() && mShowHandler && mShowHandler(mDocuments.at(index).document))
    {
        mTabBar->setCurrentIndex(currentIndex());
        return;
    }

    showDocument(index);
}

void UBDocumentTabs::onUndoStackChanged()
{
    // Each change the user makes goes through the undo stack; switching
    // documents clears it.
    if (mSwitching || !UBApplication::undoStack || UBApplication::undoStack->count() == 0)
    {
        return;
    }

    const int index = currentIndex();

    if (index >= 0 && !mDocuments.at(index).modified)
    {
        mDocuments[index].modified = true;
        updateTabText(index);
    }
}

void UBDocumentTabs::saveSession() const
{
    // While the recovery is offered, the tabs of the stopped session are kept.
    if (mStoppedUnexpectedly)
    {
        return;
    }

    QStringList session;

    for (const OpenDocument& open : mDocuments)
    {
        session << QStringList{ open.document->documentFolderName(), open.pdfFile, QString::number(open.page) }.join('\n');
    }

    // Written at once: it is needed after a stop, when nothing else is written.
    UBSettings::settings()->setValue(sSessionKey, session);
    UBSettings::settings()->save();
}

void UBDocumentTabs::offerRecovery()
{
    if (!mStoppedUnexpectedly)
    {
        saveSession();
        return;
    }

    mStoppedUnexpectedly = false;
    const QStringList session = UBSettings::settings()->value(sSessionKey).toStringList();
    QList<QStringList> documents;
    UBDocumentTreeModel* library = UBPersistenceManager::persistenceManager()->mDocumentTreeStructureModel;

    for (const QString& entry : session)
    {
        const QStringList fields = entry.split('\n');

        if (fields.size() == 3 && library->findDocumentByFolderName(fields.at(0)))
        {
            documents << fields;
        }
    }

    if (documents.isEmpty())
    {
        saveSession();
        return;
    }

    const auto answer = QMessageBox::question(window(), tr("Recover documents"),
        tr("Master Board stopped without closing. Open again the %1 documents that were open?").arg(documents.size()));

    if (answer == QMessageBox::Yes)
    {
        for (const QStringList& fields : std::as_const(documents))
        {
            std::shared_ptr<UBDocumentProxy> document = library->findDocumentByFolderName(fields.at(0));
            UBApplication::boardController->persistCurrentScene(false, true);
            UBApplication::boardController->setActiveDocumentScene(document, qBound(0, fields.at(2).toInt(), document->pageCount() - 1));

            const int index = indexOf(document);

            if (index >= 0 && !fields.at(1).isEmpty())
            {
                mDocuments[index].pdfFile = fields.at(1);
                updateTabText(index);
            }
        }

        UBApplication::applicationController->showBoard();
    }

    saveSession();
}
