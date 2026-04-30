/*
 * adc.h
 *
 * Author: Group 15
 */

#ifndef INC_ADC_H_
#define INC_ADC_H_

#include "main.h"

/* --- Constants --- */
#define FFT_LEN  1024              // Length of FFT
#define ADC_BUF_LEN (FFT_LEN * 2)  // Double buffer (Ping Pong)

/* --- Global Variables --- */
// The raw buffer filled by DMA
extern volatile uint16_t adc_buf[ADC_BUF_LEN];

// Flag to tell main loop that data is ready (0=Wait, 1=1st Half, 2=2nd Half)
extern volatile uint8_t fft_ready_flag;

// ADC Handle (defined in main.c)
extern ADC_HandleTypeDef hadc1;

/* --- Function Prototypes --- */
// Starts the ADC in DMA mode
void ADC_Start(void);

#endif /* INC_ADC_H_ */
