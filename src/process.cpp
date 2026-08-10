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

int runCommand(const std::string& commandLine)
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

} // namespace wbsys