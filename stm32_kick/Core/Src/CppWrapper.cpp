#include "CppWrapper.h"
#include "kick.hpp"
#include "main.h"

// Ensure this matches the handle in main.c
extern "C" DAC_HandleTypeDef hdac1;

void Cpp_Init(void) {
    // Crucial: Start the DAC hardware
    HAL_DAC_Start(&hdac1, DAC_CHANNEL_1);
}

void Cpp_TriggerKick(void) {
    kick.trigger(); 

    // LED ON (Check if your LED pin is correct, e.g., GPIOE PIN 3)
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_SET);

    // Synthesis loop
    // 8000 samples = roughly 200-400ms depending on CPU speed
    for (int s = 0; s < 8000; s++) {
        uint16_t dac_val = kick.process(1.0f);
        
        // Write to DAC
        HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, dac_val);
        
        // Short delay to set sample rate
        // If sound is too high-pitched, increase this number
        for(volatile int d = 0; d < 25; d++);
    }

    // LED OFF
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_RESET);
}