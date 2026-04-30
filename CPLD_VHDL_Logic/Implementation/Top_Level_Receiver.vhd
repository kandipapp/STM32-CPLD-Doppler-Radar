----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    15:41:22 03/09/2026 
-- Design Name: 
-- Module Name:    Top_Level_Receiver - Structural 
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

entity Top_Level_Receiver is
    Port ( 
        -- Physical Inputs
        clk_ext     : in  STD_LOGIC;  -- The 1.8432 MHz external clock pin
        reset_btn   : in  STD_LOGIC;  -- A clear button
        rs485_rx    : in  STD_LOGIC;  -- The 1 wire coming from the RS485 chip
        
        -- Physical Outputs (to the 7-segment displays)
        seg_unit    : out STD_LOGIC_VECTOR (0 to 6);
        seg_unit_dp : out STD_LOGIC;
        seg_tens    : out STD_LOGIC_VECTOR (0 to 6);
        seg_tens_dp : out STD_LOGIC
    );
end Top_Level_Receiver;

architecture Structural of Top_Level_Receiver is

    -- 1. DECLARE THE LEGO BLOCKS (Components)
    component Clock_Divider
        Port ( clk_in : in STD_LOGIC; reset : in STD_LOGIC; clk_out : out STD_LOGIC );
    end component;

    component Control_Logic
        Port ( clk : in STD_LOGIC; reset : in STD_LOGIC; serial_in : in STD_LOGIC; shift_enable : out STD_LOGIC; data_ready : out STD_LOGIC );
    end component;

    component Shift_Register
        Port ( clk : in STD_LOGIC; reset : in STD_LOGIC; shift_enable : in STD_LOGIC; serial_in : in STD_LOGIC; parallel_out : out STD_LOGIC_VECTOR (7 downto 0) );
    end component;

    component LED_Interface
        Port ( 
				bcd_unit : in STD_LOGIC_VECTOR (3 downto 0); 
				bcd_tens : in STD_LOGIC_VECTOR (3 downto 0); 
				seg_unit : out STD_LOGIC_VECTOR (0 to 6); 
				seg_unit_dp : out STD_LOGIC; 
				seg_tens : out STD_LOGIC_VECTOR (0 to 6); 
				seg_tens_dp : out STD_LOGIC );
    end component;

    -- 2. DECLARE THE INTERNAL WIRES (Signals)
    signal w_clk_internal   : STD_LOGIC;
    signal w_shift_enable   : STD_LOGIC;
    signal w_data_ready     : STD_LOGIC;
    signal w_parallel_data  : STD_LOGIC_VECTOR(7 downto 0);
	 
	 -- NEW: Latch signal to hold the display data steady while the next byte shifts in
    signal w_display_data   : STD_LOGIC_VECTOR(7 downto 0);

begin

	-- 3. PROCESS TO LATCH DATA (Prevents 7-segment flicker)
    process(w_clk_internal, reset_btn)
    begin
        if reset_btn = '1' then
            w_display_data <= "00000000";
        elsif rising_edge(w_clk_internal) then
            -- Only update the LED displays when the full 8-bit UART byte is ready
            if w_data_ready = '1' then
                w_display_data <= w_parallel_data;
            end if;
        end if;
    end process;

    -- 4. WIRE EVERYTHING TOGETHER (Port Mapping)
    
    Inst_Clock_Divider: Clock_Divider PORT MAP (
        clk_in  => clk_ext,
        reset   => reset_btn,
        clk_out => w_clk_internal
    );

    Inst_Control_Logic: Control_Logic PORT MAP (
        clk          => w_clk_internal,
        reset        => reset_btn,
        serial_in    => rs485_rx,
        shift_enable => w_shift_enable,
        data_ready   => w_data_ready
    );

    Inst_Shift_Register: Shift_Register PORT MAP (
        clk          => w_clk_internal,
        reset        => reset_btn,
        shift_enable => w_shift_enable,
        serial_in    => rs485_rx,
        parallel_out => w_parallel_data
    );

    Inst_LED_Interface: LED_Interface PORT MAP (
        bcd_tens    => w_display_data(7 downto 4), -- Grab the Top 4 bits for Tens digit
        bcd_unit    => w_display_data(3 downto 0), -- Grab the Bottom 4 bits for Units digit
        seg_unit    => seg_unit,
        seg_unit_dp => seg_unit_dp,
        seg_tens    => seg_tens,
        seg_tens_dp => seg_tens_dp
    );

end Structural;