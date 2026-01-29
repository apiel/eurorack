
all: build_blink flash_blink

build_blink:
	arm-none-eabi-gcc -mcpu=cortex-m7 -mthumb -O1 -g -nostdlib -T STM32H723VGTx_FLASH.ld -e _start blink.c -o blink.elf
	arm-none-eabi-objcopy -O binary blink.elf blink.bin

flash_blink:
	sudo dfu-util -a 0 -D blink.bin -s 0x08000000:leave
