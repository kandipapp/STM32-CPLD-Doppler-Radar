--------------------------------------------------------------------------------
-- Company: 
-- Engineer:
--
-- Create Date:   07:17:45 03/10/2026
-- Design Name:   
-- Module Name:   /home/ise/Desktop/Display_Board_Project_Y2/tb_Top_Level_Receiver.vhd
-- Project Name:  Display_Board_Project_Y2
-- Target Device:  
-- Tool versions:  
-- Description:   
-- 
-- VHDL Test Bench Created by ISE for module: Top_Level_Receiver
-- 
-- Dependencies:
-- 
-- Revision:
-- Revision 0.01 - File Created
-- Additional Comments:
--
-- Notes: 
-- This testbench has been automatically generated using types std_logic and
-- std_logic_vector for the ports of the unit under test.  Xilinx recommends
-- that these types always be used for the top-level I/O of a design in order
-- to guarantee that the testbench will bind correctly to the post-implementation 
-- simulation model.
--------------------------------------------------------------------------------
LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
 
-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--USE ieee.numeric_std.ALL;
 
ENTITY tb_Top_Level_Receiver IS
END tb_Top_Level_Receiver;
 
ARCHITECTURE behavior OF tb_Top_Level_Receiver IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT Top_Level_Receiver
    PORT(
         clk_ext : IN  std_logic;
         reset_btn : IN  std_logic;
         rs485_rx : IN  std_logic;
         seg_unit : OUT  std_logic_vector(0 to 6);
         seg_unit_dp : OUT  std_logic;
         seg_tens : OUT  std_logic_vector(0 to 6);
         seg_tens_dp : OUT  std_logic
        );
    END COMPONENT;
    

   --Inputs
   signal clk_ext : std_logic := '0';
   signal reset_btn : std_logic := '0';
   signal rs485_rx : std_logic := '1'; -- UART line idles HIGH

 	--Outputs
   signal seg_unit : std_logic_vector(0 to 6);
   signal seg_unit_dp : std_logic;
   signal seg_tens : std_logic_vector(0 to 6);
   signal seg_tens_dp : std_logic;

   -- Clock period definitions
	-- 4. Timing Definitions
   -- 1.8432 MHz external clock = ~542.5 ns period
   constant clk_ext_period : time := 542.5 ns;
	
	-- How long is one UART bit?
   -- The divider cuts the clock by 4, and the control logic counts 8 ticks per bit.
   -- Therefore: 4 * 8 = 32 external clock cycles per UART bit.
   constant uart_bit_period : time := clk_ext_period * 32;
 
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   uut: Top_Level_Receiver PORT MAP (
          clk_ext => clk_ext,
          reset_btn => reset_btn,
          rs485_rx => rs485_rx,
          seg_unit => seg_unit,
          seg_unit_dp => seg_unit_dp,
          seg_tens => seg_tens,
          seg_tens_dp => seg_tens_dp
        );

   -- Clock process definitions
   clk_ext_process :process
   begin
		clk_ext <= '0';
		wait for clk_ext_period/2;
		clk_ext <= '1';
		wait for clk_ext_period/2;
   end process;
 

   -- Stimulus process
   stim_proc: process
   begin		
		-- INITIALIZE
      reset_btn <= '1';
      rs485_rx  <= '1'; -- Idle high
      wait for 10 us;
		
		reset_btn <= '0';
      wait for 10 us;
		
		-- =================================================================
      -- TEST 1: Send "45" (Binary BCD: 0100 0101)
      -- UART sends LSB (Least Significant Bit) first! 
      -- Data Sequence to send: 1, 0, 1, 0, 0, 0, 1, 0
      -- =================================================================
      
      rs485_rx <= '0'; wait for uart_bit_period; -- START BIT
		
		rs485_rx <= '1'; wait for uart_bit_period; -- Bit 0 (LSB)
      rs485_rx <= '0'; wait for uart_bit_period; -- Bit 1
      rs485_rx <= '1'; wait for uart_bit_period; -- Bit 2
      rs485_rx <= '0'; wait for uart_bit_period; -- Bit 3
		rs485_rx <= '0'; wait for uart_bit_period; -- Bit 4
      rs485_rx <= '0'; wait for uart_bit_period; -- Bit 5
      rs485_rx <= '1'; wait for uart_bit_period; -- Bit 6
      rs485_rx <= '0'; wait for uart_bit_period; -- Bit 7 (MSB)
		
		rs485_rx <= '1'; wait for uart_bit_period; -- STOP BIT
      
      -- Wait to observe the displays updating to 45
      wait for 100 us;
		
		-- =================================================================
      -- TEST 2: Send "89" (Binary BCD: 1000 1001)
      -- Data Sequence to send: 1, 0, 0, 1, 0, 0, 0, 1
      -- =================================================================
      
      rs485_rx <= '0'; wait for uart_bit_period; -- START BIT
		
		rs485_rx <= '1'; wait for uart_bit_period; -- Bit 0 (LSB)
      rs485_rx <= '0'; wait for uart_bit_period; -- Bit 1
      rs485_rx <= '0'; wait for uart_bit_period; -- Bit 2
      rs485_rx <= '1'; wait for uart_bit_period; -- Bit 3
		rs485_rx <= '0'; wait for uart_bit_period; -- Bit 4
      rs485_rx <= '0'; wait for uart_bit_period; -- Bit 5
      rs485_rx <= '0'; wait for uart_bit_period; -- Bit 6
      rs485_rx <= '1'; wait for uart_bit_period; -- Bit 7 (MSB)
		
		rs485_rx <= '1'; wait for uart_bit_period; -- STOP BIT
      
      -- Wait to observe the displays updating to 89
      wait for 100 us;
      wait;
   end process;

END;
