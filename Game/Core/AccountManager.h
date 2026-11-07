#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace core {

struct AccountInfo {
    std::string username;
    std::string accountId;       // SHA-256 hash of username (safe directory name)
    std::string saltHex;         // 16-byte random salt in hex
    std::string passwordHashHex; // Derived key via multi-round HMAC-SHA256
    uint32_t iterations = 10000;
    int64_t createdTimestamp = 0;
    int64_t lastLoginTimestamp = 0;
};

class AccountManager {
public:
    explicit AccountManager(std::filesystem::path accountsDirectory);

    bool registerAccount(std::string_view username, std::string_view password);
    bool login(std::string_view username, std::string_view password);
    void logout() noexcept;

    bool isLoggedIn() const noexcept { return currentAccount_.has_value(); }
    const AccountInfo* currentAccount() const noexcept {
        return currentAccount_.has_value() ? &(*currentAccount_) : nullptr;
    }

    std::filesystem::path currentAccountDirectory() const;
    std::filesystem::path currentProfilePath() const;

    bool changePassword(std::string_view oldPassword, std::string_view newPassword);
    bool deleteCurrentAccount(std::string_view password);

    std::vector<std::string> listUsernames() const;
    bool hasAccount(std::string_view username) const;

    // Cryptographic derivation helper (zero external dependencies)
    static std::string derivePasswordHash(std::string_view password,
                                          std::string_view saltHex,
                                          uint32_t iterations);

    static std::string generateRandomSalt(std::size_t numBytes = 16);

private:
    bool loadAccountsVault();
    bool saveAccountsVault() const;
    AccountInfo* findAccount(std::string_view username);
    const AccountInfo* findAccount(std::string_view username) const;

    std::filesystem::path accountsRoot_;
    std::filesystem::path vaultPath_;
    std::vector<AccountInfo> accounts_;
    std::optional<AccountInfo> currentAccount_;
};

} // namespace core
