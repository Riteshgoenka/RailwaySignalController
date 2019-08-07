library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity uart_tx is
port (clk	  : in std_logic;
		rst     : in std_logic;
		txstart : in std_logic;
		sample  : in std_logic;
		txdata  : in std_logic_vector(7 downto 0);
		txdone  : out std_logic;
		tx	     : out std_logic);
end uart_tx;

architecture Behavioral of uart_tx is

	signal byte_2tx, byte_2tx_N : std_logic_vector (7 downto 0);
	signal bit_2tx, bit_2tx_N : std_logic;

	signal bitcntr_N, bitcntr : std_logic_vector (2 downto 0);
	signal tickcntr_N, tickcntr : integer;

	type state is (idle, start, data, stop);
	signal tx_Cstate, tx_Nstate : state;
begin

	tx_state_reg : process(clk, rst)
	begin
		if rst = '1' then
			tx_Cstate <= idle;
			tickcntr <= 0;
			bitcntr <= "000";
			byte_2tx <= X"00";
			bit_2tx <= '1';
		else
			if rising_edge(clk) then
				tx_Cstate <= tx_Nstate;
				tickcntr <= tickcntr_N;
				bitcntr <= bitcntr_N;
				byte_2tx <= byte_2tx_N;
				bit_2tx <= bit_2tx_N;
			end if;
		end if;
	end process tx_state_reg;
	
	tx_fsm : process(tx_Cstate,tickcntr,bitcntr,byte_2tx,sample,bit_2tx,txstart,txdata) --present state & external inputs
	begin
	
		tx_Nstate <= tx_Cstate;
		tickcntr_N <= tickcntr;
		bitcntr_N <= bitcntr;
		byte_2tx_N <= byte_2tx;
		bit_2tx_N <= bit_2tx;
		txdone <= '0';
		
		case tx_Cstate is
		
			when idle =>
				bit_2tx_N <= '1';
            txdone <= '1'; --AGK. To signal that initially TX module is ready to accept new byte 
				if txstart = '1' then -- To signal that TX module starts serializing/sending new byte on TX pin
					tx_Nstate <= start;
					tickcntr_N <= 0;
					byte_2tx_N <= txdata;
				end if;

			when start =>
				bit_2tx_N <= '0';
				if sample = '1' then
					if tickcntr = 15 then
						tx_Nstate <= data;
						tickcntr_N <= 0;
						bitcntr_N <= "000";
					else
						tickcntr_N <= tickcntr + 1;
					end if;
				end if;
				
			when data =>
				bit_2tx_N <= byte_2tx(0);
				if sample = '1' then
					if tickcntr = 15 then
						tickcntr_N <= 0;
						byte_2tx_N <= '0' & byte_2tx(7 downto 1);
						
						if bitcntr = "111" then
							tx_Nstate <= stop;
						else
							bitcntr_N <= std_logic_vector( unsigned(bitcntr) + 1);
						end if;
					else
						tickcntr_N <= tickcntr + 1;
					end if;
				end if;
				
			when stop =>
				bit_2tx_N <= '1';
				if sample = '1' then
					if tickcntr = 15 then
						tx_Nstate <= idle;
						txdone <= '1'; -- To signal 'TX module is ready to accept new byte' OR 'current byte is serialized/sent'
					else
						tickcntr_N <= tickcntr + 1;
					end if;
				end if;
		end case;	
	end process tx_fsm;
	
	tx <= bit_2tx;	

end Behavioral;