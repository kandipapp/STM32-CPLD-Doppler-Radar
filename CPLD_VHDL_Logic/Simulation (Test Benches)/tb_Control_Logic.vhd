--------------------------------------------------------------------------------
-- Company: 
-- Engineer:
--
-- Create Date:   09:18:50 03/09/2026
-- Design Name:   
-- Module Name:   /home/ise/Desktop/Display_Board_Project_Y2/tb_Control_Logic.vhd
-- Project Name:  Display_Board_Project_Y2
-- Target Device:  
-- Tool versions:  
-- Description:   
-- 
-- VHDL Test Bench Created by ISE for module: Control_Logic
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
 
ENTITY tb_Control_Logic IS
END tb_Control_Logic;
 
ARCHITECTURE behavior OF tb_Control_Logic IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT Control_Logic
    PORT(
         clk : IN  std_logic;
         reset : IN  std_logic;
         serial_in : IN  std_logic;
         shift_enable : OUT  std_logic;
         data_ready : OUT  std_logic
        );
    END COMPONENT;
    

   --Inputs
   signal clk : std_logic := '0';
   signal reset : std_logic := '0';
   signal serial_in : std_logic := '0';

 	--Outputs
   signal shift_enable : std_logic;
   signal data_ready : std_logic;

   -- Clock period definitions
   constant clk_period : time := 10 ns;
 
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   uut: Control_Logic PORT MAP (
          clk => clk,
          reset => reset,
          serial_in => serial_in,
          shift_enable => shift_enable,
          data_ready => data_ready
        );

   -- Clock process definitions
   clk_process :process
   begin
		clk <= '0';
		wait for clk_period/2;
		clk <= '1';
		wait for clk_period/2;
   end process;
 

   -- Stimulus process
	stim_proc: process
   begin		
      -- 1. Initialize the system
      reset <= '1';
      serial_in <= '1'; -- In UART, the idle state is ALWAYS High ('1')
      wait for 100 ns;	
      
      reset <= '0';
      wait for clk_period*10;
		
		-- =========================================================
      -- SIMULATE SENDING A UART BYTE
      -- Note: Every bit lasts for 8 clock cycles (8x oversampling)
      -- =========================================================

      -- 2. Send START BIT (Line drops to '0')
      serial_in <= '0';
      wait for clk_period * 8;
		
		-- 3. Send 8 DATA BITS (Let's send the pattern 1-0-1-0-0-0-1-1)
      serial_in <= '1'; wait for clk_period * 8; -- Bit 0
      serial_in <= '0'; wait for clk_period * 8; -- Bit 1
      serial_in <= '1'; wait for clk_period * 8; -- Bit 2
      serial_in <= '0'; wait for clk_period * 8; -- Bit 3
      serial_in <= '0'; wait for clk_period * 8; -- Bit 4
		serial_in <= '0'; wait for clk_period * 8; -- Bit 5
      serial_in <= '1'; wait for clk_period * 8; -- Bit 6
      serial_in <= '1'; wait for clk_period * 8; -- Bit 7
		
		-- 4. Send STOP BIT (Line returns to '1')
      serial_in <= '1';
      wait for clk_period * 8;
		
		-- Wait a bit to observe the final state, then stop the simulation
      wait for clk_period * 20;
      wait;
   end process;

END;
