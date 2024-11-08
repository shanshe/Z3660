----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    12:38:30 08/20/2021 
-- Design Name: 
-- Module Name:    LEBUS - Behavioral 
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

entity LEBUS_component is
    Port ( BCLK : in  STD_LOGIC;
           MAS0 : in  STD_LOGIC;
           MAS1 : in  STD_LOGIC;
           MAS2 : in  STD_LOGIC;
           MAS3 : in  STD_LOGIC;
           nRTERM : in  STD_LOGIC;
           nLSTERM : in  STD_LOGIC;
           LEBUS : out  STD_LOGIC_VECTOR (7 downto 0));
end LEBUS_component;

architecture Behavioral of LEBUS_component is

signal LEBUS_int: STD_LOGIC_VECTOR(7 downto 0);
--LEBUS
--  0   d31..24 d31..24
--  1   d23..16 d31..24
--  2   d23..16 d23..16
--  3   d15.. 8 d31..24

--  4   d15.. 8 d15.. 8
--  5   d 7.. 0 d31..24
--  6   d 7.. 0 d23..16
--  7   d 7.. 0 d 7.. 0

-- Master State Values
CONSTANT  I: std_logic_vector(3 downto 0):= "0000";
CONSTANT  A: std_logic_vector(3 downto 0):= "0001";
CONSTANT  B: std_logic_vector(3 downto 0):= "0101";
CONSTANT  C: std_logic_vector(3 downto 0):= "1101";
CONSTANT  D: std_logic_vector(3 downto 0):= "0100";
CONSTANT  E: std_logic_vector(3 downto 0):= "1100";
CONSTANT  F: std_logic_vector(3 downto 0):= "1001";
CONSTANT  G: std_logic_vector(3 downto 0):= "1011";
CONSTANT  H: std_logic_vector(3 downto 0):= "1111";
CONSTANT  J: std_logic_vector(3 downto 0):= "1110";
CONSTANT  K: std_logic_vector(3 downto 0):= "0011";
CONSTANT  L: std_logic_vector(3 downto 0):= "0010";
CONSTANT  M: std_logic_vector(3 downto 0):= "0111";
CONSTANT  N: std_logic_vector(3 downto 0):= "0110";
CONSTANT  Z: std_logic_vector(3 downto 0):= "1000";
CONSTANT DC: std_logic_vector(3 downto 0):= "1010";

-- Slave State Values
--CONSTANT  S3: std_logic_vector(3 downto 0):= "0000";
--CONSTANT  S0: std_logic_vector(3 downto 0):= "0100";
--CONSTANT  S1: std_logic_vector(3 downto 0):= "1000";
--CONSTANT  S2: std_logic_vector(3 downto 0):= "0001";
--CONSTANT S2W: std_logic_vector(3 downto 0):= "0011";
--CONSTANT S3W: std_logic_vector(3 downto 0):= "0010";
--CONSTANT  SB: std_logic_vector(3 downto 0):= "0101";
--CONSTANT SR0: std_logic_vector(3 downto 0):= "0111";
--CONSTANT SR1: std_logic_vector(3 downto 0):= "0110";

begin

	LEBUS <= LEBUS_int;

----simplified equations
	process(BCLK)
	begin
		if(BCLK'event and  BCLK='1') then
		   if((nRTERM='0') or (nLSTERM='0')) then
				LEBUS_int(0) <= '1';
				LEBUS_int(1) <= '1';
				LEBUS_int(2) <= '1';
				LEBUS_int(3) <= '1';
				LEBUS_int(4) <= '1';
				LEBUS_int(5) <= '1';
				LEBUS_int(6) <= '1';
				LEBUS_int(7) <= '1';
			else
				LEBUS_int(0) <= ( not(MAS1) and     MAS2 )
				             or (     MAS0  and     MAS2  and  MAS3 );           -- b c d e h
				LEBUS_int(1) <= ( not(MAS0) and not(MAS1) and  MAS2 );           -- d e
				LEBUS_int(2) <= (  MAS0 and not(MAS1) and  MAS2 and not(MAS3) ); -- b    
				LEBUS_int(3) <= (  MAS2 and  MAS3 );                             -- c e h j
				LEBUS_int(4) <= '0';
				LEBUS_int(5) <= '0';
				LEBUS_int(6) <= '0';
				LEBUS_int(7) <= '0';
			end if;
		end if;
	end process;

--LEBUS <= "00000000"; -- transparent bus
--LEBUS <= "11111111"; -- maintains what was previously on the bus
-- so... 1 = "maintain"
--       0 = "ream"

end Behavioral;

