#pragma once
// =============================================================================
//  Game/Core/KeyBindings.h
//
//  @version 9.1
//  @history
//    v9.1 — criado. Fase 9.1: Sistema de Configuracao de Controlos.
//
//  Mapeia GameAction -> tecla(s) fisica(s). InputManager::Key e' um
//  namespace de `constexpr int` (codigos GLFW), NAO um enum -- por isso
//  KeyBindings trabalha directamente em `int`, sem qualquer acoplamento a
//  um tipo customizado. Suporta MULTIPLAS teclas por accao (ex: MoveLeft
//  = {A, LEFT}) para preservar o comportamento actual de isLeft()/isRight().
//
//  Persistencia: ficheiro de texto simples "ACCAO=TECLA1,TECLA2" por linha,
//  no mesmo espirito dos .lvl (legivel, git-diffable). Teclas sem nome
//  conhecido (fora da tabela de KEY_NAMES) sao gravadas/lidas como o codigo
//  numerico bruto -- qualquer codigo GLFW valido funciona, mesmo sem nome.
//
//  IMPORTANTE (âmbito desta versao): MoveLeft/MoveRight/Jump existem na
//  tabela de acoes para completude (ex: futuro ecrã CONTROLS listar tudo),
//  mas NAO estao ligadas ao gameplay real ainda -- Player.cpp continua a
//  chamar input.isLeft()/isRight()/isKeyDown(Key::SPACE) directamente
//  (metodos hardcoded dentro do proprio InputManager). Ligar estas tres
//  a KeyBindings exige tocar em Player.cpp, ficheiro testado e estavel --
//  fica pendente de confirmacao explicita. As restantes seis acoes
//  (Pause, UIConfirm, UILeft, UIRight, OpenEditor, Quit) estao TOTALMENTE
//  ligadas via main.cpp nesta versao.
// =============================================================================
#include "Core/GameAction.h"
#include <string>
#include <vector>
#include <unordered_map>

namespace logic { class InputManager; }

namespace core {

class KeyBindings {
public:
    // Constroi ja preenchido com os defaults (== comportamento actual do
    // jogo antes deste sistema existir; ver defaultBindings() no .cpp).
    KeyBindings();

    // Teclas atualmente associadas a uma accao. Nunca vazio para uma accao
    // valida (defaults garantem pelo menos 1 tecla cada).
    const std::vector<int>& keysFor(GameAction action) const;

    // Substitui TODAS as teclas de `action` por uma unica `newKey` (captura
    // "prime uma tecla" simples -- sem UI de adicionar/remover tecla por
    // accao nesta versao).
    void rebind(GameAction action, int newKey);

    void resetToDefault(GameAction action);
    void resetAllToDefaults();

    // Grava no formato "AccaoSerializada=TECLA1,TECLA2\n" por linha.
    bool saveToFile(const std::string& path) const;

    // Le e substitui os bindings a partir do ficheiro. Se o ficheiro nao
    // existir ou nao abrir, devolve false e NAO toca nos bindings actuais
    // (o chamador ja tem os defaults do construtor -- fallback gracioso,
    // mesmo padrao usado em TextPipeline/SpriteRenderer). Linhas invalidas
    // ou accoes desconhecidas sao ignoradas silenciosamente (fowards-
    // compatible com ficheiros de versoes futuras que tenham mais accoes).
    bool loadFromFile(const std::string& path);

private:
    std::unordered_map<GameAction, std::vector<int>> m_bindings;

    static std::unordered_map<GameAction, std::vector<int>> defaultBindings();
};

// ─── Adaptadores para InputManager ──────────────────────────────────────────
// Verdadeiro se QUALQUER tecla associada a `action` satisfizer a consulta
// (OR logico entre teclas, tal como isLeft() ja faz internamente para A/LEFT).
bool isActionHeld(const KeyBindings& bindings, const logic::InputManager& input, GameAction action);
bool isActionJustPressed(const KeyBindings& bindings, const logic::InputManager& input, GameAction action);
bool isActionJustReleased(const KeyBindings& bindings, const logic::InputManager& input, GameAction action);

// ─── Nome <-> Tecla ──────────────────────────────────────────────────────────
// Nome legivel/estavel para teclas conhecidas (ex: "SPACE", "A", "LEFT").
// Teclas sem nome conhecido devolvem o codigo numerico como string (ex: "301").
std::string keyToString(int key);

// Inverso de keyToString -- aceita tanto nomes conhecidos como numeros crus.
// Devolve false se `s` nao for nem um nome conhecido nem um inteiro valido.
bool stringToKey(const std::string& s, int& outKey);

} // namespace core
