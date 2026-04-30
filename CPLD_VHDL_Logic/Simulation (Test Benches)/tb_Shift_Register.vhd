--------------------------------------------------------------------------------
-- Company: 
-- Engineer:
--
-- Create Date:   08:28:46 03/09/2026
-- Design Name:   
-- Module Name:   /home/ise/Desktop/Display_Board_Project_Y2/tb_Shift_Register.vhd
-- Project Name:  Display_Board_Project_Y2
-- Target Device:  
-- Tool versions:  
-- Description:   
-- 
-- VHDL Test Bench Created by ISE for module: Shift_Register
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
 
ENTITY tb_Shift_Register IS
END tb_Shift_Register;
 
ARCHITECTURE behavior OF tb_Shift_Register IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT Shift_Register
    PORT(
         clk : IN  std_logic;
         reset : IN  std_logic;
         shift_enable : IN  std_logic;
         serial_in : IN  std_logic;
         parallel_out : OUT  std_logic_vector(7 downto 0)
        );
    END COMPONENT;
    

   --Inputs
   signal clk : std_logic := '0';
   signal reset : std_logic := '0';
   signal shift_enable : std_logic := '0';
   signal serial_in : std_logic := '0';

 	--Outputs
   signal parallel_out : std_logic_vector(7 downto 0);

   -- Clock period definitions
   constant clk_period : time := 10 ns;
 
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   uut: Shift_Register PORT MAP (
          clk => clk,
          reset => reset,
          shift_enable => shift_enable,
          serial_in => serial_in,
          parallel_out => parallel_out
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
		-- 1. Start with a Reset to clear the register
      reset <= '1';
      shift_enable <= '0';
      serial_in <= '0';
      wait for 100 ns;	
      
      -- Turn off reset so we can start working
      reset <= '0';
      wait for clk_period*5;
		
		-- 2. Let's send the binary byte "10100011"
      -- Remember: UART sends the LSB (Least Significant Bit) FIRST!
      -- So we send it backwards: 1, 1, 0, 0, 0, 1, 0, 1
      
      shift_enable <= '1'; -- Tell the register to start grabbing bits
		
		serial_in <= '1'; wait for clk_period; -- Send Bit 0
      serial_in <= '1'; wait for clk_period; -- Send Bit 1
      serial_in <= '0'; wait for clk_period; -- Send Bit 2
      serial_in <= '0'; wait for clk_period; -- Send Bit 3
      serial_in <= '0'; wait for clk_period; -- Send Bit 4
      serial_in <= '1'; wait for clk_period; -- Send Bit 5
      serial_in <= '0'; wait for clk_period; -- Send Bit 6
      serial_in <= '1'; wait for clk_period; -- Send Bit 7
		
		-- 3. Stop shifting and hold the data steady
      shift_enable <= '0'; 
      serial_in <= '0';

      -- Wait forever so the simulation doesn't loop

      wait;
   end process;

END;
