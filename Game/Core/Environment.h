#pragma once
// =============================================================================
//  Game/Core/Environment.h
//
//  Cross-platform, thread-safe, secure environment variable access.
//  Uses _dupenv_s on Windows (avoiding MSVC UCRT deprecation warnings)
//  and std::getenv on POSIX systems.
// =============================================================================

#include <optional>
#include <string>

namespace core {

std::optional<std::string> getEnvironmentVariable(const char* name);

} // namespace core
