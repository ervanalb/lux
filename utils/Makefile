#CC = gcc
#CC = clang

# Lux Source files
LUX_SRC += ../liblux/linux/lux.c ../liblux/linux/crc.c
LUX_OBJ = $(LUX_SRC:%.c=%.o)

# Compiler flags
INC = -I. -I../

LIBRARIES = -lX11 -lXext

CFLAGS = -std=c99 -ggdb3 -O3 $(INC)
CFLAGS += -Wall -Wextra -Werror -Wno-unused-parameter
CFLAGS += -D_POSIX_C_SOURCE=201702 -D_XOPEN_SOURCE
CFLAGS += -DLUX_USE_LOG_H
LFLAGS = $(CFLAGS)

# Targets
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Binaries
TARGETS = luxctl lux_test lux_x11_bridge

luxctl: luxctl.o $(LUX_OBJ)
	$(CC) $(LFLAGS) -o $@ $+
lux_test: lux_test.o $(LUX_OBJ)
	$(CC) $(LFLAGS) -o $@ $+
lux_x11_bridge: lux_x11_bridge.o $(LUX_OBJ)
	$(CC) $(LFLAGS) -o $@ $+ -lX11 -lXext

.PHONY: all
all: $(TARGETS)

.PHONY: clean
clean:
	rm -f $(TARGETS) tags lux*.o $(LUX_OBJ)

tags: $(C_SRC)
	ctags -R . ../liblux/

.DEFAULT_GOAL := all
