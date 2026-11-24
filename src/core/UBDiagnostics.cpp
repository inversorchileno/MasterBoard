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



#include "UBDiagnostics.h"

#include <QAction>
#include <QCoreApplication>
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QMutex>
#include <QMutexLocker>
#include <QPushButton>
#include <QStandardPaths>
#include <QSysInfo>
#include <QUrl>

#include <csignal>
#include <cstdio>
#include <cstring>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <dbghelp.h>
#else
#include <execinfo.h>
#include <unistd.h>
#endif

#include "board/UBBoardController.h"
#include "board/UBDrawingController.h"
#include "core/UBApplication.h"
#include "core/UBApplicationController.h"
#include "core/UBSettings.h"
#include "document/UBDocumentProxy.h"
#include "gui/UBMainWindow.h"

namespace
{
    const QString sEnabledKey = QStringLiteral("App/LibertyBoardDiagnostics");

    // Kept small: the last part of the log is the useful one.
    constexpr qint64 sMaximumSize = 4 * 1024 * 1024;

    FILE* sFile = nullptr;
    bool sClosedUnexpectedly = false;
    bool sFollowing = false;
    QtMessageHandler sPreviousHandler = nullptr;
    QMutex sMutex;

    QString logPath()
    {
        return UBDiagnostics::folder() + "/masterboard.log";
    }

    QString runningPath()
    {
        return UBDiagnostics::folder() + "/running";
    }

    void writeLine(const QString& text)
    {
        QMutexLocker lock(&sMutex);

        if (!sFile)
        {
            return;
        }

        const QByteArray line = (QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz ") + text + "\n").toUtf8();
        std::fwrite(line.constData(), 1, size_t(line.size()), sFile);
        std::fflush(sFile);

        // Too big: the older half goes to another file.
        if (std::ftell(sFile) > sMaximumSize)
        {
            std::fclose(sFile);
            QFile::remove(UBDiagnostics::folder() + "/masterboard-old.log");
            QFile::rename(logPath(), UBDiagnostics::folder() + "/masterboard-old.log");
            sFile = std::fopen(QFile::encodeName(logPath()).constData(), "ab");
        }
    }

    void messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& message)
    {
        static const char* kinds[] = { "debug", "warning", "critical", "fatal", "info" };
        writeLine(QString("[%1] %2").arg(kinds[qBound(0, int(type), 4)], message));

        if (sPreviousHandler)
        {
            sPreviousHandler(type, context, message);
        }
    }

