#include "CppWrapper.h"
#include "main.h"

extern "C" DAC_HandleTypeDef hdac1;
extern "C" TIM_HandleTypeDef htim6;
extern "C" DMA_HandleTypeDef hdma_dac1_ch1;

#define BUFFER_SIZE 128

// Use RAM_D2 for better DMA performance
uint16_t audioBuffer[BUFFER_SIZE] __attribute__((section(".RAM_D2"))) __attribute__((aligned(32)));

// Diagnostics
volatile uint32_t half_complete_count = 0;
volatile uint32_t full_complete_count = 0;
volatile uint32_t dma_error_count = 0;

void Cpp_Init(void)
{
  // === DIAGNOSTIC STEP 1: Initial LED Test ===
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_SET);
  HAL_Delay(500);
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_RESET);
  HAL_Delay(500);

  // === STEP 2: Initialize buffer ===
  for (int i = 0; i < BUFFER_SIZE; i++)
  {
    audioBuffer[i] = 2048;  // Mid-scale DC value
  }

  // === STEP 3: Verify buffer address ===
  // The buffer MUST be in the 0x30000000 range (RAM_D2)
  uint32_t buffer_addr = (uint32_t)audioBuffer;
  if (buffer_addr < 0x30000000 || buffer_addr >= 0x30008000)
  {
    // ERROR: Buffer not in RAM_D2! Blink LED 5 times rapidly
    for (int i = 0; i < 10; i++)
    {
      HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_3);
      HAL_Delay(100);
    }
    while(1); // Halt
  }

  // === STEP 4: Clean D-Cache ===
  SCB_CleanDCache_by_Addr((uint32_t *)audioBuffer, BUFFER_SIZE * sizeof(uint16_t));

  // === STEP 5: Start Timer (generates DAC trigger) ===
  if (HAL_TIM_Base_Start(&htim6) != HAL_OK)
  {
    // Timer start failed - blink 3 times
    for (int i = 0; i < 6; i++)
    {
      HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_3);
      HAL_Delay(200);
    }
    while(1);
  }

  // === STEP 6: Start DAC DMA ===
  HAL_StatusTypeDef status = HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, 
                                                (uint32_t *)audioBuffer, 
                                                BUFFER_SIZE, 
                                                DAC_ALIGN_12B_R);

  if (status != HAL_OK)
  {
    // DAC DMA start failed - rapid blink forever
    while (1)
    {
      HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_3);
      HAL_Delay(50);
    }
  }

  // === SUCCESS: Single long blink ===
  HAL_Delay(200);
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_SET);
  HAL_Delay(1000);
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_RESET);
}

void Fill_Buffer(int start_index, int size)
{
  static uint32_t phase = 0;
  
  for (int i = 0; i < size; i++)
  {
    // Generate simple square wave test tone
    // Frequency: ~44kHz / 100 = 440Hz
    audioBuffer[start_index + i] = ((phase++ / 50) % 2) ? 3000 : 1000;
  }

  // CRITICAL: Clean cache so DMA reads new data
  SCB_CleanDCache_by_Addr((uint32_t *)&audioBuffer[start_index], 
                          size * sizeof(uint16_t));
}

// ===== DMA CALLBACKS =====
// These are called by HAL when DMA transfer reaches half/full completion

extern "C" void HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef *hdac)
{
  half_complete_count++;
  
  // LED heartbeat: toggle every 100 callbacks (~2.3 seconds at 44kHz)
  if (half_complete_count % 100 == 0)
  {
    HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_3);
  }
  
  // Fill first half of buffer
  Fill_Buffer(0, BUFFER_SIZE / 2);
}

extern "C" void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef *hdac)
{
  full_complete_count++;
  
  // Fill second half of buffer
  Fill_Buffer(BUFFER_SIZE / 2, BUFFER_SIZE / 2);
}

// Optional: DMA error callback for debugging
extern "C" void HAL_DAC_ErrorCallbackCh1(DAC_HandleTypeDef *hdac)
{
  dma_error_count++;
  
  // Error: blink rapidly 10 times then halt
  for (int i = 0; i < 20; i++)
  {
    HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_3);
    HAL_Delay(50);
  }
  while(1);
}
