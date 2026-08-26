# ==============================================================================
#  Vertical Precision Platformer — Makefile
#  Compiler : clang++ (LLVM)  |  Standard : C++20
#  Platforms: Windows (Git Bash / MSYS2 / PowerShell + GNU Make) + Linux
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
    SHELL    := cmd.exe
    WIN_TEST_RUNNER := Development\\Tools\\run_tests_windows.cmd
    RUN_TEST = call "$(WIN_TEST_RUNNER)"
    CAT_FILE := type
    RM_BUILD = if exist "$(BUILD_DIR)" rmdir /s /q "$(subst /,\\,$(BUILD_DIR))"
    MKDIR_ONE = if not exist "$(subst /,\\,$(1))" mkdir "$(subst /,\\,$(1))"
else
    PLATFORM := linux
    EXE_EXT  :=
    RUN_TEST = ./$(TEST_BIN)
    CAT_FILE := cat
    RM_BUILD = rm -rf "$(BUILD_DIR)"
    MKDIR_ONE = mkdir -p "$(1)"
endif

# ── Toolchain ─────────────────────────────────────────────────────────────────
CXX := clang++

# ar compatível com objetos Clang:
#   Linux   → ar do sistema
#   Windows → llvm-ar (incluído com LLVM)
ifeq ($(PLATFORM),windows)
    AR := llvm-ar
else
    AR := ar
endif

# ── Flags de Compilação ────────────────────────────────────────────────────────
# -MMD -MP: gera ficheiros .d (dependencias) ao lado de cada .o, listando os
# headers do PROJECTO incluidos por esse .cpp (nao os headers de sistema).
CXXFLAGS_BASE := -std=c++20 -Wall -Wextra -Wpedantic -Wno-unused-parameter -MMD -MP

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
GAME_DIR       := Game
TEST_DIR       := Tests
EXT_DIR        := external
BUILD_DIR      := build
GAME_BUILD_DIR := $(BUILD_DIR)/game
TEST_BUILD_DIR := $(BUILD_DIR)/test
TEST_LOG       := $(BUILD_DIR)/test_results.txt

# ── Includes ──────────────────────────────────────────────────────────────────
# Suporta tanto includes relativos ao domínio (Core/..., Logic/...) como
# includes qualificados pela raiz do repositório (Game/Core/..., Game/Logic/...).
INCLUDES := -I. -I$(GAME_DIR) -I$(EXT_DIR)

# ── Vulkan ────────────────────────────────────────────────────────────────────
ifeq ($(PLATFORM),windows)
    INCLUDES    += -I"$(VULKAN_SDK)/Include"
    LDFLAGS_REL += -L"$(VULKAN_SDK)/Lib" -lvulkan-1
    LDFLAGS_DBG += -L"$(VULKAN_SDK)/Lib" -lvulkan-1
else
    CXXFLAGS_BASE += $(shell pkg-config --cflags vulkan 2>/dev/null)
    LDFLAGS_REL   += $(shell pkg-config --libs   vulkan 2>/dev/null)
endif