    // In a crash only plain writes: the program may be broken.
    void crashWrite(const char* text)
    {
        if (sFile)
        {
            std::fputs(text, sFile);
            std::fflush(sFile);
        }
    }

#ifdef _WIN32
    void writeFrame(DWORD64 address)
    {
        char line[1024];
        HMODULE module = nullptr;
        char moduleName[MAX_PATH] = "?";

        if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                               reinterpret_cast<LPCSTR>(address), &module))
        {
            GetModuleFileNameA(module, moduleName, MAX_PATH);
        }

        const char* shortName = std::strrchr(moduleName, '\\');
        shortName = shortName ? shortName + 1 : moduleName;
        const DWORD64 offset = module ? address - reinterpret_cast<DWORD64>(module) : address;

        // The name of the function, when the symbols are beside the program.
        char symbolBuffer[sizeof(SYMBOL_INFO) + 256];
        auto symbol = reinterpret_cast<SYMBOL_INFO*>(symbolBuffer);
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen = 255;
        DWORD64 displacement = 0;

        if (SymFromAddr(GetCurrentProcess(), address, &displacement, symbol))
        {
            std::snprintf(line, sizeof(line), "  %s+0x%llx  %s+0x%llx\n", shortName, (unsigned long long)offset,
                          symbol->Name, (unsigned long long)displacement);
        }
        else
        {
            std::snprintf(line, sizeof(line), "  %s+0x%llx\n", shortName, (unsigned long long)offset);
        }

        crashWrite(line);
    }

    LONG WINAPI unhandledException(EXCEPTION_POINTERS* info)
    {
        char line[256];
        std::snprintf(line, sizeof(line), "\n*** Master Board closed: exception 0x%08lx at %p\n",
                      info->ExceptionRecord->ExceptionCode, info->ExceptionRecord->ExceptionAddress);
        crashWrite(line);

        CONTEXT context = *info->ContextRecord;
        STACKFRAME64 frame = {};
        DWORD machine = 0;
#if defined(_M_X64)
        machine = IMAGE_FILE_MACHINE_AMD64;
        frame.AddrPC.Offset = context.Rip;
        frame.AddrFrame.Offset = context.Rbp;
        frame.AddrStack.Offset = context.Rsp;
#elif defined(_M_ARM64)
        machine = IMAGE_FILE_MACHINE_ARM64;
        frame.AddrPC.Offset = context.Pc;
        frame.AddrFrame.Offset = context.Fp;
        frame.AddrStack.Offset = context.Sp;
#else
        machine = IMAGE_FILE_MACHINE_I386;
        frame.AddrPC.Offset = context.Eip;
        frame.AddrFrame.Offset = context.Ebp;
        frame.AddrStack.Offset = context.Esp;
#endif
        frame.AddrPC.Mode = AddrModeFlat;
        frame.AddrFrame.Mode = AddrModeFlat;
        frame.AddrStack.Mode = AddrModeFlat;

        for (int i = 0; i < 64; ++i)
        {
            if (!StackWalk64(machine, GetCurrentProcess(), GetCurrentThread(), &frame, &context, nullptr,
                             SymFunctionTableAccess64, SymGetModuleBase64, nullptr) || !frame.AddrPC.Offset)
            {
                break;
            }

            writeFrame(frame.AddrPC.Offset);
        }

        crashWrite("*** end\n");
        return EXCEPTION_CONTINUE_SEARCH;
    }

    void signalHandler(int signalNumber)
    {
        char line[128];
        std::snprintf(line, sizeof(line), "\n*** Master Board closed: signal %d\n", signalNumber);
        crashWrite(line);

        void* frames[64];
        const USHORT count = CaptureStackBackTrace(0, 64, frames, nullptr);

        for (USHORT i = 0; i < count; ++i)
        {
            writeFrame(reinterpret_cast<DWORD64>(frames[i]));
        }

        crashWrite("*** end\n");
        std::signal(signalNumber, SIG_DFL);
        std::raise(signalNumber);
    }
#else
    void signalHandler(int signalNumber)
    {
        char line[128];
        std::snprintf(line, sizeof(line), "\n*** Master Board closed: signal %d\n", signalNumber);
        crashWrite(line);

        if (sFile)
        {
            void* frames[64];
            const int count = backtrace(frames, 64);
            backtrace_symbols_fd(frames, count, fileno(sFile));
        }

        crashWrite("*** end\n");
        std::signal(signalNumber, SIG_DFL);
        std::raise(signalNumber);
    }
#endif

    const int sSignals[] = { SIGSEGV, SIGABRT, SIGFPE, SIGILL
#ifndef _WIN32
                             , SIGBUS
#endif
    };

    void installCrashHandlers(bool on)
    {
        for (int signalNumber : sSignals)
        {
            std::signal(signalNumber, on ? signalHandler : SIG_DFL);
        }

#ifdef _WIN32
        if (on)
        {
            // The symbols beside the program, when they are there.
            SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);
            SymInitialize(GetCurrentProcess(), QFile::encodeName(QCoreApplication::applicationDirPath()).constData(), TRUE);
            SetUnhandledExceptionFilter(unhandledException);
        }
        else
        {
            SetUnhandledExceptionFilter(nullptr);
        }
#endif
    }

    QString toolName(int tool)
    {
        // In the order of UBStylusTool.
        static const char* names[] = { "Pen", "Eraser", "Marker", "Selector", "Play", "Hand", "ZoomIn", "ZoomOut",
                                       "Pointer", "Line", "Text", "Capture", "Arrow", "Rectangle", "Ellipse",
                                       "Underline", "Highlight", "Note", "Stamp", "Comment", "Attachment", "Sound",
                                       "Spotlight", "Voice", "Emoji", "Ai", "Measure" };
        return tool >= 0 && tool < int(sizeof(names) / sizeof(names[0])) ? QString(names[tool]) : QString::number(tool);
    }
}

bool UBDiagnostics::isEnabled()
{
    return UBSettings::settings()->value(sEnabledKey, false).toBool();
}

void UBDiagnostics::setEnabled(bool on)
{
    UBSettings::settings()->setValue(sEnabledKey, on);

    if (on)
    {
        start();
        followActions();
    }
    else
    {
        note("Diagnostic log turned off");
        stop();
    }
}

QString UBDiagnostics::folder()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/diagnostics";
}

