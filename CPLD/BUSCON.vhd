----------------------------------------------------------------------------------
--
-- Z3660
-- 
-- BUSCON equations
--
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity BUSCON is
    Port ( BCLK : in  STD_LOGIC;
           nBGACK040 : in  STD_LOGIC;
           p040A0 : in  STD_LOGIC;
           p040A1 : in  STD_LOGIC;
           SIZ40 : in  STD_LOGIC_VECTOR (1 downto 0);
           nETERM : in  STD_LOGIC;
           nRDSACK1 : in  STD_LOGIC;
           nRDSACK0 : in  STD_LOGIC;
           nRBERR : in  STD_LOGIC;
--           nRHALT : in  STD_LOGIC;
           nRAVEC : in  STD_LOGIC;
           nCYCPEND : in  STD_LOGIC;
           n040RSTI : in  STD_LOGIC;
           nLSTERM : in  STD_LOGIC;
           SLV0 : buffer  STD_LOGIC;
           SLV1 : buffer  STD_LOGIC;
           SLV2 : buffer  STD_LOGIC;
           SLV3 : buffer  STD_LOGIC;
           MAS0 : buffer  STD_LOGIC;
           MAS1 : buffer  STD_LOGIC;
           MAS2 : buffer  STD_LOGIC;
           MAS3 : buffer  STD_LOGIC;
--           nDS040 : out  STD_LOGIC;
--           nAS040 : out  STD_LOGIC;
           R_W040 : in STD_LOGIC;
--           nTBI : out  STD_LOGIC;
--           nTEA : out  STD_LOGIC;
--           nTA : out  STD_LOGIC;
           nTS : in  STD_LOGIC
           );
end BUSCON;

architecture Behavioral of BUSCON is

impure function byteport return std_logic is begin
  return (nRBERR and not(nRDSACK0) and nRDSACK1); end byteport;
impure function wordport return std_logic is begin
  return (nRBERR and nRDSACK0 and not(nRDSACK1)); end wordport;
impure function longport return std_logic is begin
  return (nRBERR and not(nRDSACK0) and not(nRDSACK1)); end longport;
impure function noport return std_logic is begin
  return (nRBERR and nRDSACK0 and nRDSACK1); end noport;

begin

SLV3 <= not(
        ( not(SLV1) and SLV2 and not(SLV0) )
     or ( SLV1 and not(SLV2) and not(SLV0) ) );
   process(BCLK,n040RSTI)
   begin
      if(n040RSTI='0') then
         SLV0 <= '0';
         SLV1 <= '0';
         SLV2 <= '0';
