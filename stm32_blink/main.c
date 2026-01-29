// Forward declaration so the vector table can see it
void _start(void);

// Register Addresses for STM32H723
#define RCC_BASE      0x58024400
#define RCC_AHB4ENR   (*(volatile unsigned int *)(RCC_BASE + 0xE0))

#define GPIOE_BASE    0x58021000
#define GPIOE_MODER   (*(volatile unsigned int *)(GPIOE_BASE + 0x00))
#define GPIOE_ODR     (*(volatile unsigned int *)(GPIOE_BASE + 0x14))

// Vector Table
// The "section" attribute ensures the linker puts this at the start of FLASH
__attribute__((section(".isr_vector")))
const void* interrupt_vector_table[] = {
    (void*)0x24020000, // Stack Pointer (Top of AXI SRAM for H723)
    (void*)_start      // Reset Vector
};

void delay(volatile int count) {
    while (count--) {
        __asm__("nop");
    }
}

void _start(void) {
    // 1. Enable Clock for GPIOE (Bit 4 in AHB4ENR)
    RCC_AHB4ENR |= (1 << 4);
    
    // Dummy read to let the clock stabilize
    volatile unsigned int dummy = RCC_AHB4ENR;
    (void)dummy;

    // 2. Set PE3 as Output
    // MODER3 is bits [7:6]. 01 = General purpose output mode.
    GPIOE_MODER &= ~(3 << (3 * 2)); // Clear bits 6 and 7
    GPIOE_MODER |=  (1 << (3 * 2)); // Set bit 6 to 1

    while (1) {
        GPIOE_ODR ^= (1 << 3); // Toggle Pin 3
        delay(1000000 * 3);        // Wait
    }
}
