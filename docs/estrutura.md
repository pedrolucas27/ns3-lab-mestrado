# Estrutura e funcionamento do projeto

Este documento descreve a organização do repositório e como os componentes se conectam para compilar e executar simulações com o [ns-3.48](https://www.nsnam.org/).

## Estrutura de diretórios

```
ns3-lab-mestrado/
├── Makefile              # Automação de configure, build e run
├── README.md             # Instruções de compilação e execução
├── docs/
│   └── estrutura.md      # Este documento
├── experiments/          # Seus programas de simulação (.cc)
│   └── hello-simulator.cc
├── scripts/
│   └── check-deps.sh     # Verifica dependências do sistema
├── results/              # Saídas de experimentos (ignorado pelo git)
└── ns-3.48/              # Código-fonte do simulador
    ├── .ns3rc            # Config mínima (sem exemplos/testes)
    ├── build/            # Binários compilados (ignorado pelo git)
    ├── cmake-cache/      # Cache do CMake (ignorado pelo git)
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

Isso permite:

- Manter o código dos experimentos versionado na raiz do repositório, separado do código-fonte do ns-3.
- O ns-3 detectar automaticamente cada `.cc` com função `main()` em `scratch/experiments/` e gerar um executável independente.
- Subpastas também funcionam (ex.: `experiments/rede/wifi-test.cc` → `make run EXP=rede/wifi-test`).

### Makefile - variáveis principais

| Variável | Padrão | Descrição |
|----------|--------|-----------|
| `BUILD_PROFILE` | `default` | Perfil de compilação (`default`, `debug`, `release`, `optimized`) |
| `JOBS` | `0` | Jobs paralelos na compilação (`0` = automático) |
| `EXP` | `hello-simulator` | Nome do experimento a executar |
| `RUN_ARGS` | *(vazio)* | Argumentos repassados ao executável |

### Script de dependências

`scripts/check-deps.sh` verifica a presença de `g++`, `cmake`, `python3` e `ninja` (ou `make`) antes de configurar o ns-3. É invocado automaticamente por `make configure` e também pode ser chamado diretamente com `make deps`.

## Criar novos experimentos

1. Crie um arquivo `.cc` em `experiments/` com função `main()`:

```bash
cp experiments/hello-simulator.cc experiments/meu-experimento.cc
```

2. Edite o código conforme necessário.

3. Compile e execute:

```bash
make run EXP=meu-experimento
```

Cada arquivo `.cc` com `main()` vira um executável independente. O template `hello-simulator.cc` demonstra o esqueleto mínimo: inclusão de módulos, `CommandLine`, `Simulator::Run()` e `Simulator::Destroy()`.

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

Documentação oficial:

- [Guia de instalação](https://www.nsnam.org/docs/release/3.48/installation/html/quick-start.html)
- [Tutorial](https://www.nsnam.org/docs/tutorial/html/index.html)
- [Manual CMake](https://www.nsnam.org/docs/manual/html/working-with-cmake.html)

## Configuração mínima (.ns3rc)

O arquivo `ns-3.48/.ns3rc` desabilita exemplos e testes do ns-3 para acelerar a compilação. Todos os módulos padrão permanecem habilitados.

Para habilitar exemplos ou testes:

```bash
cd ns-3.48
./ns3 configure --enable-examples --enable-tests
```

## Notas

- **Logs (`NS_LOG_*`)**: desabilitados no perfil `release`. Use `std::cout` para saída garantida ou compile com `BUILD_PROFILE=default`.
- **Artefatos de build**: ficam em `ns-3.48/build/` e `ns-3.48/cmake-cache/` (ignorados pelo git).
- **Resultados**: salve traces, PCAPs etc. em `results/` (também ignorado pelo git).

## Solução de problemas

**Erro de dependências:**

```bash
make deps
```

**Build inconsistente após mudanças:**

```bash
make rebuild
```

**Experimento não encontrado:**

```bash
make list
make build
```

**Reconfigurar após adicionar módulo ou subpasta com CMakeLists.txt:**

```bash
make configure build
```
