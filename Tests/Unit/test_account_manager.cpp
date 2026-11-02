#include "doctest/doctest.h"
#include "Core/AccountManager.h"
#include "Core/Security.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

namespace {

class TempDir {
public:
    TempDir() {
        const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
        path_ = std::filesystem::temp_directory_path() / ("ascendendo-account-test-" + std::to_string(stamp));
        std::filesystem::create_directories(path_);
    }

    ~TempDir() {
        std::error_code ec;
        std::filesystem::remove_all(path_, ec);
    }

    const std::filesystem::path& path() const { return path_; }

private:
    std::filesystem::path path_;
};

} // namespace

TEST_SUITE("Account Manager — Security & Privacy") {
    TEST_CASE("Registration and Login Lifecycle") {
        TempDir temp;
        core::AccountManager manager(temp.path());

        // Invalid registration attempts
        CHECK(!manager.registerAccount("ab", "password123"));        // Username too short
        CHECK(!manager.registerAccount("user@name", "password123"));  // Invalid char
        CHECK(!manager.registerAccount("valid_user", "123"));        // Password too short

        // Valid registration
        REQUIRE(manager.registerAccount("player_one", "super_secret_jump!"));
        CHECK(manager.isLoggedIn());
        REQUIRE(manager.currentAccount() != nullptr);
        CHECK(manager.currentAccount()->username == "player_one");
        CHECK(manager.hasAccount("player_one"));
        CHECK(manager.listUsernames().size() == 1);

        // Account directory and profile path are isolated
        const auto accDir = manager.currentAccountDirectory();
        CHECK(std::filesystem::is_directory(accDir));
        CHECK(manager.currentProfilePath() == accDir / "profile.secure");

        // Duplicate registration rejected
        CHECK(!manager.registerAccount("player_one", "another_pass"));

        // Logout
        manager.logout();
        CHECK(!manager.isLoggedIn());
        CHECK(manager.currentAccount() == nullptr);

        // Login with wrong password
        CHECK(!manager.login("player_one", "wrong_password"));
        CHECK(!manager.isLoggedIn());

        // Login with correct password
        REQUIRE(manager.login("player_one", "super_secret_jump!"));
        CHECK(manager.isLoggedIn());
    }

    TEST_CASE("Password Change and Multi-Account Isolation") {
        TempDir temp;
        core::AccountManager manager(temp.path());

        REQUIRE(manager.registerAccount("alice", "alice_pass_1"));
        const auto aliceDir = manager.currentAccountDirectory();
        manager.logout();

        REQUIRE(manager.registerAccount("bob", "bob_pass_2"));
        const auto bobDir = manager.currentAccountDirectory();

        // Check that Alice and Bob have distinct isolated directories
        CHECK(aliceDir != bobDir);
        CHECK(manager.listUsernames().size() == 2);

        // Change password for Bob
        CHECK(!manager.changePassword("wrong_old", "bob_new_pass"));
        REQUIRE(manager.changePassword("bob_pass_2", "bob_new_pass"));

        manager.logout();
        CHECK(!manager.login("bob", "bob_pass_2"));
        REQUIRE(manager.login("bob", "bob_new_pass"));
    }

    TEST_CASE("Vault Tampering Detection") {
        TempDir temp;
        const auto vaultPath = temp.path() / "accounts.secure";

        {
            core::AccountManager manager(temp.path());
            REQUIRE(manager.registerAccount("charlie", "charlie_pass"));
        }

        REQUIRE(std::filesystem::exists(vaultPath));

        // Read file bytes, tamper one byte, and overwrite
        std::ifstream in(vaultPath, std::ios::binary);
        std::string raw((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        in.close();

        REQUIRE(raw.size() > 48);
        raw[raw.size() - 2] ^= 0x55;

        std::ofstream out(vaultPath, std::ios::binary | std::ios::trunc);
        out.write(raw.data(), static_cast<std::streamsize>(raw.size()));
        out.close();

        // Tampered vault is rejected
        core::AccountManager tamperedManager(temp.path());
        CHECK(!tamperedManager.hasAccount("charlie"));
        CHECK(tamperedManager.listUsernames().empty());
    }
}
