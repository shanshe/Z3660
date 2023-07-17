----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    20:16:21 08/20/2021 
-- Design Name: 
-- Module Name:    RST - Behavioral 
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

---- simplified equations
--   process(BCLK)
--   begin
--      if(BCLK'event and  BCLK='1') then
--         nPWRST <= nT1A;
--         nT1A <= nERST;
--         nERST <= nPORESET;
--         nLSTERM <= nPLSTERM or nAS040;
--         n040EMUL <= nEMUL or n040RSTI;
--      end if;
--   end process;

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


-- "original equations"
--   process(BCLK)
--   begin
--      if(BCLK'event and  BCLK='1') then
----register
--n040EMUL <= not(
--       ( not(nEMUL) and  n040RSTI ) );
----register
--nPWRST <= not(
--       ( not(nT1A) ) );
----register
--nERST <= not(
--       ( not(nPORESET) ) );
----register
--nT1A <= not(
--       ( not(nERST) ) );
----register
--nLSTERM <= not(
--       ( not(nAS040) and not(nPLSTERM) ) );
--      end if;
--   end process;

end Behavioral;

