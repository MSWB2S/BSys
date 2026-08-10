//Copyright (c) The MSWB2S Project.
//Copyright (c) NoJuo, LLC. All Rights Reserved
//
// This file is a part of the Microsoft Windows
// Back-to-Source ("MSWB2S") Project. And is
// hosted at the `github.com/mswb2s/bsys`
// repository.

#include <windows.h>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <sys/stat.h>

namespace fs = std::filesystem;

// ****************************************************
// * Small string helpers
// ****************************************************

static std::string trim(const std::string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

static std::vector<std::string> splitSemicolons(const std::string& s) {
    std::vector<std::string> out;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, ';')) {
        item = trim(item);
        if (!item.empty()) out.push_back(item);
    }
    return out;
}

// ****************************************************
// * Manifest data model
// ****************************************************

struct FileSpec {
    std::string path;              // relative source path, eg. src/main.cpp
    std::vector<std::string> flags;
    std::vector<std::string> defines;
    std::vector<std::string> includes;
};

struct Project {
    std::string name        = "project";
    std::string output      = "build\\out.exe";
    std::string clExe       = "cl.exe";
    std::string linkExe     = "link.exe";
    std::string objDir      = "build\\obj";
    std::string linkFlags   = "/NOLOGO";

    std::vector<std::string> defaultFlags;
    std::vector<std::string> defaultDefines;
    std::vector<std::string> defaultIncludes;

    std::vector<FileSpec> files;
};

// ****************************************************
// * .ini parser
// ****************************************************

static Project parseManifest(const std::string& manifestPath) {
    std::ifstream in(manifestPath);
    if (!in) {
        std::cerr << "WBSys: cannot open manifest: " << manifestPath << "\n";
        std::exit(1);
    }

    Project proj;
    std::string line, section;
    FileSpec* currentFile = nullptr;

    auto flushFile = [&]() {
        if (currentFile) {
            proj.files.push_back(*currentFile);
            delete currentFile;
            currentFile = nullptr;
        }
    };

    while (std::getline(in, line)) {
        std::string raw = trim(line);
        if (raw.empty() || raw[0] == ';' || raw[0] == '#') continue;

        if (raw.front() == '[' && raw.back() == ']') {
            flushFile();
            section = raw.substr(1, raw.size() - 2);
            if (section.rfind("file:", 0) == 0) {
                currentFile = new FileSpec();
                currentFile->path = trim(section.substr(5));
            }
            continue;
        }

        size_t eq = raw.find('=');
        if (eq == std::string::npos) continue;
        std::string key = trim(raw.substr(0, eq));
        std::string val = trim(raw.substr(eq + 1));

        if (section == "project") {
            if (key == "name") proj.name = val;
            else if (key == "output") proj.output = val;
            else if (key == "cl") proj.clExe = val;
            else if (key == "link") proj.linkExe = val;
            else if (key == "objdir") proj.objDir = val;
            else if (key == "linkflags") proj.linkFlags = val;
        } else if (section == "defaults") {
            if (key == "flags") {
                std::stringstream ss(val);
                std::string tok;
                while (ss >> tok) proj.defaultFlags.push_back(tok);
            }
            else if (key == "defines") proj.defaultDefines = splitSemicolons(val);
            else if (key == "includes") proj.defaultIncludes = splitSemicolons(val);
        } else if (currentFile) {
            if (key == "flags") {
                std::stringstream ss(val);
                std::string tok;
                while (ss >> tok) currentFile->flags.push_back(tok);
            } else if (key == "defines") {
                currentFile->defines = splitSemicolons(val);
            } else if (key == "includes") {
                currentFile->includes = splitSemicolons(val);
            }
        }
    }
    flushFile();
    return proj;
}

// ******************************************************
// * process execution (A CreateProcess wrapper)
//
// * CommandLineToArgv needs a mutable buffer on Windows,
// * hence the std::string copy into a vector
// ******************************************************

