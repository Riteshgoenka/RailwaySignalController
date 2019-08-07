----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    14:18:33 01/23/2018 
-- Design Name: 
-- Module Name:    encrypter - Behavioral 
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
use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx primitives in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity encrypter is
    Port ( clock : in  STD_LOGIC;
           K : in  STD_LOGIC_VECTOR (31 downto 0);
           P : in  STD_LOGIC_VECTOR (31 downto 0);
	   done : out STD_LOGIC;
           C : out  STD_LOGIC_VECTOR (31 downto 0);
           reset : in  STD_LOGIC;
           enable : in  STD_LOGIC);
end encrypter;

architecture Behavioral of encrypter is

signal T : std_logic_vector(3 downto 0);
signal Ctemp : std_logic_vector(31 downto 0);
signal Count : integer range 0 to 33;

begin

    process(clock, reset, enable)
	 begin	 
		if (reset = '1') then
			--resets the session ie intializes Ctemp and Count 
			Ctemp <= "00000000000000000000000000000000";
			done <= '0';
			Count <= 0;
			T(3) <= k(31) xor k(27) xor k(23) xor k(19) xor k(15) xor k(11) xor k(7) xor k(3);
			T(2) <= k(30) xor k(26) xor k(22) xor k(18) xor k(14) xor k(10) xor k(6) xor k(2);
			T(1) <= k(29) xor k(25) xor k(21) xor k(17) xor k(13) xor k(9) xor k(5) xor k(1);
			T(0) <= k(28) xor k(24) xor k(20) xor k(16) xor k(12) xor k(8) xor k(4) xor k(0);

		elsif (clock'event and clock = '1' and enable = '1')  then
			if Count = 0 then
				Ctemp <= P;
				
			elsif Count < 33 and K(Count-1)='1' then
			--instead of finding N0,thid loop will be executed only when count'th bit of k is 1
			Ctemp <= Ctemp xor (T & T & T & T & T & T & T & T);
			T <= std_logic_vector(unsigned(T)+"0001");
			
			end if;
			
			if Count = 33 then
				done <= '1';
			end if;
			
			Count <= Count + 1;
			
		end if;		
		C<=Ctemp;--assigning the output to Ctemp
		
	 end process;

end Behavioral;

