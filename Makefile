CC=arm-none-eabi-gcc
CFLAGS=-mcpu=cortex-m3 -mthumb -Wall -O0 -ffreestanding -nostdlib -I./drivers
LDFLAGS=-T linker.ld -nostdlib -nostartfiles --specs=nosys.specs

SRC=startup.c main.c drivers/gpio.c
OBJ=$(SRC:.c=.o)

all: blink.elf

blink.elf: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@ $(LDFLAGS)

clean:
	rm -f $(OBJ) blink.elf

