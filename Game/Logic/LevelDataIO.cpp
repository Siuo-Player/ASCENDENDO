#include "Logic/LevelDataIO.h"

#include "Core/Config.h"

#include <fstream>
#include <iomanip>
#include <sstream>

namespace logic {

std::optional<LevelData> LevelDataIO::load(const std::filesystem::path& path) {
    std::ifstream file(path);
    if (!file.is_open()) return std::nullopt;

    LevelData data;
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty() || line.front() == '#') continue;

        std::istringstream input(line);
        std::string type;
        input >> type;

        if (type == "NAME") {
            std::getline(input >> std::ws, data.name);
            continue;
        }

        if (type == "PLATFORM") {
            float x = 0.0f;
            float y = 0.0f;
            float w = 0.0f;
            float h = 0.0f;
            if (!(input >> x >> y >> w >> h)) return std::nullopt;
            data.platforms.push_back({{x, y}, {x + w, y + h}});
            continue;
        }

        if (type == "FLAG") {
            float x = 0.0f;
            float y = 0.0f;
            float w = 0.0f;
            float h = 0.0f;
            if (!(input >> x >> y >> w >> h)) return std::nullopt;
            data.flag = AABB{{x, y}, {x + w, y + h}};
            continue;
        }

        if (type == "SPAWN") {
            if (!(input >> data.spawnPosition.x >> data.spawnPosition.y)) {
                return std::nullopt;
            }
            continue;
        }

        // Preserve the historical permissive grammar: unknown metadata/comments
        // are ignored, while recognized records must be syntactically valid.
    }

    if (data.platforms.empty()) return std::nullopt;

    // Historical .lvl files have no SPAWN record. Keep the previous gameplay
    // default when loading those files so this tranche remains behavior-neutral.
    if (data.spawnPosition == Vec2{}) {
        data.spawnPosition = {config::LOGICAL_WIDTH / 2.0f, 40.0f};
    }

    return data;
}

bool LevelDataIO::save(const LevelData& data,
                       const std::filesystem::path& path) {
    try {
        if (path.has_parent_path()) {
            std::filesystem::create_directories(path.parent_path());
        }

        std::ofstream out(path, std::ios::trunc);
        if (!out.is_open()) return false;

        out << std::fixed << std::setprecision(2);
        out << "NAME " << data.name << '\n';
        out << "# Gerado pelo Editor de Niveis ASCENDENDO\n";
        out << "# Grid de edicao: 4 px; area jogavel: 640x360\n";
        out << "SPAWN " << data.spawnPosition.x << ' ' << data.spawnPosition.y << '\n';

        for (const auto& platform : data.platforms) {
            out << "PLATFORM "
                << platform.min.x << ' ' << platform.min.y << ' '
                << platform.width() << ' ' << platform.height() << '\n';
        }

        if (data.flag) {
            out << "FLAG "
                << data.flag->min.x << ' ' << data.flag->min.y << ' '
                << data.flag->width() << ' ' << data.flag->height() << '\n';
        }

        return out.good();
    } catch (...) {
        return false;
    }
}

} // namespace logic
