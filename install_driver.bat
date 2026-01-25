@echo off
:: Verifica se esta rodando como Administrador
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo [ERRO] Por favor, execute como ADMINISTRADOR.
    pause
    exit /b
)

SET "JSON_PATH=C:\Projeto\xvdriver\build\Release\xvdriver.json"

:: Adiciona ao Registro do Windows como uma Camada Explicita
reg add "HKEY_LOCAL_MACHINE\SOFTWARE\Khronos\Vulkan\ExplicitLayers" /v "%JSON_PATH%" /t REG_DWORD /d 0 /f

echo [SUCESSO] XVDriver registrado no sistema!
echo Agora voce pode abrir qualquer emulador ou jogo Vulkan.
pause