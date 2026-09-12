#include "doctest/doctest.h"
#include "Logic/GameSession.h"
#include "Logic/InputManager.h"
#include "Core/KeyBindings.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace {

struct CampaignFixture {
    std::filesystem::path root;
    std::filesystem::path campaign;
    std::filesystem::path firstLevel;
    std::filesystem::path secondLevel;
    std::filesystem::path runs;

    CampaignFixture() {
        root = std::filesystem::temp_directory_path() / "ascendendo-campaign-session-test";
        std::error_code ec;
        std::filesystem::remove_all(root, ec);
        std::filesystem::create_directories(root);

        firstLevel = root / "first.lvl";
        secondLevel = root / "second.lvl";
        campaign = root / "campaign.txt";
        runs = root / "runs.csv";

        {
            std::ofstream first(firstLevel);
            first << "NAME First\n"
                  << "PLATFORM 160 80 128 16\n"
                  << "PLATFORM 256 160 128 16\n";
        }
        {
            std::ofstream second(secondLevel);
            second << "NAME Second\n"
                   << "PLATFORM 160 80 128 16\n"
                   << "PLATFORM 420 160 128 16\n";
        }
        {
            std::ofstream list(campaign);
            list << "# canonical campaign order\n"
                 << "first.lvl\n"
                 << "second.lvl\n";
        }
    }

    ~CampaignFixture() {
        std::error_code ec;
        std::filesystem::remove_all(root, ec);
    }
};

void tap(logic::InputManager& input, int key) {
    input.beginFrame();
    input.onKeyEvent(key, logic::Action::PRESS);
    input.onKeyEvent(key, logic::Action::RELEASE);
}

void configure(logic::GameSession& session, const CampaignFixture& fixture) {
    session.configureCampaignEditor(fixture.campaign.string());
}

} // namespace

TEST_SUITE("GameSession — Campaign Editor integration") {

TEST_CASE("C abre o Campaign Editor e preserva a campanha carregada") {
    CampaignFixture fixture;
    std::vector<std::filesystem::path> levels{fixture.firstLevel, fixture.secondLevel};
    logic::GameSession session(std::move(levels), "test-campaign", fixture.runs.string());
    configure(session, fixture);
    logic::InputManager input;
    core::KeyBindings bindings;

    tap(input, logic::Key::C);
    const auto result = session.update(0.0f, input, bindings, 640, 360, 640.0f, 360.0f);
    CHECK(result.stateChanged);
    CHECK(session.state() == core::GameState::CAMPAIGN_EDITOR);
    CHECK(session.campaignEditor().levelCount() == 2);
    CHECK(session.campaignEditor().selectedIndex() == 0);
}

TEST_CASE("abrir nivel e voltar preserva identidade da selecao") {
    CampaignFixture fixture;
    std::vector<std::filesystem::path> levels{fixture.firstLevel, fixture.secondLevel};
    logic::GameSession session(std::move(levels), "test-campaign", fixture.runs.string());
    configure(session, fixture);
    logic::InputManager input;
    core::KeyBindings bindings;

    tap(input, logic::Key::C);
    session.update(0.0f, input, bindings, 640, 360, 640.0f, 360.0f);
    tap(input, logic::Key::DOWN);
    session.update(0.0f, input, bindings, 640, 360, 640.0f, 360.0f);

    const std::string selectedPath = session.campaignEditor().selectedLevel()->path;
    tap(input, logic::Key::SPACE);
    session.update(0.0f, input, bindings, 640, 360, 640.0f, 360.0f);

    CHECK(session.state() == core::GameState::EDITOR);
    CHECK(session.editorSession().persistencePath() == selectedPath);
    CHECK(session.editorSession().document().isFinalCampaignLevel());
    CHECK(session.editorSession().document().spawnPosition() == logic::Vec2{320.0f, 16.0f});
    CHECK(session.editorSession().document().platformCount() == 2);
    CHECK(session.editorSession().document().hasFlag());

    tap(input, logic::Key::ESCAPE);
    session.update(0.0f, input, bindings, 640, 360, 640.0f, 360.0f);

    CHECK(session.state() == core::GameState::CAMPAIGN_EDITOR);
    CHECK(session.campaignEditor().selectedIndex() == 1);
    CHECK(session.campaignEditor().selectedLevel()->path == selectedPath);
}

} // TEST_SUITE
