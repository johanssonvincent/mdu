CC = gcc
CFLAGS = -g -std=gnu11 -Werror -Wall -Wextra -Wpedantic -Wmissing-declarations -Wmissing-prototypes -Wold-style-definition -lpthread
OBJ = mdu.o

mdu: $(OBJ)
		$(CC) -o $@ $^ $(CFLAGS)
