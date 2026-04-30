#include "keypad.h"
#include <string.h>

/* Private function prototypes */
static ButtonName getButtonNameFromADC(uint32_t adc);
static const char* getButtonNameStringInternal(ButtonName button);

/* Initialize keypad */
void Keypad_Init(Keypad_HandleTypeDef* hkeypad, ADC_HandleTypeDef* hadc)
{
    hkeypad->hadc = hadc;
    hkeypad->sampleIndex = 0;
    hkeypad->currentAdcValue = 0;
    hkeypad->currentButton = BTN_NONE;
    hkeypad->lastProcessedButton = BTN_NONE;
    hkeypad->buttonState = BTN_STATE_RELEASED;
    hkeypad->buttonPressTime = 0;

    /* Initialize ADC samples */
    for (int i = 0; i < SAMPLE_COUNT; i++) {
        hkeypad->adcSamples[i] = 0;
    }
}

/* Convert ADC value to button name */
static ButtonName getButtonNameFromADC(uint32_t adc)
{
	/* Shield Calibration Fix:
	       - No Button: ~4095
	       - Select:    ~3600 - 4050 (We will set cutoff high to catch this)
	       - Left:      ~3000 - 3600
	       - Down:      ~2000 - 3000
	       - Up:        ~1000 - 2000
	       - Right:     ~0
	    */

	    // 1. Check "No Button" first (Only if truly at max voltage)
	    if (adc > 4050) return BTN_NONE;

	    // 2. Check thresholds (Values raised to fix your shifting issue)

	    if (adc < 200)   return BTN_RIGHT;  // Usually 0
	    if (adc < 1800)  return BTN_UP;     // Raised from 1500
	    if (adc < 2800)  return BTN_DOWN;   // Raised from 2500 (catches your "High" Down button)
	    if (adc < 3600)  return BTN_LEFT;   // Raised from 3200 (catches your "High" Left button)
	    if (adc <= 4050) return BTN_SELECT; // Raised from 3800 (catches your Select button)

	    return BTN_NONE;
}

/* Get button name as string */
static const char* getButtonNameStringInternal(ButtonName button)
{
    switch(button) {
        case BTN_UP:     return "UP";
        case BTN_DOWN:   return "DOWN";
        case BTN_LEFT:   return "LEFT";
        case BTN_SELECT: return "SELECT";
        default:         return "NONE";
    }
}

/* Public function to get button name string */
const char* Keypad_GetButtonNameString(ButtonName button)
{
    return getButtonNameStringInternal(button);
}

/* Get short 1-letter button name */
const char* Keypad_GetButtonShortName(ButtonName button)
{
    switch(button) {
        case BTN_UP:     return "U";
        case BTN_DOWN:   return "D";
        case BTN_LEFT:   return "L";
        case BTN_SELECT: return "S";
        default:         return "-";
    }
}

/* Get stable button with debouncing and state machine */
ButtonName Keypad_GetStableButton(Keypad_HandleTypeDef* hkeypad)
{
    static uint32_t lastStableAdc = 0;
    static ButtonName lastDetectedButton = BTN_NONE;
    static int consistentCount = 0;
    uint32_t currentTime = HAL_GetTick();

    /* Store sample */
    hkeypad->adcSamples[hkeypad->sampleIndex] = hkeypad->currentAdcValue;
    hkeypad->sampleIndex = (hkeypad->sampleIndex + 1) % SAMPLE_COUNT;

    /* Calculate moving average */
    uint32_t avgAdc = 0;
    for (int i = 0; i < SAMPLE_COUNT; i++) {
        avgAdc += hkeypad->adcSamples[i];
    }
    avgAdc /= SAMPLE_COUNT;

    /* Get button name from averaged ADC value */
    ButtonName detectedButton = getButtonNameFromADC(avgAdc);

    /* Check if reading is stable */
    if (lastStableAdc == 0) {
        lastStableAdc = avgAdc;
        lastDetectedButton = detectedButton;
        consistentCount = 1;
    } else {
        uint32_t diff = (avgAdc > lastStableAdc) ? (avgAdc - lastStableAdc) : (lastStableAdc - avgAdc);

        if (diff <= 10 && detectedButton == lastDetectedButton) {
            consistentCount++;
        } else {
            consistentCount = 1;
            lastStableAdc = avgAdc;
            lastDetectedButton = detectedButton;
        }
    }

    /* State machine for button handling */
    switch (hkeypad->buttonState) {
        case BTN_STATE_RELEASED:
            if (consistentCount >= 3 && detectedButton != BTN_NONE) {
                /* Button pressed */
                hkeypad->buttonState = BTN_STATE_PRESSED;
                hkeypad->buttonPressTime = currentTime;
                hkeypad->currentButton = detectedButton;
                return detectedButton;
            }
            break;

        case BTN_STATE_PRESSED:
            if (detectedButton == BTN_NONE || detectedButton != hkeypad->currentButton) {
                /* Button released or changed */
                hkeypad->buttonState = BTN_STATE_RELEASED;
                hkeypad->currentButton = BTN_NONE;
            } else if ((currentTime - hkeypad->buttonPressTime) > HOLD_THRESHOLD) {
                /* Button held */
                hkeypad->buttonState = BTN_STATE_HELD;
                hkeypad->currentButton = detectedButton;
            }
            break;

        case BTN_STATE_HELD:
            if (detectedButton == BTN_NONE || detectedButton != hkeypad->currentButton) {
                /* Button released or changed */
                hkeypad->buttonState = BTN_STATE_RELEASED;
                hkeypad->currentButton = BTN_NONE;
            }
            /* Don't return anything while held to prevent multiple triggers */
            break;
    }

    return BTN_NONE;
}

/* Get current ADC value */
uint32_t Keypad_GetCurrentADCValue(Keypad_HandleTypeDef* hkeypad)
{
    return hkeypad->currentAdcValue;
}
