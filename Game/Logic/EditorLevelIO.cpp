#include "Logic/EditorLevelIO.h"
#include "Logic/LevelEditor.h"
#include "Logic/LevelDataIO.h"

namespace logic {

bool saveEditorLevel(const LevelEditorDocument& document,
                     const std::string& path,
                     const std::string& name) {
    return LevelDataIO::save(document.toLevelData(name), path);
}

} // namespace logic
