// =============================================================================
//  Game/Core/Environment.cpp
// =============================================================================

#include "Core/Environment.h"
#include <cstdlib>

namespace core {

std::optional<std::string> getEnvironmentVariable(const char* name) {
    if (name == nullptr || *name == '\0') {
        return std::nullopt;
    }

#if defined(_WIN32)
    char* value = nullptr;
    size_t length = 0;
    if (_dupenv_s(&value, &length, name) == 0 && value != nullptr && length > 1) {
        std::string result(value);
        std::free(value);
        return result;
    }
    if (value != nullptr) {
        std::free(value);
    }
    return std::nullopt;
#else
    const char* value = std::getenv(name);
    if (value != nullptr && *value != '\0') {
        return std::string(value);
    }
    return std::nullopt;
#endif
}

} // namespace core
