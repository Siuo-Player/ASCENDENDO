#include "Logic/EditorLevelIO.h"
#include "Logic/LevelEditor.h"

#include <filesystem>
#include <fstream>
#include <iomanip>

namespace logic {

bool saveEditorLevel(const LevelEditorDocument& document,
                     const std::string& path,
                     const std::string& name) {
    try {
        const std::filesystem::path filePath(path);
        if (filePath.has_parent_path())
            std::filesystem::create_directories(filePath.parent_path());

        std::ofstream out(path, std::ios::trunc);
        if (!out.is_open()) return false;

        out << std::fixed << std::setprecision(2);
        out << "NAME " << name << "\n";
        out << "# Gerado pelo Editor de Niveis ASCENDENDO\n";
        out << "# Grid de edicao: " << 4 << " px; area jogavel: 640x360\n";
        out << "PLATFORM 0 0 640 20\n";

        for (const auto& platform : document.platforms()) {
            const AABB& b = platform.bounds;
            out << "PLATFORM "
                << b.min.x << ' ' << b.min.y << ' '
                << b.width() << ' ' << b.height() << "\n";
        }

        if (document.hasFlag() && document.flag()) {
            const AABB& b = *document.flag();
            out << "FLAG "
                << b.min.x << ' ' << b.min.y << ' '
                << b.width() << ' ' << b.height() << "\n";
        }

        return out.good();
    } catch (...) {
        return false;
    }
}

} // namespace logic
