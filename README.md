# EEEE2072: Doppler Radar Speed Measurement System (Group 15)

## Project Overview
This repository contains the complete firmware and hardware description source code for a two-semester Electronic Systems Group Design Project (EEEE2072). The objective of the project is to build a highly robust, mixed-signal embedded prototype capable of acquiring, processing, and displaying the velocity of moving objects (such as electric scooters) using a Doppler radar. 

The system leverages a hardware/software co-design strategy to resolve complex engineering constraints, offloading heavy mathematical processing to an ARM Cortex-M4 CPU while maintaining strict asynchronous timing via a resource-constrained 64-macrocell CPLD.

## System Architecture
The project is divided into two major embedded subsystems:

### 1. Data Acquisition & Processing (STM32L476RG)
* **Language:** C/C++ (STM32CubeIDE / HAL Libraries)
* **Features:** 
  * Acquires conditioned analogue Doppler signals via ADC and Comparator peripherals.
  * Utilizes CMSIS-DSP Fast Fourier Transform (FFT) algorithms to extract peak frequencies.
  * Calculates real-time velocity and displays it on a local 16x2 LCD Keypad Shield.
  * Encodes the speed into a Packed BCD format and transmits it asynchronously via an RS-485 transceiver (UART).

### 2. Digital Receiver & Remote Display (Xilinx XC2C64A CPLD)
* **Language:** VHDL (Xilinx ISE Design Suite)
* **Features:**
  * Custom digital logic receiver constrained to an absolute limit of 64 macrocells [4].
  * Features an 8× oversampled Clock Divider and Control Logic Finite State Machine (FSM) to decode the RS-485 serial stream.
  * Utilizes an internal anti-flicker double-buffer latch and an Active-Low BCD-to-7-Segment Decoder to drive a remote dual-LED display synchronously.

## Repository Structure
To facilitate assessment and independent verification, the repository is organized into modular directories corresponding to the specific hardware and software tasks:

* `/STM32_Firmware/` - Contains the complete STM32CubeIDE workspace, including individual peripheral drivers (`adc.c`, `comparator.c`, `rs485.c`, `lcd16x2_v2.c`), digital signal processing algorithms (`processing.c`), and the final Unified System main loop.
* `/CPLD_VHDL_Logic/` - Contains the complete Xilinx ISE Design Suite project, including the structural Top-Level routing matrix (`Top_Level_Receiver.vhd`), behavioral sub-modules (`Shift_Register.vhd`, `Control_Logic.vhd`), testbenches, and the physical hardware constraints mapping file (`pinout.ucf`).

## Team Members (Group 15)
* Eugene Ooi You Qi (Lead Firmware Developer & Digital Logic Architect)
* Syivesh Rukmanikanthan (Lead Hardware, Interface & Communication Engineer)
* Chunyong (Interface Engineer)

