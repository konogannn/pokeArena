
CC          = clang
FORMAT      = clang-format

TARGET      = pokeArena
UNIT_TESTS  = unit_tests

CFLAGS      = -std=c99             \
              -Wall                \
              -Wextra              \
              -Wpedantic           \
              -Wshadow             \
              -Wconversion         \
              -Wstrict-prototypes  \
              -Wmissing-prototypes \
              -Wformat=2           \
              -Wundef              \
              -Werror

CPPFLAGS    = -I./include
LDFLAGS     =
LDLIBS      =

SCF         = main.c          \
              ai.c            \
              battle.c        \
              color.c         \
              display.c       \
              gym.c           \
              moves.c         \
              pokemon.c       \
              report.c        \
              save.c          \
              team.c          \
              tsv.c           \
              type.c          \
              ui.c            \
              utils.c

SRC         = $(addprefix src/, $(SCF))
OBJ         = $(SRC:.c=.o)
DEP         = $(OBJ:.o=.d)

HEADERS     = $(wildcard include/*.h)

# Main target
all: $(TARGET)

$(TARGET): $(OBJ)
	@$(CC) $(LDFLAGS) -o $@ $(OBJ) $(LDLIBS)

# Compilation
src/%.o: src/%.c
	@$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

# Unit tests
$(UNIT_TESTS): $(filter-out src/main.o, $(OBJ)) tests/test_main.c
	@$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

tests_run: $(UNIT_TESTS)
	@./$(UNIT_TESTS)

# Formatting
format:
	@$(FORMAT) -i $(SRC) $(HEADERS)

format-check:
	@$(FORMAT) --dry-run --Werror $(SRC) $(HEADERS)

# Static analysis
cppcheck:
	@cppcheck --enable=all                    \
	          --std=c99                       \
	          --language=c                    \
	          --error-exitcode=1              \
	          --suppress=missingIncludeSystem \
                 --check-level=exhaustive        \
	          $(SRC) -I include

# Runtime sanitizers
sanitize:
	@$(CC) $(CPPFLAGS) -std=c99 -g -O1        \
	       -Wall -Wextra -Wpedantic           \
	       -fsanitize=address,undefined       \
	       -fno-omit-frame-pointer            \
	       $(SRC) -o $(TARGET)_san $(LDLIBS)

# Run all static checks
lint: format-check cppcheck

# Cleanup
clean:
	@rm -f $(OBJ) $(DEP)

fclean: clean
	@rm -f $(TARGET) $(UNIT_TESTS) $(TARGET)_san

re: fclean all

-include $(DEP)

.PHONY: all clean fclean re format format-check cppcheck sanitize lint tests_run
