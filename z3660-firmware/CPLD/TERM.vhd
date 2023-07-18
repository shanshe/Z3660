----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    18:29:45 08/20/2021 
-- Design Name: 
-- Module Name:    TERM - Behavioral 
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

entity TERM is
    Port (
           BCLK : in  STD_LOGIC;
           nPLSTERM : in  STD_LOGIC;
           nRAVEC_out : out  STD_LOGIC;
           nRBERR : in  STD_LOGIC;
--           nTEND : in  STD_LOGIC;
           nAS040 : in  STD_LOGIC;
           nAVEC : in  STD_LOGIC;
           nDSACK : in  STD_LOGIC_VECTOR (1 downto 0);
           nRDSACK1 : out  STD_LOGIC;
           nBERR : in  STD_LOGIC;
           nRDSACK0 : out  STD_LOGIC;
           nIACK: in STD_LOGIC;
           nETERM : out  STD_LOGIC;
           nRTERM : out  STD_LOGIC);
end TERM;

architecture Behavioral of TERM is
signal nRAVEC_PRE,nRAVEC,nPLSTERM_D,nDSACK0_D,nDSACK1_D: STD_LOGIC;
signal nETERM_int: STD_LOGIC;
begin


nRAVEC_out <= nRAVEC;

nRAVEC_PRE <= not(
       ( not(nAS040) and not(nAVEC) ) );
nRTERM <= not(
       ( not(nDSACK0_D) )
    or ( not(nDSACK1_D) ) );
nETERM <= not(
       ( not(nRAVEC) and not(nAS040) and not(nAVEC) )
    or ( not(nRBERR) and not(nBERR) and not(nAS040) )
    or ( not(nDSACK0_D) and not(nDSACK(0)) and not(nAS040) )
    or ( not(nDSACK1_D) and not(nAS040) and not(nDSACK(1)) ) );
--    or ( not(nDSACK0_D) and not(nAS040) )
--    or ( not(nDSACK1_D) and not(nAS040) ) );
nRDSACK0 <= not(
       ( not(nDSACK(0)) )
    or ( not(nPLSTERM) )
    or ( not(nRAVEC) )
    or ( not(nDSACK0_D) )
    or ( not(nPLSTERM_D) ) 
    );
nRDSACK1 <= not(
       ( not(nDSACK(1)) )
    or ( not(nPLSTERM) )
    or ( not(nDSACK1_D) )
    or ( not(nPLSTERM_D) ) 
    );

   process(BCLK)
   begin
      if(BCLK'event and  BCLK='1') then
         nRAVEC <= nRAVEC_PRE;
         nPLSTERM_D <= nPLSTERM;
         nDSACK0_D <= nDSACK(0);
         nDSACK1_D <= nDSACK(1);
      end if;
   end process;

end Behavioral;

