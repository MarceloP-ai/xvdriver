$LayerDir = "C:\Users\mr795\OneDrive\Documentos\Projeto\xvdriver\layer"
$JsonPath = Join-Path $LayerDir "xvdriver_layer.json"

Write-Host "📝 Criando xvdriver_layer.json..."

@"
{
    "file_format_version": "1.2.0",
    "layer": {
        "name": "VK_LAYER_XVDRIVER_xvdriver",
        "type": "GLOBAL",
        "library_path": "xvdriver.dll",
        "api_version": "1.3.275",
        "implementation_version": 1,
        "description": "XVDriver Vulkan Global Layer"
    }
}
"@ | Set-Content -Encoding utf8 $JsonPath

Write-Host "✅ JSON criado em:"
Write-Host $JsonPath
