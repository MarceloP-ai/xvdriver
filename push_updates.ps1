Write-Host "🚀 Iniciando atualização do GitHub..." -ForegroundColor Cyan

# 1. Configurar identidade (caso não esteja configurada)
git config user.email "seu-email@exemplo.com"
git config user.name "Marcelo"

# 2. Adicionar todos os arquivos de código
git add CMakeLists.txt xvdriver.def
git add src/*.cpp
git add src/*.h
git add include/*.h

# 3. Forçar a inclusão do JSON da layer (mesmo que esteja no .gitignore)
git add -f bin/xvdriver_layer.json

# 4. Criar o commit com data e hora para rastreio
$data = Get-Date -Format "dd/MM/yyyy HH:mm"
git commit -m "Update: $data - Correções de hooks e estrutura"

# 5. Enviar para a branch main
Write-Host "📤 Enviando para o servidor..." -ForegroundColor Yellow
git push origin main

if ($LASTEXITCODE -eq 0) {
    Write-Host "✅ GitHub atualizado com sucesso em $data!" -ForegroundColor Green
} else {
    Write-Host "❌ Erro ao enviar para o GitHub. Verifique sua conexão ou permissões." -ForegroundColor Red
}