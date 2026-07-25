# ====================================================================
#              Makefile Puente Principal - CPU MILO
# ====================================================================

# Ejecución por defecto: compila absolutamente todos los tests del proyecto
all: tests compilacion

# --------------------------------------------------------------------
# 🧪 Grupo 1: Tests del Compilador (Lexer y Parser)
# --------------------------------------------------------------------
tests:
	make -f makefiles/Makefile.tests

test_lexer:
	make -f makefiles/Makefile.tests test_lexer

test_parser:
	make -f makefiles/Makefile.tests test_parser


# --------------------------------------------------------------------
# ⚙️ Grupo 2: Tests de Compilación a ROM Física
# --------------------------------------------------------------------
compilacion:
	make -f makefiles/Makefile.compilacion

test_movi:
	make -f makefiles/Makefile.compilacion test_movi

test_movimiento_general:
	make -f makefiles/Makefile.compilacion test_movimiento_general

test_alu:
	make -f makefiles/Makefile.compilacion test_alu

test_flags:
	make -f makefiles/Makefile.compilacion test_flags

test_pc:
	make -f makefiles/Makefile.compilacion test_pc

test_gpu:
	make -f makefiles/Makefile.compilacion test_gpu

test_inputs:
	make -f makefiles/Makefile.compilacion test_inputs


# --------------------------------------------------------------------
# 🧹 Limpieza General
# --------------------------------------------------------------------
clean:
	make -f makefiles/Makefile.tests clean
	make -f makefiles/Makefile.compilacion clean