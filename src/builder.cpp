// Copyright (c) The MSWB2S Project.
// Copyright (c) NoJuo, LLC. All Rights Reserved
//
// This file is a part of the Microsoft Windows
// Back-to-Source ("MSWB2S") Project. And is
// hosted at the `github.com/mswb2s/bsys`
// repository.

#include "wbsys/builder.h"
#include "wbsys/process.h"
#include "wbsys/utility.h"

#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <system_error>
#include <unordered_map>
#include <vector>
#include <cctype>

namespace fs = std::filesystem;

namespace wbsys {

namespace {

bool isResourceFile(
    const std::string& source)
{
    std::string extension =
        fs::path(source)
            .extension()
            .string();

    for (auto& character : extension)
        character =
            static_cast<char>(
                std::tolower(
                    static_cast<unsigned char>(
                        character)));

    return extension == ".rc";
}

std::string objectPathFor(
    const Project& project,
    const std::string& source)
{
    fs::path relative(source);

    relative.replace_extension(
        isResourceFile(source)
            ? ".res"
            : ".obj");

    return (
        fs::path(project.objDir) /
        relative
    ).string();
}

bool needsRebuild(
    const std::string& source,
    const std::string& object)
{
    std::error_code error;

    if (!fs::is_regular_file(object, error))
        return true;

    const auto sourceTime =
        fs::last_write_time(
            source,
            error);

    if (error)
        return true;

    const auto objectTime =
        fs::last_write_time(
            object,
            error);

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

    for (const auto& flag :
         project.defaultFlags)
        command << flag << ' ';

    for (const auto& flag :
         file.flags)
        command << flag << ' ';

    for (const auto& define :
         project.defaultDefines)
        command << "/D" << define << ' ';

    for (const auto& define :
         file.defines)
        command << "/D" << define << ' ';

    for (const auto& include :
         project.defaultIncludes)
        command
            << "/I\""
            << include
            << "\" ";

    for (const auto& include :
         file.includes)
        command
            << "/I\""
            << include
            << "\" ";

    if (!project.pdb.empty())
        command
            << "/Zi /Fd\""
            << project.pdb
            << "\" ";

    command
        << "/Fo\""
        << object
        << "\" ";

    command
        << "\""
        << file.path
        << "\"";

    return command.str();
}

std::string makeResourceCommand(
    const Project& project,
    const FileSpec& file,
    const std::string& object)
{
    std::ostringstream command;

    command << project.rcExe << " /nologo ";

    for (const auto& define :
         project.defaultDefines)
        command << "/d" << define << ' ';

    for (const auto& define :
         file.defines)
        command << "/d" << define << ' ';

    for (const auto& include :
         project.defaultIncludes)
        command
            << "/i\""
            << include
            << "\" ";

    for (const auto& include :
         file.includes)
        command
            << "/i\""
            << include
            << "\" ";

    for (const auto& flag :
         file.flags)
        command << flag << ' ';

    command
        << "/fo\""
        << object
        << "\" ";

    command
        << "\""
        << file.path
        << "\"";

    return command.str();
}

std::unordered_map<std::string, std::string>
makeVariables(
    const Project& project,
    const std::string& file = {},
    const std::string& object = {})
{
    std::unordered_map<std::string, std::string>
        variables = project.variables;

    variables["name"] = project.name;
    variables["output"] = project.output;
    variables["objdir"] = project.objDir;
    variables["cl"] = project.clExe;
    variables["link"] = project.linkExe;
    variables["file"] = file;
    variables["object"] = object;

    return variables;
}

void printBuildMessage(
    const Project& project)
{
    if (project.buildMessage.empty())
        return;

    std::cout
        << expandVariables(
            project.buildMessage,
            makeVariables(project))
        << '\n';
}

void printFileBuildMessage(
    const Project& project,
    const FileSpec& file,
    const std::string& object)
{
    if (file.compileMessage.empty())
        return;

    std::cout
        << expandVariables(
            file.compileMessage,
            makeVariables(
                project,
                file.path,
                object))
        << '\n';
}

int doBuild(const Project& project)
{
    std::error_code error;

    fs::create_directories(
        project.objDir,
        error);

    std::vector<std::string> objectFiles;

    int built = 0;
    int skipped = 0;

    printBuildMessage(project);

    for (const auto& file : project.files) {
        const std::string object =
            objectPathFor(
                project,
                file.path);

        fs::create_directories(
            fs::path(object).parent_path(),
            error);

        objectFiles.push_back(object);

        if (!needsRebuild(
                file.path,
                object)) {

            std::cout
                << "[skip] "
                << file.path
                << " (up to date)\n";

            ++skipped;
            continue;
        }

        const bool hasCustomMessage =
            !file.compileMessage.empty();

        const bool showCustomMessage =
            hasCustomMessage &&
            project.verbosity != Verbosity::ErrorsOnly;

        const bool showDefaultLine =
            project.verbosity == Verbosity::All ||
            (project.verbosity == Verbosity::Normal &&
             !hasCustomMessage);

        const bool captureOutput =
            project.verbosity == Verbosity::ErrorsOnly ||
            (project.verbosity == Verbosity::Normal &&
             hasCustomMessage);

        if (showCustomMessage)
            printFileBuildMessage(
                project,
                file,
                object);

        const bool resource =
            isResourceFile(file.path);

        const std::string command =
            resource
                ? makeResourceCommand(
                    project,
                    file,
                    object)
                : makeCompileCommand(
                    project,
                    file,
                    object);

        const char* const label =
            resource
                ? "[rc]   "
                : "[cl]   ";

        if (showDefaultLine)
            std::cout
                << label
                << file.path
                << '\n';

        std::string capturedOutput;

        const int result =
            runCommand(
                command,
                captureOutput ? &capturedOutput : nullptr);

        if (result != 0) {
            if (captureOutput) {
                if (!showDefaultLine)
                    std::cout
                        << label
                        << file.path
                        << '\n';

                if (!capturedOutput.empty())
                    std::cout << capturedOutput;
            }

            std::cerr
                << "WBSys: "
                << (resource ? "resource compile" : "compile")
                << " failed ("
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
        fs::path(project.output)
            .parent_path(),
        error);

    std::ostringstream linkCommand;

    linkCommand
        << project.linkExe
        << ' '
        << project.linkFlags
        << " /OUT:\""
        << project.output
        << "\" ";

    if (!project.pdb.empty())
        linkCommand
            << "/DEBUG /PDB:\""
            << project.pdb
            << "\" ";

    for (const auto& object :
         objectFiles) {

        linkCommand
            << "\""
            << object
            << "\" ";
    }

    std::cout
        << "[link] -> "
        << project.output
        << '\n';

    const int result =
        runCommand(
            linkCommand.str());

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