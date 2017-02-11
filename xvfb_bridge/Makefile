PROJECT = xvfb_bridge
#CC = gcc
#CC = clang

# Source files
C_SRC  = main.c
C_SRC += ../liblux/linux/lux.c ../liblux/linux/crc.c

OBJECTS = $(C_SRC:%.c=%.o)

# Compiler flags
INC = -I. -I../liblux/

LIBRARIES = -lX11 -lXext

CFLAGS = -std=c99 -ggdb3 -O3 $(INC)
CFLAGS += -Wall -Wextra -Werror -Wno-unused-parameter
CFLAGS += -D_POSIX_C_SOURCE=20170206 -D_XOPEN_SOURCE=20170206
LFLAGS = $(CFLAGS)

# Targets
$(PROJECT): $(OBJECTS)
	$(CC) $(LFLAGS) -o $(PROJECT) $(OBJECTS) $(LIBRARIES)

%.o: %.c
	$(CC) $(CFLAGS) $(APP_INC) -c -o $@ $<

.PHONY: all
all: $(PROJECT)

.PHONY: clean
clean:
	rm -f $(PROJECT) tags $(OBJECTS)

tags: $(C_SRC)
	ctags -R .

.DEFAULT_GOAL := all
