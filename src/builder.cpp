// Copyright (c) The MSWB2S Project.
// Copyright (c) NoJuo, LLC. All Rights Reserved
//
// This file is a part of the Microsoft Windows
// Back-to-Source ("MSWB2S") Project. And is
// hosted at the `github.com/mswb2s/bsys`
// repository.

#include "wbsys/builder.h"
#include "wbsys/process.h"

#include <filesystem>
#include <iostream>
#include <sstream>
#include <system_error>
#include <vector>

namespace fs = std::filesystem;

namespace wbsys {

namespace {

std::string objectPathFor(
    const Project& project,
    const std::string& source)
{
    fs::path relative(source);
    relative.replace_extension(".obj");

    return (fs::path(project.objDir) / relative).string();
}

bool needsRebuild(
    const std::string& source,
    const std::string& object)
{
    std::error_code error;

    if (!fs::is_regular_file(object, error))
        return true;

    const auto sourceTime =
        fs::last_write_time(source, error);

    if (error)
        return true;

    const auto objectTime =
        fs::last_write_time(object, error);

    if (error)
        return true;

    return sourceTime > objectTime;
}

std::string makeCompileCommand(
    const Project& project,
    const FileSpec& file,
    const std::string& object)
{
    std::ostringstream command;

    command << project.clExe << " /c ";

    for (const auto& flag : project.defaultFlags)
        command << flag << ' ';

    for (const auto& flag : file.flags)
        command << flag << ' ';

    for (const auto& define : project.defaultDefines)
        command << "/D" << define << ' ';

    for (const auto& define : file.defines)
        command << "/D" << define << ' ';

    for (const auto& include : project.defaultIncludes)
        command << "/I\"" << include << "\" ";

    for (const auto& include : file.includes)
        command << "/I\"" << include << "\" ";

    command << "/Fo\"" << object << "\" ";
    command << "\"" << file.path << "\"";

    return command.str();
}

int doBuild(const Project& project)
{
    std::error_code error;

    fs::create_directories(project.objDir, error);

    std::vector<std::string> objectFiles;

    int built = 0;
    int skipped = 0;

    for (const auto& file : project.files) {
        const std::string object =
            objectPathFor(project, file.path);

        fs::create_directories(
            fs::path(object).parent_path(),
            error);

        objectFiles.push_back(object);

        if (!needsRebuild(file.path, object)) {
            std::cout
                << "[skip] "
                << file.path
                << " (up to date)\n";

            ++skipped;
            continue;
        }

        const std::string command =
            makeCompileCommand(
                project,
                file,
                object);

        std::cout
            << "[cl]   "
            << file.path
            << '\n';

        const int result =
            runCommand(command);

        if (result != 0) {
            std::cerr
                << "WBSys: compile failed ("
                << result
                << "): "
                << file.path
                << '\n';

            return result;
        }

        ++built;
    }

    std::cout
        << "\nCompiled "
        << built
        << " file(s), skipped "
        << skipped
        << " (up to date)\n";

    fs::create_directories(
        fs::path(project.output).parent_path(),
        error);

    std::ostringstream linkCommand;

    linkCommand
        << project.linkExe
        << ' '
        << project.linkFlags
        << " /OUT:\""
        << project.output
        << "\" ";

    for (const auto& object : objectFiles)
        linkCommand << "\"" << object << "\" ";

    std::cout
        << "[link] -> "
        << project.output
        << '\n';

    const int result =
        runCommand(linkCommand.str());

    if (result != 0) {
        std::cerr
            << "WBSys: link failed ("
            << result
            << ")\n";

        return result;
    }

    std::cout
        << "Build OK: "
        << project.output
        << '\n';

    return 0;
}

} // namespace

int build(const Project& project)
{
    return doBuild(project);
}

int rebuild(const Project& project)
{
    clean(project);
    return doBuild(project);
}

void clean(const Project& project)
{
    std::error_code error;

    fs::remove_all(
        project.objDir,
        error);

    fs::remove(
        project.output,
        error);

    std::cout
        << "Cleaned "
        << project.objDir
        << " and "
        << project.output
        << '\n';
}

} // namespace wbsys