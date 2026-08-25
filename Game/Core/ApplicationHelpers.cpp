#include "Core/ApplicationHelpers.h"

#include "Graphics/RenderSnapshot.h"
#include "Logic/Player.h"
#include "Logic/Level.h"

#include <GLFW/glfw3.h>
#include <fstream>

namespace app {

gfx::RenderSnapshot buildRenderSnapshot(const logic::Player& player,
                                        const logic::Level& level) {
    gfx::RenderSnapshot snapshot;
    snapshot.player.bounds = {
        player.position().x,
        player.position().y,
        player.body.width,
        player.body.height
    };
    snapshot.player.facingDirection = player.facingDirection;

    snapshot.platforms.reserve(level.platforms().size());
    for (const auto& platform : level.platforms()) {
        snapshot.platforms.push_back({
            platform.bounds.min.x,
            platform.bounds.min.y,
            platform.bounds.width(),
            platform.bounds.height()
        });
    }

    snapshot.flag.visible = level.hasFlag;
    if (level.hasFlag) {
        snapshot.flag.bounds = {
            level.flagBounds.min.x,
            level.flagBounds.min.y,
            level.flagBounds.width(),
            level.flagBounds.height()
        };
    }

    return snapshot;
}

std::vector<std::string> loadCampaignLevels(const std::string& levelsDir) {
    std::vector<std::string> campaign;
    std::ifstream file(levelsDir + "/campaign.txt");
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (!line.empty() && line[0] != '#') {
            campaign.push_back(levelsDir + "/" + line);
        }
    }
    return campaign;
}

void setMenuTitle(GLFWwindow* window) {
    glfwSetWindowTitle(window, "ASCENDENDO | MENU | A/D navegar  ESPACO confirmar  E editor  Q sair");
}

void setPlayingTitle(GLFWwindow* window) {
    glfwSetWindowTitle(window, "ASCENDENDO | E editor  Q voltar ao menu  ESC pausa");
}

void setEditorTitle(GLFWwindow* window) {
    glfwSetWindowTitle(window, "ASCENDENDO | EDITOR | G STAMP/DRAG  [/] tamanho  ESC voltar");
}

void navigateMenu(int& selection, int delta, int count) {
    if (count <= 0) {
        selection = 0;
        return;
    }
    selection = (selection + delta + count) % count;
}

} // namespace app
