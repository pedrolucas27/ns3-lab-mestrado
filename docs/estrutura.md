# Estrutura e funcionamento do projeto

Este documento descreve a organização do repositório e como os componentes se conectam para compilar e executar simulações com o [ns-3.48](https://www.nsnam.org/).

## Estrutura de diretórios

```
ns3-lab-mestrado/
├── Makefile              # Automação de configure, build e run
├── README.md             # Instruções de compilação e execução
├── docs/
│   └── estrutura.md      # Este documento
├── experiments/          # Seus programas de simulação
│   ├── CMakeLists.txt    # Um executável por .cc
│   ├── hello-simulator.cc
├── scripts/
│   └── check-deps.sh     # Verifica dependências do sistema
├── results/              # Saídas de experimentos 
└── ns-3.48/              # Código-fonte do simulador
    ├── .ns3rc            # Config mínima (sem exemplos/testes)
    ├── build/            # Binários compilados 
    ├── cmake-cache/      # Cache do CMake 
    └── scratch/
        └── experiments → ../../experiments  (symlink)
```

## Como funciona

### Fluxo de trabalho

O `Makefile` na raiz do projeto encapsula os comandos do ns-3 e padroniza o fluxo:

```
make configure  →  make build  →  make run
     │                  │               │
     ▼                  ▼               ▼
  ./ns3 configure   ./ns3 build    ./ns3 run experiments/<EXP>
```

1. **`make configure`** - executa `scripts/check-deps.sh` e configura o build do ns-3 com o perfil escolhido (`BUILD_PROFILE`, padrão `default`).
2. **`make build`** - compila o simulador e todos os experimentos detectados em `experiments/`.
3. **`make run`** - compila (se necessário) e executa o experimento indicado por `EXP` (padrão: `hello-simulator`).

### Integração com o ns-3 via symlink

Os arquivos em `experiments/` **não** ficam dentro do diretório `ns-3.48/`. Em vez disso, um symlink conecta:

```
ns-3.48/scratch/experiments  →  ../../experiments
```

Isso permite manter o código dos experimentos versionado na raiz, separado do ns-3.

Subpastas funcionam: `experiments/rede/wifi-test.cc` → `make run EXP=rede/wifi-test`.

### Makefile - variáveis principais

| Variável | Padrão | Descrição |
|----------|--------|-----------|
| `BUILD_PROFILE` | `default` | Perfil de compilação (`default`, `debug`, `release`, `optimized`) |
| `JOBS` | `0` | Jobs paralelos na compilação (`0` = automático) |
| `EXP` | `hello-simulator` | Nome do experimento a executar |
| `RUN_ARGS` | *(vazio)* | Argumentos repassados ao executável |

## Criar novos experimentos

1. Crie um arquivo `.cc` em `experiments/` com função `main()`:

```bash
cp experiments/hello-simulator.cc experiments/meu-experimento.cc
```

3. Compile e execute (o `make run` já recompila se o `.cc` mudou):

```bash
make run EXP=meu-experimento
```

O `EXP` é o nome do arquivo **sem** `.cc`. Subpastas: `experiments/rede/wifi-test.cc` → `make run EXP=rede/wifi-test`.

Cada arquivo `.cc` com `main()` vira um executável independente.

## Uso direto do ns3 (opcional)

É possível invocar o ns-3 sem o `Makefile`:

```bash
cd ns-3.48

# Configurar
./ns3 configure -d default

# Compilar
./ns3 build

# Executar experimento
./ns3 run experiments/hello-simulator

# Executar sem recompilar
./ns3 run experiments/hello-simulator --no-build

# Depurar com gdb
./ns3 run experiments/hello-simulator --gdb
```