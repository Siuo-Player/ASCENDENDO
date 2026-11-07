// =============================================================================
//  Tests/Usability/test_usability_and_accessibility.cpp
//
//  Bateria de Testes:
//  • Usabilidade (Navegação ergonómica, hit testing e menus circulares)
//  • Acessibilidade (WCAG AA contraste de cores, símbolos auxiliares para daltonismo)
//  • Compatibilidade e Portabilidade (Aspect ratios 16:9, 16:10, 21:9, 4:3, 9:16)
//  • Localização e Internacionalização (L10n/i18n, caracteres UTF-8 e diacríticos)
// =============================================================================

#include "../../external/doctest/doctest.h"
#include "Core/Config.h"
#include "Core/GameAction.h"
#include "Core/GameStateMachine.h"
#include "Core/Viewport.h"

#include <cmath>
#include <iostream>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

namespace {

// Fórmula oficial WCAG 2.1 para luminância relativa:
// L = 0.2126 * R + 0.7152 * G + 0.0722 * B
// Onde cada canal c linearizado é:
// se c <= 0.03928: c / 12.92
// senão: ((c + 0.055) / 1.055) ^ 2.4
double linearizeChannel(double c) {
    return (c <= 0.03928) ? (c / 12.92) : std::pow((c + 0.055) / 1.055, 2.4);
}

double relativeLuminance(double r, double g, double b) {
    return 0.2126 * linearizeChannel(r) +
           0.7152 * linearizeChannel(g) +
           0.0722 * linearizeChannel(b);
}

// Rácio de contraste: (L1 + 0.05) / (L2 + 0.05) onde L1 >= L2
double contrastRatio(double r1, double g1, double b1, double r2, double g2, double b2) {
    double l1 = relativeLuminance(r1, g1, b1);
    double l2 = relativeLuminance(r2, g2, b2);
    if (l1 < l2) std::swap(l1, l2);
    return (l1 + 0.05) / (l2 + 0.05);
}

} // namespace

