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

// Vector Table
__attribute__((section(".isr_vector")))
const void* interrupt_vector_table[] = {
    (void*)0x24020000, 
    (void*)_start      
};

// 32-point Sine Table (12-bit values: 0 to 4095)
// Centered at 2048 with an amplitude of ~2000
static const unsigned short sine_table[32] = {
    2048, 2447, 2831, 3185, 3495, 3750, 3939, 4056, 
    4095, 4056, 3939, 3750, 3495, 3185, 2831, 2447, 
    2048, 1648, 1264, 910,  600,  345,  156,  39, 
    0,    39,   156,  345,  600,  910,  1264, 1648
};

void delay(volatile int count) {
    while (count--) __asm__("nop");
}

void _start(void) {
    // 1. Enable Clocks (GPIOA and DAC1)
    RCC_AHB4ENR  |= (1 << 0);    
    RCC_APB1LENR |= (1 << 29);   
    
    for(volatile int i=0; i<100; i++); // Wait for clock

    // 2. Set PA4 to Analog Mode (11)
    GPIOA_MODER |= (3 << (4 * 2));

    // 3. Enable DAC1 Channel 1
    DAC1_CR |= (1 << 0);

    int index = 0;
    while (1) {
        // 4. Write value from table to DAC
        DAC1_DHR12R1 = sine_table[index];

        // 5. Advance index
        index = (index + 1) % 32;

        // 6. Control LFO speed (Higher = Slower)
        delay(10000); 
    }
}

