#include "Logic/LevelDataIO.h"

#include <fstream>
#include <iomanip>
#include <sstream>

namespace logic {

namespace {

bool hasTrailingTokens(std::istringstream& input) {
    std::string extra;
    return static_cast<bool>(input >> extra);
}

} // namespace

std::optional<LevelData> LevelDataIO::load(const std::filesystem::path& path) {
    std::ifstream file(path);
    if (!file.is_open()) return std::nullopt;

    LevelData data;
    bool screensSeen = false;
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
        if (type == "SCREENS") {
            if (screensSeen) return std::nullopt;
            std::size_t count = 0;
            if (!(input >> count) || count == 0 || hasTrailingTokens(input)) return std::nullopt;
            data.screenCount = count;
            screensSeen = true;
            continue;
        }
        if (type == "PLATFORM") {
            float x = 0.0f, y = 0.0f, w = 0.0f, h = 0.0f;
            if (!(input >> x >> y >> w >> h) || hasTrailingTokens(input)) return std::nullopt;
            data.platforms.push_back({{x, y}, {x + w, y + h}});
            continue;
        }
        // SPAWN and FLAG are campaign-derived concepts, not authored level data.
        if (type == "SPAWN" || type == "FLAG") return std::nullopt;

        // The parser is intentionally strict about the current grammar.
        return std::nullopt;
    }

    return data;
}

bool LevelDataIO::save(const LevelData& data,
                       const std::filesystem::path& path) {
    try {
        if (data.screenCount == 0) return false;
        if (path.has_parent_path()) {
            std::filesystem::create_directories(path.parent_path());
        }

        std::ofstream out(path, std::ios::trunc);
        if (!out.is_open()) return false;

        out << std::fixed << std::setprecision(2);
        out << "NAME " << data.name << '\n';
        out << "SCREENS " << data.screenCount << '\n';
        out << "# Gerado pelo Editor de Niveis ASCENDENDO\n";
        out << "# Largura fixa: 640 px; cada tela: 640x360; progressao: vertical ascendente\n";

        for (const auto& platform : data.platforms) {
            out << "PLATFORM "
                << platform.min.x << ' ' << platform.min.y << ' '
                << platform.width() << ' ' << platform.height() << '\n';
        }

        // Ground, spawn and campaign goal are derived by the runtime/editor and are never serialized.
        return out.good();
    } catch (...) {
        return false;
    }
}

} // namespace logic
