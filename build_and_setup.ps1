# build_and_setup.ps1
# Script para compilar e registrar a Vulkan layer XVDriver no Windows

# Caminhos
$ProjectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$BuildDir = Join-Path $ProjectRoot "build"
$LayerDir = Join-Path $ProjectRoot "layer"

Write-Host "`n=== XVDriver Build & Setup ===`n"

# 1️⃣ Limpar build antigo
if (Test-Path $BuildDir) {
    Write-Host "Limpando pasta build..."
    Remove-Item $BuildDir -Recurse -Force
}
# Criar build
Write-Host "Criando pasta build..."
New-Item -ItemType Directory -Path $BuildDir | Out-Null

# 2️⃣ Gerar build com CMake
Write-Host "`nExecutando CMake..."
Push-Location $BuildDir
cmake .. | Write-Host

# 3️⃣ Compilar a layer
Write-Host "`nCompilando XVDriver..."
cmake --build . | Write-Host

# 4️⃣ Copiar DLL para layer
# Procurar a DLL gerada dentro da pasta de build (Debug/Release)
$DllFile = Get-ChildItem -Path $BuildDir -Recurse -Filter "xvdriver.dll" | Select-Object -First 1

if ($DllFile -eq $null) {
    Write-Error "❌ DLL não encontrada após a compilação!"
    exit 1
}

if (-Not (Test-Path $LayerDir)) {
    Write-Host "Criando pasta layer..."
    New-Item -ItemType Directory -Path $LayerDir | Out-Null
}

$DllTarget = Join-Path $LayerDir "xvdriver.dll"
Write-Host "`nCopiando $($DllFile.FullName) para $DllTarget ..."
Copy-Item -Path $DllFile.FullName -Destination $DllTarget -Force



# 5️⃣ Configurar variável de ambiente para a sessão atual
$Env:VK_LAYER_PATH = $LayerDir
Write-Host "`nVK_LAYER_PATH configurado para:"
Write-Host $Env:VK_LAYER_PATH

# 6️⃣ Testar se a layer foi registrada
Write-Host "`nVerificando XVDriver com vulkaninfo..."
vulkaninfo | findstr XV

Pop-Location

Write-Host "`n=== Build & Setup concluído! ===`n"
