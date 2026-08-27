#include "Core/RuntimePaths.h"

#include <cstdlib>
#include <string>
#include <vector>

#if defined(_WIN32)
    #include <windows.h>
#elif defined(__APPLE__)
    #include <mach-o/dyld.h>
#elif defined(__linux__)
    #include <unistd.h>
#endif

namespace core {
namespace {

std::filesystem::path executableDirectory(const char* argv0) {
#if defined(_WIN32)
    std::wstring buffer(32768, L'\0');
    const DWORD length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    if (length != 0 && length < buffer.size()) {
        buffer.resize(length);
        return std::filesystem::path(buffer).parent_path();
    }
#elif defined(__APPLE__)
    uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size);
    if (size != 0) {
        std::vector<char> buffer(size + 1, '\0');
        if (_NSGetExecutablePath(buffer.data(), &size) == 0) {
            return std::filesystem::weakly_canonical(buffer.data()).parent_path();
        }
    }
#elif defined(__linux__)
    std::vector<char> buffer(4096, '\0');
    const ssize_t length = readlink("/proc/self/exe", buffer.data(), buffer.size() - 1);
    if (length > 0) {
        buffer[static_cast<size_t>(length)] = '\0';
        return std::filesystem::path(buffer.data()).parent_path();
    }
#endif

    if (argv0 != nullptr && *argv0 != '\0') {
        const std::filesystem::path candidate(argv0);
        if (candidate.is_absolute() || candidate.has_parent_path()) {
            std::error_code ec;
            const auto canonical = std::filesystem::weakly_canonical(candidate, ec);
            if (!ec) return canonical.parent_path();
        }
    }

    std::error_code ec;
    const auto cwd = std::filesystem::current_path(ec);
    return ec ? std::filesystem::path{} : cwd;
}

#if defined(_WIN32)
std::filesystem::path windowsUserDataDirectory() {
    char* value = nullptr;
    size_t length = 0;
    if (_dupenv_s(&value, &length, "LOCALAPPDATA") == 0 && value != nullptr && length > 1) {
        std::filesystem::path result = std::filesystem::path(value) / "ASCENDENDO";
        std::free(value);
        return result;
    }
    std::free(value);
    return {};
}
#endif

std::filesystem::path userDataDirectory() {
#if defined(_WIN32)
    if (const auto localAppData = windowsUserDataDirectory(); !localAppData.empty()) {
        return localAppData;
    }
#elif defined(__APPLE__)
    if (const char* home = std::getenv("HOME"); home && *home) {
        return std::filesystem::path(home) / "Library" / "Application Support" / "ASCENDENDO";
    }
#else
    if (const char* stateHome = std::getenv("XDG_STATE_HOME"); stateHome && *stateHome) {
        return std::filesystem::path(stateHome) / "ASCENDENDO";
    }
    if (const char* home = std::getenv("HOME"); home && *home) {
        return std::filesystem::path(home) / ".local" / "state" / "ASCENDENDO";
    }
#endif

    std::error_code ec;
    const auto temp = std::filesystem::temp_directory_path(ec);
    return ec ? std::filesystem::path{} : temp / "ASCENDENDO";
}

} // namespace

RuntimePaths::RuntimePaths(std::filesystem::path executableRoot,
                           std::filesystem::path userDataRoot)
    : executableRoot_(executableRoot),
      userDataRoot_(userDataRoot) {}

RuntimePaths RuntimePaths::fromProcess(const char* argv0) {
    return RuntimePaths(executableDirectory(argv0), userDataDirectory());
}

std::filesystem::path RuntimePaths::assetsRoot() const {
    return executableRoot_ / "Game" / "Assets";
}

std::filesystem::path RuntimePaths::levelsRoot() const {
    return assetsRoot() / "Levels";
}

std::filesystem::path RuntimePaths::campaignFile() const {
    return levelsRoot() / "campaign.txt";
}

std::filesystem::path RuntimePaths::playerSprite() const {
    return assetsRoot() / "Sprites" / "personagem.png";
}

std::filesystem::path RuntimePaths::controlsFile() const {
    return userDataRoot_ / "Settings" / "controls.cfg";
}

std::filesystem::path RuntimePaths::runsFile() const {
    return userDataRoot_ / "Runs" / "runs.csv";
}

bool RuntimePaths::ensureUserDirectories() const {
    std::error_code ec;
    std::filesystem::create_directories(controlsFile().parent_path(), ec);
    if (ec) return false;
    std::filesystem::create_directories(runsFile().parent_path(), ec);
    return !ec;
}

} // namespace core
