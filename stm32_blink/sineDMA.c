// This is not working ^^

void _start(void);

// Register Definitions (Same as before)
#define RCC_BASE      0x58024400
#define RCC_CR        (*(volatile unsigned int *)(RCC_BASE + 0x00))
#define RCC_PLLCKSELR (*(volatile unsigned int *)(RCC_BASE + 0x18))
#define RCC_PLL1DIVR  (*(volatile unsigned int *)(RCC_BASE + 0x28))
#define RCC_D1CFGR    (*(volatile unsigned int *)(RCC_BASE + 0x10))
#define RCC_AHB4ENR   (*(volatile unsigned int *)(RCC_BASE + 0xE0))
#define RCC_AHB1ENR   (*(volatile unsigned int *)(RCC_BASE + 0xD8))
#define RCC_APB1LENR  (*(volatile unsigned int *)(RCC_BASE + 0xE8))
#define PWR_BASE      0x58024800
#define PWR_D3CR      (*(volatile unsigned int *)(PWR_BASE + 0x18))
#define FLASH_ACR     (*(volatile unsigned int *)(0x52002000))
#define GPIOA_MODER   (*(volatile unsigned int *)(0x58020000))
#define DAC1_CR       (*(volatile unsigned int *)(0x40007400))
#define DAC1_DHR12R1  (*(volatile unsigned int *)(0x40007408))
#define TIM6_CR1      (*(volatile unsigned int *)(0x40001000))
#define TIM6_CR2      (*(volatile unsigned int *)(0x40001004))
#define TIM6_ARR      (*(volatile unsigned int *)(0x4000102C))
#define DMA1_S0CR     (*(volatile unsigned int *)(0x40020010))
#define DMA1_S0NDTR   (*(volatile unsigned int *)(0x40020014))
#define DMA1_S0PAR    (*(volatile unsigned int *)(0x40020018))
#define DMA1_S0M0AR   (*(volatile unsigned int *)(0x4002001C))
#define DMAMUX1_C0CR  (*(volatile unsigned int *)(0x40020800))

// This stays in FLASH
static const unsigned short sine_lookup[64] = {
    2048, 2248, 2447, 2642, 2831, 3013, 3185, 3347, 3495, 3630, 3750, 3853, 3939, 4007, 4056, 4085,
    4095, 4085, 4056, 4007, 3939, 3853, 3750, 3630, 3495, 3347, 3185, 3013, 2831, 2642, 2447, 2248,
    2048, 1847, 1648, 1453, 1264, 1082, 910, 748, 600, 465, 345, 242, 156, 88, 39, 10,
    0, 10, 39, 88, 156, 242, 345, 465, 600, 748, 910, 1082, 1264, 1453, 1648, 1847
};

// This is where the DMA will look (AXI SRAM)
volatile unsigned short ram_sine_table[64];

__attribute__((section(".isr_vector")))
const void* interrupt_vector_table[] = { (void*)0x24020000, (void*)_start };

void _start(void) {
    // 1. Manually copy Flash -> RAM (Crucial for DMA access on H7)
    for(int i=0; i<64; i++) {
        ram_sine_table[i] = sine_lookup[i];
    }

    // 2. Clock Setup (550MHz)
    (*(volatile unsigned int *)0x58024400) |= (1 << 16); // HSE ON
    while(!((*(volatile unsigned int *)0x58024400) & (1 << 17)));
    PWR_D3CR |= (3 << 14); // VOS0
    FLASH_ACR = 0x4;
    RCC_PLLCKSELR = (1 << 0) | (5 << 4); 
    RCC_PLL1DIVR = (219 << 0) | (1 << 9); 
    (*(volatile unsigned int *)0x58024400) |= (1 << 24); // PLL ON
    while(!((*(volatile unsigned int *)0x58024400) & (1 << 25)));
    RCC_D1CFGR = 0x03;

    // 3. Peripherals
    RCC_AHB4ENR |= (1 << 0);           // GPIOA
    RCC_AHB1ENR |= (1 << 0) | (1 << 1); // DMA1 + DMAMUX
    RCC_APB1LENR |= (1 << 29) | (1 << 4); // DAC1 + TIM6

    GPIOA_MODER |= (3 << 8); // PA4 Analog

    // 4. DMA Configuration
    DMAMUX1_C0CR = 67; 
    DMA1_S0PAR = (unsigned int)&DAC1_DHR12R1;
    DMA1_S0M0AR = (unsigned int)ram_sine_table; // Point to RAM address
    DMA1_S0NDTR = 64;
    DMA1_S0CR = (1 << 8) | (1 << 10) | (1 << 11) | (1 << 13) | (1 << 6) | (1 << 0);

    // 5. Timer 6 (~440Hz Tone)
    TIM6_ARR = 9000; 
    TIM6_CR2 |= (2 << 4);
    TIM6_CR1 |= 1;

    // 6. DAC
    DAC1_CR |= (1 << 2) | (1 << 12) | (1 << 0);

    while(1);
}