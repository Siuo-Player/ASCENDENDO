@echo off
setlocal

set "TEST_EXE=build\test\tests.exe"
set "TEST_LOG=build\test_results.txt"
set "MODE=%~1"

if not exist "%TEST_EXE%" (
    echo [ERRO] Test executable not found: %TEST_EXE%
    exit /b 2
)

if /I "%MODE%"=="fast" (
    "%TEST_EXE%" --test-suite-exclude="*Renderer*,*Vulkan*,*Window*,*Swapchain*,*RenderPass*" > "%TEST_LOG%" 2>&1
) else if /I "%MODE%"=="verbose" (
    "%TEST_EXE%" --success > "%TEST_LOG%" 2>&1
) else (
    "%TEST_EXE%" > "%TEST_LOG%" 2>&1
)

set "TEST_EXIT=%ERRORLEVEL%"

type "%TEST_LOG%"

exit /b %TEST_EXIT%
