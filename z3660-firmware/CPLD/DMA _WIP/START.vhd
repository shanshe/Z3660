----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    10:54:22 08/20/2021 
-- Design Name: 
-- Module Name:    START - Behavioral 
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

entity START is
    Port ( BCLK : in  STD_LOGIC;
           nAS040 : in  STD_LOGIC;
           nRCIIN : in  STD_LOGIC;
           nTA : in  STD_LOGIC;
           nTEA : in  STD_LOGIC;
           nRAVEC : in  STD_LOGIC;
--           nBGACK040 : in  STD_LOGIC;
           nTS : in  STD_LOGIC;
           n040RSTI : in  STD_LOGIC;
           nTEND : out  STD_LOGIC;
--           nCYCPEND : out  STD_LOGIC;
           nTCI : out  STD_LOGIC;
           nAVEC040 : out  STD_LOGIC);
end START;

architecture Behavioral of START is
--signal nSTART:  STD_LOGIC;
--signal nTEND_int: STD_LOGIC;
signal nTEND_int: STD_LOGIC;
signal nTCI_int: STD_LOGIC;
signal nAVEC040_int: STD_LOGIC;

begin
--nCYCPEND <= nCYCPEND_int;
nTCI <= nTCI_int;
nAVEC040 <= nAVEC040_int;
nTEND <= nTEND_int;
   
   process(BCLK,n040RSTI)
   begin
      if(n040RSTI='0') then
--         nCYCPEND_int <= '1';
         nTCI_int <= '1';
         nAVEC040_int <= '1';
         nTEND_int <= '1';
      elsif(RISING_EDGE(BCLK)) then
         if(nAVEC040_int='1') then
            nAVEC040_int <= nAS040 or nRAVEC;
         else
            nAVEC040_int <= not(nTA and nTEA);
         end if;
         if(nTCI_int='1') then
            nTCI_int <= nAS040 or nRCIIN;
         else
            nTCI_int <= not(nTA and nTEA);
         end if;
         if(nTEND_int='1') then
            nTEND_int <= nTS;
         else
            nTEND_int <= not(nTA and nTEA);
         end if;
      end if;
   end process;
end Behavioral;

