
all: build flash

build:
	arm-none-eabi-gcc -mcpu=cortex-m7 -mthumb -O1 -g -nostdlib -T STM32H723VGTx_FLASH.ld -e _start main.c -o blink.elf
	arm-none-eabi-objcopy -O binary blink.elf blink.bin

flash:
	sudo dfu-util -a 0 -D blink.bin -s 0x08000000:leave
