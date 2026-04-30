/*
 * rs485.c
 *
 *  Created on: Feb 10, 2026
 *      Author: eugen
 */

#include "rs485.h"

// Import the specific UART handle we configured in the .ioc (USART3)
extern UART_HandleTypeDef huart3;

/**
  * @brief  Formats and sends the speed to the remote display via RS-485.
  * @param  speed_val: The speed in float format (e.g., 45.3)
  * @retval None
  */
void RS485_Transmit_Speed(float speed_val) {

    // 1. Convert float speed to integer
    int speed_int = (int)speed_val;

    // 2. Cap the speed to 99 (Requirement: 2 digits max)
    if (speed_int > 99) speed_int = 99;
    if (speed_int < 0) speed_int = 0;

    // 3. Convert Decimal to Packed BCD
    // As per Task 9a requirements: "two 4-bit binary coded decimal numbers TASK9 into one byte"
    // Example: 45 -> tens=4 (0100), units=5 (0101) -> Result=0x45 (01000101)
    uint8_t tens = speed_int / 10;
    uint8_t units = speed_int % 10;
    uint8_t bcd_byte = (tens << 4) | units;

    // 4. Enable Driver (Talk Mode)
    // Pull DE High to enable the transmitter on the SN65HVD1780P
    HAL_GPIO_WritePin(RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_SET);

    // 5. Transmit Data via USART3
    // Sending 1 byte with a 10ms timeout
    HAL_UART_Transmit(&huart3, &bcd_byte, 1, 10);

    // 6. CRITICAL: Wait for Transmission Complete (TC) flag
    // The HAL_UART_Transmit function returns once data is in the holding register,
    // but the bit might still be shifting out. If we cut DE too early, the last bit gets chopped.
    while (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_TC) == RESET);

    // 7. Disable Driver (Listen Mode)
    // Pull DE Low to put the transceiver back into receive/idle mode
    HAL_GPIO_WritePin(RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_RESET);
}
