/*
 * processing.c
 *
 * Created on: Dec 23, 2025
 * Author: User
 */

#include "processing.h"
#include "adc.h" // Access to adc_buf and fft_ready_flag
#include <stdio.h>
#include <string.h>

/* --- Private DSP Buffers --- */
static float fft_input_buf[FFT_LEN];    // Input to FFT
static float fft_output_buf[FFT_LEN];   // Output of FFT (Complex)
static float fft_mag_buf[FFT_LEN / 2];  // Magnitude

/* --- DSP Handler --- */
arm_rfft_fast_instance_f32 fft_handler;

/* --- Output Globals --- */
float currentSpeed = 0.0f;
float currentFrequency = 0.0f;

extern UART_HandleTypeDef huart2;

void FFT_Init(void)
{
    arm_rfft_fast_init_f32(&fft_handler, FFT_LEN);
}

void Process_FFT_Data(void)
{
    // Check if ADC data is ready via the flag from adc.c
    if (fft_ready_flag > 0)
    {
        uint16_t *source_ptr;
        char uart_msg[64];

        // 1. Determine which half of the buffer to read
        if (fft_ready_flag == 1) {
            source_ptr = (uint16_t*)&adc_buf[0];
        } else {
            source_ptr = (uint16_t*)&adc_buf[FFT_LEN];
        }

        // Clear flag so we don't re-enter immediately
        fft_ready_flag = 0;

        // 2. Convert Raw (uint16) to Float
        for (int i = 0; i < FFT_LEN; i++) {
            fft_input_buf[i] = (float)source_ptr[i];
        }

        // 3. Perform FFT
        arm_rfft_fast_f32(&fft_handler, fft_input_buf, fft_output_buf, 0);

        // 4. Calculate Magnitude
        arm_cmplx_mag_f32(fft_output_buf, fft_mag_buf, FFT_LEN / 2);

        // 5. Find Peak Frequency
        float max_value;
        uint32_t max_index;

        // Ignore DC offset (Index 0)
        arm_max_f32(&fft_mag_buf[1], (FFT_LEN / 2) - 1, &max_value, &max_index);
        max_index = max_index + 1;

        // Squelch: If signal is too weak, force 0
        if (max_value < 50.0f) {
            max_index = 0;
        }

        // 6. Calculate Frequency & Speed
        float F_SAMPLE = 10000.0f;
        float frequency = (float)max_index * F_SAMPLE / (float)FFT_LEN;

        // Doppler Constant for HB100 (10.525GHz)
        // speed = Freq Difference * (Speed of Light)/(2 * Freq Transmitted)
        float speed = frequency * 0.01424f;

        // 7. Update Globals (main.c reads these)
        currentFrequency = frequency;
        currentSpeed = speed;

        // 8. Optional UART Debug
        sprintf(uart_msg, "ADC-F: %.2f Hz, Spd: %.2f\r\n", frequency, speed);
        HAL_UART_Transmit(&huart2, (uint8_t*)uart_msg, strlen(uart_msg), 10);
    }
}
