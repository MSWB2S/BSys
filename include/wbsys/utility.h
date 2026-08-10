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

std::string trim(const std::string& value);

std::vector<std::string> splitSemicolons(
    const std::string& value);

std::string expandVariables(
    const std::string& value,
    const std::unordered_map<std::string, std::string>& variables);

} // namespace wbsys