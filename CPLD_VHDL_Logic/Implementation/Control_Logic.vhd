library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity Control_Logic is
    Port ( 
        clk          : in  STD_LOGIC;  
        reset        : in  STD_LOGIC;  
        serial_in    : in  STD_LOGIC;  
        shift_enable : out STD_LOGIC;  
        data_ready   : out STD_LOGIC   
    );
end Control_Logic;

architecture Behavioral of Control_Logic is

    -- Define the 4 States
    type state_type is (IDLE, START_BIT, DATA_BITS, STOP_BIT);
    signal current_state : state_type := IDLE;

    -- Counters for timing and tracking bits
    signal tick_count : integer range 0 to 15 := 0; 
    signal bit_count  : integer range 0 to 8 := 0;

begin

    process(clk, reset)
    begin
        if reset = '1' then
            current_state <= IDLE;
            tick_count <= 0;
            bit_count <= 0;
            shift_enable <= '0';
            data_ready <= '0';
            
        elsif rising_edge(clk) then
            -- Default to keeping these low unless we explicitly turn them on
            shift_enable <= '0';
            data_ready <= '0';
            
            case current_state is
                
                -- STATE 0: Wait for line to drop to '0'
                when IDLE =>
                    tick_count <= 0;
                    bit_count <= 0;
                    if serial_in = '0' then
                        current_state <= START_BIT;
                    end if;
                    
                -- STATE 1: Confirm Start Bit 
                when START_BIT =>
                    if tick_count = 3 then 
                        if serial_in = '0' then 
                            tick_count <= 0;
                            current_state <= DATA_BITS;
                        else
                            current_state <= IDLE; 
                        end if;
                    else
                        tick_count <= tick_count + 1;
                    end if;
                    
                -- STATE 2: Read the 8 Data Bits
                when DATA_BITS =>
                    if tick_count = 7 then 
                        tick_count <= 0;
                        shift_enable <= '1'; 
                        
                        if bit_count = 7 then 
                            current_state <= STOP_BIT;
                        else
                            bit_count <= bit_count + 1;
                        end if;
                    else
                        tick_count <= tick_count + 1;
                    end if;
                    
                -- STATE 3: Stop Bit
                when STOP_BIT =>
                    if tick_count = 7 then 
                        data_ready <= '1'; 
                        current_state <= IDLE;
                    else
                        tick_count <= tick_count + 1;
                    end if;
                    
            end case;
        end if;
    end process;

end Behavioral;