--
-- Copyright (C) 2009-2012 Chris McClelland
--
-- This program is free software: you can redistribute it and/or modify
-- it under the terms of the GNU Lesser General Public License as published by
-- the Free Software Foundation, either version 3 of the License, or
-- (at your option) any later version.
--
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU Lesser General Public License for more details.
--
-- You should have received a copy of the GNU Lesser General Public License
-- along with this program.  If not, see <http://www.gnu.org/licenses/>.
--
library ieee;

use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

architecture rtl of swled is

	-- Signals used in the program
	signal flags : std_logic_vector(3 downto 0);
	signal channel_read : std_logic_vector(6 downto 0) := "0000001";
	signal channel_write : std_logic_vector(6 downto 0) := "0000000";
	signal d1,d2,e1,e2,r1,r2,data_received,ctr,wishes_fpga,wishes_uart : std_logic := '0';
	signal NextSignal,direction : std_logic_vector(23 downto 0) := (others => '0');
	signal Red,Amber,Green : std_logic_vector(23 downto 0) := (others => '0');
	signal uart_data,reg0,trainwait : std_logic_vector(7 downto 0) := (others => '0');
	signal garbage_counter,start_receiving,mc,c1,c2,c3,c4,e_n,d_n,start,count,c_256,dec_start,start_switch,oc,c_s1,wait_t0,uart_dir: integer := 0;
	signal overall_state: integer := 2;
	signal coordinates : std_logic_vector(31 downto 0) := "00000000" & "11111111" & "00000000" & "00100010";
	signal checkcoordinates,outcoordinates,fpga_data : std_logic_vector(31 downto 0) := (others => '0');
	signal ack1 : std_logic_vector(31 downto 0) := "00000000" & "00000000" & "11111111" & "11111111";
	signal ack2 : std_logic_vector(31 downto 0) := "11111111" & "11111111" & "00000000" & "00000000";
	signal outack1,outack2,p1,p2,c_1,c_2,dedata_1,dedata_2,checkack2,checkfinalack2: std_logic_vector(31 downto 0) := (others => '0');
	signal data_64,fdata_64 : std_logic_vector(63 downto 0) := (others => '0');
	signal garbage_data : std_logic_vector(31 downto 0) := "10010000" & "10000000" & "11011101" & "10101010";
	signal garbage_data2 : std_logic_vector(31 downto 0) := "10010000" & "10000000" & "11011101" & "10110100";

--  components and uart integration
	component baudrate_gen is
	port   (clk	: in std_logic;
			rst	: in std_logic;
			sample: out std_logic);
	end component baudrate_gen;
	signal sample : std_logic;

    component uart_rx is            
    port   (clk  : in std_logic;
            rst   : in std_logic;
            rx    : in std_logic;
            sample: in STD_LOGIC;
            rxdone: out std_logic;
            rxdata: out std_logic_vector(7 downto 0));
    end component uart_rx;

    signal rxdone : std_logic;
    signal rxdata : std_logic_vector(7 downto 0);
    
    component uart_tx is            
    port (clk    : in std_logic;
            rst    : in std_logic;
            txstart: in std_logic;
            sample : in std_logic;
            txdata : in std_logic_vector(7 downto 0);
            txdone : out std_logic;
            tx      : out std_logic);
    end component uart_tx;
    signal txstart: std_logic;
    signal txdata : std_logic_vector(7 downto 0);
    signal txdone : std_logic; --tx rdy to get new byte 
    signal start_left,reset_help: std_logic  := '1';
	signal c_res: integer := 1;
	signal data_rec:std_logic_vector(7 downto 0);
	

	component encrypter is
	Port ( clock : in  STD_LOGIC;
		   K : in  STD_LOGIC_VECTOR (31 downto 0);
		   P : in  STD_LOGIC_VECTOR (31 downto 0);
		   C : out  STD_LOGIC_VECTOR (31 downto 0);
		   done : out STD_LOGIC;
		   reset : in  STD_LOGIC;
		   enable : in  STD_LOGIC);
	end component;

	component decrypter is
	Port ( clock : in  STD_LOGIC;
		   K : in  STD_LOGIC_VECTOR (31 downto 0);
		   C : in  STD_LOGIC_VECTOR (31 downto 0);
		   P : out  STD_LOGIC_VECTOR (31 downto 0);
		   done : out STD_LOGIC;
		   reset : in  STD_LOGIC;
		   enable : in  STD_LOGIC);
	end component;

