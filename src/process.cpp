// Copyright (c) The MSWB2S Project.
// Copyright (c) NoJuo, LLC. All Rights Reserved
//
// This file is a part of the Microsoft Windows
// Back-to-Source ("MSWB2S") Project. And is
// hosted at the `github.com/mswb2s/bsys`
// repository.

#include "wbsys/process.h"

#include <windows.h>

#include <iostream>
#include <vector>

namespace wbsys {

namespace {

int runCommandInherited(const std::string& commandLine)
{
    STARTUPINFOA startupInfo{};
    PROCESS_INFORMATION processInfo{};

    startupInfo.cb = sizeof(startupInfo);

    std::vector<char> buffer(
        commandLine.begin(),
        commandLine.end());

    buffer.push_back('\0');

    if (!CreateProcessA(
            nullptr,
            buffer.data(),
            nullptr,
            nullptr,
            FALSE,
            0,
            nullptr,
            nullptr,
            &startupInfo,
            &processInfo)) {
        std::cerr
            << "WBSys: failed to launch: "
            << commandLine
            << '\n';

        return -1;
    }

    WaitForSingleObject(
        processInfo.hProcess,
        INFINITE);

    DWORD exitCode = 0;

    GetExitCodeProcess(
        processInfo.hProcess,
        &exitCode);

    CloseHandle(processInfo.hProcess);
    CloseHandle(processInfo.hThread);

    return static_cast<int>(exitCode);
}

int runCommandCaptured(
    const std::string& commandLine,
    std::string& output)
{
    SECURITY_ATTRIBUTES security{};
    security.nLength = sizeof(security);
    security.bInheritHandle = TRUE;

    HANDLE readPipe = nullptr;
    HANDLE writePipe = nullptr;

    if (!CreatePipe(
            &readPipe,
            &writePipe,
            &security,
            0)) {
        std::cerr
            << "WBSys: failed to create pipe: "
            << commandLine
            << '\n';

        return -1;
    }

    SetHandleInformation(
        readPipe,
        HANDLE_FLAG_INHERIT,
        0);

    STARTUPINFOA startupInfo{};
    PROCESS_INFORMATION processInfo{};

    startupInfo.cb = sizeof(startupInfo);
    startupInfo.dwFlags |= STARTF_USESTDHANDLES;
    startupInfo.hStdOutput = writePipe;
    startupInfo.hStdError = writePipe;
    startupInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);

    std::vector<char> buffer(
        commandLine.begin(),
        commandLine.end());

    buffer.push_back('\0');

    const BOOL created =
        CreateProcessA(
            nullptr,
            buffer.data(),
            nullptr,
            nullptr,
            TRUE,
            0,
            nullptr,
            nullptr,
            &startupInfo,
            &processInfo);

    CloseHandle(writePipe);

    if (!created) {
        CloseHandle(readPipe);

        std::cerr
            << "WBSys: failed to launch: "
            << commandLine
            << '\n';

        return -1;
    }

    char chunk[4096];
    DWORD bytesRead = 0;

    while (ReadFile(
        readPipe,
        chunk,
        sizeof(chunk),
        &bytesRead,
        nullptr) && bytesRead > 0) {

        output.append(chunk, bytesRead);
    }

    CloseHandle(readPipe);

    WaitForSingleObject(
        processInfo.hProcess,
        INFINITE);

    DWORD exitCode = 0;

    GetExitCodeProcess(
        processInfo.hProcess,
        &exitCode);

    CloseHandle(processInfo.hProcess);
    CloseHandle(processInfo.hThread);

    return static_cast<int>(exitCode);
}

} // namespace

int runCommand(const std::string& commandLine, std::string* output)
{
    if (output)
        return runCommandCaptured(commandLine, *output);

    return runCommandInherited(commandLine);
}

} // namespace wbsys