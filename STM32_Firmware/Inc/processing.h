/*
 * processing.h
 *
 * Author: Group 15
 */

#ifndef INC_PROCESSING_H_
#define INC_PROCESSING_H_

#include "main.h"
#include <arm_math.h> // CMSIS-DSP Library

/* --- Global Variables --- */
// Calculated results accessible by main.c (for LCD display)
extern float currentSpeed;
extern float currentFrequency;

// UART Handle for debug printing
extern UART_HandleTypeDef huart2;

/* --- Function Prototypes --- */
// Initializes the FFT structures
void FFT_Init(void);

// Checks the flag, processes FFT, calculates speed, and updates globals
void Process_FFT_Data(void);

#endif /* INC_PROCESSING_H_ */
