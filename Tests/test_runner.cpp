// =============================================================================
//  Tests/test_runner.cpp — Ponto de entrada da bateria de testes
//
//  Este é o ÚNICO ficheiro que deve ter DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN.
//  O doctest gera a função main() automaticamente a partir desta define.
//  Todos os outros ficheiros de teste incluem doctest.h sem esta define.
// =============================================================================

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#ifdef GLFW_AVAILABLE
#include <GLFW/glfw3.h>

namespace {

// Integration tests construct gfx::Window directly, so the test process owns
// GLFW library lifetime once, independently from any individual Window object.
class GlfwTestRuntime {
public:
    GlfwTestRuntime() : initialized_(glfwInit() == GLFW_TRUE) {}

    ~GlfwTestRuntime() {
        if (initialized_) glfwTerminate();
    }

    GlfwTestRuntime(const GlfwTestRuntime&) = delete;
    GlfwTestRuntime& operator=(const GlfwTestRuntime&) = delete;

private:
    bool initialized_ = false;
};

const GlfwTestRuntime g_glfwTestRuntime{};

} // namespace
#endif // GLFW_AVAILABLE
