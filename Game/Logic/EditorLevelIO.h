#pragma once

#include <string>

namespace logic {
class LevelEditorDocument;

bool saveEditorLevel(const LevelEditorDocument& document,
                     const std::string& path,
                     const std::string& name = "Editor Level");

} // namespace logic
