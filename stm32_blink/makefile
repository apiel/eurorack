
all: build_blink flash_blink

build_blink:
	arm-none-eabi-gcc -mcpu=cortex-m7 -mthumb -O1 -g -nostdlib -T STM32H723VGTx_FLASH.ld -e _start blink.c -o blink.elf
	arm-none-eabi-objcopy -O binary blink.elf blink.bin

flash_blink:
	sudo dfu-util -a 0 -D blink.bin -s 0x08000000:leave

build_sine:
	arm-none-eabi-gcc -mcpu=cortex-m7 -mthumb -O1 -g -nostdlib -T STM32H723VGTx_FLASH.ld -e _start sine.c -o sine.elf
	arm-none-eabi-objcopy -O binary sine.elf sine.bin

flash_sine:
	sudo dfu-util -a 0 -D sine.bin -s 0x08000000:leave