begin     

	--BEGIN_SNIPPET(registers)
	-- Infer registers
	-- declaring entities
	F1 : encrypter PORT MAP(clock => clk_in, K => "11001100110011001100110011000001",
							P => p1,C=> c_1,done => d1,reset => r1, enable => e1);

	F2 : decrypter PORT MAP(clock => clk_in, K => "11001100110011001100110011000001",
							P => p2,C=> c_2,done => d2,reset => r2, enable => e2);
	
	i_brg : baudrate_gen port map (clk => clk_in, rst => reset_help, sample => sample);
	
	i_rx : uart_rx port map( clk => clk_in, rst => reset_help,
                            rx => rx, sample => sample,
                            rxdone => rxdone, rxdata => rxdata);
									
	i_tx : uart_tx port map( clk => clk_in, rst => reset_help,
                            txstart => txstart,
                            sample => sample, txdata => txdata,
                            txdone => txdone, tx => tx);
	
	process(clk_in)
	begin
		if ( rising_edge(clk_in) ) then
		
			if (reset_in = '1' or reset = '1') then
	
				reg0 <= (others => '0');f2hData_out <= (others => '0');
				c1 <= 0; c2 <= 0; c3 <= 0; c4 <= 0; mc <= 0; c_256 <= 0;
				direction <= (others => '0'); NextSignal <= (others => '0');
				Red <= (others => '0'); Amber <= (others => '0'); Green <= (others => '0');
				start_receiving <= 0;e_n <= 0; d_n <= 0;start <= 0;
				count<= 0;oc <= 0;dec_start<=0;start_switch<=0;
				coordinates <= "00000000" & "11111111" & "00000000" & "00100010";
				checkcoordinates <= (others => '0') ;outcoordinates <= (others => '0');
				ack1 <="00000000" & "00000000" & "11111111" & "11111111";
				ack2 <= "11111111" & "11111111" & "00000000" & "00000000" ;
				outack1 <= (others => '0');outack2 <= (others => '0');
				checkack2<= (others => '0');checkfinalack2 <= (others => '0');
				dedata_1<= (others => '0');dedata_2<= (others => '0');
				fdata_64 <= (others => '0');data_64 <= (others => '0');
                trainwait<=(others => '0');data_received <= '0';c_res <=1; ctr <= '0';
				overall_state <= 1;c_s1 <=0;wait_t0<=0;
				wishes_uart <= '0'; wishes_fpga <= '0';

			elsif (overall_state=1 and c_s1 < 48000000*3) then
				reg0 <= "11111111";
				c_s1 <= c_s1+1;

			elsif (overall_state = 1 and c_s1 = 48000000*3) then
				reg0 <= "00000000";
				overall_state <= 2;

			end if;

			if ( c_res = 1) then
					reset_help <='1';
					c_res <=2;
			elsif (c_res = 2) then
					c_res <=3;
					reset_help <= '0';
			end if;

			-- encrypting coordinates, ack1 and ack2
			if (e_n = 0) then
				if(count = 0) then
					r1 <='1';e1<='0';count<=count+1;
				elsif (count = 1) then
					r1<='0';e1<='1';p1<=coordinates;count<=count+1;
				elsif (count = 2 and d1 = '1') then
					outcoordinates <= c_1;e_n<=e_n+1;count <=0; 
				end if;

			elsif (e_n = 1) then
				if(count = 0) then
					r1 <='1';e1<='0';count<=count+1;
				elsif (count = 1) then
					r1<='0';e1<='1';p1<=ack1;count<=count+1;
				elsif (count = 2 and d1 = '1') then
					outack1 <= c_1;e_n<=e_n+1;count <=0; 
				end if;

			elsif (e_n = 2) then
				if(count = 0) then
					r1 <='1';e1<='0';count<=count+1;
				elsif (count = 1) then
					r1<='0';e1<='1';p1<=ack2;count<=count+1;
				elsif (count = 2 and d1 = '1') then
					outack2 <= c_1;e_n<=e_n+1;count <=0;
				end if;
			
			end if ;
			
			-- sending encrypted coordinates
			if (f2hReady_in = '0' ) then
				f2hValid_out <= '0';
			end if;

			if (f2hReady_in = '1' and overall_state = 2 ) then
				f2hValid_out <= '1';
			end if;
			
			if(overall_state = 2 and chanAddr_in = channel_write and f2hReady_in = '1' and e_n=3 and mc >=0  and mc < 4) then
				f2hData_out <= outcoordinates(7+8*(mc mod 4) downto 0+8*(mc mod 4));
				mc <= mc+1;
			end if;
			
			if (mc = 4 and h2fValid_in = '0' and c_256 <= 48000000*5) then
				c_256 <= c_256 +1;
				if(c_256 = 48000000*5) then
					c_256 <=0;
					mc <= 0; --if not received within 256 sec go back to C1
				end if;
			end if;
			
			-- reading coordinates to check 
			if(chanAddr_in = channel_read and h2fValid_in = '1' and mc >= 4 and mc < 8 and e_n=3 and c_256 < 48000000*5) then
				checkcoordinates(7+8*(mc mod 4) downto 8*(mc mod 4)) <= h2fData_in;
				mc <=mc+1;
				c_256 <= 0;
			end if;
			

			-- sending ack or coordinates depending on checkcoordinates
			if(chanAddr_in = channel_write and mc >= 8 and mc < 12 and f2hReady_in = '1' and e_n=3) then
				if(checkcoordinates=outcoordinates) then
					f2hData_out <= outack1(7+8*(mc mod 4) downto 0+8*(mc mod 4));
					mc <= mc+1;
				else
					mc <= 0; --if wrong coordinates go back to C1
				end if;
			end if;
			
			if (mc = 12 and h2fValid_in = '0' and c_256 <= 48000000*5) then
				c_256 <= c_256 +1;
				if(c_256 = 48000000*5) then
					c_256 <=0;
					mc <= 0; --if not received within 256 sec go back to C1
				end if;
			end if;
			
			-- reading ack2 after confirming that the coordinates sent are correct
			if(chanAddr_in = channel_read and h2fValid_in = '1' and mc>=12 and mc<16 and e_n=3 and c_256 < 48000000*5) then
				checkack2(7+8*(mc mod 4) downto 8*(mc mod 4)) <= h2fData_in;
				mc<=mc+1;
				c_256 <= 0;
			end if;
			
			-- checking whether correct ack2 is recieved or not
			if(start_receiving=0 and mc=16 and e_n=3) then
				if(checkack2 = outack2) then
					start_receiving <= 1;
				else
					mc <= 0; ---if wrong ack2 go back to C1
				end if;
			end if;
			
			-- reading the first half encrypted message 
			if(start_receiving =1 and chanAddr_in = channel_read and h2fValid_in = '1' and mc>=16 and mc<20 and e_n=3) then
				data_64(7+8*(mc mod 4) downto 8*(mc mod 4)) <= h2fData_in;
				mc <= mc+1;
			end if;
			
			-- sending encrypted ack1
			if(chanAddr_in = channel_write and f2hReady_in = '1' and mc >= 20 and mc<24 and e_n=3) then
					f2hData_out <= outack1(7+8*(mc mod 4) downto 0+8*(mc mod 4));
					mc <= mc+1;
			end if;
			
			-- reading second half encr message
			if(chanAddr_in = channel_read and h2fValid_in = '1' and mc>=24 and mc<28 and e_n=3) then
				data_64(32+7+8*(mc mod 4) downto 32+8*(mc mod 4)) <= h2fData_in;
				mc <= mc +1;
			end if;
			
			-- sending encrypted ack1 
			if(chanAddr_in = channel_write and f2hReady_in = '1' and mc>=28 and mc<32 and e_n=3) then
					f2hData_out <= outack1(7+8*(mc mod 4) downto 0+8*(mc mod 4));
					mc <= mc+1;
			end if;
			
			if (mc = 32 and h2fValid_in = '0' and c_256 <= 48000000*5) then
				c_256 <= c_256 +1;
				if(c_256 = 48000000*5) then
					c_256 <=0;
					mc <= 0; ---if not received within 256 sec go back to C1
				end if;
			end if;
			
			-- reading ack2
			if(chanAddr_in = channel_read and h2fValid_in = '1' and mc>=32 and mc<36 and e_n=3 and c_256 < 48000000*5) then
				checkfinalack2(7+8*(mc mod 4) downto 8*(mc mod 4)) <= h2fData_in;
				c_256<=0;
				mc<=mc+1;
			end if; 
			
			if(dec_start=0 and mc=36 and e_n=3) then
				if(checkfinalack2 = outack2) then
					dec_start <= 1;
				else
					mc <= 0; ---if wrong ack2 go back to C1
				end if;
			end if;
			
			-- decrypting the received first half of data
			if ( dec_start=1 and d_n = 0 and e_n = 3 and mc=36 and start_switch=0 ) then
				if ( count = 0 ) then
					r2 <= '1'; e2 <= '0';
					count <= count+1;
				elsif (count = 1) then
					r2 <= '0'; e2 <= '1';
					c_2 <= data_64(31 downto 0);
					count <= count+1;
				elsif (count = 2 and d2 = '1') then
					dedata_1 <= p2; d_n <= d_n+1;
					count <= 0; 
				end if;

			-- decrypting the received second half of data
			elsif ( d_n = 1 and e_n = 3 and mc=36 and start_switch=0 ) then
				if(count = 0) then
					r2 <= '1';e2 <= '0';
					count <= count+1;
				elsif (count = 1) then
					r2 <= '0';e2 <= '1';
					c_2 <= data_64(63 downto 32);
					count <= count+1;
				elsif (count = 2 and d2 = '1') then
					dedata_2 <= p2;
					d_n <= d_n+1;
					count <= 0; 
				end if;
			end if;
			
			-- combing both decrypted datas
			if ( mc=36 and start_switch=0 and d_n = 2 and e_n = 3 and start=0 ) then
				start_switch <= 1;
				fdata_64 <= dedata_2 & dedata_1; 
			end if;
			if ( start_switch = 1 and start = 0 ) then
				trainwait <= sw_in;
				start <= 1;
			end if;

			if ( c1 < 8 and c2 = 0 and e_n = 3 and start = 1 ) then
				if( fdata_64(8*c1+4) = '0' or fdata_64(8*c1+3) = '0' or trainwait(c1)='0' or 
					(c1 = unsigned(uart_data(7 downto 5)) and uart_data(3) = '0' and uart_data(4) = '1') ) then
					Red(3*c1) <= '1';
					Red(3*c1+1) <= '1';
					Red(3*c1+2) <= '1';

				elsif ( fdata_64(8*c1+3) = '1' and trainwait(c1)='1' ) then
					if ( c1 >= 4 )then
						if ( trainwait(c1-4) = '0' ) then
							if ( ((unsigned(fdata_64(8*c1+2 downto 8*c1)) = 1) and
							 	   not(c1 = unsigned(uart_data(7 downto 5)))) or
								 ((unsigned(uart_data(2 downto 0)) = 1) and
								  (c1 = unsigned(uart_data(7 downto 5)))) ) then
								Amber(3*c1) <= '1';
								Amber(3*c1+1) <= '1';
								Amber(3*c1+2) <= '1';
							else    
								Green(3*c1) <= '1';
								Green(3*c1+1) <= '1';
								Green(3*c1+2) <= '1';
							end if;
						elsif ( trainwait(c1-4)='1' ) then
							Green(3*c1) <= '1';
							Amber(3*c1+1) <= '1';
							Red(3*c1+2) <= '1';
						end if;
					else
						if ( trainwait(c1+4)='0' ) then
							if ( ((unsigned(fdata_64(8*c1+2 downto 8*c1)) = 1) and
							       not(c1 = unsigned(uart_data(7 downto 5)))) or
								 ((unsigned(uart_data(2 downto 0)) = 1) and
								  (c1 = unsigned(uart_data(7 downto 5)))) ) then
								Amber(3*c1) <= '1';
								Amber(3*c1+1) <= '1';
								Amber(3*c1+2) <= '1';
							else
								Green(3*c1) <= '1';
								Green(3*c1+1) <= '1';
								Green(3*c1+2) <= '1';
							end if;
						elsif ( trainwait(c1+4)='1' ) then
							Red(3*c1) <= '1';
							Red(3*c1+1) <= '1';
							Red(3*c1+2) <= '1';
						end if;
					end if;
				end if;
				c1 <= c1 + 1;
				direction(3*c1+2 downto 3*c1) <= fdata_64(7+8*c1 downto 5+8*c1);
			
			elsif ( c1=8 and c2 < 24 and e_n=3 and start=1 ) then
				if ( c3=48000000 ) then
					reg0 <= direction(3*oc+2 downto 3*oc) & "00" & Green(c2) & Amber(c2) & Red(c2);
					c2 <= c2+1;
					c3 <= 0;
				else 
					c3 <= c3+1;
				end if;
			end if;
			
			if ( c1 = 8 and c2 mod 3 = 2 ) then 
				oc <= oc+1;
			end if;

			if ( c2 = 24 ) then
				overall_state <= 3;
				reg0 <= "00000000";
				c2 <= 25;
			end if;

			if ( overall_state = 3 ) then
				if ( up = '1' and wishes_fpga ='0' ) then
					wishes_fpga <= '1';
				elsif ( wishes_fpga = '1' ) then
					if ( down = '1' ) then
						if ( count = 0 ) then
							r1 <= '1';e1 <= '0';
							count <= 1;
						elsif ( count = 1 ) then
							r1 <= '0'; e1 <= '1';
							p1 <= sw_in & sw_in & sw_in & sw_in;
							count <= 2;
						elsif (count = 2 and d1 = '1' ) then
							fpga_data <= c_1;
							count <= 3;
						elsif ( count >= 3 and count < 7 and chanAddr_in = channel_write and f2hReady_in = '1' ) then 
							f2hValid_out <= '1';
							f2hData_out <= fpga_data(8*(count-3)+7 downto 8*(count - 3));
							count <= count + 1;
						elsif ( count = 7 ) then 
							overall_state <= 4;
							count <= 0;
						end if;
					end if;
				else
					overall_state <= 4;
				end if;
			end if;

			if ( rxdone = '1' and overall_state /= 5 ) then
				data_received <= '1';
			end if;

			if ( overall_state = 4 ) then
				if ( left = '1' and wishes_uart = '0' ) then
					wishes_uart <= '1';
				elsif ( wishes_uart = '1' ) then
					if ( right = '1' ) then
						if ( txstart = '0' ) then
							txstart <= '1';
							txdata <= sw_in;
						elsif ( txstart = '1' ) then
							txstart <= '0';
							overall_state <= 5;
						end if;
					end if;
				else
					overall_state <= 5;
				end if;
			end if;

			if ( overall_state = 5 ) then
				if ( data_received = '1' ) then
					uart_data <= rxdata;
					data_received <= '0';
					overall_state <= 6;
				else
					uart_data <= "00000000";
					overall_state <= 6;
				end if;
			end if;

			if ( overall_state = 6 ) then
				wait_t0 <= wait_t0 + 1;
				if ( wait_t0 = 48000000*5 ) then
					c1 <= 0; c2 <= 0; c3 <= 0; c4 <= 0; mc <= 0; c_256 <= 0;
					f2hData_out <= (others => '0');trainwait <= (others => '0');
					direction <= (others => '0'); NextSignal <= (others => '0');
					Red <= (others => '0'); Amber <= (others => '0'); Green <= (others => '0'); 
					start_receiving <= 0;d_n <= 0; start <= 0;
					count <= 0; oc <= 0; dec_start <= 0; start_switch <= 0;
					checkcoordinates <= (others => '0');
					checkack2 <= (others => '0'); checkfinalack2 <= (others => '0');
					dedata_1 <= (others => '0'); dedata_2 <= (others => '0');
					fdata_64 <= (others => '0'); data_64 <= (others => '0');
					overall_state <= 2; c_s1 <= 0; wait_t0 <= 0;
					wishes_uart <= '0'; wishes_fpga <= '0';
				end if;
			end if;

		end if;
			
	end process;
	
	
	-- Assert that there's always data for reading, and always room for writing
	h2fReady_out <= '1';
	--END_SNIPPET(registers)

	-- LEDs and 7-seg display
	led_out <= reg0;
	flags <= "00" & f2hReady_in & reset_in;
	seven_seg : entity work.seven_seg
		port map(
			clk_in     => clk_in,
			data_in    => reg0 & reg0 ,
			dots_in    => flags,
			segs_out   => sseg_out,
			anodes_out => anode_out
		);
	
end architecture;