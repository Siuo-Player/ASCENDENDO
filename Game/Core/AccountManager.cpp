#include "Core/AccountManager.h"
#include "Core/Security.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>

namespace core {

namespace {

int64_t currentTimestampSeconds() {
    return std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}

bool isValidUsername(std::string_view username) {
    if (username.size() < 3 || username.size() > 32) return false;
    for (char c : username) {
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') || c == '_' || c == '-')) {
            return false;
        }
    }
    return true;
}

} // namespace

AccountManager::AccountManager(std::filesystem::path accountsDirectory)
    : accountsRoot_(std::move(accountsDirectory)),
      vaultPath_(accountsRoot_ / "accounts.secure") {
    std::error_code ec;
    std::filesystem::create_directories(accountsRoot_, ec);
    loadAccountsVault();
}

std::string AccountManager::generateRandomSalt(std::size_t numBytes) {
    std::random_device rd;
    std::mt19937_64 gen(rd() ^ static_cast<uint64_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count()));
    std::uniform_int_distribution<uint32_t> dist(0, 255);

    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (std::size_t i = 0; i < numBytes; ++i) {
        oss << std::setw(2) << dist(gen);
    }
    return oss.str();
}

std::string AccountManager::derivePasswordHash(std::string_view password,
                                              std::string_view saltHex,
                                              uint32_t iterations) {
    if (iterations == 0) iterations = 1;
    // Multi-round HMAC-SHA256 key stretching
    std::string key(password);
    std::string currentHash = Security::hmacSha256(key, std::string(saltHex));
    for (uint32_t i = 1; i < iterations; ++i) {
        currentHash = Security::hmacSha256(key, currentHash);
    }
    return currentHash;
}

AccountInfo* AccountManager::findAccount(std::string_view username) {
    for (auto& acc : accounts_) {
        if (acc.username == username) return &acc;
    }
    return nullptr;
}

const AccountInfo* AccountManager::findAccount(std::string_view username) const {
    for (const auto& acc : accounts_) {
        if (acc.username == username) return &acc;
    }
    return nullptr;
}

bool AccountManager::hasAccount(std::string_view username) const {
    return findAccount(username) != nullptr;
}

std::vector<std::string> AccountManager::listUsernames() const {
    std::vector<std::string> names;
    names.reserve(accounts_.size());
    for (const auto& acc : accounts_) {
        names.push_back(acc.username);
    }
    return names;
}

bool AccountManager::registerAccount(std::string_view username, std::string_view password) {
    if (!isValidUsername(username)) return false;
    if (password.size() < 4) return false;
    if (hasAccount(username)) return false;

    const std::string salt = generateRandomSalt(16);
    const uint32_t iters = 10000;
    const std::string hash = derivePasswordHash(password, salt, iters);
    const std::string accountId = Security::sha256(std::string(username));
    const int64_t now = currentTimestampSeconds();

    AccountInfo info;
    info.username = std::string(username);
    info.accountId = accountId;
    info.saltHex = salt;
    info.passwordHashHex = hash;
    info.iterations = iters;
    info.createdTimestamp = now;
    info.lastLoginTimestamp = now;

    accounts_.push_back(info);

    // Create isolated user account directory
    std::error_code ec;
    std::filesystem::create_directories(accountsRoot_ / accountId, ec);

    if (!saveAccountsVault()) {
        accounts_.pop_back();
        return false;
    }

    currentAccount_ = info;
    return true;
}

bool AccountManager::login(std::string_view username, std::string_view password) {
    AccountInfo* acc = findAccount(username);
    if (!acc) return false;

    const std::string candidateHash =
        derivePasswordHash(password, acc->saltHex, acc->iterations);

    if (!Security::constantTimeEquals(candidateHash, acc->passwordHashHex)) {
        return false;
    }

    acc->lastLoginTimestamp = currentTimestampSeconds();
    currentAccount_ = *acc;
    saveAccountsVault();

    // Ensure account directory exists
    std::error_code ec;
    std::filesystem::create_directories(currentAccountDirectory(), ec);
    return true;
}

