// Copyright (c) The MSWB2S Project.
// Copyright (c) NoJuo, LLC. All Rights Reserved
//
// This file is a part of the Microsoft Windows
// Back-to-Source ("MSWB2S") Project. And is
// hosted at the `github.com/mswb2s/bsys`
// repository.

#pragma once

#include "wbsys/model.h"

#include <string>

namespace wbsys {

bool parseManifest(const std::string& manifestPath, Project& project);

} // namespace wbsys