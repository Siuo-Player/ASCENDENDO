#include "Logic/EditorLevelIO.h"
#include "Logic/LevelEditor.h"
#include "Logic/LevelDataIO.h"

#include <filesystem>
#include <system_error>

namespace logic {

bool saveEditorLevel(const LevelEditorDocument& document,
                     const std::string& path,
                     const std::string& name) {
    const std::filesystem::path destination(path);
    if (destination.empty()) return false;

    const std::filesystem::path temporary =
        destination.parent_path() /
        (destination.filename().string() + ".tmp-save");

    std::error_code ec;
    std::filesystem::remove(temporary, ec);

    if (!LevelDataIO::save(document.toLevelData(name), temporary)) {
        std::filesystem::remove(temporary, ec);
        return false;
    }

    std::filesystem::rename(temporary, destination, ec);
    if (!ec) return true;

    // Some platforms refuse an overwrite rename. Replace the destination only
    // after the complete temporary file exists, never after truncating it.
    std::filesystem::remove(destination, ec);
    if (ec) {
        std::filesystem::remove(temporary, ec);
        return false;
    }

    ec.clear();
    std::filesystem::rename(temporary, destination, ec);
    if (ec) {
        std::filesystem::remove(temporary, ec);
        return false;
    }
    return true;
}

} // namespace logic
