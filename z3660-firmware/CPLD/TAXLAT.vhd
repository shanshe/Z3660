----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    11:13:47 08/20/2021 
-- Design Name: 
-- Module Name:    TAXLAT - Behavioral 
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

entity TAXLAT is
    Port ( BCLK : in  STD_LOGIC;
           p040A0 : in  STD_LOGIC;
           p040A1 : in  STD_LOGIC;
           p040A2 : in  STD_LOGIC;
           p040A3 : in  STD_LOGIC;
           SIZ40 : in  STD_LOGIC_VECTOR (1 downto 0);
           TT : in  STD_LOGIC_VECTOR (1 downto 0);
           TM : in  STD_LOGIC_VECTOR (2 downto 0);
           MAS0 : in  STD_LOGIC;
           MAS1 : in  STD_LOGIC;
           MAS2 : in  STD_LOGIC;
           MAS3 : in  STD_LOGIC;
           FC : out  STD_LOGIC_VECTOR (2 downto 0);
           SIZ : out  STD_LOGIC_VECTOR (1 downto 0);
           A : out  STD_LOGIC_VECTOR (3 downto 0);
           nIACK : out  STD_LOGIC;
           nBGACK040 : in  STD_LOGIC);
end TAXLAT;

architecture Behavioral of TAXLAT is
signal MAS_state: std_logic_vector(3 downto 0);
signal address: std_logic_vector(1 downto 0);
--SIZ40 values
CONSTANT long: std_logic_vector(1 downto 0):= "00";
CONSTANT byte: std_logic_vector(1 downto 0):= "01";
CONSTANT word: std_logic_vector(1 downto 0):= "10";
CONSTANT line: std_logic_vector(1 downto 0):= "11";

-- Master State Values
CONSTANT  I: std_logic_vector(3 downto 0):= "0000";
CONSTANT  Astate: std_logic_vector(3 downto 0):= "0001";
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

CONSTANT norm   : std_logic_vector(1 downto 0):= "00";
CONSTANT mov16  : std_logic_vector(1 downto 0):= "01";
CONSTANT alt    : std_logic_vector(1 downto 0):= "10";
CONSTANT intack : std_logic_vector(1 downto 0):= "11";

CONSTANT sup_dat : std_logic_vector(2 downto 0):= "100";
CONSTANT sup_cod : std_logic_vector(2 downto 0):= "110";
CONSTANT usr_dat : std_logic_vector(2 downto 0):= "000";
CONSTANT usr_cod : std_logic_vector(2 downto 0):= "010";
CONSTANT cpu_spc : std_logic_vector(2 downto 0):= "111";

begin
MAS_state <= MAS3 & MAS2 & MAS1 & MAS0;
address <= p040A1 & p040A0;

nIACK <= '0' when (address="11" and TT=intack) else '1';

--nIACK <= not(
--        ( p040A0 and p040A1 and TT(0) and TT(1) ) );

--	process(nBGACK040,MAS_state,p040A0,p040A1,p040A2,p040A3,TM,SIZ40,TT)--,nIACK)
--	begin
--		if(nBGACK040='0') then
--			if ( MAS_state=I or MAS_state=Z or MAS_state=DC ) then
--				A(0) <= p040A0;
--			elsif( MAS_state=C or MAS_state=E or MAS_state=H or MAS_state=J or MAS_state=L or MAS_state=N )  then
--				A(0) <= '1';
--			else
--				A(0) <= '0';
--			end if;
----			if(nIACK='0') then
--			if(( p040A0 and p040A1 and TT(0) and TT(1) )='1') then
--				A(3 downto 1) <= TM(2 downto 0);
--			else
--				if ( MAS_state=I or MAS_state=Z or MAS_state=DC ) then
--					A(1) <= p040A1;
--				elsif( MAS_state=B or MAS_state=D or MAS_state=E or MAS_state=G or MAS_state=J or MAS_state=M or MAS_state=N ) then
--					A(1) <= '1';
--				else
--					A(1) <= '0';
--				end if;
--				A(2) <= p040A2;
--				A(3) <= p040A3;
--			end if;
--			if ( MAS_state=I or MAS_state=Z or MAS_state=DC ) then
--				SIZ <= SIZ40;
--			elsif( MAS_state=B or MAS_state=D or MAS_state=F or MAS_state=G ) then
--				SIZ <= word;
--			elsif( MAS_state=C or MAS_state=E or MAS_state=H or MAS_state=J or MAS_state=K or MAS_state=L or MAS_state=M or MAS_state=N ) then
--				SIZ <= byte;
--			else
--				SIZ <= long;
--			end if;
--			if (TT=intack) then
--				FC <= cpu_spc;
--			elsif (TT=mov16) then
--				if(TM(2)='0') then
--					FC <= usr_dat;
--				else
--					FC <= sup_dat;
--				end if;
--			elsif (TT=alt) then
--				FC <= TM;
--			else --if (TT=norm) then
--				if(TM=usr_cod or TM=sup_cod) then
--					FC <= sup_cod;
--				else
--					FC <= sup_dat;
--				end if;
--			end if;
--		else
--			A <= "HHHH";
--			SIZ <= "HH";
--			FC <= "HHH";
--		end if;
--	end process;

