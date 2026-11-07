#pragma once
// =============================================================================
//  Game/Core/Security.h
//
//  Ascendendo Data & Information Security Subsystem
//  - NIST FIPS 180-4 SHA-256 implementation (zero external dependencies)
//  - RFC 2104 HMAC-SHA256 authenticated integrity
//  - Constant-time verification to prevent timing side channels
//  - Path traversal & sandbox containment validation
//  - CSV injection prevention
//  - Atomic write & crash-resilient authenticated file storage
// =============================================================================

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

namespace core {

class Security {
public:
    // ── Cryptographic Hashing (SHA-256) ──────────────────────────────────────
    static std::string sha256(std::string_view input);
    static std::string sha256(const uint8_t* data, std::size_t length);
    static std::optional<std::string> sha256File(const std::filesystem::path& path);

    // ── Message Authentication (HMAC-SHA256) ─────────────────────────────────
    static std::string hmacSha256(std::string_view key, std::string_view message);

    // Constant-time string comparison (prevents timing side-channel attacks)
    static bool constantTimeEquals(std::string_view a, std::string_view b);

    // ── Path Sanitization & Sandbox Containment ───────────────────────────────
    // Validates that target path is contained strictly within rootDirectory.
    // Rejects directory climbing (".."), null bytes, symlink escapes, etc.
    static bool isPathContained(const std::filesystem::path& target,
                                const std::filesystem::path& rootDirectory);

    // Strips invalid/dangerous characters from filenames and enforces length.
    static std::string sanitizeFilename(std::string_view filename);

    // Escapes/quotes CSV fields to prevent formula injection and corruption.
    static std::string sanitizeCsvField(std::string_view field);

    // ── Authenticated Secure Storage ─────────────────────────────────────────
    // Writes payload to temporary file, flushes to disk, and replaces atomically.
    // Embeds an HMAC or SHA-256 integrity signature in the file header.
    static bool writeSecureFile(const std::filesystem::path& path,
                                std::string_view payload,
                                std::string_view hmacKey = "ascendendo-1.0-secure-channel");

    // Reads file and verifies signature. If tampered or malformed, returns nullopt.
    static std::optional<std::string> readSecureFile(
        const std::filesystem::path& path,
        std::string_view hmacKey = "ascendendo-1.0-secure-channel");
};

} // namespace core
