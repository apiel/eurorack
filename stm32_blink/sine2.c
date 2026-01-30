#include <math.h>

void _start(void);

// Registers
#define RCC_BASE          0x58024400
#define RCC_AHB4ENR       (*(volatile unsigned int *)(RCC_BASE + 0xE0))
#define RCC_APB1LENR      (*(volatile unsigned int *)(RCC_BASE + 0xE8))

#define GPIOA_BASE        0x58020000
#define GPIOA_MODER       (*(volatile unsigned int *)(GPIOA_BASE + 0x00))

#define DAC1_BASE         0x40007400
#define DAC1_CR           (*(volatile unsigned int *)(DAC1_BASE + 0x00))
#define DAC1_DHR12R1      (*(volatile unsigned int *)(DAC1_BASE + 0x08))

// FPU System Control Space (SCS) Registers
#define CPACR             (*(volatile unsigned int *)(0xE000ED88))

__attribute__((section(".isr_vector")))
const void* interrupt_vector_table[] = {
    (void*)0x24020000, 
    (void*)_start      
};

void _start(void) {
    // --- ENABLE FPU ---
    // Enable CP10 and CP11 coprocessors (bits 20-23) to allow FPU use
    CPACR |= (0xF << 20);
    __asm__("dsb"); // Data Synchronization Barrier
    __asm__("isb"); // Instruction Synchronization Barrier

    // --- SETUP HARDWARE ---
    RCC_AHB4ENR  |= (1 << 0);    
    RCC_APB1LENR |= (1 << 29);   
    for(volatile int i=0; i<100; i++);

    GPIOA_MODER |= (3 << (4 * 2)); // PA4 Analog
    DAC1_CR |= (1 << 0);           // Enable DAC1

    float angle = 0.0f;
    while (1) {
        // Calculate sine: range -1.0 to 1.0
        // Offset to 1.0 (range 0 to 2.0) then scale to 12-bit (0 to 4095)
        float s = sinf(angle);
        unsigned int dac_val = (unsigned int)((s + 1.0f) * 2047.5f);
        
        DAC1_DHR12R1 = dac_val;

        angle += 0.1f;
        if (angle > 6.28318f) angle = 0.0f;

        for(volatile int i=0; i<5000; i++); // Delay
    }
}
