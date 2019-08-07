library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity uart_rx is		
port (clk	: in std_logic;
		rst	: in std_logic;
		rx		: in std_logic;
		sample: in STD_LOGIC;
		rxdone: out std_logic;
		rxdata: out std_logic_vector(7 downto 0));
end uart_rx;

architecture Behavioral of uart_rx is

	signal rxd_byte, rxd_byte_N : STD_LOGIC_VECTOR (7 downto 0);
	signal tickcntr, tickcntr_N : integer;
	signal bitcntr, bitcntr_N : STD_LOGIC_VECTOR (2 downto 0);
	
	type state is (idle, start, data, stop);
	signal rx_Cstate, rx_Nstate : state;
	
begin

	rx_state_reg : process(clk, rst)
	begin
		if rst = '1' then
			rx_Cstate <= idle;
			tickcntr <= 0;
			bitcntr <= "000";
			rxd_byte <= X"00";
		else
			if rising_edge(clk) then
				rx_Cstate <= rx_Nstate;
				tickcntr <= tickcntr_N;
				bitcntr <= bitcntr_N;
				rxd_byte <= rxd_byte_N;
			end if;
		end if;
	end process rx_state_reg;
	
	rx_fsm : process(rx_Cstate,tickcntr,bitcntr,rxd_byte,sample,rx) --present state & external inputs
	begin
	
		rx_Nstate <= rx_Cstate;
		tickcntr_N <= tickcntr;
		bitcntr_N <= bitcntr;
		rxd_byte_N <= rxd_byte;
		rxdone <= '0';
		
		case rx_Cstate is
		
			when idle =>
				if rx = '0' then
					rx_Nstate <= start;
					tickcntr_N <= 0;
				end if;

			when start =>
				if sample = '1' then
					if tickcntr = 7 then
						rx_Nstate <= data;
						tickcntr_N <= 0;
						bitcntr_N <= "000";
					else
						tickcntr_N <= tickcntr + 1;
					end if;
				end if;
				
			when data =>
				if sample = '1' then
					if tickcntr = 15 then
						tickcntr_N <= 0;
						rxd_byte_N <= rx & rxd_byte(7 downto 1);
						
						if bitcntr = "111" then
							rx_Nstate <= stop;
						else
							bitcntr_N <= std_logic_vector(unsigned (bitcntr) + 1);
						end if;
					else
						tickcntr_N <= tickcntr + 1;
					end if;
				end if;
				
			when stop =>
				if sample = '1' then
					if tickcntr = 15 then
						rx_Nstate <= idle;
						rxdone <= '1';
					else
						tickcntr_N <= tickcntr + 1;
					end if;
				end if;
		end case;	
	end process rx_fsm;
	
	rxdata <= rxd_byte;
				
end Behavioral;