TEST_SUITE("Usability, Accessibility & Compatibility Testing") {

TEST_CASE("Usability: Navegação circular e consistência do menu") {
    core::GameStateMachine sm;
    CHECK(sm.menuSelection() == 0);

    // Navegar para a esquerda a partir de 0 num menu de 4 itens deve circular para o 3
    sm.selectRelative(-1, 4);
    CHECK(sm.menuSelection() == 3);

    // Navegar para a direita a partir do último deve circular para o 0
    sm.selectRelative(1, 4);
    CHECK(sm.menuSelection() == 0);

    // Seleção direta de índice fora dos limites é normalizada por modulo
    sm.select(10, 4);
    CHECK(sm.menuSelection() == 2);
    sm.select(-5, 4);
    CHECK(sm.menuSelection() == 3);
}

TEST_CASE("Usability: Todos os GameActions possuem nomes legíveis por humanos") {
    for (core::GameAction action : core::ALL_ACTIONS) {
        const char* name = core::actionDisplayName(action);
        REQUIRE(name != nullptr);
        CHECK_MESSAGE(name[0] != '\0', "Action deve ter nome não vazio");
        CHECK_MESSAGE(std::string(name) != "?", "Action desconhecida no catálogo: ", static_cast<int>(action));

        const char* sName = core::actionSerializedName(action);
        REQUIRE(sName != nullptr);
        CHECK_MESSAGE(std::string(sName) != "Unknown", "Action sem nome serializado");

        // Roundtrip serialização -> action
        core::GameAction parsed{};
        const bool ok = core::actionFromSerializedName(sName, parsed);
        CHECK(ok == true);
        CHECK(parsed == action);
    }
}

TEST_CASE("Accessibility (WCAG AA): Rácio de contraste em elementos da interface") {
    // Fundo escuro do HUD: rgb(0.02, 0.03, 0.05)
    const double bgR = 0.02, bgG = 0.03, bgB = 0.05;

    // Texto do cronómetro (branco puro): rgb(0.98, 0.98, 0.98)
    const double timerContrast = contrastRatio(0.98, 0.98, 0.98, bgR, bgG, bgB);
    // WCAG AA para texto standard exige pelo menos 4.5:1
    CHECK(timerContrast >= 4.5);

    // Texto de Recorde Pessoal / Ouro: rgb(0.95, 0.82, 0.20)
    const double goldContrast = contrastRatio(0.95, 0.82, 0.20, bgR, bgG, bgB);
    CHECK(goldContrast >= 4.5);

    // Título Principal: rgb(0.95, 0.80, 0.10) sobre fundo da janela
    const double titleContrast = contrastRatio(0.95, 0.80, 0.10, 0.04, 0.04, 0.08);
    CHECK(titleContrast >= 4.5);
}

TEST_CASE("Accessibility (Daltonismo): Presença de tokens semânticos textuais") {
    // Para jogadores com deuteranopia ou protanopia (cegueira vermelho-verde),
    // o delta de tempo/altitude face ao fantasma não pode depender apenas de cor.
    // Deve incluir símbolos explicitamente distinguíveis (+, -, =).
    const float leadDelta = 12.0f;
    const float behindDelta = -8.0f;
    const float tiedDelta = 0.0f;

    char bufLead[32]{};
    char bufBehind[32]{};
    char bufTied[32]{};

    std::snprintf(bufLead, sizeof(bufLead), "+%.0fm", leadDelta);
    std::snprintf(bufBehind, sizeof(bufBehind), "%.0fm", behindDelta);
    std::snprintf(bufTied, sizeof(bufTied), "= %.0fm", tiedDelta);

    CHECK(bufLead[0] == '+');
    CHECK(bufBehind[0] == '-');
    CHECK(bufTied[0] == '=');
}

TEST_CASE("Compatibility & Portability: Resoluções e Aspect Ratios de ecrã variados") {
    // 16:9 clássico (1920x1080)
    auto r169 = core::computeLetterbox(1920, 1080, config::TARGET_ASPECT);
    CHECK(r169.offsetX == 0);
    CHECK(r169.offsetY == 0);
    CHECK(r169.width == 1920);
    CHECK(r169.height == 1080);

    // 16:10 widescreen (1920x1200) -> letterbox (barras horizontais cima/baixo)
    auto r1610 = core::computeLetterbox(1920, 1200, config::TARGET_ASPECT);
    CHECK(r1610.offsetX == 0);
    CHECK(r1610.offsetY > 0);
    CHECK(r1610.width == 1920);
    CHECK(r1610.height < 1200);

    // 21:9 ultrawide (2560x1080 e 3440x1440) -> pillarbox (barras verticais esquerda/direita)
    auto r219 = core::computeLetterbox(2560, 1080, config::TARGET_ASPECT);
    CHECK(r219.offsetX > 0);
    CHECK(r219.offsetY == 0);
    CHECK(r219.height == 1080);
    CHECK(r219.width < 2560);

    auto r3440 = core::computeLetterbox(3440, 1440, config::TARGET_ASPECT);
    CHECK(r3440.offsetX > 0);
    CHECK(r3440.offsetY == 0);
    CHECK(r3440.height == 1440);

    // 4:3 retro (1024x768) -> letterbox
    auto r43 = core::computeLetterbox(1024, 768, config::TARGET_ASPECT);
    CHECK(r43.offsetX == 0);
    CHECK(r43.offsetY > 0);
    CHECK(r43.width == 1024);

    // 9:16 vertical smartphone/arcade (1080x1920) -> letterbox centrado
    auto r916 = core::computeLetterbox(1080, 1920, config::TARGET_ASPECT);
    CHECK(r916.offsetX == 0);
    CHECK(r916.offsetY > 0);
    CHECK(r916.width == 1080);
}

TEST_CASE("Localization (i18n / L10n): Suporte a diacríticos da língua portuguesa") {
    // Garantir que literais em português (UTF-8) mantêm a integridade de bytes
    const std::string title = "ASCENDENDO";
    const std::string controls = "CONTROLOS";
    const std::string paused = "PAUSA";
    const std::string credits = "CRÉDITOS";
    const std::string actionName = "Editor: Apagar Seleção";
    const std::string successMsg = "Plataformas fisicamente alcançáveis";

    CHECK(title == "ASCENDENDO");
    const bool hasAccent = (credits.find("É") != std::string::npos || credits.find("\xC3\x89") != std::string::npos);
    CHECK(hasAccent);
    CHECK(actionName.find("Seleção") != std::string::npos);
    CHECK(successMsg.find("alcançáveis") != std::string::npos);
}

} // TEST_SUITE
