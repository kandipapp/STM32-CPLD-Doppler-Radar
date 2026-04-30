----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    08:02:25 03/09/2026 
-- Design Name: 
-- Module Name:    Clock_Divider - Behavioral 
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
use IEEE.STD_LOGIC_UNSIGNED.ALL; -- This allows us to do math like "count + 1"

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx primitives in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity Clock_Divider is
Port ( 
        clk_in  : in  STD_LOGIC;  -- The 1.8432 MHz external clock
        reset   : in  STD_LOGIC;  -- Optional: A button to reset the clock
        clk_out : out STD_LOGIC   -- The 460.8 kHz internal clock (Divide by 4)
    );
end Clock_Divider;

architecture Behavioral of Clock_Divider is
	-- Create a 2-bit internal counter (counts 0, 1, 2, 3...)
    signal count : STD_LOGIC_VECTOR(1 downto 0) := "00";
begin
	 -- This process triggers every time the external clock ticks
    process(clk_in, reset)
    begin
		  if reset = '1' then
            count <= "00"; -- Reset the counter to 0
        elsif rising_edge(clk_in) then
            count <= count + 1; -- Increment the counter by 1
        end if;
    end process;
	 
	 -- The highest bit of a 2-bit counter toggles at exactly 1/4th the speed of the main clock!
    -- So we just connect our output wire directly to that highest bit.
    clk_out <= count(1);
	 
end Behavioral;

