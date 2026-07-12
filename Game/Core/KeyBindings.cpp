// =============================================================================
//  Game/Core/KeyBindings.cpp
//
//  @version 9.1
// =============================================================================
#include "Core/KeyBindings.h"
#include "Logic/InputManager.h"

#include <fstream>
#include <sstream>
#include <filesystem>
#include <cctype>
#include <algorithm>

namespace core {

namespace Key = logic::Key;
using logic::InputManager;

// ─── Defaults ────────────────────────────────────────────────────────────────
// Espelham EXACTAMENTE o comportamento do jogo antes deste sistema existir:
//   - MoveLeft/MoveRight: os mesmos pares que isLeft()/isRight() ja testam.
//   - Jump: a mesma tecla que Player.cpp consulta via Key::SPACE.
//   - Pause: Key::ESCAPE, tal como o `escPressed` de main.cpp.
//   - UIConfirm/UILeft/UIRight: as mesmas teclas que os menus ja usavam
//     (Key::SPACE / Key::LEFT / Key::RIGHT, verificadas directamente).
//   - OpenEditor/Quit: acoes novas (Fase 9) -- Key::E / Key::Q.
std::unordered_map<GameAction, std::vector<int>> KeyBindings::defaultBindings() {
    return {
        { GameAction::MoveLeft,   { Key::A, Key::LEFT } },
        { GameAction::MoveRight,  { Key::D, Key::RIGHT } },
        { GameAction::Jump,       { Key::SPACE } },
        { GameAction::Pause,      { Key::ESCAPE } },
        { GameAction::UIConfirm,  { Key::SPACE } },
        { GameAction::UILeft,     { Key::LEFT } },
        { GameAction::UIRight,    { Key::RIGHT } },
        { GameAction::OpenEditor, { Key::E } },
        { GameAction::Quit,       { Key::Q } },
    };
}

KeyBindings::KeyBindings() : m_bindings(defaultBindings()) {}

const std::vector<int>& KeyBindings::keysFor(GameAction action) const {
    static const std::vector<int> empty; // so' alcancado se `action` nao existir na tabela (nao deveria acontecer)
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

// ─── Nome <-> Tecla ──────────────────────────────────────────────────────────

namespace {
struct KeyName { int key; const char* name; };
constexpr KeyName KEY_NAMES[] = {
    { Key::LEFT,   "LEFT"   },
    { Key::RIGHT,  "RIGHT"  },
    { Key::UP,     "UP"     },
    { Key::DOWN,   "DOWN"   },
    { Key::A,      "A"      },
    { Key::D,      "D"      },
    { Key::W,      "W"      },
    { Key::S,      "S"      },
    { Key::E,      "E"      },
    { Key::Q,      "Q"      },
    { Key::SPACE,  "SPACE"  },
    { Key::ESCAPE, "ESCAPE" },
};
} // namespace anonimo

std::string keyToString(int key) {
    for (const auto& k : KEY_NAMES) {
        if (k.key == key) return k.name;
    }
    return std::to_string(key); // fallback: codigo GLFW cru, sempre reversivel
}

bool stringToKey(const std::string& s, int& outKey) {
    if (s.empty()) return false;

    for (const auto& k : KEY_NAMES) {
        if (s == k.name) { outKey = k.key; return true; }
    }

    // Fallback numerico: aceita qualquer codigo GLFW valido, mesmo sem nome.
    bool allDigits = std::all_of(s.begin(), s.end(), [](unsigned char c) {
        return std::isdigit(c);
    });
    if (!allDigits) return false;

    try {
        outKey = std::stoi(s);
        return true;
    } catch (...) {
        return false;
    }
}

// ─── Persistência ────────────────────────────────────────────────────────────

bool KeyBindings::saveToFile(const std::string& path) const {
    try {
        std::filesystem::path p(path);
        if (p.has_parent_path())
            std::filesystem::create_directories(p.parent_path());
    } catch (...) {
        return false;
    }

    std::ofstream f(path, std::ios::trunc);
    if (!f.is_open()) return false;

    f << "# controls.cfg -- gerado pelo ASCENDENDO. Formato: ACCAO=TECLA1,TECLA2\n";
    for (GameAction action : ALL_ACTIONS) {
        f << actionSerializedName(action) << "=";
        const auto& keys = keysFor(action);
        for (size_t i = 0; i < keys.size(); ++i) {
            if (i > 0) f << ",";
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
        if (eq == std::string::npos) continue; // linha invalida -- ignora

        std::string actionName = line.substr(0, eq);
        std::string keysStr    = line.substr(eq + 1);

        GameAction action{};
        if (!actionFromSerializedName(actionName.c_str(), action)) continue; // accao desconhecida -- ignora (forward-compat)

        std::vector<int> keys;
        std::istringstream iss(keysStr);
        std::string token;
        while (std::getline(iss, token, ',')) {
            int key = 0;
            if (stringToKey(token, key)) keys.push_back(key);
        }

        if (!keys.empty()) parsed[action] = keys; // linha sem nenhuma tecla valida -- mantem o default
    }

    if (parsed.empty()) return false; // ficheiro existia mas nao continha nada valido

    // Merge: accoes presentes no ficheiro substituem; accoes ausentes mantêm
    // o valor actual (defaults do construtor). Assim um controls.cfg antigo,
    // gravado antes de uma accao nova existir, continua a funcionar.
    for (auto& [action, keys] : parsed) m_bindings[action] = std::move(keys);

    return true;
}

// ─── Adaptadores para InputManager ──────────────────────────────────────────

bool isActionHeld(const KeyBindings& bindings, const InputManager& input, GameAction action) {
    for (int key : bindings.keysFor(action)) {
        if (input.isKeyDown(key)) return true;
    }
    return false;
}

bool isActionJustPressed(const KeyBindings& bindings, const InputManager& input, GameAction action) {
    for (int key : bindings.keysFor(action)) {
        if (input.isKeyJustPressed(key)) return true;
    }
    return false;
}

bool isActionJustReleased(const KeyBindings& bindings, const InputManager& input, GameAction action) {
    for (int key : bindings.keysFor(action)) {
        if (input.isKeyJustReleased(key)) return true;
    }
    return false;
}

} // namespace core
