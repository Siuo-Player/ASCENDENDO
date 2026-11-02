#include "Logic/ProfileStorage.h"
#include "Core/Security.h"

#include <algorithm>
#include <iostream>
#include <sstream>

namespace logic {

void UserProfile::recordCompletion(const std::string& campaignID, float elapsedSeconds) {
    if (campaignID.empty()) return;
    totalCompletions++;
    totalPlayTimeSeconds += elapsedSeconds;
    lastPlayedCampaignID = campaignID;

    for (auto& rec : records) {
        if (rec.campaignID == campaignID) {
            rec.completions++;
            rec.lastTimeSeconds = elapsedSeconds;
            if (rec.bestTimeSeconds <= 0.0f || elapsedSeconds < rec.bestTimeSeconds) {
                rec.bestTimeSeconds = elapsedSeconds;
            }
            return;
        }
    }

    records.push_back(CampaignRecord{
        campaignID,
        1,
        elapsedSeconds,
        elapsedSeconds
    });
}

const CampaignRecord* UserProfile::findRecord(const std::string& campaignID) const {
    for (const auto& rec : records) {
        if (rec.campaignID == campaignID) {
            return &rec;
        }
    }
    return nullptr;
}

std::string UserProfile::serialize() const {
    std::ostringstream oss;
    oss << "VERSION " << version << "\n";
    oss << "TOTAL_COMPLETIONS " << totalCompletions << "\n";
    oss << "TOTAL_TIME " << totalPlayTimeSeconds << "\n";
    oss << "LAST_CAMPAIGN " << (lastPlayedCampaignID.empty() ? "-" : lastPlayedCampaignID) << "\n";
    for (const auto& rec : records) {
        oss << "RECORD " << rec.campaignID << " "
            << rec.completions << " "
            << rec.bestTimeSeconds << " "
            << rec.lastTimeSeconds << "\n";
    }
    return oss.str();
}

std::optional<UserProfile> UserProfile::deserialize(std::string_view text) {
    UserProfile profile;
    std::istringstream stream((std::string(text)));
    std::string line;

    bool versionFound = false;

    while (std::getline(stream, line)) {
        if (line.empty() || line.front() == '#') continue;
        if (!line.empty() && line.back() == '\r') line.pop_back();

        std::istringstream lineStream(line);
        std::string tag;
        if (!(lineStream >> tag)) continue;

        if (tag == "VERSION") {
            if (!(lineStream >> profile.version)) return std::nullopt;
            versionFound = true;
        } else if (tag == "TOTAL_COMPLETIONS") {
            lineStream >> profile.totalCompletions;
        } else if (tag == "TOTAL_TIME") {
            lineStream >> profile.totalPlayTimeSeconds;
        } else if (tag == "LAST_CAMPAIGN") {
            lineStream >> profile.lastPlayedCampaignID;
            if (profile.lastPlayedCampaignID == "-") {
                profile.lastPlayedCampaignID.clear();
            }
        } else if (tag == "RECORD") {
            CampaignRecord rec;
            if (lineStream >> rec.campaignID >> rec.completions >> rec.bestTimeSeconds >> rec.lastTimeSeconds) {
                profile.records.push_back(std::move(rec));
            }
        }
    }

    if (!versionFound) return std::nullopt;
    return profile;
}

bool ProfileStorage::save(const std::filesystem::path& path, const UserProfile& profile) {
    const std::string serialized = profile.serialize();
    return core::Security::writeSecureFile(path, serialized);
}

UserProfile ProfileStorage::load(const std::filesystem::path& path) {
    if (!std::filesystem::exists(path)) {
        return UserProfile{};
    }

    const auto data = core::Security::readSecureFile(path);
    if (!data) {
        std::cerr << "[AVISO DE SEGURANCA] Ficheiro de perfil corrompido ou adulterado em: "
                  << path.string() << "\n";
        return UserProfile{};
    }

    const auto profile = UserProfile::deserialize(*data);
    if (!profile) {
        std::cerr << "[AVISO DE SEGURANCA] Formato invalido no conteudo seguro do perfil em: "
                  << path.string() << "\n";
        return UserProfile{};
    }

    return *profile;
}

} // namespace logic
