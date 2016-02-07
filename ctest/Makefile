C_SRC = main.c ../liblux/lux.c ../liblux/lux_hal_unix.c ../liblux/crc/crc.c
OBJECTS = $(patsubst %.c,%.o,$(C_SRC))
INC = -I../liblux/ -I.
CFLAGS = -g -O2 -std=c99 -Wall -Wextra $(INC)

%.o : %.c
	gcc $(CFLAGS) -c -o $@ $<

lux_test: $(OBJECTS)
	gcc $(CFLAGS) -o $@ $(OBJECTS)

.PHONY: clean all

clean:
	rm $(OBJECTS)

all: lux_test
