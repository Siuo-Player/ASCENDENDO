// =============================================================================
//  Game/Core/KeyBindings.cpp
// =============================================================================
#include "Core/KeyBindings.h"
#include "Logic/InputManager.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace core {

namespace Key = logic::Key;
using logic::InputManager;

std::unordered_map<GameAction, std::vector<int>> KeyBindings::defaultBindings() {
    return {
        { GameAction::MoveLeft,         { Key::A, Key::LEFT } },
        { GameAction::MoveRight,        { Key::D, Key::RIGHT } },
        { GameAction::Jump,             { Key::SPACE } },
        { GameAction::Pause,            { Key::ESCAPE } },
        { GameAction::UIConfirm,        { Key::SPACE } },
        { GameAction::UILeft,           { Key::LEFT } },
        { GameAction::UIRight,          { Key::RIGHT } },
        { GameAction::OpenEditor,       { Key::E } },
        { GameAction::Quit,             { Key::Q } },
        { GameAction::EditorPanUp,      { Key::W } },
        { GameAction::EditorPanDown,    { Key::S } },
        { GameAction::EditorToggleMode, { Key::G } },
        { GameAction::EditorSizeDown,   { Key::LBRACKET } },
        { GameAction::EditorSizeUp,     { Key::RBRACKET } },
        { GameAction::DeleteSelection,  { Key::DELETE_KEY, Key::BACKSPACE } },
    };
}

KeyBindings::KeyBindings() : m_bindings(defaultBindings()) {}

const std::vector<int>& KeyBindings::keysFor(GameAction action) const {
    static const std::vector<int> empty;
    auto it = m_bindings.find(action);
    return it != m_bindings.end() ? it->second : empty;
}

void KeyBindings::rebind(GameAction action, int newKey) {
    m_bindings[action] = { newKey };
}

void KeyBindings::resetToDefault(GameAction action) {
    auto defaults = defaultBindings();
    auto it = defaults.find(action);
    if (it != defaults.end()) m_bindings[action] = it->second;
}

void KeyBindings::resetAllToDefaults() {
    m_bindings = defaultBindings();
}

namespace {
struct KeyName { int key; const char* name; };
constexpr KeyName KEY_NAMES[] = {
    { Key::LEFT, "LEFT" }, { Key::RIGHT, "RIGHT" },
    { Key::UP, "UP" }, { Key::DOWN, "DOWN" },
    { Key::A, "A" }, { Key::D, "D" }, { Key::W, "W" }, { Key::S, "S" },
    { Key::E, "E" }, { Key::Q, "Q" }, { Key::G, "G" },
    { Key::LBRACKET, "LBRACKET" }, { Key::RBRACKET, "RBRACKET" },
    { Key::DELETE_KEY, "DELETE" }, { Key::BACKSPACE, "BACKSPACE" },
    { Key::SPACE, "SPACE" }, { Key::ESCAPE, "ESCAPE" },
};
}

std::string keyToString(int key) {
    for (const auto& k : KEY_NAMES) if (k.key == key) return k.name;
    return std::to_string(key);
}

bool stringToKey(const std::string& s, int& outKey) {
    if (s.empty()) return false;
    for (const auto& k : KEY_NAMES) {
        if (s == k.name) { outKey = k.key; return true; }
    }
    if (!std::all_of(s.begin(), s.end(), [](unsigned char c){ return std::isdigit(c); })) return false;
    try {
        outKey = std::stoi(s);
        return true;
    } catch (...) {
        return false;
    }
}

bool KeyBindings::saveToFile(const std::string& path) const {
    try {
        std::filesystem::path p(path);
        if (p.has_parent_path()) std::filesystem::create_directories(p.parent_path());
    } catch (...) {
        return false;
    }

    std::ofstream f(path, std::ios::trunc);
    if (!f.is_open()) return false;

    f << "# controls.cfg -- ACCAO=TECLA1,TECLA2\n";
    for (GameAction action : ALL_ACTIONS) {
        f << actionSerializedName(action) << "=";
        const auto& keys = keysFor(action);
        for (size_t i = 0; i < keys.size(); ++i) {
            if (i) f << ",";
            f << keyToString(keys[i]);
        }
        f << "\n";
    }
    return true;
}

bool KeyBindings::loadFromFile(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) return false;

    std::unordered_map<GameAction, std::vector<int>> parsed;
    std::string line;
    while (std::getline(f, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty() || line[0] == '#') continue;

        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;

        GameAction action{};
        if (!actionFromSerializedName(line.substr(0, eq).c_str(), action)) continue;

        std::vector<int> keys;
        std::istringstream iss(line.substr(eq + 1));
        std::string token;
        while (std::getline(iss, token, ',')) {
            int key = 0;
            if (stringToKey(token, key)) keys.push_back(key);
        }
        if (!keys.empty()) parsed[action] = std::move(keys);
    }

    if (parsed.empty()) return false;
    for (auto& [action, keys] : parsed) m_bindings[action] = std::move(keys);
    return true;
}

bool isActionHeld(const KeyBindings& bindings, const InputManager& input, GameAction action) {
    for (int key : bindings.keysFor(action)) if (input.isKeyDown(key)) return true;
    return false;
}

bool isActionJustPressed(const KeyBindings& bindings, const InputManager& input, GameAction action) {
    for (int key : bindings.keysFor(action)) if (input.isKeyJustPressed(key)) return true;
    return false;
}

bool isActionJustReleased(const KeyBindings& bindings, const InputManager& input, GameAction action) {
    for (int key : bindings.keysFor(action)) if (input.isKeyJustReleased(key)) return true;
    return false;
}

} // namespace core
