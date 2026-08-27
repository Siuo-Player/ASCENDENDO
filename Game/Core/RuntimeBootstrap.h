#pragma once

#include "Core/RuntimePaths.h"

#include <filesystem>
#include <string>
#include <vector>

namespace core {

struct RuntimeBootstrapResult {
    RuntimePaths paths;
    bool userDirectoriesReady = false;
    std::vector<std::filesystem::path> campaign;
    std::string campaignID;

    std::filesystem::path runsFile() const { return paths.runsFile(); }
    std::filesystem::path controlsFile() const { return paths.controlsFile(); }
    std::filesystem::path playerSprite() const { return paths.playerSprite(); }
};

class RuntimeBootstrap {
public:
    static RuntimeBootstrapResult prepare(RuntimePaths paths);
    static RuntimeBootstrapResult fromProcess(const char* argv0 = nullptr);
};

} // namespace core
