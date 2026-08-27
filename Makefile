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
    WIN_TEST_RUNNER := Development\Tools\run_tests_windows.cmd
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

# Clang targeting the MSVC ABI must use the same dynamic CRT model as the
# Visual Studio-built GLFW library staged by Windows CI. Keep this explicit so
# the executable and the GLFW import/dependency library do not mix CRT modes.
ifeq ($(PLATFORM),windows)
    CXXFLAGS_BASE += /MD
endif

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
GLFW_DIR := external/glfw
ifneq ($(wildcard $(GLFW_DIR)/include/GLFW/glfw3.h),)
    CXXFLAGS_BASE += -DGLFW_AVAILABLE
    INCLUDES      += -I$(GLFW_DIR)/include
    ifeq ($(PLATFORM),windows)
        LDFLAGS_REL += -L$(GLFW_DIR)/lib-vc2022 -lglfw3 -lgdi32 -luser32 -lshell32
        LDFLAGS_DBG += -L$(GLFW_DIR)/lib-vc2022 -lglfw3 -lgdi32 -luser32 -lshell32
    else
        LDFLAGS_REL += $(shell pkg-config --libs glfw3 2>/dev/null)
        LDFLAGS_DBG += $(shell pkg-config --libs glfw3 2>/dev/null)
    endif
endif

