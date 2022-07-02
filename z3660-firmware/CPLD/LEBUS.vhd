----------------------------------------------------------------------------------
--
-- Z3660
-- 
-- LEBUS equations
--
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity LEBUS_component is
    Port ( BCLK : in  STD_LOGIC;
           MAS0 : in  STD_LOGIC;
           MAS1 : in  STD_LOGIC;
           MAS2 : in  STD_LOGIC;
           MAS3 : in  STD_LOGIC;
           SLV0 : in  STD_LOGIC;
           SLV1 : in  STD_LOGIC;
           SLV2 : in  STD_LOGIC;
           SLV3 : in  STD_LOGIC;
           nRTERM : in  STD_LOGIC;
           nLSTERM : in  STD_LOGIC;
           R_W040 : in STD_LOGIC;
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

begin

   LEBUS <= LEBUS_int;

   process(BCLK)
   begin
      if(BCLK'event and  BCLK='1') then
--register
LEBUS_int(0) <= (
       ( not(nRTERM) )
    or ( not(nLSTERM) )
    or ( not(MAS1) and  MAS2 )
    or (  MAS0 and  MAS2 and  MAS3 ) );
--register
LEBUS_int(1) <= (
       ( not(nRTERM) )
    or ( not(nLSTERM) )
    or ( not(MAS0) and not(MAS1) and  MAS2 ) );
--register
LEBUS_int(2) <= (
       ( not(nRTERM) )
    or ( not(nLSTERM) )
    or (  MAS0 and not(MAS1) and  MAS2 and not(MAS3) ) );
--register
LEBUS_int(3) <= (
       ( not(nRTERM) )
    or ( not(nLSTERM) )
    or (  MAS2 and  MAS3 ) );
--register
LEBUS_int(4) <= (
       ( not(nRTERM) )
    or ( not(nLSTERM) ) );
--register
LEBUS_int(5) <= (
       ( not(nRTERM) )
    or ( not(nLSTERM) ) );
--register
LEBUS_int(6) <= (
       ( not(nRTERM) )
    or ( not(nLSTERM) ) );
--register
LEBUS_int(7) <= (
       ( not(nRTERM) )
    or ( not(nLSTERM) ) );
      end if;
   end process;

end Behavioral;

