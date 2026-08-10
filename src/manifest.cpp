// Copyright (c) The MSWB2S Project.
// Copyright (c) NoJuo, LLC. All Rights Reserved
//
// This file is a part of the Microsoft Windows
// Back-to-Source ("MSWB2S") Project. And is
// hosted at the `github.com/mswb2s/bsys`
// repository.

#include "wbsys/manifest.h"
#include "wbsys/utility.h"

#include <fstream>
#include <iostream>
#include <sstream>

namespace wbsys {

namespace {

void parseFlags(
    const std::string& value,
    std::vector<std::string>& flags)
{
    std::stringstream stream(value);
    std::string flag;

    while (stream >> flag)
        flags.push_back(flag);
}

} // namespace

bool parseManifest(
    const std::string& manifestPath,
    Project& project)
{
    std::ifstream input(manifestPath);

    if (!input) {
        std::cerr
            << "WBSys: cannot open manifest: "
            << manifestPath
            << '\n';

        return false;
    }

    std::string line;
    std::string section;
    FileSpec* currentFile = nullptr;

    while (std::getline(input, line)) {
        const std::string raw = trim(line);

        if (raw.empty() ||
            raw[0] == ';' ||
            raw[0] == '#')
            continue;

        if (raw.front() == '[' &&
            raw.back() == ']') {

            currentFile = nullptr;

            section =
                trim(raw.substr(
                    1,
                    raw.size() - 2));

            if (section.rfind("file:", 0) == 0) {
                project.files.push_back({});

                currentFile =
                    &project.files.back();

                currentFile->path =
                    trim(section.substr(5));
            }

            continue;
        }

        const size_t equals =
            raw.find('=');

        if (equals == std::string::npos)
            continue;

        const std::string key =
            trim(raw.substr(0, equals));

        const std::string value =
            trim(raw.substr(equals + 1));

        if (section == "project") {
            if (key == "name")
                project.name = value;
            else if (key == "output")
                project.output = value;
            else if (key == "cl")
                project.clExe = value;
            else if (key == "link")
                project.linkExe = value;
            else if (key == "objdir")
                project.objDir = value;
            else if (key == "linkflags")
                project.linkFlags = value;
            else if (key == "message")
                project.buildMessage = value;
        }
        else if (section == "variables") {
            project.variables[key] = value;
        }
        else if (section == "defaults") {
            if (key == "flags")
                parseFlags(
                    value,
                    project.defaultFlags);
            else if (key == "defines")
                project.defaultDefines =
                    splitSemicolons(value);
            else if (key == "includes")
                project.defaultIncludes =
                    splitSemicolons(value);
        }
        else if (currentFile) {
            if (key == "flags")
                parseFlags(
                    value,
                    currentFile->flags);
            else if (key == "defines")
                currentFile->defines =
                    splitSemicolons(value);
            else if (key == "includes")
                currentFile->includes =
                    splitSemicolons(value);
        }
    }

    return true;
}

} // namespace wbsys