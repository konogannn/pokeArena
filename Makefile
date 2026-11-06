CC	=	gcc

TARGET	=	pokeArena

UNIT_TESTS	=	unit_tests

CFLAGS	=	-g -Wall -Wextra -Wshadow -I./include

SCF	=	main.c

SRC	=	$(addprefix src/, $(SCF))

OBJ	=	$(SRC:.c=.o)

all: $(TARGET)

$(TARGET):	$(OBJ)
	@$(CC) -o $(TARGET) $(OBJ) $(CFLAGS)

clean:
	@rm -f $(OBJ)

fclean: clean
	@rm -f $(TARGET)

re: fclean all

.PHONY: all clean fclean re
