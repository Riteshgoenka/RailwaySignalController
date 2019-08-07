library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity baudrate_gen is
port(clk		: in std_logic;
	  rst		: in std_logic;
	  sample	: out std_logic);
end baudrate_gen;

architecture Behavioral of baudrate_gen is

	signal sample_counter : integer;
	
begin
	sampling: process (clk,rst) is
	begin
		if rst = '1' then
			sample <= '0';
			sample_counter <= 0;
		else
			if rising_edge(clk) then
				if sample_counter = 26 then -- baudrate 115200; 16*baudrate => divisor 54
				  sample <= '1';
				  sample_counter <= 0;
				else
				  sample <= '0';
				  sample_counter <= sample_counter + 1;
				end if;
			end if;
		end if;
	end process sampling;
end Behavioral;