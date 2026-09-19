CC = arm-none-eabi-gcc

# --- FREERTOS PATH (Matches your 'Source' folder) ---
FREERTOS_ROOT = Source

# --- INCLUDE DIRECTORIES ---
INCLUDES = -I . \
           -I $(FREERTOS_ROOT)/include \
           -I $(FREERTOS_ROOT)/portable/GCC/ARM_CM4F \
           -I $(HOME)/tiva-c

# --- COMPILER & LINKER FLAGS ---
CFLAGS = -mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -Os -c $(INCLUDES) -DPART_TM4C123GH6PM
LDFLAGS = -T stellaris.ld -L $(HOME)/tiva-c/driverlib/gcc -mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -Wl,--gc-sections -ldriver
# --- VPATH (Tells make where to find .c files inside Source) ---
VPATH = . \
        $(FREERTOS_ROOT) \
        $(FREERTOS_ROOT)/portable/GCC/ARM_CM4F \
        $(FREERTOS_ROOT)/portable/MemMang

# --- OBJECT FILES ---
OBJS = main.o \
       startup_gcc.o \
       tasks.o \
       list.o \
       queue.o \
       timers.o \
       event_groups.o \
       port.o \
       heap_4.o

# --- BUILD RULES ---
all: main.elf

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

main.elf: $(OBJS) stellaris.ld
	$(CC) $(OBJS) $(LDFLAGS) -o main.elf

flash: main.elf
	sudo openocd -f board/ek-tm4c123gxl.cfg -c "program main.elf verify reset exit"

clean:
	rm -f *.o main.elf