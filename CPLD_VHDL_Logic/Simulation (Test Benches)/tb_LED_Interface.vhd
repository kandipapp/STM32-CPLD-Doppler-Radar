LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
 
ENTITY tb_LED_Interface IS
END tb_LED_Interface;

ARCHITECTURE behavior OF tb_LED_Interface IS 
 
    -- 1. Declare the Component
    COMPONENT LED_Interface
    PORT(
         bcd_unit : IN  std_logic_vector(3 downto 0);
         bcd_tens : IN  std_logic_vector(3 downto 0);
         seg_unit : OUT  std_logic_vector(0 to 6);
         seg_unit_dp : OUT  std_logic;
         seg_tens : OUT  std_logic_vector(0 to 6);
         seg_tens_dp : OUT  std_logic
        );
    END COMPONENT;
	 
	 -- 2. Declare Virtual Wires (Signals)
    signal bcd_unit : std_logic_vector(3 downto 0) := (others => '0');
    signal bcd_tens : std_logic_vector(3 downto 0) := (others => '0');

    signal seg_unit : std_logic_vector(0 to 6);
    signal seg_unit_dp : std_logic;
    signal seg_tens : std_logic_vector(0 to 6);
    signal seg_tens_dp : std_logic;
	 
	 
BEGIN
 
    -- 3. Wire it up
	uut: LED_Interface PORT MAP (
          bcd_unit => bcd_unit,
          bcd_tens => bcd_tens,
          seg_unit => seg_unit,
          seg_unit_dp => seg_unit_dp,
          seg_tens => seg_tens,
          seg_tens_dp => seg_tens_dp
        );
		  
	 -- 4. The Stimulus Process (Testing the Logic)
    stim_proc: process
    begin		
        -- Test Case 1: Display "00"
        bcd_tens <= "0000"; bcd_unit <= "0000";
        wait for 100 ns;
		  
		  -- Test Case 2: Display "45" (Proving normal operation)
        bcd_tens <= "0100"; bcd_unit <= "0101";
        wait for 100 ns;

        -- Test Case 3: Display "91"
        bcd_tens <= "1001"; bcd_unit <= "0001";
        wait for 100 ns;

        -- Test Case 4: Inject Invalid Data "FF" (Proving the safety blanking works)
        bcd_tens <= "1111"; bcd_unit <= "1111";
        wait for 100 ns;
		  
		  wait; -- Stop Simulation
    end process;

END;
		  
		  
		  
	 