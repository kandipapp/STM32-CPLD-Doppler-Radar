LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
 
ENTITY tb_Clock_Divider IS
END tb_Clock_Divider;
 
ARCHITECTURE behavior OF tb_Clock_Divider IS

	-- 1. Declare the Unit Under Test (UUT)
    COMPONENT Clock_Divider
    PORT(
         clk_in : IN  std_logic;
         reset : IN  std_logic;
         clk_out : OUT  std_logic
        );
    END COMPONENT;
	 
	 -- 2. Create the virtual wires (Signals)
    signal clk_in : std_logic := '0';
    signal reset : std_logic := '0';
    signal clk_out : std_logic;
	 
	 -- 3. Define the clock speed
    -- 1.8432 MHz is roughly a 543 ns period
    constant clk_in_period : time := 543 ns;
	 
BEGIN
 
    -- 4. Plug the virtual wires into your hardware block
    uut: Clock_Divider PORT MAP (
          clk_in => clk_in,
          reset => reset,
          clk_out => clk_out
        );
		  
	 -- 5. The Clock Generator Process
    -- This creates the infinite pulsing of the 1.8432 MHz crystal
    clk_process :process
    begin
        clk_in <= '0';
        wait for clk_in_period/2;
        clk_in <= '1';
        wait for clk_in_period/2;
    end process;
	 
	 -- 6. The Stimulus Process
    stim_proc: process
    begin		
        -- Start by hitting the reset button to clear the counter
        reset <= '1';
        wait for 100 ns;	
        
        -- Release the reset button and let it run!
        reset <= '0';

        -- Let the simulation run for 20 clock cycles to observe the division
        wait for clk_in_period * 20;

        -- Stop the simulation so it doesn't loop forever
        wait;
	 end process;

END;