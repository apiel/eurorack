#include "CppWrapper.h"
#include "main.h"

extern "C" DAC_HandleTypeDef hdac1;
extern "C" TIM_HandleTypeDef htim6;

#define BUFFER_SIZE 128

// This is the CRITICAL line.
// If your LED doesn't blink, change ".RAM_D1" to ".axi_sram" or "RAM"
// (check your STM32H723VGTX_FLASH.ld file for the name of the memory at 0x24000000)
// Ensure the section name matches exactly what you put in the .ld file
uint16_t audioBuffer[BUFFER_SIZE] __attribute__((section(".RAM_D1"))) __attribute__((aligned(32)));

void Cpp_Init(void)
{
  // DIAGNOSTIC 1: Turn LED ON to show Cpp_Init started
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_SET);
  HAL_Delay(500);
  HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_3); // Turn LED OFF

  for (int i = 0; i < BUFFER_SIZE; i++)
    audioBuffer[i] = 2048;

  // Flush Cache
  SCB_CleanDCache_by_Addr((uint32_t *)audioBuffer, BUFFER_SIZE * 2);

  // Start DMA
  HAL_StatusTypeDef status = HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (uint32_t *)audioBuffer, BUFFER_SIZE, DAC_ALIGN_12B_R);

  // Start Timer
  HAL_TIM_Base_Start(&htim6);

  // HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_3);
  // HAL_Delay(500);
  // HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_3);

  // DIAGNOSTIC 2: If DMA fails to start, BLINK RAPIDLY forever
  if (status != HAL_OK)
  {
    while (1)
    {
      // HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_3);
      // for(volatile int i=0; i<2000000; i++);

      HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_3);
      HAL_Delay(100);
      HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_3);
      HAL_Delay(100);
      HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_3);
      HAL_Delay(500);
    }
  }
}

int yo = 0;
// THE WORKHORSE
void Fill_Buffer(int start_index, int size)
{
  yo++;
  if (yo > 100)
  {
    // HEARTBEAT: Toggle LED to show the interrupt is alive!
    HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_3);
    yo = 0;
  }

  for (int i = 0; i < size; i++)
  {
    static uint32_t t = 0;
    // Simple test tone: 440Hz-ish square wave
    audioBuffer[start_index + i] = ((t++ / 50) % 2) ? 3000 : 1000;
  }

  // PUSH CACHE TO RAM
  SCB_CleanDCache_by_Addr((uint32_t *)&audioBuffer[start_index], size * 2);
}

// INTERRUPT CALLBACKS
extern "C" void HAL_DAC_CH1_HalfConvCpltCallbackCh1(DAC_HandleTypeDef *hdac)
{
  Fill_Buffer(0, BUFFER_SIZE / 2);
}

extern "C" void HAL_DAC_CH1_ConvCpltCallbackCh1(DAC_HandleTypeDef *hdac)
{
  Fill_Buffer(BUFFER_SIZE / 2, BUFFER_SIZE / 2);
}