// =============================================================================
//  Game/Core/Security.cpp
// =============================================================================

#include "Core/Security.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>

namespace core {

namespace {

// ── SHA-256 Constants and Primitives (NIST FIPS 180-4) ───────────────────────

inline uint32_t rotr(uint32_t x, uint32_t n) {
    return (x >> n) | (x << (32 - n));
}

inline uint32_t ch(uint32_t x, uint32_t y, uint32_t z) {
    return (x & y) ^ (~x & z);
}

inline uint32_t maj(uint32_t x, uint32_t y, uint32_t z) {
    return (x & y) ^ (x & z) ^ (y & z);
}

inline uint32_t sig0(uint32_t x) {
    return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22);
}

inline uint32_t sig1(uint32_t x) {
    return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25);
}

inline uint32_t theta0(uint32_t x) {
    return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3);
}

inline uint32_t theta1(uint32_t x) {
    return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10);
}

constexpr std::array<uint32_t, 64> K = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

std::array<uint8_t, 32> sha256Raw(const uint8_t* data, std::size_t length) {
    uint32_t h0 = 0x6a09e667;
    uint32_t h1 = 0xbb67ae85;
    uint32_t h2 = 0x3c6ef372;
    uint32_t h3 = 0xa54ff53a;
    uint32_t h4 = 0x510e527f;
    uint32_t h5 = 0x9b05688c;
    uint32_t h6 = 0x1f83d9ab;
    uint32_t h7 = 0x5be0cd19;

    const uint64_t totalBits = static_cast<uint64_t>(length) * 8ULL;
    std::size_t paddedLength = length + 1;
    while ((paddedLength % 64) != 56) {
        ++paddedLength;
    }
    paddedLength += 8;

    std::vector<uint8_t> buffer(paddedLength, 0);
    if (length > 0) {
        std::copy_n(data, length, buffer.begin());
    }
    buffer[length] = 0x80;

    for (int i = 0; i < 8; ++i) {
        buffer[paddedLength - 1 - i] = static_cast<uint8_t>((totalBits >> (i * 8)) & 0xFF);
    }

    std::array<uint32_t, 64> w{};
    for (std::size_t chunk = 0; chunk < paddedLength; chunk += 64) {
        for (std::size_t i = 0; i < 16; ++i) {
            w[i] = (static_cast<uint32_t>(buffer[chunk + i * 4]) << 24) |
                   (static_cast<uint32_t>(buffer[chunk + i * 4 + 1]) << 16) |
                   (static_cast<uint32_t>(buffer[chunk + i * 4 + 2]) << 8) |
                   (static_cast<uint32_t>(buffer[chunk + i * 4 + 3]));
        }
        for (std::size_t i = 16; i < 64; ++i) {
            w[i] = theta1(w[i - 2]) + w[i - 7] + theta0(w[i - 15]) + w[i - 16];
        }

        uint32_t a = h0;
        uint32_t b = h1;
        uint32_t c = h2;
        uint32_t d = h3;
        uint32_t e = h4;
        uint32_t f = h5;
        uint32_t g = h6;
        uint32_t h = h7;

        for (std::size_t i = 0; i < 64; ++i) {
            const uint32_t t1 = h + sig1(e) + ch(e, f, g) + K[i] + w[i];
            const uint32_t t2 = sig0(a) + maj(a, b, c);
            h = g;
            g = f;
            f = e;
            e = d + t1;
            d = c;
            c = b;
            b = a;
            a = t1 + t2;
        }

        h0 += a;
        h1 += b;
        h2 += c;
        h3 += d;
        h4 += e;
        h5 += f;
        h6 += g;
        h7 += h;
    }

    std::array<uint8_t, 32> digest{};
    const std::array<uint32_t, 8> state = {h0, h1, h2, h3, h4, h5, h6, h7};
    for (std::size_t i = 0; i < 8; ++i) {
        digest[i * 4]     = static_cast<uint8_t>((state[i] >> 24) & 0xFF);
        digest[i * 4 + 1] = static_cast<uint8_t>((state[i] >> 16) & 0xFF);
        digest[i * 4 + 2] = static_cast<uint8_t>((state[i] >> 8) & 0xFF);
        digest[i * 4 + 3] = static_cast<uint8_t>(state[i] & 0xFF);
    }
    return digest;
}

