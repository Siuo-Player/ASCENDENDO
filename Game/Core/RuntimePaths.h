#pragma once

#include <filesystem>

namespace core {

class RuntimePaths {
public:
    RuntimePaths(std::filesystem::path executableRoot,
                 std::filesystem::path userDataRoot);

    static RuntimePaths fromProcess(const char* argv0 = nullptr);

    const std::filesystem::path& executableRoot() const noexcept { return executableRoot_; }
    const std::filesystem::path& userDataRoot() const noexcept { return userDataRoot_; }

    std::filesystem::path assetsRoot() const;
    std::filesystem::path levelsRoot() const;
    std::filesystem::path campaignFile() const;
    std::filesystem::path playerSprite() const;
    std::filesystem::path controlsFile() const;
    std::filesystem::path runsFile() const;

    bool ensureUserDirectories() const;

private:
    std::filesystem::path executableRoot_;
    std::filesystem::path userDataRoot_;
};

} // namespace core
