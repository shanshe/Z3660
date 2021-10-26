----------------------------------------------------------------------------------
--
-- Z3660
-- 
-- START equations
--
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity START is
    Port ( BCLK : in  STD_LOGIC;
           nAS040 : in  STD_LOGIC;
           nRCIIN : in  STD_LOGIC;
           nTA : in  STD_LOGIC;
           nTEA : in  STD_LOGIC;
           nRAVEC : in  STD_LOGIC;
           nBGACK040 : in  STD_LOGIC;
           nTS : in  STD_LOGIC;
           n040RSTI : in  STD_LOGIC;
--           nTEND : out  STD_LOGIC;
           nCYCPEND : out  STD_LOGIC;
           nTCI : out  STD_LOGIC;
           nAVEC040 : out  STD_LOGIC);
end START;

architecture Behavioral of START is
--signal nSTART:  STD_LOGIC;
--signal nTEND_int: STD_LOGIC;
signal nCYCPEND_int: STD_LOGIC;
signal nTCI_int: STD_LOGIC;
signal nAVEC040_int: STD_LOGIC;

begin
nCYCPEND <= nCYCPEND_int;
nTCI <= nTCI_int;
nAVEC040 <= nAVEC040_int;
--nTEND <= nTEND_int;
   
   process(BCLK,n040RSTI)
   begin
      if(n040RSTI='0') then
         nCYCPEND_int <= '1';
         nTCI_int <= '1';
         nAVEC040_int <= '1';
      elsif(BCLK'event and  BCLK='1') then
--         if(nAVEC040='1') then
--            nAVEC040 <= nAS040 or nRAVEC;
--         else
--            nAVEC040 <= not(nTA and nTEA and n040RSTI);
--         end if;
--         if(nTCI='1') then
--            nTCI <= nAS040 or nRCIIN;
--         else
--            nTCI <= not(nTA and nTEA and n040RSTI);
--         end if;
            
--register
nAVEC040_int <= not(
        ( not(nAS040) and nAVEC040_int and not(nRAVEC) )
     or ( not(nAVEC040_int) and nTA and nTEA and n040RSTI ) );
--register
nTCI_int <= not(
        ( not(nAS040) and not(nRCIIN) and nTCI_int )
     or ( nTA and not(nTCI_int) and nTEA and n040RSTI ) );
--nSTART <= not(
--         ( not(nBGACK040) and not(nTS) )
--     or ( not(nCYCPEND_int) and not(nBGACK040) ) );
--register
nCYCPEND_int <= not(
        ( nBGACK040 and not(nTS) )
     or ( not(nCYCPEND_int) and nBGACK040 ) );
--register
--nTEND_int <= not(
--        ( nTA and nTEA and not(nTEND_int) and n040RSTI )
--     or ( nTEND_int and not(nTS) ) );

      end if;
   end process;

end Behavioral;

