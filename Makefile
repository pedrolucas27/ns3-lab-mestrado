# Diretório raiz do ns-3
NS3_DIR := ns-3.48

# Perfil de compilação: default | debug | release | optimized
BUILD_PROFILE ?= default

# Número de jobs paralelos (0 = automático)
JOBS ?= 0

# Experimento padrão para `make run`
EXP ?= hello-simulator

# Argumentos repassados ao executar um experimento
RUN_ARGS ?=

NS3 := ./$(NS3_DIR)/ns3

.PHONY: help deps configure build rebuild run list clean distclean check

help:
	@echo "Comandos disponíveis:"
	@echo "  make deps       - verifica dependências do sistema"
	@echo "  make configure  - configura o ns-3 (primeira vez ou após mudanças)"
	@echo "  make build      - compila o simulador e os experimentos"
	@echo "  make rebuild    - recompila do zero"
	@echo "  make run        - executa um experimento (EXP=nome, RUN_ARGS='...')"
	@echo "  make list       - lista experimentos em experiments/"
	@echo "  make clean      - remove artefatos de compilação"
	@echo "  make distclean  - remove build e cache do CMake"
	@echo ""
	@echo "Exemplos:"
	@echo "  make configure build"
	@echo "  make run"
	@echo "  make run EXP=hello-simulator RUN_ARGS='--help'"

deps:
	@./scripts/check-deps.sh

configure: deps
	$(NS3) configure -d $(BUILD_PROFILE)

build:
	$(NS3) build -j $(JOBS)

rebuild: distclean configure build

run: build
	$(NS3) run experiments/$(EXP) -- $(RUN_ARGS)

list:
	@echo "Experimentos disponíveis:"
	@ls -1 experiments/*.cc 2>/dev/null | xargs -n1 basename | sed 's/\.cc$$//' | sed 's/^/  - /'

clean:
	$(NS3) clean

distclean:
	$(NS3) distclean

check: build run
