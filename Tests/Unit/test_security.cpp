#include "../../external/doctest/doctest.h"
#include "../../Game/Core/Security.h"
#include "../../Game/Core/Environment.h"

#include <filesystem>
#include <fstream>

using namespace core;

TEST_SUITE("Security") {

TEST_CASE("SHA-256 matches NIST standard test vectors") {
    // NIST Test Vector 1: Empty string
    CHECK(Security::sha256("") ==
          "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");

    // NIST Test Vector 2: "abc"
    CHECK(Security::sha256("abc") ==
          "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");

    // NIST Test Vector 3: 448 bits
    CHECK(Security::sha256("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq") ==
          "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1");
}

TEST_CASE("HMAC-SHA256 matches known cryptographic test vectors") {
    CHECK(Security::hmacSha256("key", "The quick brown fox jumps over the lazy dog") ==
          "f7bc83f430538424b13298e6aa6fb143ef4d59a14946175997479dbc2d1a3cd8");
}

TEST_CASE("constantTimeEquals behaves correctly and rejects length or content mismatch") {
    CHECK(Security::constantTimeEquals("abcdef", "abcdef"));
    CHECK_FALSE(Security::constantTimeEquals("abcdef", "abcdeg"));
    CHECK_FALSE(Security::constantTimeEquals("abcdef", "abcde"));
    CHECK_FALSE(Security::constantTimeEquals("", "a"));
    CHECK(Security::constantTimeEquals("", ""));
}

TEST_CASE("path containment correctly permits child paths and rejects traversal attempts") {
    const std::filesystem::path root = std::filesystem::temp_directory_path() / "ascendendo_test_root";
    std::filesystem::create_directories(root);

    const std::filesystem::path validChild = root / "levels" / "level1.lvl";
    CHECK(Security::isPathContained(validChild, root));

    // Rejection of directory traversal
    const std::filesystem::path traversal1 = root / ".." / "system32";
    CHECK_FALSE(Security::isPathContained(traversal1, root));

    const std::filesystem::path traversal2 = root / "levels" / ".." / ".." / "passwd";
    CHECK_FALSE(Security::isPathContained(traversal2, root));

    // Rejection of raw strings containing ".."
    CHECK_FALSE(Security::isPathContained("levels/../../secret", root));

    std::error_code ec;
    std::filesystem::remove_all(root, ec);
}

TEST_CASE("filename sanitization strips prohibited and reserved names") {
    CHECK(Security::sanitizeFilename("normal_level_01.lvl") == "normal_level_01.lvl");
    CHECK(Security::sanitizeFilename("invalid/name\\with:chars*.lvl") == "invalidnamewithchars.lvl");
    CHECK(Security::sanitizeFilename("CON") == "CON_safe");
    CHECK(Security::sanitizeFilename("aux") == "aux_safe");
    CHECK(Security::sanitizeFilename("   ") == "unnamed");
}

TEST_CASE("CSV field sanitization prevents CSV injection and corrupt formatting") {
    CHECK(Security::sanitizeCsvField("Safe Campaign") == "Safe Campaign");
    CHECK(Security::sanitizeCsvField("Campaign, with comma") == "\"Campaign, with comma\"");
    CHECK(Security::sanitizeCsvField("Line1\nLine2") == "\"Line1 Line2\"");
    CHECK(Security::sanitizeCsvField("Title with \"quotes\"") == "\"Title with \"\"quotes\"\"\"");

    // Neutralization of dangerous formula execution prefixes (=, +, -, @)
    std::string formula1 = "=cmd|' /C calc'!A0";
    std::string sanitized1 = Security::sanitizeCsvField(formula1);
    CHECK(sanitized1.find("'=") != std::string::npos);

    std::string formula2 = "+2+3";
    std::string sanitized2 = Security::sanitizeCsvField(formula2);
    CHECK(sanitized2.find("'+") != std::string::npos);
}

TEST_CASE("authenticated secure storage writes, verifies and detects tampering") {
    const std::filesystem::path savePath =
        std::filesystem::temp_directory_path() / "ascendendo_test_secure_save.dat";
    std::error_code ec;
    std::filesystem::remove(savePath, ec);

    const std::string testPayload =
        "PLAYER_PROGRESS\n"
        "CAMPAIGNS_COMPLETED=5\n"
        "BEST_TIME=142.50\n"
        "STARS=15\n";

    const std::string secretKey = "custom-auth-key-test-99";

    // 1. Write secure authenticated file
    REQUIRE(Security::writeSecureFile(savePath, testPayload, secretKey));

    // 2. Read and verify integrity
    auto readBack = Security::readSecureFile(savePath, secretKey);
    REQUIRE(readBack.has_value());
    CHECK(*readBack == testPayload);

    // 3. Reject with wrong key
    auto wrongKeyRead = Security::readSecureFile(savePath, "different-wrong-key");
    CHECK_FALSE(wrongKeyRead.has_value());

    // 4. Tamper with file content
    {
        std::string rawContent;
        {
            std::ifstream in(savePath, std::ios::binary);
            rawContent.assign((std::istreambuf_iterator<char>(in)),
                              std::istreambuf_iterator<char>());
        }
        // Change one byte in the payload
        size_t pos = rawContent.find("CAMPAIGNS_COMPLETED=5");
        REQUIRE(pos != std::string::npos);
        rawContent[pos + 20] = '9'; // changed 5 to 9

        std::ofstream out(savePath, std::ios::binary | std::ios::trunc);
        out << rawContent;
    }

    // Tampered file MUST fail closed
    auto tamperedRead = Security::readSecureFile(savePath, secretKey);
    CHECK_FALSE(tamperedRead.has_value());

    std::filesystem::remove(savePath, ec);
}

TEST_CASE("environment variable reader handles present and missing variables safely") {
    auto missing = core::getEnvironmentVariable("NON_EXISTENT_VAR_ASCENDENDO_xyz123456");
    CHECK_FALSE(missing.has_value());

#if defined(_WIN32)
    auto path = core::getEnvironmentVariable("PATH");
    CHECK(path.has_value());
    CHECK_FALSE(path->empty());
#endif
}

} // TEST_SUITE
