# ==============================================================================
#  Vertical Precision Platformer — Makefile
#  Compiler : clang++ (LLVM)  |  Standard : C++20
#  Platforms: Windows (Git Bash / MSYS2) + Linux
#
#  Targets:
#    make tests         — compila e executa testes (silencioso, mostra resumo)
#    make tests-verbose — compila e executa testes (mostra todos os que passam)
#    make game          — compila o binário do jogo (release)
#    make clean         — remove artefactos de build
#    make help          — mostra esta ajuda
# ==============================================================================

# ── Deteção de Plataforma ──────────────────────────────────────────────────────
ifeq ($(OS),Windows_NT)
    PLATFORM := windows
    EXE_EXT  := .exe
else
    PLATFORM := linux
    EXE_EXT  :=
endif

# ── Toolchain ─────────────────────────────────────────────────────────────────
CXX := clang++

# ar compatível com objetos Clang:
#   Linux   → ar do sistema
#   Windows → llvm-ar (incluído com LLVM; se não estiver em PATH, usa ar)
ifeq ($(PLATFORM),windows)
    AR := $(shell command -v llvm-ar > /dev/null 2>&1 && echo llvm-ar || echo ar)
else
    AR := ar
endif

# ── Flags de Compilação ───────────────────────────────────────────────────────
CXXFLAGS_BASE := -std=c++20 -Wall -Wextra -Wpedantic -Wno-unused-parameter

# Debug: sanitizers só em Linux (suporte limitado no Windows com Clang standalone)
ifeq ($(PLATFORM),linux)
    CXXFLAGS_DBG := -g -O0 -DDEBUG -fsanitize=address,undefined -fno-omit-frame-pointer
    LDFLAGS_DBG  := -fsanitize=address,undefined
else
    CXXFLAGS_DBG := -g -O0 -DDEBUG
    LDFLAGS_DBG  :=
endif

CXXFLAGS_REL := -O2 -DNDEBUG

# ── Diretórios ────────────────────────────────────────────────────────────────
GAME_DIR  := Game
TEST_DIR  := Tests
EXT_DIR   := external
BUILD_DIR := build

# ── Includes ──────────────────────────────────────────────────────────────────
INCLUDES := -I$(GAME_DIR) -I$(EXT_DIR)

# ── Vulkan ────────────────────────────────────────────────────────────────────
ifeq ($(PLATFORM),windows)
    INCLUDES    += -I"$(VULKAN_SDK)/Include"
    LDFLAGS_REL += -L"$(VULKAN_SDK)/Lib" -lvulkan-1
    LDFLAGS_DBG += -L"$(VULKAN_SDK)/Lib" -lvulkan-1
else
    CXXFLAGS_BASE += $(shell pkg-config --cflags vulkan 2>/dev/null)
    LDFLAGS_REL   += $(shell pkg-config --libs   vulkan 2>/dev/null)
    LDFLAGS_DBG   += $(shell pkg-config --libs   vulkan 2>/dev/null)
endif

# ── GLFW (Fase 2.3) ───────────────────────────────────────────────────────────
# Detetado automaticamente: se external/glfw/include/GLFW/glfw3.h existir,
# GLFW_AVAILABLE e definido e os testes de janela sao ativados.
# Instalacao: https://www.glfw.org/download.html → extrair para external/glfw/
GLFW_DIR := external/glfw
ifneq ($(wildcard $(GLFW_DIR)/include/GLFW/glfw3.h),)
    CXXFLAGS_BASE += -DGLFW_AVAILABLE
    INCLUDES      += -I$(GLFW_DIR)/include
    ifeq ($(PLATFORM),windows)
        # O Clang no Windows usa o lld-link (MSVC ABI), logo usamos a lib-vc2022
        LDFLAGS_REL += -L$(GLFW_DIR)/lib-vc2022 -lglfw3 -lgdi32 -luser32 -lshell32
        LDFLAGS_DBG += -L$(GLFW_DIR)/lib-vc2022 -lglfw3 -lgdi32 -luser32 -lshell32
    else
        LDFLAGS_REL += $(shell pkg-config --libs glfw3 2>/dev/null)
        LDFLAGS_DBG += $(shell pkg-config --libs glfw3 2>/dev/null)
    endif
endif

