# ====================================================================
#              Makefile Principal - CPU MILO
# ====================================================================

CC = gcc

CFLAGS = -Wall -Wextra -g

# Directorio donde está este Makefile.
# Se obtiene independientemente del directorio desde donde se invoque make.
COMPILADOR_DIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))

# La ruta queda embebida en el ejecutable milo durante la compilación.
MILO_DEFINE = -DMILO_COMPILADOR_DIR=\"$(COMPILADOR_DIR)\"


# ====================================================================
#                    FUENTES DEL COMPILADOR
# ====================================================================

SRC_COMPILADOR = \
	main.c \
	lexer/lexer.c \
	parser/parser.c \
	encoder/encoder.c \
	helpers/compilador_helper.c \
	helpers/compile_text.c \
	helpers/escritor_rom.c \
	helpers/tag_manager.c


# ====================================================================
#                         EJECUTABLE MILO
# ====================================================================

milo: $(SRC_COMPILADOR)
	$(CC) $(CFLAGS) $(MILO_DEFINE) $^ -o $@


# ====================================================================
#                    TESTS DEL COMPILADOR
# ====================================================================

tests:
	$(MAKE) test_lexer
	$(MAKE) test_parser


test_lexer:
	$(CC) $(CFLAGS) \
		lexer/lexer.c \
		tests/test_lexer.c \
		-o test_lexer_exec


test_parser:
	$(CC) $(CFLAGS) \
		lexer/lexer.c \
		parser/parser.c \
		helpers/tag_manager.c \
		tests/test_parser.c \
		-o test_parser_exec


# ====================================================================
#                           LIMPIEZA
# ====================================================================

clean:
	rm -f milo
	rm -f test_lexer_exec
	rm -f test_parser_exec