--         SLV3 <= '0';
         MAS0 <= '0';
         MAS1 <= '0';
         MAS2 <= '0';
         MAS3 <= '0';
      elsif(BCLK'event and  BCLK='1') then

--register
MAS1 <= (
        ( MAS1 and MAS2 and not(SLV0) )
     or ( MAS1 and not(MAS3) and not(SLV0) )
     or ( MAS1 and MAS0 and not(SLV0) and n040RSTI )
     or ( MAS0 and not(MAS2) and MAS3 and SLV0 and byteport )
     or ( not(nTS) and not(MAS1) and not(MAS0) and not(MAS2) and SIZ40(0) and not(MAS3) and not(SIZ40(1)) )
     or ( not(nTS) and not(MAS1) and not(p040A0) and not(MAS0) and p040A1 and not(MAS2) and not(SIZ40(0)) and not(MAS3) and SIZ40(1) ) );
--register
MAS0 <= (
        ( MAS0 and not(SLV0) and n040RSTI )
     or ( MAS1 and MAS0 and MAS2 and not(SLV0) )
     or ( MAS0 and not(MAS2) and not(MAS3) and not(SLV0) )
     or ( not(nTS) and not(MAS1) and not(p040A0) and not(MAS0) and not(MAS2) and not(MAS3) )
     or ( not(MAS1) and MAS0 and not(MAS2) and not(MAS3) and wordport )
     or ( not(MAS1) and MAS0 and not(MAS2) and SLV0 and byteport )
     or ( not(nTS) and not(MAS1) and not(MAS0) and not(MAS2) and SIZ40(0) and not(MAS3) and SIZ40(1) )
     or ( not(nTS) and not(MAS1) and not(MAS0) and not(MAS2) and not(SIZ40(0)) and not(MAS3) and not(SIZ40(1)) ) );
--register
MAS2 <= (
        ( MAS1 and MAS2 and not(SLV0) )
     or ( MAS2 and not(SLV0) and n040RSTI )
     or ( not(MAS1) and MAS0 and not(MAS2) and SLV0 and byteport )
     or ( not(MAS1) and MAS0 and MAS3 and SLV0 and byteport )
     or ( MAS0 and not(MAS2) and MAS3 and SLV0 and byteport )
     or ( not(MAS1) and MAS0 and not(MAS2) and not(MAS3) and SLV0 and wordport )
     or ( not(MAS1) and not(MAS0) and MAS2 and not(MAS3) and SLV0 and byteport )
     or ( not(nTS) and not(MAS1) and not(MAS0) and p040A1 and not(MAS2) and SIZ40(0) and not(MAS3) and not(SIZ40(1)) ) );
--register
MAS3 <= (
        ( MAS0 and SLV0 and not(nRBERR) )
     or ( MAS2 and SLV0 and not(nRBERR) )
     or ( MAS1 and not(MAS3) and SLV0 and not(nRBERR) )
     or ( MAS1 and MAS2 and MAS3 and not(SLV0) )
     or ( MAS0 and MAS3 and not(SLV0) and n040RSTI )
     or ( MAS2 and MAS3 and not(SLV0) and n040RSTI )
     or ( not(MAS1) and MAS0 and not(MAS2) and nRDSACK1 and SLV0 and not(nRDSACK0) )
     or ( MAS0 and not(MAS2) and MAS3 and nRDSACK1 and SLV0 and not(nRDSACK0) )
     or ( not(MAS1) and not(MAS0) and MAS2 and not(MAS3) and nRDSACK1 and SLV0 and not(nRDSACK0) )
     or ( not(nTS) and not(MAS1) and not(p040A0) and not(MAS0) and not(MAS2) and not(SIZ40(0)) and not(MAS3) and SIZ40(1) ) );

--register
SLV1 <= (
        ( SLV1 and not(SLV2) and SLV0 and not(nRBERR) )
     or ( MAS0 and not(SLV1) and not(SLV2) and not(SLV0) )
     or ( MAS1 and not(SLV1) and not(SLV2) and not(SLV0) )
     or ( MAS2 and not(SLV1) and not(SLV2) and not(SLV0) )
     or ( MAS3 and not(SLV1) and not(SLV2) and not(SLV0) )
     or ( not(nTS) and not(SLV1) and not(SLV2) and not(SLV0) )
     or ( not(SLV1) and not(nETERM) and SLV2 and nRDSACK1 and not(SLV0) and nRDSACK0 )
     or ( not(MAS1) and MAS0 and not(MAS2) and not(SLV1) and not(nETERM) and nRDSACK1 and not(SLV0) )
     or ( not(MAS1) and MAS0 and MAS3 and not(SLV1) and not(nETERM) and nRDSACK1 and not(SLV0) )
     or ( MAS0 and not(MAS2) and MAS3 and not(SLV1) and not(nETERM) and nRDSACK1 and not(SLV0) )
     or ( not(MAS1) and MAS0 and not(MAS2) and not(MAS3) and not(SLV1) and not(nETERM) and not(SLV0) and nRDSACK0 )
     or ( not(MAS1) and not(MAS0) and MAS2 and not(MAS3) and not(SLV1) and not(nETERM) and nRDSACK1 and not(SLV0) ) );
--register
SLV2 <= (
        ( SLV1 and not(SLV2) and not(nRBERR) )
     or ( SLV1 and not(SLV2) and not(SLV0) )
     or ( not(SLV1) and nETERM and SLV2 and not(SLV0) and nLSTERM ) );
--register
SLV0 <= (
        ( not(SLV1) and SLV2 and not(SLV0) and nRBERR and not(nLSTERM) )
     or ( not(SLV1) and not(nETERM) and SLV2 and not(SLV0) and nRBERR )
     or ( not(SLV1) and not(nETERM) and SLV2 and nRDSACK1 and not(SLV0) and nRDSACK0 )
     or ( not(MAS1) and MAS0 and not(MAS2) and not(SLV1) and not(nETERM) and SLV2 and nRDSACK1 and not(SLV0) )
     or ( not(MAS1) and MAS0 and MAS3 and not(SLV1) and not(nETERM) and SLV2 and nRDSACK1 and not(SLV0) )
     or ( MAS0 and not(MAS2) and MAS3 and not(SLV1) and not(nETERM) and SLV2 and nRDSACK1 and not(SLV0) )
     or ( not(MAS1) and MAS0 and not(MAS2) and not(MAS3) and not(SLV1) and not(nETERM) and SLV2 and not(SLV0) and nRDSACK0 )
     or ( not(MAS1) and not(MAS0) and MAS2 and not(MAS3) and not(SLV1) and not(nETERM) and SLV2 and nRDSACK1 and not(SLV0) ) );
      end if;
   end process;

end Behavioral;

