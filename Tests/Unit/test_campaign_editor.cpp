#include "doctest/doctest.h"
#include "Logic/CampaignEditor.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

namespace {

class TempTree {
public:
    TempTree() {
        const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
        root_ = std::filesystem::temp_directory_path() /
                ("ascendendo-campaign-editor-" + std::to_string(stamp));
        std::filesystem::create_directories(root_);
    }

    ~TempTree() {
        std::error_code ec;
        std::filesystem::remove_all(root_, ec);
    }

    const std::filesystem::path& root() const { return root_; }

private:
    std::filesystem::path root_;
};

void writeLevel(const std::filesystem::path& path, const std::string& name) {
    std::ofstream file(path);
    REQUIRE(file.is_open());
    file << "NAME " << name << "\n";
    file << "PLATFORM 0 4 640 16\n";
}

} // namespace

TEST_SUITE("Campaign Editor") {
    TEST_CASE("reorders and persists the canonical campaign playlist") {
        TempTree temp;
        const auto levels = temp.root() / "Levels";
        std::filesystem::create_directories(levels);
        writeLevel(levels / "a.lvl", "A");
        writeLevel(levels / "b.lvl", "B");
        writeLevel(levels / "c.lvl", "C");

        const auto campaign = levels / "campaign.txt";
        {
            std::ofstream file(campaign);
            REQUIRE(file.is_open());
            file << "a.lvl\n"
                 << "b.lvl\n"
                 << "c.lvl\n";
        }

        logic::CampaignEditorDocument editor;
        REQUIRE(editor.loadFromCampaignFile(campaign.string()));
        REQUIRE(editor.levelCount() == 3);
        CHECK(editor.levels()[0].name == "A");
        CHECK(editor.levels()[1].name == "B");
        CHECK(editor.levels()[2].name == "C");

        REQUIRE(editor.moveLevel(2, 0));
        CHECK(editor.levels()[0].name == "C");
        CHECK(editor.levels()[1].name == "A");
        CHECK(editor.levels()[2].name == "B");

        const auto validation = editor.validateCampaign(campaign.string());
        REQUIRE(validation.valid);

        const auto saved = editor.saveToCampaignFile(campaign.string());
        REQUIRE(saved.success);
        CHECK(!std::filesystem::exists(campaign.string() + ".tmp-save"));

        logic::CampaignEditorDocument reloaded;
        REQUIRE(reloaded.loadFromCampaignFile(campaign.string()));
        REQUIRE(reloaded.levelCount() == 3);
        CHECK(reloaded.levels()[0].name == "C");
        CHECK(reloaded.levels()[1].name == "A");
        CHECK(reloaded.levels()[2].name == "B");
    }

    TEST_CASE("rejects missing referenced levels") {
        TempTree temp;
        const auto levels = temp.root() / "Levels";
        std::filesystem::create_directories(levels);

        const auto campaign = levels / "campaign.txt";
        std::ofstream file(campaign);
        REQUIRE(file.is_open());
        file << "missing.lvl\n";
        file.close();

        logic::CampaignEditorDocument editor;
        REQUIRE(editor.loadFromCampaignFile(campaign.string()));
        const auto validation = editor.validateCampaign(campaign.string());
        CHECK_FALSE(validation.valid);
        CHECK(validation.message.find("existing level") != std::string::npos);
        CHECK_FALSE(editor.saveToCampaignFile(campaign.string()).success);
    }

    TEST_CASE("rejects duplicate references and path escape") {
        TempTree temp;
        const auto levels = temp.root() / "Levels";
        std::filesystem::create_directories(levels);
        writeLevel(levels / "a.lvl", "A");
        writeLevel(temp.root() / "outside.lvl", "Outside");

        const auto campaign = levels / "campaign.txt";
        logic::CampaignEditorDocument editor;

        editor.levels() = {
            { (levels / "a.lvl").string(), "A", 0, 0.0f, false },
            { (levels / "a.lvl").string(), "A", 1, 0.0f, false }
        };
        auto validation = editor.validateCampaign(campaign.string());
        CHECK_FALSE(validation.valid);
        CHECK(validation.message.find("duplicate") != std::string::npos);

        editor.levels() = {
            { (temp.root() / "outside.lvl").string(), "Outside", 0, 0.0f, false }
        };
        validation = editor.validateCampaign(campaign.string());
        CHECK_FALSE(validation.valid);
        CHECK(validation.message.find("escapes") != std::string::npos);
    }
}
