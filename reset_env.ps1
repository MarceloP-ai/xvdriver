Write-Host "🔄 Limpando variáveis de ambiente Vulkan..."

Remove-Item Env:VK_INSTANCE_LAYERS -ErrorAction SilentlyContinue
Remove-Item Env:VK_LAYER_PATH -ErrorAction SilentlyContinue
Remove-Item Env:VK_LOADER_DEBUG -ErrorAction SilentlyContinue

Write-Host "✅ Ambiente Vulkan limpo"
