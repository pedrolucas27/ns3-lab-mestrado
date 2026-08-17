# ns-3 Lab Mestrado - Ambiente de Experimentos

Ambiente preparado para desenvolver, compilar e executar simulações com o [ns-3.48](https://www.nsnam.org/) de forma automatizada.

Para entender a organização do repositório e como os componentes se conectam, consulte [docs/estrutura.md](docs/estrutura.md).

## Pré-requisitos

| Ferramenta | Versão mínima |
|------------|---------------|
| g++ ou clang++ | g++ ≥ 11.1 |
| cmake | ≥ 3.25 |
| python3 | ≥ 3.10 |
| ninja ou make | qualquer |

**Ubuntu/Debian:**

```bash
sudo apt update
sudo apt install g++ cmake python3 ninja-build
```

Verifique com:

```bash
make deps
```

## Primeiro uso

Na raiz do projeto:

```bash
# 1. Configurar o ns-3 (só na primeira vez ou após mudanças estruturais)
make configure

# 2. Compilar simulador + experimentos
make build

# 3. Executar o experimento padrão
make run
```

Ou tudo de uma vez após clonar o repositório:

```bash
make configure build run
```

Saída esperada:

```
Experimento hello-simulator executado com sucesso.
```

## Comandos disponíveis

| Comando | Descrição |
|---------|-----------|
| `make deps` | Verifica dependências instaladas |
| `make configure` | Configura o build do ns-3 |
| `make build` | Compila simulador e experimentos |
| `make run` | Compila (se necessário) e executa um experimento |
| `make list` | Lista experimentos em `experiments/` |
| `make clean` | Remove artefatos de compilação |
| `make distclean` | Remove build e cache do CMake |
| `make rebuild` | Reconfigura e recompila do zero |
| `make help` | Mostra todos os comandos |

### Executar um experimento específico

```bash
make run EXP=hello-simulator
```

### Passar argumentos ao experimento

```bash
make run EXP=hello-simulator RUN_ARGS="--help"
```

### Perfil de compilação

Por padrão usa `default` (com asserts e logging). Para simulações longas:

```bash
make configure BUILD_PROFILE=release build
```

Perfis disponíveis: `default`, `debug`, `release`, `optimized`.

### Paralelismo na compilação

```bash
make build JOBS=8
```

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

Para mais detalhes sobre estrutura, criação de experimentos e configuração do ns-3, veja [docs/estrutura.md](docs/estrutura.md).