std::string bytesToHex(const uint8_t* data, std::size_t length) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (std::size_t i = 0; i < length; ++i) {
        oss << std::setw(2) << static_cast<int>(data[i]);
    }
    return oss.str();
}

} // namespace

std::string Security::sha256(const uint8_t* data, std::size_t length) {
    const auto raw = sha256Raw(data, length);
    return bytesToHex(raw.data(), raw.size());
}

std::string Security::sha256(std::string_view input) {
    return sha256(reinterpret_cast<const uint8_t*>(input.data()), input.size());
}

std::optional<std::string> Security::sha256File(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) return std::nullopt;

    std::vector<uint8_t> buffer(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());
    return sha256(buffer.data(), buffer.size());
}

std::string Security::hmacSha256(std::string_view key, std::string_view message) {
    constexpr std::size_t blockSize = 64;
    std::array<uint8_t, blockSize> keyPad{};

    if (key.size() > blockSize) {
        const auto keyHash = sha256Raw(reinterpret_cast<const uint8_t*>(key.data()), key.size());
        std::copy(keyHash.begin(), keyHash.end(), keyPad.begin());
    } else {
        std::copy(key.begin(), key.end(), keyPad.begin());
    }

    std::vector<uint8_t> innerBuffer(blockSize + message.size());
    for (std::size_t i = 0; i < blockSize; ++i) {
        innerBuffer[i] = keyPad[i] ^ 0x36;
    }
    if (!message.empty()) {
        std::copy(message.begin(), message.end(), innerBuffer.begin() + blockSize);
    }
    const auto innerHash = sha256Raw(innerBuffer.data(), innerBuffer.size());

    std::vector<uint8_t> outerBuffer(blockSize + 32);
    for (std::size_t i = 0; i < blockSize; ++i) {
        outerBuffer[i] = keyPad[i] ^ 0x5C;
    }
    std::copy(innerHash.begin(), innerHash.end(), outerBuffer.begin() + blockSize);
    const auto finalDigest = sha256Raw(outerBuffer.data(), outerBuffer.size());

    return bytesToHex(finalDigest.data(), finalDigest.size());
}

bool Security::constantTimeEquals(std::string_view a, std::string_view b) {
    if (a.size() != b.size()) return false;
    uint8_t diff = 0;
    for (std::size_t i = 0; i < a.size(); ++i) {
        diff |= static_cast<uint8_t>(a[i] ^ b[i]);
    }
    return diff == 0;
}

bool Security::isPathContained(const std::filesystem::path& target,
                               const std::filesystem::path& rootDirectory) {
    try {
        const std::string rawStr = target.string();
        if (rawStr.find('\0') != std::string::npos) return false;
        if (rawStr.find("..") != std::string::npos) return false;

        std::error_code ec;
        const auto canonicalRoot = std::filesystem::weakly_canonical(rootDirectory, ec);
        if (ec) return false;

        const auto canonicalTarget = std::filesystem::weakly_canonical(target, ec);
        if (ec) return false;

        const auto rootStr = canonicalRoot.lexically_normal().string();
        const auto targetStr = canonicalTarget.lexically_normal().string();

        if (targetStr.size() < rootStr.size()) return false;
        if (targetStr.rfind(rootStr, 0) != 0) return false;

        if (targetStr.size() > rootStr.size()) {
            char sep = targetStr[rootStr.size()];
            if (sep != '/' && sep != '\\') return false;
        }
        return true;
    } catch (...) {
        return false;
    }
}

