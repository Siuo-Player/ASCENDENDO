#pragma once

#include <string>
#include <vector>

struct GLFWwindow;

namespace gfx { struct RenderSnapshot; }
namespace logic { class Player; class Level; }

namespace app {

gfx::RenderSnapshot buildRenderSnapshot(const logic::Player& player,
                                        const logic::Level& level);

std::vector<std::string> loadCampaignLevels(const std::string& levelsDir);

void setMenuTitle(GLFWwindow* window);
void setPlayingTitle(GLFWwindow* window);
void setEditorTitle(GLFWwindow* window);

void navigateMenu(int& selection, int delta, int count);

} // namespace app
