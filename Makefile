CC = gcc
CFLAGS = -g -std=gnu11 -Werror -Wall -Wextra -Wpedantic -Wmissing-declarations -Wmissing-prototypes -Wold-style-definition -lpthread
OBJ = mdu.o

%.o: %.c $(DEPS)
		$(CC) -c -o $@ $< $(CFLAGS)

mdu: $(OBJ)
		$(CC) -o $@ $^ $(CFLAGS)
