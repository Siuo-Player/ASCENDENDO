#include "doctest/doctest.h"
#include "Logic/ProfileStorage.h"
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
        path_ = std::filesystem::temp_directory_path() / ("ascendendo-profile-test-" + std::to_string(stamp));
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

TEST_SUITE("Profile Storage & Security") {
    TEST_CASE("UserProfile serialization and deserialization round-trip") {
        logic::UserProfile original;
        original.version = 1;
        original.recordCompletion("01-iniciacao", 45.25f);
        original.recordCompletion("01-iniciacao", 42.10f); // Faster run updates best time
        original.recordCompletion("02-ritmo", 112.50f);

        CHECK(original.totalCompletions == 3);
        CHECK(original.totalPlayTimeSeconds == doctest::Approx(45.25f + 42.10f + 112.50f));
        CHECK(original.lastPlayedCampaignID == "02-ritmo");

        const auto* rec1 = original.findRecord("01-iniciacao");
        REQUIRE(rec1 != nullptr);
        CHECK(rec1->completions == 2);
        CHECK(rec1->bestTimeSeconds == doctest::Approx(42.10f));
        CHECK(rec1->lastTimeSeconds == doctest::Approx(42.10f));

        const std::string serialized = original.serialize();
        CHECK(!serialized.empty());

        const auto loaded = logic::UserProfile::deserialize(serialized);
        REQUIRE(loaded.has_value());
        CHECK(loaded->version == 1);
        CHECK(loaded->totalCompletions == original.totalCompletions);
        CHECK(loaded->totalPlayTimeSeconds == doctest::Approx(original.totalPlayTimeSeconds));
        CHECK(loaded->lastPlayedCampaignID == "02-ritmo");

        const auto* loadedRec1 = loaded->findRecord("01-iniciacao");
        REQUIRE(loadedRec1 != nullptr);
        CHECK(loadedRec1->completions == 2);
        CHECK(loadedRec1->bestTimeSeconds == doctest::Approx(42.10f));

        const auto* loadedRec2 = loaded->findRecord("02-ritmo");
        REQUIRE(loadedRec2 != nullptr);
        CHECK(loadedRec2->completions == 1);
        CHECK(loadedRec2->bestTimeSeconds == doctest::Approx(112.50f));
    }

    TEST_CASE("UserProfile deserialization rejects invalid format") {
        CHECK(!logic::UserProfile::deserialize("").has_value());
        CHECK(!logic::UserProfile::deserialize("NOT_A_VALID_FORMAT\nFOO BAR").has_value());
    }

    TEST_CASE("ProfileStorage saves and loads with HMAC authentication") {
        TempDir temp;
        const auto savePath = temp.path() / "Save" / "profile.secure";

        logic::UserProfile profile;
        profile.recordCompletion("01-iniciacao", 50.0f);
        profile.recordCompletion("02-ritmo", 90.0f);

        REQUIRE(logic::ProfileStorage::save(savePath, profile));
        REQUIRE(std::filesystem::exists(savePath));

        const auto loaded = logic::ProfileStorage::load(savePath);
        CHECK(loaded.totalCompletions == 2);
        CHECK(loaded.lastPlayedCampaignID == "02-ritmo");
        const auto* r = loaded.findRecord("01-iniciacao");
        REQUIRE(r != nullptr);
        CHECK(r->bestTimeSeconds == doctest::Approx(50.0f));
    }

    TEST_CASE("ProfileStorage rejects tampered files") {
        TempDir temp;
        const auto savePath = temp.path() / "Save" / "profile.secure";

        logic::UserProfile profile;
        profile.recordCompletion("01-iniciacao", 50.0f);
        REQUIRE(logic::ProfileStorage::save(savePath, profile));

        // Read file bytes, tamper one byte, and overwrite
        std::ifstream in(savePath, std::ios::binary);
        std::string raw((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        in.close();

        REQUIRE(raw.size() > 48);
        raw[raw.size() - 1] ^= 0xFF; // Flip bits in payload

        std::ofstream out(savePath, std::ios::binary | std::ios::trunc);
        out.write(raw.data(), static_cast<std::streamsize>(raw.size()));
        out.close();

        // Tampered file MUST be rejected and return default empty profile
        const auto tamperedLoad = logic::ProfileStorage::load(savePath);
        CHECK(tamperedLoad.totalCompletions == 0);
        CHECK(tamperedLoad.records.empty());
    }
}
