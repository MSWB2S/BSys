// Copyright (c) The MSWB2S Project.
// Copyright (c) NoJuo, LLC. All Rights Reserved
//
// This file is a part of the Microsoft Windows
// Back-to-Source ("MSWB2S") Project. And is
// hosted at the `github.com/mswb2s/bsys`
// repository.

#include "wbsys/builder.h"
#include "wbsys/manifest.h"

#include <windows.h>

#include <filesystem>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

namespace {

void printUsage()
{
    std::cout
        << "WBSys, the Windows back2source Build System\n\n"
        << "Copyright (c) The MSWB2S Project.\n"
        << "Copyright (c) NoJuo, LLC. All Rights Reserved.\n"
        << "Usage:\n"
        << "  WBSys.exe <manifest.ini> build\n"
        << "  WBSys.exe <manifest.ini> rebuild\n"
        << "  WBSys.exe <manifest.ini> clean\n";
}

} // namespace

int main(int argc, char** argv)
{
    if (argc < 3) {
        printUsage();
        return 1;
    }

    const fs::path manifestPath =
        fs::absolute(argv[1]);

    const std::string command = argv[2];

    wbsys::Project project;

    if (!wbsys::parseManifest(
            manifestPath.string(),
            project)) {
        return 1;
    }

    const fs::path manifestDirectory =
        manifestPath.parent_path();

    if (!SetCurrentDirectoryA(
            manifestDirectory.string().c_str())) {
        std::cerr
            << "WBSys: cannot change directory to "
            << "manifest directory: "
            << manifestDirectory.string()
            << '\n';

        return 1;
    }

    if (project.files.empty()) {
        std::cerr
            << "WBSys: manifest has no "
            << "[file:...] sections\n";

        return 1;
    }

    if (command == "build")
        return wbsys::build(project);

    if (command == "rebuild")
        return wbsys::rebuild(project);

    if (command == "clean") {
        wbsys::clean(project);
        return 0;
    }

    std::cerr
        << "WBSys: unknown command '"
        << command
        << "'\n";

    return 1;
}