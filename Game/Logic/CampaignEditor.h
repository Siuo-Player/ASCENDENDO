#pragma once
// =============================================================================
// Game/Logic/CampaignEditor.h
// =============================================================================
// Modelo do editor de campanha. Cada nível mantém a sua geometria original
// 640x360; apenas a representação da campanha é comprimida e posicionada
// verticalmente para facilitar navegação, ordenação e teste de transições.

#include <cstddef>
#include <string>
#include <vector>

namespace logic {

struct CampaignLevelBlock {
    std::string path;
    std::string name;
    std::size_t order = 0;
    float topY = 0.0f;
    bool selected = false;
};

struct CampaignValidationResult {
    bool valid = false;
    std::string message;
};

struct CampaignSaveResult {
    bool success = false;
    std::string path;
    std::string message;
};

class CampaignEditorDocument {
public:
    static constexpr float LEVEL_LOGICAL_WIDTH = 640.0f;
    static constexpr float LEVEL_LOGICAL_HEIGHT = 360.0f;
    static constexpr float PREVIEW_WIDTH = 192.0f;
    static constexpr float PREVIEW_HEIGHT = 108.0f;
    static constexpr float PREVIEW_GAP = 16.0f;

    bool loadFromCampaignFile(const std::string& campaignPath);
    bool moveLevel(std::size_t index, std::size_t newOrder);

    CampaignValidationResult validateCampaign() const;
    CampaignSaveResult saveToCampaignFile(const std::string& campaignPath) const;

    const std::vector<CampaignLevelBlock>& levels() const { return m_levels; }
    std::vector<CampaignLevelBlock>& levels() { return m_levels; }
    std::size_t levelCount() const { return m_levels.size(); }
    std::size_t selectedIndex() const { return m_selectedIndex; }
    void select(std::size_t index);

    float contentHeight() const;
    float topYForOrder(std::size_t order) const;

private:
    void rebuildPositions();

    std::vector<CampaignLevelBlock> m_levels;
    std::size_t m_selectedIndex = static_cast<std::size_t>(-1);
};

} // namespace logic
