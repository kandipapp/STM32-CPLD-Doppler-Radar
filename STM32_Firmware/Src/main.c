/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
#include "adc.h"        // Modular ADC
#include "processing.h" // Modular FFT Processing
#include "lcd16x2_v2.h" // LCD Lib
#include "keypad.h"     // Keypad Lib
#include "comparator.h" // Comparator Lib

#include "rs485.h"		// RS485 Communication Lib


/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* Menu System Enums and Structs */
typedef enum {
    SPEED_MPH,
    SPEED_KPH,
    SPEED_MS
} SpeedUnit;

typedef enum {
    MODE_ADC,       // FFT Method
    MODE_COMPARATOR // Zero-Crossing Method
} SensorMode;

typedef enum {
    DISPLAY_MAIN,
    DISPLAY_MENU,
    DISPLAY_SETTINGS,
    DISPLAY_UNITS,
	DISPLAY_MODES
} DisplayState;

typedef struct {
    const char* name;
    DisplayState targetState;
} MenuItem;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;
DMA_HandleTypeDef hdma_adc1;

COMP_HandleTypeDef hcomp1;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim6;

UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */


/* --- Menu/LCD Variables --- */
Keypad_HandleTypeDef hkeypad;
SpeedUnit currentUnit = SPEED_MPH;
DisplayState currentState = DISPLAY_MAIN;
SensorMode currentMode = MODE_ADC; // Default to ADC
int currentMenuItem = 0;


/* --- Live Data Variables --- */
float displaySpeed = 0.0f;     // The speed to show on LCD (can be from ADC or COM)
float displayFrequency = 0.0f; // The freq to show on LCD (can be from ADC or COM)

/* --- Menu Structure Definition --- */
const MenuItem mainMenu[] = {
    {"Settings", DISPLAY_SETTINGS},
    {"Speed Display", DISPLAY_MAIN},
    {"System Info", DISPLAY_MAIN}
};

const MenuItem settingsMenu[] = {
    {"Speed Units", DISPLAY_UNITS},
	{"Sensor Mode", DISPLAY_MODES}, // NEW OPTION
    {"Back", DISPLAY_MENU}
};

const MenuItem unitsMenu[] = {
    {"mph", DISPLAY_SETTINGS},
    {"km/h", DISPLAY_SETTINGS},
    {"m/s", DISPLAY_SETTINGS},
    {"Back", DISPLAY_SETTINGS}
};

const MenuItem modesMenu[] = {
    {"ADC (FFT)", DISPLAY_SETTINGS},
    {"Comparator", DISPLAY_SETTINGS},
    {"Back", DISPLAY_SETTINGS}
};

