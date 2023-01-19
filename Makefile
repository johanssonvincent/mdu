CC = gcc
CFLAGS = -g -std=gnu11 -Werror -Wall -Wextra -Wpedantic -Wmissing-declarations -Wmissing-prototypes -Wold-style-definition -lpthread
DEPS = stack.h mdu.h thread_funcs.h data.h error.h
OBJ = mdu.o stack.o thread_funcs.o data.o error.o

mdu: $(OBJ)
		$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean
clean:
		-rm *.o mdu