----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    03:01:55 02/11/2026 
-- Design Name: 
-- Module Name:    LED_Interface - Behavioral 
-- Project Name: 
-- Target Devices: 
-- Tool versions: 
-- Description: 
--
-- Dependencies: 
--
-- Revision: 
-- Revision 0.01 - File Created
-- Additional Comments: 
--
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx primitives in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity LED_Interface is
    Port ( 
		   -- INPUTS: The 4-bit numbers coming from the STM32
         bcd_unit    : in  STD_LOGIC_VECTOR (3 downto 0);
         bcd_tens    : in  STD_LOGIC_VECTOR (3 downto 0);
			
			-- OUTPUTS: The signals going to the LEDs 
			-- "out" means the signal leaves the chip to go to the LEDs.
			-- STD_LOGIC_VECTOR (6 downto 0) means it's a bundle of 7 wires, not just one.
			seg_unit    : out  STD_LOGIC_VECTOR (0 to 6);  -- Segments a-g

			-- STD_LOGIC means this is a single wire (for the decimal point).
			seg_unit_dp : out  STD_LOGIC;                			-- Decimal Point
			seg_tens    : out  STD_LOGIC_VECTOR (0 to 6); 	-- Segments a-g
			seg_tens_dp : out  STD_LOGIC                			-- Decimal Point
        ); 
end LED_Interface;

architecture Behavioral of LED_Interface is

begin
		-- PROCESS: A "Process" block is code that runs only when signals change.
		-- We want this to run whenever 'bcd_unit' or 'bcd_tens' changes.
		process(bcd_unit, bcd_tens)
		begin
		
				-- 1. DECODER FOR UNIT DIGIT
				case bcd_unit is 
						-- Input (4-bit)      Output (7-bit: a b c d e f g) (0=ON) 
						when "0000" => seg_unit <= "0000001"; -- 0
						when "0001" => seg_unit <= "1001111"; -- 1
						when "0010" => seg_unit <= "0010010"; -- 2
						when "0011" => seg_unit <= "0000110"; -- 3
						when "0100" => seg_unit <= "1001100"; -- 4
						when "0101" => seg_unit <= "0100100"; -- 5
						when "0110" => seg_unit <= "0100000"; -- 6
						when "0111" => seg_unit <= "0001111"; -- 7
						when "1000" => seg_unit <= "0000000"; -- 8
						when "1001" => seg_unit <= "0000100"; -- 9
						
						-- "others" covers invalid inputs (A-F). We turn the display OFF (all 1s).
						-- This is CRITICAL. In hardware, you must define every possible state.
						when others => seg_unit <= "1111111";
				end case; 
				
				-- 2. DECODER FOR TENS DIGIT (Same logic, different signals)
				case bcd_tens is
						when "0000" => seg_tens <= "0000001"; -- 0
						when "0001" => seg_tens <= "1001111"; -- 1
						when "0010" => seg_tens <= "0010010"; -- 2
						when "0011" => seg_tens <= "0000110"; -- 3
						when "0100" => seg_tens <= "1001100"; -- 4
						when "0101" => seg_tens <= "0100100"; -- 5
						when "0110" => seg_tens <= "0100000"; -- 6
						when "0111" => seg_tens <= "0001111"; -- 7
						when "1000" => seg_tens <= "0000000"; -- 8
						when "1001" => seg_tens <= "0000100"; -- 9
						when others => seg_tens <= "1111111";
				end case; 
				
				-- 3. Turn off Decimal Points (Active Low: 1 = OFF)
			  seg_unit_dp <= '1';
			  seg_tens_dp <= '1';
			  
		end process;
		
end Behavioral;

