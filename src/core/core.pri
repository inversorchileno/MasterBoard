
HEADERS      += src/core/UB.h \
                src/core/UBShortcutManager.h \
                src/core/UBApplication.h \
                src/core/UBSettings.h \
                src/core/UBSetting.h \
                src/core/UBOpenedFiles.h \
                src/core/UBDiagnostics.h \
                src/core/UBPersistenceManager.h \
                src/core/UBSceneCache.h \
                src/core/UBPreferencesController.h \
                src/core/UBMimeData.h \
                src/core/UBIdleTimer.h \
                src/core/UBDisplayManager.h \
                src/core/UBDocumentManager.h \
                src/core/UBApplicationController.h \
                src/core/UBGitHubUpdater.h \
                src/core/UBDownloadManager.h \
                src/core/UBDownloadThread.h \
                src/core/UBTextTools.h \
    src/core/UBPersistenceWorker.h \
    $$PWD/UBForeignObjectsHandler.h

SOURCES      += src/core/main.cpp \
                src/core/UBShortcutManager.cpp \
                src/core/UBApplication.cpp \
                src/core/UBSettings.cpp \
                src/core/UBSetting.cpp \
                src/core/UBOpenedFiles.cpp \
                src/core/UBDiagnostics.cpp \
                src/core/UBPersistenceManager.cpp \
                src/core/UBSceneCache.cpp \
                src/core/UBPreferencesController.cpp \
                src/core/UBMimeData.cpp \
                src/core/UBIdleTimer.cpp \
                src/core/UBDisplayManager.cpp \
                src/core/UBDocumentManager.cpp \
                src/core/UBApplicationController.cpp \
                src/core/UBGitHubUpdater.cpp \
                src/core/UBDownloadManager.cpp \
                src/core/UBDownloadThread.cpp \
                src/core/UBTextTools.cpp \
    src/core/UBPersistenceWorker.cpp \
    $$PWD/UBForeignObjectsHandler.cpp

# The diagnostic log reads the call stack when Master Board closes unexpectedly.
win32: LIBS += -lDbgHelp
