// Copyright (c) The MSWB2S Project.
// Copyright (c) NoJuo, LLC. All Rights Reserved
//
// This file is a part of the Microsoft Windows
// Back-to-Source ("MSWB2S") Project. And is
// hosted at the `github.com/mswb2s/bsys`
// repository.

#include "wbsys/utility.h"

#include <sstream>

namespace wbsys {

std::string trim(const std::string& value)
{
    const size_t first = value.find_first_not_of(" \t\r\n");

    if (first == std::string::npos)
        return {};

    const size_t last = value.find_last_not_of(" \t\r\n");

    return value.substr(first, last - first + 1);
}

std::vector<std::string> splitSemicolons(const std::string& value)
{
    std::vector<std::string> result;
    std::stringstream stream(value);
    std::string item;

    while (std::getline(stream, item, ';')) {
        item = trim(item);

        if (!item.empty())
            result.push_back(item);
    }

    return result;
}

} // namespace wbsys