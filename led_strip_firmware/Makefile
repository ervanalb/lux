# Makefile for STM32F373 using cmsis and GNU toolchain.

# Files to include
APP_C_SRC  = $(wildcard app/src/*.c)
BL_C_SRC   = $(wildcard bl/src/*.c)
C_SRC  = $(wildcard lib/STM32F0xx_StdPeriph_Driver/src/*.c)
C_SRC += $(wildcard lib/lux/src/*.c)

APP_OBJECTS = $(patsubst %.c,%.o,$(C_SRC) $(APP_C_SRC)) app/src/startup.o
BL_OBJECTS  = $(patsubst %.c,%.o,$(C_SRC) $(BL_C_SRC)) bl/src/startup.o

LIBRARIES = 

INC  = -Ilib/lux/inc -Iinc
INC += -Ilib/CMSIS/Include -Ilib/CMSIS/Device/ST/STM32F0xx/Include
INC += -Ilib/STM32F0xx_StdPeriph_Driver/inc
APP_INC = -Iapp/inc
BL_INC = -Ibl/inc

# Assembler, compiler, and linker flags
AFLAGS  = -mlittle-endian -mthumb -mcpu=cortex-m0 -g
CFLAGS  = $(AFLAGS) -O1 $(INC) -std=c99 -DUSE_STDPERIPH_DRIVER -Wall
CFLAGS += -DSTM32F031
CFLAGS += -DUSE_STDPERIPH_DRIVER
CFLAGS += -DWS2811
CFLAGS += -fdata-sections -ffunction-sections
LFLAGS  = $(CFLAGS) -nostartfiles -Wl,--gc-sections

# Targets
all: main.hex
clean:
	-rm -f $(APP_OBJECTS) $(BL_OBJECTS) *.elf *.bin *.map *.hex
main.hex: app.hex bl.hex
	cat app.hex bl.hex > combined.hex
app.hex: app.elf
	arm-none-eabi-objcopy app.elf -O ihex app.hex
bl.hex: bl.elf
	arm-none-eabi-objcopy bl.elf -O ihex bl.hex
app.bin: app.elf
	arm-none-eabi-objcopy -O binary app.elf app.bin
bl.bin: bl.elf
	arm-none-eabi-objcopy -O binary bl.elf bl.bin
app.elf: $(APP_OBJECTS)
	arm-none-eabi-gcc $(LFLAGS) -Tapp.ld -Wl,-Map=app.map -o app.elf $(APP_OBJECTS) $(LIBRARIES)
bl.elf: $(BL_OBJECTS)
	arm-none-eabi-gcc $(LFLAGS) -Tbl.ld -Wl,-Map=bl.map -o bl.elf $(BL_OBJECTS) $(LIBRARIES)
app/src/startup.o: app/src/startup.s
	arm-none-eabi-as $(AFLAGS) app/src/startup.s -o app/src/startup.o
bl/src/startup.o: bl/src/startup.s
	arm-none-eabi-as $(AFLAGS) bl/src/startup.s -o bl/src/startup.o
lib/%.o: lib/%.c
	echo wersers
	arm-none-eabi-gcc $(CFLAGS) -c -o $@ $<
app/%.o: app/%.c
	arm-none-eabi-gcc $(CFLAGS) $(APP_INC) -c -o $@ $<
bl/%.o: bl/%.c
	arm-none-eabi-gcc $(CFLAGS) $(BL_INC) -c -o $@ $<
gdb: app.elf
	arm-none-eabi-gdb app.elf -x init.gdb
load: app.elf
	arm-none-eabi-gdb app.elf -x init.gdb -ex load
loadquit: app.elf
	arm-none-eabi-gdb app.elf -batch -x init.gdb -ex load -ex kill -ex quit
gdb_bl: bl.elf
	arm-none-eabi-gdb bl.elf -x init.gdb
load_bl: bl.elf
	arm-none-eabi-gdb bl.elf -x init.gdb -ex load
loadquit_bl: bl.elf
	arm-none-eabi-gdb bl.elf -batch -x init.gdb -ex load -ex kill -ex quit
stlink:
	st-util -p 4244
stload: app.bin
	st-flash write app.bin 0x8000000
stload_bl: bl.bin
	st-flash write bl.bin 0x8000000
sterase:
	st-flash erase
luxload: app.bin
	python ../python/luxflash.py /dev/ttyUSB0 0x08000000 app.bin


