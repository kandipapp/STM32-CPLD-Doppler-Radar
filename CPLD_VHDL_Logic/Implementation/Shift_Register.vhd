----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    08:14:41 03/09/2026 
-- Design Name: 
-- Module Name:    Shift_Register - Behavioral 
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

entity Shift_Register is
	Port ( 
        clk          : in  STD_LOGIC;                     -- The internal clock (from your divider)
        reset        : in  STD_LOGIC;                     -- Clears the register
        shift_enable : in  STD_LOGIC;                     -- Tells the register WHEN to grab a bit
        serial_in    : in  STD_LOGIC;                     -- The 1 wire from the RS485 chip
        parallel_out : out STD_LOGIC_VECTOR (7 downto 0)  -- The 8 wires going to the BCD decoder
    );
end Shift_Register;

architecture Behavioral of Shift_Register is
	-- Internal memory to hold the 8 bits while they are shifting
    signal shift_reg : STD_LOGIC_VECTOR(7 downto 0) := "00000000";
begin
	process(clk, reset)
    begin
        if reset = '1' then
            shift_reg <= "00000000"; -- Clear the memory
			elsif rising_edge(clk) then
            if shift_enable = '1' then
                -- Shift the bits!
                -- UART sends the LSB (Least Significant Bit) first.
                -- So we push the new bit into the MSB (position 7), and slide everything else down.
                shift_reg <= serial_in & shift_reg(7 downto 1);
            end if;
        end if;
    end process;
	 
	 -- Send the internal memory out to the parallel output pins
    parallel_out <= shift_reg;
end Behavioral;