const int mainMenuCount = sizeof(mainMenu) / sizeof(MenuItem);
const int settingsMenuCount = sizeof(settingsMenu) / sizeof(MenuItem);
const int unitsMenuCount = sizeof(unitsMenu) / sizeof(MenuItem);
const int modesMenuCount = sizeof(modesMenu) / sizeof(MenuItem);

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM2_Init(void);
static void MX_ADC2_Init(void);
static void MX_COMP1_Init(void);
static void MX_TIM6_Init(void);
static void MX_USART3_UART_Init(void);
/* USER CODE BEGIN PFP */
/* Menu Functions Prototypes */
void updateDisplay(void);
void handleButtonPress(ButtonName button);
float getConvertedSpeed(float base_speed_ms);
void convertAndDisplaySpeed(float speed);
void showMainScreen(void);
void showMenuScreen(void);
void showSettingsScreen(void);
void showUnitsScreen(void);
void showModesScreen(void); // NEW
const char* getCurrentDirectory(void);
void navigateUp(void);
void navigateDown(void);
void selectMenuItem(void);
void goBack(void);
int getMenuCount(void);
int canScrollUp(void);
int canScrollDown(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* --- Menu Navigation & Display Logic --- */
const char* getCurrentDirectory(void)
{
    switch(currentState) {
        case DISPLAY_MENU: return "MENU";
        case DISPLAY_SETTINGS: return "SETTINGS";
        case DISPLAY_UNITS: return "UNITS";
        case DISPLAY_MODES: return "MODES";
        default: return (currentMode == MODE_ADC) ? "ADC MODE" : "COM MODE";
    }
}

int getMenuCount(void)
{
    switch(currentState) {
        case DISPLAY_MENU: return mainMenuCount;
        case DISPLAY_SETTINGS: return settingsMenuCount;
        case DISPLAY_UNITS: return unitsMenuCount;
        case DISPLAY_MODES: return modesMenuCount;
        default: return 0;
    }
}

int canScrollUp(void) { return (currentMenuItem > 0); }
int canScrollDown(void) { return (currentMenuItem < getMenuCount() - 1); }

/* Helper to calculate the final speed based on current unit settings */
float getConvertedSpeed(float base_speed_ms) {
    switch(currentUnit) {
        case SPEED_KPH: return base_speed_ms * 3.6f;
        case SPEED_MPH: return base_speed_ms * 2.23694f;
        case SPEED_MS:
        default: return base_speed_ms;
    }
}

/* --- Display Functions --- */
void convertAndDisplaySpeed(float speed_in_ms)
{
	char speedStr[16];

	// Get the converted number using the helper
	float finalValue = getConvertedSpeed(speed_in_ms);
	const char* unitStr = "";

	// Just grab the correct text label
	switch(currentUnit) {
		case SPEED_MS:  unitStr = "m/s"; break;
		case SPEED_KPH: unitStr = "km/h"; break;
		case SPEED_MPH: unitStr = "mph"; break;
	}

	snprintf(speedStr, sizeof(speedStr), "%.1f %s", finalValue, unitStr);
	lcd16x2_printf("%s", speedStr);
}

void showMainScreen(void)
{
    lcd16x2_clear();

    //Line 1: Frequency & Source Label
    lcd16x2_1stLine();
    if(currentMode == MODE_ADC) {
		lcd16x2_printf("ADC-F: %.2fHz", displayFrequency);
	} else {
		lcd16x2_printf("COM-F: %.2fHz", displayFrequency);
	}

    //Line 2: Speed & Source Label
    lcd16x2_2ndLine();
    if(currentMode == MODE_ADC) {
            lcd16x2_printf("ADC-V: ");
            convertAndDisplaySpeed(displaySpeed);
        } else {
        	lcd16x2_printf("COM-V: ");
        	convertAndDisplaySpeed(displaySpeed);
        }
}

/* Helper to render generic menu lists */
void renderMenuGeneric(const MenuItem* menuItems, int count) {
    lcd16x2_clear();
    lcd16x2_1stLine();
    lcd16x2_printf("%-12s", getCurrentDirectory());

    lcd16x2_2ndLine();
    if (currentMenuItem < count) {
        char displayLine[17];
        char nav[4] = "  ";
        if (canScrollUp() && canScrollDown()) strcpy(nav, "<>");
        else if (canScrollUp()) strcpy(nav, "< ");
        else if (canScrollDown()) strcpy(nav, " >");

        snprintf(displayLine, sizeof(displayLine), "%s %s", nav, menuItems[currentMenuItem].name);
        lcd16x2_printf("%s", displayLine);
    }
}

void showMenuScreen(void) { renderMenuGeneric(mainMenu, mainMenuCount); }
void showSettingsScreen(void) { renderMenuGeneric(settingsMenu, settingsMenuCount); }
void showUnitsScreen(void) { renderMenuGeneric(unitsMenu, unitsMenuCount); }
void showModesScreen(void) { renderMenuGeneric(modesMenu, modesMenuCount); }


/* --- Navigation Logic --- */
void navigateUp(void) {
    if (canScrollUp()) currentMenuItem--;
    else currentMenuItem = getMenuCount() - 1;
}

void navigateDown(void) {
    if (canScrollDown()) currentMenuItem++;
    else currentMenuItem = 0;
}

void goBack(void) {
    switch(currentState) {
        case DISPLAY_MENU: currentState = DISPLAY_MAIN; break;
        case DISPLAY_SETTINGS:
        case DISPLAY_UNITS:
        case DISPLAY_MODES: currentState = DISPLAY_MENU; currentMenuItem = 0; break;
        default: currentState = DISPLAY_MAIN; break;
    }
}

void selectMenuItem(void) {
    switch(currentState) {
        case DISPLAY_MAIN:
            currentState = DISPLAY_MENU;
            currentMenuItem = 0;
            break;
        case DISPLAY_MENU:
            currentState = mainMenu[currentMenuItem].targetState;
            currentMenuItem = 0;
            break;
        case DISPLAY_SETTINGS:
            if (strcmp(settingsMenu[currentMenuItem].name, "Back") == 0) goBack();
            else {
                currentState = settingsMenu[currentMenuItem].targetState;
                currentMenuItem = 0;
            }
            break;
        case DISPLAY_UNITS:
            if (strcmp(unitsMenu[currentMenuItem].name, "Back") == 0) goBack();
            else {
                if (strcmp(unitsMenu[currentMenuItem].name, "mph") == 0) currentUnit = SPEED_MPH;
                else if (strcmp(unitsMenu[currentMenuItem].name, "km/h") == 0) currentUnit = SPEED_KPH;
                else if (strcmp(unitsMenu[currentMenuItem].name, "m/s") == 0) currentUnit = SPEED_MS;
                currentState = DISPLAY_MAIN;
            }
            break;
        case DISPLAY_MODES:
			if (strcmp(modesMenu[currentMenuItem].name, "Back") == 0) goBack();
			else {
				if (strcmp(modesMenu[currentMenuItem].name, "ADC (FFT)") == 0) currentMode = MODE_ADC;
				else if (strcmp(modesMenu[currentMenuItem].name, "Comparator") == 0) currentMode = MODE_COMPARATOR;
				currentState = DISPLAY_MAIN; // Return to main to see change
			}
			break;
    }
}

void handleButtonPress(ButtonName button) {
    if (button == BTN_NONE) return;
    if (button == BTN_UP) navigateUp();
    else if (button == BTN_DOWN) navigateDown();
    else if (button == BTN_SELECT) selectMenuItem();
    else if (button == BTN_LEFT) goBack();
}

void updateDisplay(void) {
    switch(currentState) {
        case DISPLAY_MAIN: showMainScreen(); break;
        case DISPLAY_MENU: showMenuScreen(); break;
        case DISPLAY_SETTINGS: showSettingsScreen(); break;
        case DISPLAY_UNITS: showUnitsScreen(); break;
        case DISPLAY_MODES: showModesScreen(); break;
    }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_ADC1_Init();
  MX_TIM2_Init();
  MX_ADC2_Init();
  MX_COMP1_Init();
  MX_TIM6_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */

  // --- 1. Initialize LCD ---
  lcd16x2_init_4bits(GPIOA, RS_Pin, GPIOC, E_Pin,
                     GPIOB, D4_Pin, GPIOB, D5_Pin, GPIOB, D6_Pin, GPIOA, D7_Pin);

  // --- 2. Initialize Keypad ---
  Keypad_Init(&hkeypad, &hadc2);

  // 3. Initialize FFT (Modular)
  FFT_Init();

  // 4. Start ADC (Modular)
  ADC_Start();

  // 5. Start the Timer (This starts the "ticking" that triggers the ADC)
  HAL_TIM_Base_Start(&htim2);

  // Add this before Comparator_Start() in main.c
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

  // 6. Start Comparator System (from comparator.c)
  Comparator_Start();

  // Show initial screen
  updateDisplay();

  /* Variables for Loop Timing */
  uint32_t last_display_update = 0;
  uint32_t last_keypad_poll = 0;


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  uint32_t now = HAL_GetTick();
	  /* ---------------------------------------------------------
	   * TASK A: FFT Data Processing (Signal Speed)
	   * --------------------------------------------------------- */
	  // Checks internal flag, runs FFT, updates 'currentSpeed' & 'currentFrequency'
	  Process_FFT_Data();

	  /* ---------------------------------------------------------
	   * TASK B: Data Selection Logic (ADC vs Comparator)
	   * --------------------------------------------------------- */
	  if(currentMode == MODE_ADC) {
		  // Read from processing.c variables
		  displaySpeed = currentSpeed;
		  displayFrequency = currentFrequency;
	  }
	  else if (currentMode == MODE_COMPARATOR) {
		  // Read from comparator.c global 'measured_frequency'
		  // Convert Comparator Frequency to Speed
		  // HB100 X-Band Constant: 19.49 Hz = 1 km/h
		  float comp_speed_ms = measured_frequency * 0.01425f;

	      displaySpeed = comp_speed_ms;
		  displayFrequency = measured_frequency;
	  }

	  /* ---------------------------------------------------------
	   * TASK C: Keypad Polling (Every ~50ms)
	   * --------------------------------------------------------- */
	    if (now - last_keypad_poll > 50)
	    {
			  // Poll ADC2 for Keypad
			  HAL_ADC_Start(&hadc2);
			  HAL_ADC_PollForConversion(&hadc2, 10);
			  uint32_t kpad_val = HAL_ADC_GetValue(&hadc2);
			  HAL_ADC_Stop(&hadc2);

			  hkeypad.currentAdcValue = kpad_val;
			  ButtonName btn = Keypad_GetStableButton(&hkeypad);

			  if (btn != BTN_NONE) {
				  handleButtonPress(btn);
				  updateDisplay(); // Update immediately on button press
			  }
			  last_keypad_poll = now;
	     }

		/* ---------------------------------------------------------
		 * TASK D: Display Refresh (Live Speed Update)
		 * Only refresh periodically if we are on the Main Screen
		 * to show the changing speed numbers.
		 * --------------------------------------------------------- */
	    if (currentState == DISPLAY_MAIN && (now - last_display_update > 200))
	    {
			// Refresh the speed readout (don't clear whole screen to avoid flicker)
	    	// Ideally, optimize showMainScreen to only update the number,
	    	// but calling the function is safer for now.
			// 1. Refresh the local LCD
			showMainScreen();

			// 2. Calculate the final speed.
			// We use 'displaySpeed' instead of 'currentSpeed' so it respects
			// whether the user chose ADC or Comparator mode!
			float final_rs485_speed = getConvertedSpeed(displaySpeed);

			// 3. Send Speed to Remote Display via RS-485
			// 'currentSpeed' is your global float variable from processing.h
			RS485_Transmit_Speed(final_rs485_speed);

			last_display_update = now;
	    }

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCCLKSOURCE_PLLSAI1;
  PeriphClkInit.PLLSAI1.PLLSAI1Source = RCC_PLLSOURCE_HSI;
  PeriphClkInit.PLLSAI1.PLLSAI1M = 1;
  PeriphClkInit.PLLSAI1.PLLSAI1N = 8;
  PeriphClkInit.PLLSAI1.PLLSAI1P = RCC_PLLP_DIV7;
  PeriphClkInit.PLLSAI1.PLLSAI1Q = RCC_PLLQ_DIV2;
  PeriphClkInit.PLLSAI1.PLLSAI1R = RCC_PLLR_DIV2;
  PeriphClkInit.PLLSAI1.PLLSAI1ClockOut = RCC_PLLSAI1_ADC1CLK;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_MultiModeTypeDef multimode = {0};
  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIG_T2_TRGO;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the ADC multi-mode
  */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_3;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief ADC2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC2_Init(void)
{

  /* USER CODE BEGIN ADC2_Init 0 */

  /* USER CODE END ADC2_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC2_Init 1 */

  /* USER CODE END ADC2_Init 1 */

  /** Common config
  */
  hadc2.Instance = ADC2;
  hadc2.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc2.Init.Resolution = ADC_RESOLUTION_12B;
  hadc2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc2.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc2.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc2.Init.LowPowerAutoWait = DISABLE;
  hadc2.Init.ContinuousConvMode = DISABLE;
  hadc2.Init.NbrOfConversion = 1;
  hadc2.Init.DiscontinuousConvMode = DISABLE;
  hadc2.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc2.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc2.Init.DMAContinuousRequests = DISABLE;
  hadc2.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc2.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC2_Init 2 */

  /* USER CODE END ADC2_Init 2 */

}

/**
  * @brief COMP1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_COMP1_Init(void)
{

  /* USER CODE BEGIN COMP1_Init 0 */

  /* USER CODE END COMP1_Init 0 */

  /* USER CODE BEGIN COMP1_Init 1 */

  /* USER CODE END COMP1_Init 1 */
  hcomp1.Instance = COMP1;
  hcomp1.Init.InvertingInput = COMP_INPUT_MINUS_3_4VREFINT;
  hcomp1.Init.NonInvertingInput = COMP_INPUT_PLUS_IO1;
  hcomp1.Init.OutputPol = COMP_OUTPUTPOL_NONINVERTED;
  hcomp1.Init.Hysteresis = COMP_HYSTERESIS_HIGH;
  hcomp1.Init.BlankingSrce = COMP_BLANKINGSRC_NONE;
  hcomp1.Init.Mode = COMP_POWERMODE_HIGHSPEED;
  hcomp1.Init.WindowMode = COMP_WINDOWMODE_DISABLE;
  hcomp1.Init.TriggerMode = COMP_TRIGGERMODE_IT_RISING;
  if (HAL_COMP_Init(&hcomp1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN COMP1_Init 2 */

  /* USER CODE END COMP1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 79;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 99;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 7999;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 9999;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 57600;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LD2_Pin|D7_Pin|RS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, D6_Pin|D5_Pin|D4_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(E_GPIO_Port, E_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LD2_Pin D7_Pin RS_Pin */
  GPIO_InitStruct.Pin = LD2_Pin|D7_Pin|RS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : D6_Pin D5_Pin D4_Pin */
  GPIO_InitStruct.Pin = D6_Pin|D5_Pin|D4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : E_Pin */
  GPIO_InitStruct.Pin = E_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(E_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : RS485_DE_Pin */
  GPIO_InitStruct.Pin = RS485_DE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(RS485_DE_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
