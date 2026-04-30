/*
 * adc.c
 *
 * Author: Group 15
 */

#include "adc.h"

/* --- Private Variables --- */
volatile uint16_t adc_buf[ADC_BUF_LEN];
volatile uint8_t fft_ready_flag = 0;

/* --- Functions --- */

/**
 * @brief Starts the ADC in DMA mode with the circular buffer
 */
void ADC_Start(void)
{
    // Start ADC1 using DMA, filling the 'adc_buf' array
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buf, ADC_BUF_LEN);
}

/* --- Interrupt Callbacks --- */

/**
 * @brief  Conversion Half Complete Callback in non-blocking mode
 * @param  hadc: ADC handle
 * @retval None
 */
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc)
{
    // Check if this interrupt came from the correct ADC
    if (hadc->Instance == ADC1)
    {
        fft_ready_flag = 1; // Tell main loop: "First half (0-1023) is ready!"
    }
}

/**
 * @brief  Conversion Complete Callback in non-blocking mode
 * @param  hadc: ADC handle
 * @retval None
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    if (hadc->Instance == ADC1)
    {
        fft_ready_flag = 2; // Tell main loop: "Second half (1024-2047) is ready!"
    }
}
