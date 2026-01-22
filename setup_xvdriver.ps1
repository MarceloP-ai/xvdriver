Write-Host "🧹 Limpando registros e variáveis antigas..."
Remove-Item Env:VK_LAYER_PATH -ErrorAction SilentlyContinue
Remove-Item Env:VK_INSTANCE_LAYERS -ErrorAction SilentlyContinue

$projectRoot = "C:\Projeto\xvdriver"
$binDir = Join-Path $projectRoot "bin"
$jsonPath = Join-Path $binDir "xvdriver_layer.json"

# Criando o JSON robusto
$jsonContent = @"
{
  "file_format_version": "1.0.0",
  "layer": {
    "name": "VK_LAYER_XVDRIVER",
    "type": "GLOBAL",
    "library_path": "C:\\Projeto\\xvdriver\\bin\\xvdriver.dll",
    "api_version": "1.3.0",
    "implementation_version": "1",
    "description": "XVDriver HUD",
    "functions": {
      "vkGetInstanceProcAddr": "vkGetInstanceProcAddr",
      "vkGetDeviceProcAddr": "vkGetDeviceProcAddr"
    }
  }
}
"@

# Salva o JSON
Set-Content -Path $jsonPath -Value $jsonContent -Encoding ASCII

# Limpa registros antigos do OneDrive e do Sistema
reg delete "HKCU\Software\Khronos\Vulkan\ImplicitLayers" /f 2>$null
reg delete "HKCU\Software\Khronos\Vulkan\ExplicitLayers" /f 2>$null

# Registra a nova localização (User level é melhor para testes)
reg add "HKCU\Software\Khronos\Vulkan\ImplicitLayers" /v "$jsonPath" /t REG_DWORD /d 0 /f

Write-Host "✅ Layer registrada em: $jsonPath"
Write-Host "🚀 Testando com vkcube..."
vkcube