static int runCommand(const std::string& cmdLine) {
    STARTUPINFOA si{};
    PROCESS_INFORMATION pi{};
    si.cb = sizeof(si);

    std::vector<char> buf(cmdLine.begin(), cmdLine.end());
    buf.push_back('\0');

    BOOL ok = CreateProcessA(
        nullptr, buf.data(), nullptr, nullptr, FALSE,
        0, nullptr, nullptr, &si, &pi);

    if (!ok) {
        std::cerr << "WBSys: failed to launch: " << cmdLine << "\n";
        return -1;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exitCode = 0;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return static_cast<int>(exitCode);
}

// ****************************************************
// * build logic
// ****************************************************

static std::string objPathFor(const Project& proj, const std::string& srcRel) {
    fs::path rel(srcRel);
    fs::path objRel = rel;
    objRel.replace_extension(".obj");
    fs::path full = fs::path(proj.objDir) / objRel;
    return full.string();
}

static bool needsRebuild(const std::string& src, const std::string& obj) {
    if (!fs::exists(obj)) return true;
    auto srcTime = fs::last_write_time(src);
    auto objTime = fs::last_write_time(obj);
    return srcTime > objTime;
}

static std::string buildCompileCommand(const Project& proj, const FileSpec& f, const std::string& objPath) {
    std::ostringstream cmd;
    cmd << proj.clExe << " ";

    for (auto& fl : proj.defaultFlags) cmd << fl << " ";
    for (auto& fl : f.flags) cmd << fl << " ";

    for (auto& d : proj.defaultDefines) cmd << "/D" << d << " ";
    for (auto& d : f.defines) cmd << "/D" << d << " ";

    for (auto& inc : proj.defaultIncludes) cmd << "/I\"" << inc << "\" ";
    for (auto& inc : f.includes) cmd << "/I\"" << inc << "\" ";

    cmd << "/Fo\"" << objPath << "\" ";
    cmd << "\"" << f.path << "\"";
    return cmd.str();
}

static int doBuild(const Project& proj) {
    fs::create_directories(proj.objDir);

    std::vector<std::string> objPaths;
    int built = 0, skipped = 0;

    for (auto& f : proj.files) {
        std::string obj = objPathFor(proj, f.path);
        fs::create_directories(fs::path(obj).parent_path());
        objPaths.push_back(obj);

        if (!needsRebuild(f.path, obj)) {
            std::cout << "[skip] " << f.path << " (up to date)\n";
            skipped++;
            continue;
        }

        std::string cmd = buildCompileCommand(proj, f, obj);
        std::cout << "[cl]   " << f.path << "\n";
        int rc = runCommand(cmd);
        if (rc != 0) {
            std::cerr << "WBSys: compile failed (" << rc << "): " << f.path << "\n";
            return rc;
        }
        built++;
    }

    std::cout << "\nCompiled " << built << " file(s), skipped " << skipped << " (up to date)\n";

    fs::create_directories(fs::path(proj.output).parent_path());

    std::ostringstream link;
    link << proj.linkExe << " " << proj.linkFlags << " /OUT:\"" << proj.output << "\" ";
    for (auto& o : objPaths) link << "\"" << o << "\" ";

    std::cout << "[link] -> " << proj.output << "\n";
    int rc = runCommand(link.str());
    if (rc != 0) {
        std::cerr << "WBSys: link failed (" << rc << ")\n";
        return rc;
    }

    std::cout << "Build OK: " << proj.output << "\n";
    return 0;
}

static void doClean(const Project& proj) {
    std::error_code ec;
    fs::remove_all(proj.objDir, ec);
    fs::remove(proj.output, ec);
    std::cout << "Cleaned " << proj.objDir << " and " << proj.output << "\n";
}

// ****************************************************
// * Entry point
// ****************************************************

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cout << "WBSys, the Windows back2source Build System\n\n"
                  << "Copyright (c) The MSWB2S Project."
                  << "Copyright (c) NoJuo, LLC. All Rights Reserved."
                  << "Usage:\n"
                  << "  WBSys.exe <manifest.ini> build\n"
                  << "  WBSys.exe <manifest.ini> rebuild\n"
                  << "  WBSys.exe <manifest.ini> clean\n";
        return 1;
    }

    std::string manifestPath = argv[1];
    std::string cmd = argv[2];

    Project proj = parseManifest(manifestPath);

    // Fix path issues
    // change work dir to manifest folder
    fs::path manifestDir = fs::absolute(fs::path(manifestPath)).parent_path();
    SetCurrentDirectoryA(manifestDir.string().c_str());

    if (proj.files.empty()) {
        std::cerr << "WBSys: manifest has no [file:...] sections\n";
        return 1;
    }

    if (cmd == "build") {
        return doBuild(proj);
    } else if (cmd == "rebuild") {
        doClean(proj);
        return doBuild(proj);
    } else if (cmd == "clean") {
        doClean(proj);
        return 0;
    } else {
        std::cerr << "WBSys: unknown command '" << cmd << "'\n";
        return 1;
    }
}
