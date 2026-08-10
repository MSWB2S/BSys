// Copyright (c) The MSWB2S Project.
// Copyright (c) NoJuo, LLC. All Rights Reserved
//
// This file is a part of the Microsoft Windows
// Back-to-Source ("MSWB2S") Project. And is
// hosted at the `github.com/mswb2s/bsys`
// repository.

#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace wbsys {

enum class Verbosity {
    Normal,
    All,
    ErrorsOnly,
};

struct FileSpec {
    std::string path;
    std::vector<std::string> flags;
    std::vector<std::string> defines;
    std::vector<std::string> includes;
    std::string compileMessage;
};

struct Project {
    std::string name = "project";
    std::string output = "build\\out.exe";
    std::string clExe = "cl.exe";
    std::string linkExe = "link.exe";
    std::string rcExe = "rc.exe";
    std::string objDir = "build\\obj";
    std::string linkFlags = "/NOLOGO";
    std::string buildMessage;
    std::string pdb;
    Verbosity verbosity = Verbosity::Normal;

    std::vector<std::string> defaultFlags;
    std::vector<std::string> defaultDefines;
    std::vector<std::string> defaultIncludes;

    std::unordered_map<std::string, std::string> variables;

    std::vector<FileSpec> files;
};

} // namespace wbsys