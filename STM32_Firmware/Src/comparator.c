#include "comparator.h"

extern COMP_HandleTypeDef hcomp1;
extern TIM_HandleTypeDef htim6;

volatile uint32_t pulse_count = 0;
volatile float measured_frequency = 0.0;

// Minimum time between valid pulses in microseconds
// For 600Hz max frequency, minimum period = 1,000,000/600 = ~1667us
// Set guard to ~80% of that = ~1300us to be safe
#define MIN_PULSE_INTERVAL_US 720

void Comparator_Start(void) {
    HAL_COMP_Start_IT(&hcomp1);
    HAL_TIM_Base_Start_IT(&htim6);
}

void HAL_COMP_TriggerCallback(COMP_HandleTypeDef *hcomp) {
    if (hcomp == &hcomp1) {
        uint32_t now = DWT->CYCCNT;  // CPU cycle counter, runs at 80MHz
        static uint32_t last_trigger = 0;

        // Convert MIN_PULSE_INTERVAL_US to CPU cycles (80MHz = 80 cycles/us)
        uint32_t min_cycles = MIN_PULSE_INTERVAL_US * 80;

        if ((now - last_trigger) >= min_cycles) {
            pulse_count++;
            last_trigger = now;
        }
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM6) {
        measured_frequency = (float)pulse_count;
        pulse_count = 0;
    }
}
