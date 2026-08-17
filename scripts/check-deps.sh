#!/usr/bin/env bash
set -euo pipefail

errors=0

check_cmd() {
    local cmd="$1"
    local label="$2"
    if command -v "$cmd" >/dev/null 2>&1; then
        printf '  [ok] %-12s %s\n' "$label" "$("$cmd" --version 2>/dev/null | head -1 || "$cmd" -V 2>/dev/null || echo 'instalado')"
    else
        printf '  [!!] %-12s não encontrado\n' "$label"
        errors=$((errors + 1))
    fi
}

echo "Verificando dependências para ns-3.48..."
check_cmd g++ "g++"
check_cmd cmake "cmake"
check_cmd python3 "python3"

if command -v ninja >/dev/null 2>&1; then
    printf '  [ok] %-12s %s\n' "ninja" "$(ninja --version)"
else
    printf '  [ok] %-12s disponível\n' "make"
fi

if [[ "$errors" -gt 0 ]]; then
    echo
    echo "Instale os pacotes faltantes (Ubuntu/Debian):"
    echo "  sudo apt update"
    echo "  sudo apt install g++ cmake python3 ninja-build"
    exit 1
fi

echo
echo "Todas as dependências necessárias estão instaladas."
