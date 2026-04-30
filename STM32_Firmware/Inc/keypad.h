#ifndef KEYPAD_H
#define KEYPAD_H

#include "main.h"
#include <stdint.h>

/* Button detection constants */
#define SAMPLE_COUNT 5
#define DEBOUNCE_MS 50
#define HOLD_THRESHOLD 500

/* Button state machine */
typedef enum {
    BTN_STATE_RELEASED,
    BTN_STATE_PRESSED,
    BTN_STATE_HELD
} ButtonState;

/* Button names */
typedef enum {
    BTN_NONE,
    BTN_UP,
    BTN_DOWN,
    BTN_LEFT,
	BTN_RIGHT,
    BTN_SELECT
} ButtonName;

/* Keypad structure */
typedef struct {
    ADC_HandleTypeDef* hadc;
    uint32_t adcSamples[SAMPLE_COUNT];
    int sampleIndex;
    uint32_t currentAdcValue;
    ButtonName currentButton;
    ButtonName lastProcessedButton;
    ButtonState buttonState;
    uint32_t buttonPressTime;
} Keypad_HandleTypeDef;

/* Function prototypes */
void Keypad_Init(Keypad_HandleTypeDef* hkeypad, ADC_HandleTypeDef* hadc);
ButtonName Keypad_GetStableButton(Keypad_HandleTypeDef* hkeypad);
const char* Keypad_GetButtonNameString(ButtonName button);
const char* Keypad_GetButtonShortName(ButtonName button);
uint32_t Keypad_GetCurrentADCValue(Keypad_HandleTypeDef* hkeypad);

#endif /* KEYPAD_H */
