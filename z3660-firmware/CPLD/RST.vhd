----------------------------------------------------------------------------------
--
-- Z3660
-- 
-- RST equations
--
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity RST is
    Port ( BCLK : in  STD_LOGIC;
--           nAS040 : in  STD_LOGIC;
--           nPLSTERM : in  STD_LOGIC;
           nPORESET : in  STD_LOGIC;
--           nEMUL : in  STD_LOGIC;
--           n040RSTI : in  STD_LOGIC;
--           nLSTERM : out  STD_LOGIC;
           nPWRST : out  STD_LOGIC
--           n040EMUL : out  STD_LOGIC
          );
end RST;

architecture Behavioral of RST is
signal nT1A,nERST: STD_LOGIC;
begin

nPWRST <= nT1A;

   process(BCLK)
   begin
      if(BCLK'event and  BCLK='1') then
--         n040EMUL <= nEMUL or not(n040RSTI);
--         nPWRST <= nT1A;
--         nLSTERM <= nAS040 or nPLSTERM;
         nERST <= nPORESET;
         nT1A <= nERST;
      end if;
   end process;

end Behavioral;

