#include "Logic/LevelDataIO.h"
#include "Core/Config.h"

#include <cmath>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace logic {
namespace {

bool finite(float value) {
    return std::isfinite(value);
}

bool validRect(float x, float y, float w, float h) {
    return finite(x) && finite(y) && finite(w) && finite(h) &&
           w > 0.0f && h > 0.0f &&
           x >= 0.0f && y >= 0.0f &&
           x + w <= config::LOGICAL_WIDTH &&
           y + h <= config::LOGICAL_HEIGHT;
}

bool validPoint(float x, float y) {
    return finite(x) && finite(y) &&
           x >= 0.0f && x <= config::LOGICAL_WIDTH &&
           y >= 0.0f && y <= config::LOGICAL_HEIGHT;
}

} // namespace

std::optional<LevelData> LevelDataIO::load(const std::filesystem::path& path) {
    std::ifstream file(path);
    if (!file.is_open()) return std::nullopt;

    LevelData data;
    std::string line;
    bool sawVersion = false;
    int version = 0; // Historical NAME/PLATFORM/FLAG/SPAWN format.

    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty() || line.front() == '#') continue;

        std::istringstream input(line);
        std::string type;
        input >> type;
        if (type.empty()) continue;

        if (type == "VERSION") {
            if (sawVersion) return std::nullopt;
            if (!(input >> version) || input.rdbuf()->in_avail() != 0 || version != CURRENT_VERSION) {
                return std::nullopt;
            }
            sawVersion = true;
            continue;
        }

        if (type == "NAME") {
            std::string value;
            std::getline(input >> std::ws, value);
            if (value.empty()) return std::nullopt;
            data.name = value;
            continue;
        }
        if (type == "PLATFORM") {
            float x = 0.0f, y = 0.0f, w = 0.0f, h = 0.0f;
            if (!(input >> x >> y >> w >> h) || input.rdbuf()->in_avail() != 0 || !validRect(x, y, w, h)) {
                return std::nullopt;
            }
            data.platforms.push_back({{x, y}, {x + w, y + h}});
            continue;
        }
        if (type == "FLAG") {
            float x = 0.0f, y = 0.0f, w = 0.0f, h = 0.0f;
            if (!(input >> x >> y >> w >> h) || input.rdbuf()->in_avail() != 0 || !validRect(x, y, w, h)) {
                return std::nullopt;
            }
            data.flag = AABB{{x, y}, {x + w, y + h}};
            continue;
        }
        if (type == "SPAWN") {
            Vec2 spawn{};
            if (!(input >> spawn.x >> spawn.y) || input.rdbuf()->in_avail() != 0 || !validPoint(spawn.x, spawn.y)) {
                return std::nullopt;
            }
            data.spawnPosition = spawn;
            continue;
        }

        // Unknown non-comment records are not silently ignored. This keeps the
        // declarative boundary closed and prevents future UGC/schema typos from
        // becoming valid-but-partially-parsed content.
        return std::nullopt;
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
        out << "VERSION " << CURRENT_VERSION << '\n';
        out << "NAME " << data.name << '\n';
        out << "# Gerado pelo Editor de Niveis ASCENDENDO\n";
        out << "# Grid de edicao: 4 px; area jogavel: 640x360\n";
        if (data.spawnPosition && validPoint(data.spawnPosition->x, data.spawnPosition->y)) {
            out << "SPAWN " << data.spawnPosition->x << ' '
                << data.spawnPosition->y << '\n';
        }

        for (const auto& platform : data.platforms) {
            out << "PLATFORM "
                << platform.min.x << ' ' << platform.min.y << ' '
                << platform.width() << ' ' << platform.height() << '\n';
        }

        if (data.flag && validRect(data.flag->min.x, data.flag->min.y,
                                  data.flag->width(), data.flag->height())) {
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
