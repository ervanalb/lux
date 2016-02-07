C_SRC = main.c ../liblux/linux/lux.c ../liblux/linux/crc.c
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