void AccountManager::logout() noexcept {
    currentAccount_.reset();
}

std::filesystem::path AccountManager::currentAccountDirectory() const {
    if (!currentAccount_.has_value()) return {};
    return accountsRoot_ / currentAccount_->accountId;
}

std::filesystem::path AccountManager::currentProfilePath() const {
    const auto dir = currentAccountDirectory();
    if (dir.empty()) return {};
    return dir / "profile.secure";
}

bool AccountManager::changePassword(std::string_view oldPassword, std::string_view newPassword) {
    if (!currentAccount_.has_value()) return false;
    if (newPassword.size() < 4) return false;

    AccountInfo* acc = findAccount(currentAccount_->username);
    if (!acc) return false;

    const std::string checkHash =
        derivePasswordHash(oldPassword, acc->saltHex, acc->iterations);
    if (!Security::constantTimeEquals(checkHash, acc->passwordHashHex)) {
        return false;
    }

    acc->saltHex = generateRandomSalt(16);
    acc->passwordHashHex = derivePasswordHash(newPassword, acc->saltHex, acc->iterations);
    currentAccount_ = *acc;
    return saveAccountsVault();
}

bool AccountManager::deleteCurrentAccount(std::string_view password) {
    if (!currentAccount_.has_value()) return false;
    AccountInfo* acc = findAccount(currentAccount_->username);
    if (!acc) return false;

    const std::string checkHash =
        derivePasswordHash(password, acc->saltHex, acc->iterations);
    if (!Security::constantTimeEquals(checkHash, acc->passwordHashHex)) {
        return false;
    }

    const auto accountDir = currentAccountDirectory();
    std::error_code ec;
    std::filesystem::remove_all(accountDir, ec);

    const std::string targetUser = currentAccount_->username;
    logout();

    accounts_.erase(std::remove_if(accounts_.begin(), accounts_.end(),
        [&](const AccountInfo& a) { return a.username == targetUser; }),
        accounts_.end());

    return saveAccountsVault();
}

bool AccountManager::saveAccountsVault() const {
    std::ostringstream oss;
    oss << "VAULT_VERSION 1\n";
    for (const auto& acc : accounts_) {
        oss << "ACCOUNT "
            << acc.username << " "
            << acc.accountId << " "
            << acc.saltHex << " "
            << acc.passwordHashHex << " "
            << acc.iterations << " "
            << acc.createdTimestamp << " "
            << acc.lastLoginTimestamp << "\n";
    }
    return Security::writeSecureFile(vaultPath_, oss.str());
}

bool AccountManager::loadAccountsVault() {
    accounts_.clear();
    currentAccount_.reset();

    if (!std::filesystem::exists(vaultPath_)) {
        return true;
    }

    const auto payload = Security::readSecureFile(vaultPath_);
    if (!payload) {
        std::cerr << "[AVISO DE SEGURANCA] Cofre de contas corrompido ou adulterado em: "
                  << vaultPath_.string() << "\n";
        return false;
    }

    std::istringstream stream(*payload);
    std::string line;
    while (std::getline(stream, line)) {
        if (line.empty() || line.front() == '#') continue;
        if (!line.empty() && line.back() == '\r') line.pop_back();

        std::istringstream ls(line);
        std::string tag;
        if (!(ls >> tag)) continue;

        if (tag == "ACCOUNT") {
            AccountInfo acc;
            if (ls >> acc.username
                   >> acc.accountId
                   >> acc.saltHex
                   >> acc.passwordHashHex
                   >> acc.iterations
                   >> acc.createdTimestamp
                   >> acc.lastLoginTimestamp) {
                accounts_.push_back(std::move(acc));
            }
        }
    }
    return true;
}

} // namespace core