# ── Fontes ────────────────────────────────────────────────────────────────────
GAME_SRCS := $(wildcard $(GAME_DIR)/Core/*.cpp)     \
             $(wildcard $(GAME_DIR)/Graphics/*.cpp) \
             $(wildcard $(GAME_DIR)/Logic/*.cpp)

TEST_SRCS := $(wildcard $(TEST_DIR)/*.cpp)            \
             $(wildcard $(TEST_DIR)/Unit/*.cpp)        \
             $(wildcard $(TEST_DIR)/Integration/*.cpp) \
             $(wildcard $(TEST_DIR)/System/*.cpp)      \
             $(wildcard $(TEST_DIR)/Regression/*.cpp)  \
             $(wildcard $(TEST_DIR)/Acceptance/*.cpp)

GAME_MAIN_SRC := main.cpp
GAME_MAIN_OBJ := $(GAME_BUILD_DIR)/main.o

# ── Objects ───────────────────────────────────────────────────────────────────
# Game e testes usam configurações de compilação diferentes. Separar os objetos
# evita que objetos compilados com ASan/UBSan sejam reutilizados pelo binário
# release e elimina mismatches entre compilação e linkagem.
GAME_OBJS := $(patsubst %.cpp,$(GAME_BUILD_DIR)/%.o,$(GAME_SRCS))
TEST_OBJS := $(patsubst %.cpp,$(TEST_BUILD_DIR)/%.o,$(TEST_SRCS))

# ── Dependencias de headers ───────────────────────────────────────────────────
DEPS := $(GAME_OBJS:.o=.d) $(TEST_OBJS:.o=.d) $(GAME_MAIN_OBJ:.o=.d)
-include $(DEPS)

# ── Binários ──────────────────────────────────────────────────────────────────
GAME_LIB  := $(GAME_BUILD_DIR)/libgame.a
GAME_BIN  := $(GAME_BUILD_DIR)/game$(EXE_EXT)
TEST_BIN  := $(TEST_BUILD_DIR)/tests$(EXE_EXT)

ifneq ($(strip $(GAME_OBJS)),)
    TEST_LINK_DEPS := $(GAME_LIB)
endif

# ── Shaders ────────────────────────────────────────────────────────────────────
GLSLC       := glslc
SHADER_DIR  := Game/Assets/Shaders
SHADER_SRCS := $(wildcard $(SHADER_DIR)/*.vert) $(wildcard $(SHADER_DIR)/*.frag)
SHADER_OBJS := $(patsubst %,%.spv,$(SHADER_SRCS))

$(SHADER_DIR)/%.vert.spv: $(SHADER_DIR)/%.vert
	@echo "[GLSL] $<"
	@$(GLSLC) $< -o $@

$(SHADER_DIR)/%.frag.spv: $(SHADER_DIR)/%.frag
	@echo "[GLSL] $<"
	@$(GLSLC) $< -o $@

.PHONY: shaders
shaders: $(SHADER_OBJS)

# ── Targets Principais ────────────────────────────────────────────────────────
.PHONY: all game tests tests-verbose tests-fast clean help

all: help

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
ifeq ($(PLATFORM),windows)
	@$(RUN_TEST) normal
else
	@$(RUN_TEST) > $(TEST_LOG) 2>&1 || ( $(CAT_FILE) $(TEST_LOG) && exit 1 )
	@$(CAT_FILE) $(TEST_LOG)
endif
	@echo ""

## tests-fast — corre apenas os testes de Lógica e Matemática (ignora Vulkan/GLFW)
tests-fast: shaders $(TEST_BIN)
	@echo ""
	@echo "  ==========================================="
	@echo "  A executar testes TDD (MUITO RÁPIDO)..."
	@echo "  ==========================================="
ifeq ($(PLATFORM),windows)
	@$(RUN_TEST) fast
else
	@$(RUN_TEST) --test-suite-exclude="*Renderer*,*Vulkan*,*Window*,*Swapchain*,*RenderPass*" > $(TEST_LOG) 2>&1 || ( $(CAT_FILE) $(TEST_LOG) && exit 1 )
	@$(CAT_FILE) $(TEST_LOG)
endif
	@echo ""

## tests-verbose — compila e corre testes imprimindo mensagens detalhadas e sucessos
tests-verbose: shaders $(TEST_BIN)
	@echo ""
	@echo "  ==========================================="
	@echo "  A executar testes (modo detalhado)..."
	@echo "  ==========================================="
ifeq ($(PLATFORM),windows)
	@$(RUN_TEST) verbose
else
	@$(RUN_TEST) --success > $(TEST_LOG) 2>&1 || ( $(CAT_FILE) $(TEST_LOG) && exit 1 )
	@$(CAT_FILE) $(TEST_LOG)
endif
	@echo ""

game: shaders $(GAME_MAIN_OBJ) $(GAME_BIN)
	@echo "[OK ] Jogo compilado: $(GAME_BIN)"

# ── Regras de Linkagem ────────────────────────────────────────────────────────

$(TEST_BIN): $(TEST_OBJS) $(TEST_LINK_DEPS) | $(BUILD_DIR)
	@echo "[LNK] $(notdir $@)"
	@$(CXX) $(CXXFLAGS_BASE) $(CXXFLAGS_DBG) $(INCLUDES) -o $@ $(TEST_OBJS) $(TEST_LINK_DEPS) $(LDFLAGS_DBG)

$(GAME_BIN): $(GAME_MAIN_OBJ) $(GAME_LIB) | $(BUILD_DIR)
	@echo "[LNK] $(notdir $@)"
	@$(CXX) $(CXXFLAGS_BASE) $(CXXFLAGS_REL) $(INCLUDES) -o $@ $^ $(LDFLAGS_REL)

$(GAME_LIB): $(GAME_OBJS) | $(BUILD_DIR)
	@echo "[LIB] $(notdir $@)"
	@$(AR) rcs $@ $^

# ── Regras de Compilação .cpp → .o ─────────────────────────────────────────────
$(GAME_BUILD_DIR)/%.o: %.cpp
	@$(call MKDIR_ONE,$(dir $@))
	@echo "[CC ] $<"
	@$(CXX) $(CXXFLAGS_BASE) $(CXXFLAGS_REL) $(INCLUDES) -c $< -o $@

$(TEST_BUILD_DIR)/%.o: %.cpp
	@$(call MKDIR_ONE,$(dir $@))
	@echo "[CC ] $<"
	@$(CXX) $(CXXFLAGS_BASE) $(CXXFLAGS_DBG) $(INCLUDES) -c $< -o $@

# ── Utilitários ───────────────────────────────────────────────────────────────
$(BUILD_DIR):
	@$(call MKDIR_ONE,$(BUILD_DIR))

clean:
	@echo "[CLN] A remover $(BUILD_DIR)/..."
	@$(RM_BUILD)
	@echo "[CLN] Concluído."