-- original equations
--	process(nBGACK040,MAS0,MAS1,MAS2,MAS3,p040A0,p040A1,p040A2,p040A3,TM,SIZ40,TT)--,nIACK)
	process(BCLK)
	begin
		if(BCLK'event and  BCLK='1') then
		if(nBGACK040='0') then
A(0) <= (
        ( p040A1 and not(MAS0) and not(MAS2) and not(MAS1) )     -- I Z
     or ( p040A1 and MAS3 and not(MAS0) and not(MAS2) and MAS1 ) -- DC
     or ( not(MAS3) and not(MAS0) and MAS1 ) -- L N
     or ( MAS3 and MAS2 ) );                 -- C E H J
--			if(nIACK='0') then
			if(( p040A0 and p040A1 and TT(0) and TT(1) )='1') then
				A(1) <= TM(0);
				A(2) <= TM(1);
				A(3) <= TM(2);
			else
A(1) <= (
        ( p040A0 and not(MAS0) and not(MAS2) and not(MAS1) )     -- I Z
     or ( p040A0 and MAS3 and not(MAS0) and not(MAS2) and MAS1 ) -- DC
     or ( not(MAS3) and MAS2 )                       -- B D M N
     or ( MAS3 and not(MAS0) and MAS2 )              -- E J
     or ( MAS3 and MAS0 and not(MAS2) and MAS1 ) );  -- G
A(2) <= p040A2;
A(3) <= p040A3;
			end if;
SIZ(1) <= (
        ( SIZ40(0) and not(SIZ40(1)) and not(MAS3) and not(MAS0) and not(MAS2) and not(MAS1) )
     or ( SIZ40(0) and MAS3 and not(MAS0) and not(MAS2) )
     or ( not(MAS3) and MAS2 and not(MAS1) ) -- B D
     or ( MAS3 and MAS0 and not(MAS2) ) );   -- F G
SIZ(0) <= (
        ( not(SIZ40(0)) and SIZ40(1) and not(MAS3) and not(MAS0) and not(MAS2) and not(MAS1) ) -- I
     or ( SIZ40(1) and MAS3 and not(MAS0) and not(MAS2) )                                      -- Z DC
     or ( not(MAS3) and MAS1 ) -- K L M N
     or ( MAS3 and MAS2 ) ); -- C E H J
FC(2) <= (
        ( TT(0) and TT(1) )
     or ( not(TT(0)) and TT(1) and TM(2) )
     or ( TT(0) and not(TT(1)) and TM(2) )
     or ( not(TT(0)) and not(TT(1)) and not(TM(1)) and not(TM(0)) )
     or ( not(TT(0)) and not(TT(1)) and TM(1) and TM(0) )
     or ( not(TT(0)) and not(TT(1)) and TM(2) and not(TM(1)) and TM(0) )
     or ( not(TT(0)) and not(TT(1)) and TM(2) and TM(1) and not(TM(0)) ) );
FC(1) <= (
        ( TT(0) and TT(1) )
     or ( TT(0) and not(TT(1)) and TM(1) )
     or ( not(TT(0)) and not(TT(1)) and TM(1) and not(TM(0)) ) );
FC(0) <= (
        ( TT(0) and TT(1) )
     or ( not(TT(0)) and TT(1) and TM(0) )
     or ( TT(0) and not(TT(1)) and TM(0) )
     or ( not(TT(0)) and not(TT(1)) and not(TM(1)) )
     or ( not(TT(0)) and not(TT(1)) and TM(1) and TM(0) ) );
		else
			A <= "HHHH";
			SIZ <= "HH";
			FC <= "HHH";
		end if;
		end if;
	end process;

end Behavioral;

