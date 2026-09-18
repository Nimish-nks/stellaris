CC = arm-none-eabi-gcc
CFLAGS = -mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -Os -c -I $(HOME)/tiva-c -DPART_TM4C123GH6PM
LDFLAGS = -T stellaris.ld -L $(HOME)/tiva-c/driverlib/gcc -mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -Wl,--gc-sections -ldriver

OBJS = main.o startup_gcc.o

all: main.elf

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

main.elf: $(OBJS) stellaris.ld
	$(CC) $(OBJS) $(LDFLAGS) -o main.elf

flash: main.elf
	sudo openocd -f board/ek-tm4c123gxl.cfg -c "program main.elf verify reset exit"

clean:
	rm -f *.o *.elf