# ── Fontes ────────────────────────────────────────────────────────────────────
# Game: todos os .cpp em Game/ e subpastas (exceto main — adicionado à mão)
GAME_SRCS := $(wildcard $(GAME_DIR)/*.cpp)          \
             $(wildcard $(GAME_DIR)/Graphics/*.cpp) \
             $(wildcard $(GAME_DIR)/Logic/*.cpp)

# Tests: test_runner.cpp (define main via doctest) + todos os .cpp em subpastas
TEST_SRCS := $(wildcard $(TEST_DIR)/*.cpp)            \
             $(wildcard $(TEST_DIR)/Unit/*.cpp)        \
             $(wildcard $(TEST_DIR)/Integration/*.cpp) \
             $(wildcard $(TEST_DIR)/System/*.cpp)      \
             $(wildcard $(TEST_DIR)/Regression/*.cpp)  \
             $(wildcard $(TEST_DIR)/Acceptance/*.cpp)

# ── Objects ───────────────────────────────────────────────────────────────────
GAME_OBJS := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(GAME_SRCS))
TEST_OBJS := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(TEST_SRCS))

# ── Binários ──────────────────────────────────────────────────────────────────
GAME_LIB  := $(BUILD_DIR)/libgame.a
GAME_BIN  := $(BUILD_DIR)/game$(EXE_EXT)
TEST_BIN  := $(BUILD_DIR)/tests$(EXE_EXT)

# Quando há código de jogo compilado, os testes linkam também a game library.
# Em Fase 1 (sem código de jogo), TEST_LINK_DEPS fica vazio.
ifneq ($(strip $(GAME_OBJS)),)
    TEST_LINK_DEPS := $(GAME_LIB)
endif

# ── Shaders ───────────────────────────────────────────────────────────────────
GLSLC       := glslc
SHADER_DIR  := Game/Assets/Shaders
SHADER_SRCS := $(wildcard $(SHADER_DIR)/*.vert) $(wildcard $(SHADER_DIR)/*.frag)
SHADER_OBJS := $(patsubst %,%.spv,$(SHADER_SRCS))

# Regra para compilar Shaders GLSL para SPIR-V
$(SHADER_DIR)/%.vert.spv: $(SHADER_DIR)/%.vert
	@echo "[GLSL] $<"
	@$(GLSLC) $< -o $@

$(SHADER_DIR)/%.frag.spv: $(SHADER_DIR)/%.frag
	@echo "[GLSL] $<"
	@$(GLSLC) $< -o $@

# Criar um target isolado para os shaders
.PHONY: shaders
shaders: $(SHADER_OBJS)

# ── Targets Principais ────────────────────────────────────────────────────────
.PHONY: all game tests tests-verbose clean help

all: help

## help  — mostra os targets disponíveis
help:
	@echo ""
	@echo "  Vertical Precision Platformer — sistema de build"
	@echo "  ─────────────────────────────────────────────────"
	@echo "  make tests         compila e executa testes (silencioso)"
	@echo "  make tests-verbose compila e executa testes (detalhado)"
	@echo "  make game          compila o binário do jogo (release)"
	@echo "  make clean         remove a pasta build/"
	@echo "  make help          mostra esta mensagem"
	@echo ""

## tests — compila e corre todos os testes de forma silenciosa (ideal para commits)
tests: shaders $(TEST_BIN)
	@echo ""
	@echo "  ==========================================="
	@echo "  A executar testes..."
	@echo "  ==========================================="
	@./$(TEST_BIN) > build/test_results.txt
	@cat build/test_results.txt
	@echo ""

## tests-fast — corre apenas os testes de Lógica e Matemática (ignora Vulkan/GLFW)
tests-fast: shaders $(TEST_BIN)
	@echo ""
	@echo "  ==========================================="
	@echo "  A executar testes TDD (MUITO RÁPIDO)..."
	@echo "  ==========================================="
	@./$(TEST_BIN) --test-suite-exclude="*Renderer*,*Vulkan*,*Window*,*Swapchain*,*RenderPass*" > build/test_results.txt
	@cat build/test_results.txt
	@echo ""

## tests-verbose — compila e corre testes imprimindo mensagens detalhadas e sucessos
tests-verbose: shaders $(TEST_BIN)
	@echo ""
	@echo "  ==========================================="
	@echo "  A executar testes (modo detalhado)..."
	@echo "  ==========================================="
	@./$(TEST_BIN) --success > build/test_results.txt
	@cat build/test_results.txt
	@echo ""

## game — compila o binário do jogo em modo release
game: $(GAME_BIN)
	@echo "[OK ] Jogo compilado: $(GAME_BIN)"

# ── Regras de Linkagem ────────────────────────────────────────────────────────

$(TEST_BIN): $(TEST_OBJS) $(TEST_LINK_DEPS) | $(BUILD_DIR)
	@echo "[LNK] $(notdir $@)"
	@$(CXX) $(CXXFLAGS_BASE) $(CXXFLAGS_DBG) $(INCLUDES) \
            -o $@ $(TEST_OBJS) $(TEST_LINK_DEPS) $(LDFLAGS_DBG)

$(GAME_BIN): $(GAME_LIB) | $(BUILD_DIR)
	@echo "[LNK] $(notdir $@)"
	@$(CXX) $(CXXFLAGS_BASE) $(CXXFLAGS_REL) $(INCLUDES) \
            -o $@ $^ $(LDFLAGS_REL)

$(GAME_LIB): $(GAME_OBJS) | $(BUILD_DIR)
	@echo "[LIB] $(notdir $@)"
	@$(AR) rcs $@ $^

# ── Regra de Compilação Genérica .cpp → .o ────────────────────────────────────
$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@echo "[CC ] $<"
	@$(CXX) $(CXXFLAGS_BASE) $(CXXFLAGS_DBG) $(INCLUDES) -c $< -o $@

# ── Utilitários ───────────────────────────────────────────────────────────────
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

## clean — remove todos os artefactos de compilação
clean:
	@echo "[CLN] A remover $(BUILD_DIR)/..."
	@rm -rf $(BUILD_DIR)
	@echo "[CLN] Concluído."