std::string Security::sanitizeFilename(std::string_view filename) {
    std::string result;
    result.reserve(filename.size());

    for (char c : filename) {
        if (c == '/' || c == '\\' || c == ':' || c == '*' || c == '?' ||
            c == '"' || c == '<' || c == '>' || c == '|' ||
            static_cast<unsigned char>(c) < 32 || c == 127) {
            continue;
        }
        result.push_back(c);
    }

    while (!result.empty() && (result.back() == '.' || result.back() == ' ')) {
        result.pop_back();
    }

    if (result.empty()) {
        return "unnamed";
    }

    if (result.size() > 64) {
        result.resize(64);
    }

    // Windows reserved filenames
    static const std::array<std::string_view, 22> reserved = {
        "CON", "PRN", "AUX", "NUL",
        "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9",
        "LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9"
    };

    std::string upperResult = result;
    for (char& c : upperResult) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));

    for (const auto& r : reserved) {
        if (upperResult == r) {
            return result + "_safe";
        }
    }

    return result;
}

std::string Security::sanitizeCsvField(std::string_view field) {
    std::string clean;
    clean.reserve(field.size());

    // Disallow formula execution characters at start (=, +, -, @, tab, cr)
    size_t startIdx = 0;
    while (startIdx < field.size() && (field[startIdx] == ' ' || field[startIdx] == '\t')) {
        startIdx++;
    }
    if (startIdx < field.size()) {
        char first = field[startIdx];
        if (first == '=' || first == '+' || first == '-' || first == '@') {
            clean.push_back('\''); // prepend apostrophe to neutralize formula
        }
    }

    bool needsQuotes = false;
    for (char c : field) {
        if (c == '\r' || c == '\n') {
            clean.push_back(' ');
            needsQuotes = true;
        } else if (c == '"') {
            clean.push_back('"');
            clean.push_back('"');
            needsQuotes = true;
        } else {
            if (c == ',' || c == ';') needsQuotes = true;
            clean.push_back(c);
        }
    }

    if (needsQuotes) {
        return "\"" + clean + "\"";
    }
    return clean;
}

bool Security::writeSecureFile(const std::filesystem::path& path,
                               std::string_view payload,
                               std::string_view hmacKey) {
    try {
        if (path.has_parent_path()) {
            std::filesystem::create_directories(path.parent_path());
        }

        const auto signature = hmacSha256(hmacKey, payload);
        const auto tempPath = path.string() + ".tmp-secure";

        {
            std::ofstream out(tempPath, std::ios::binary | std::ios::trunc);
            if (!out.is_open()) return false;

            out << "ASCENDENDO_SECURE_V1\n";
            out << "HMAC:" << signature << "\n";
            out << payload;
            out.flush();
            if (!out.good()) return false;
        }

        std::error_code ec;
        std::filesystem::rename(tempPath, path, ec);
        if (ec) {
            std::filesystem::remove(path, ec);
            std::filesystem::rename(tempPath, path, ec);
            if (ec) return false;
        }
        return true;
    } catch (...) {
        return false;
    }
}

std::optional<std::string> Security::readSecureFile(
    const std::filesystem::path& path,
    std::string_view hmacKey) {
    try {
        std::ifstream in(path, std::ios::binary);
        if (!in.is_open()) return std::nullopt;

        std::string header;
        if (!std::getline(in, header)) return std::nullopt;
        if (!header.empty() && header.back() == '\r') header.pop_back();
        if (header != "ASCENDENDO_SECURE_V1") return std::nullopt;

        std::string sigLine;
        if (!std::getline(in, sigLine)) return std::nullopt;
        if (!sigLine.empty() && sigLine.back() == '\r') sigLine.pop_back();
        if (sigLine.rfind("HMAC:", 0) != 0) return std::nullopt;
        const std::string expectedSig = sigLine.substr(5);

        std::string payload(
            (std::istreambuf_iterator<char>(in)),
            std::istreambuf_iterator<char>());

        const std::string computedSig = hmacSha256(hmacKey, payload);
        if (!constantTimeEquals(expectedSig, computedSig)) {
            return std::nullopt; // Tampering or corruption detected
        }

        return payload;
    } catch (...) {
        return std::nullopt;
    }
}

} // namespace core