void UBDiagnostics::start()
{
    if (sFile || !isEnabled())
    {
        return;
    }

    QDir().mkpath(folder());

    // Left by a start that never reached its end: an unexpected close.
    sClosedUnexpectedly = QFile::exists(runningPath());

    // A new log for each start; the one before is kept.
    if (QFileInfo(logPath()).size() > sMaximumSize / 2)
    {
        QFile::remove(folder() + "/masterboard-old.log");
        QFile::rename(logPath(), folder() + "/masterboard-old.log");
    }

    sFile = std::fopen(QFile::encodeName(logPath()).constData(), "ab");

    if (!sFile)
    {
        return;
    }

    QFile running(runningPath());
    running.open(QIODevice::WriteOnly);

    writeLine(QString("=== Master Board %1, %2, %3")
                  .arg(QCoreApplication::applicationVersion(), QSysInfo::prettyProductName(), QSysInfo::currentCpuArchitecture()));

    if (sClosedUnexpectedly)
    {
        writeLine("The last time, Master Board closed unexpectedly (see above).");
    }

    sPreviousHandler = qInstallMessageHandler(messageHandler);
    installCrashHandlers(true);
}

void UBDiagnostics::followActions()
{
    if (sFollowing || !sFile || !UBApplication::mainWindow)
    {
        return;
    }

    sFollowing = true;

    // Every function of the menus and the palettes.
    for (QAction* action : UBApplication::mainWindow->findChildren<QAction*>())
    {
        QObject::connect(action, &QAction::triggered, qApp, [action](bool checked) {
            QString name = action->objectName().isEmpty() ? action->text() : action->objectName();
            note(QString("Function %1%2").arg(name, action->isCheckable() ? (checked ? " on" : " off") : QString()));
        });
    }

    if (UBApplication::boardController)
    {
        QObject::connect(UBApplication::boardController, &UBBoardController::activeSceneChanged, qApp, []() {
            UBBoardController* board = UBApplication::boardController;
            std::shared_ptr<UBDocumentProxy> document = board ? board->selectedDocument() : nullptr;
            note(QString("Page %1 of %2 (%3)")
                     .arg(board ? board->activeSceneIndex() + 1 : 0)
                     .arg(document ? document->pageCount() : 0)
                     .arg(document ? document->name() : QString()));
        });
    }

    QObject::connect(UBDrawingController::drawingController(), &UBDrawingController::stylusToolChanged, qApp,
                     [](int tool) { note("Tool " + toolName(tool)); });

    if (UBApplication::applicationController)
    {
        QObject::connect(UBApplication::applicationController, &UBApplicationController::mainModeChanged, qApp,
                         [](UBApplicationController::MainMode mode) { note(QString("Mode %1").arg(int(mode))); });
        QObject::connect(UBApplication::applicationController, &UBApplicationController::desktopMode, qApp,
                         [](bool on) { note(on ? "Desktop mode" : "Board mode"); });
    }
}

void UBDiagnostics::stop()
{
    if (!sFile)
    {
        return;
    }

    writeLine("=== Normal end");
    installCrashHandlers(false);
    qInstallMessageHandler(sPreviousHandler);
    sPreviousHandler = nullptr;
    QFile::remove(runningPath());

    QMutexLocker lock(&sMutex);
    std::fclose(sFile);
    sFile = nullptr;
}

void UBDiagnostics::note(const QString& text)
{
    if (sFile)
    {
        writeLine(text);
    }
}

bool UBDiagnostics::closedUnexpectedly()
{
    return sClosedUnexpectedly;
}

void UBDiagnostics::offerReport()
{
    if (!sClosedUnexpectedly || !isEnabled())
    {
        return;
    }

    sClosedUnexpectedly = false;

    QMessageBox box(UBApplication::mainWindow);
    box.setIcon(QMessageBox::Information);
    box.setWindowTitle(QCoreApplication::translate("UBDiagnostics", "Diagnostic log"));
    box.setText(QCoreApplication::translate("UBDiagnostics", "Master Board closed unexpectedly the last time. "
                                                             "The diagnostic log says where: you can send its files."));
    QPushButton* open = box.addButton(QCoreApplication::translate("UBDiagnostics", "Open the folder"), QMessageBox::AcceptRole);
    box.addButton(QMessageBox::Close);
    box.exec();

    if (box.clickedButton() == open)
    {
        openFolder();
    }
}

void UBDiagnostics::openFolder()
{
    QDir().mkpath(folder());
    QDesktopServices::openUrl(QUrl::fromLocalFile(folder()));
}
