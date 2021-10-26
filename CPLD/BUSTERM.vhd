----------------------------------------------------------------------------------
--
-- Z3660
-- 
-- BUSTERM equations
--
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity BUSTERM is
    Port ( BCLK : in  STD_LOGIC;
           R_W040 : in  STD_LOGIC;
           SLV0 : in  STD_LOGIC;
           SLV1 : in  STD_LOGIC;
           SLV2 : in  STD_LOGIC;
           SLV3 : in  STD_LOGIC;
           nCYCPEND : in  STD_LOGIC;
           SIZ40 : in  STD_LOGIC_VECTOR (1 downto 0);
           nPLSTERM : in  STD_LOGIC;
           nRTERM : in  STD_LOGIC;
           nTS : in  STD_LOGIC;
           nDS040 : out  STD_LOGIC;
           nAS040 : out  STD_LOGIC;
           nTEA : out  STD_LOGIC;
--           nTBI : out  STD_LOGIC;
           n040RSTI : in  STD_LOGIC;
           nTA : out  STD_LOGIC;
           nBGACK040 : in  STD_LOGIC
         );
end BUSTERM;

architecture Behavioral of BUSTERM is
signal IO2,IO5: STD_LOGIC:='1';
signal nDS040_int,nAS040_int: STD_LOGIC:='1';

begin

nDS040 <= nDS040_int;
nAS040 <= nAS040_int;
nTA <= not(
        ( SLV0 and not(SLV1) and not(SLV2) ) );
nTEA <= not(
        ( not(SLV0) and SLV1 and SLV2 ) );
nAS040_int <= not(
        ( not(SLV0) and not(SLV1) and SLV2 )
--     or ( not(SLV0) and SLV1 and not(SLV2) and IO1 ) );
     or ( not(SLV0) and SLV1 and not(SLV2)  ) );
nDS040_int <= not(
        ( not(SLV0) and not(SLV1) and SLV2 )
--     or ( not(SLV0) and SLV1 and not(SLV2) and IO1 ) );
     or ( not(SLV0) and SLV1 and not(SLV2)  ) );
-- ???
--nTBI <= '0';


end Behavioral;

