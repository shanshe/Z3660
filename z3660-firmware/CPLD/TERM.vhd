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
           nBGACK : in STD_LOGIC;
           nPLSTERM : in  STD_LOGIC;
           nRAVEC : out  STD_LOGIC;
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
signal nRAVEC_PRE,nPLSTERM_D,nDSACK0_D,nDSACK1_D: STD_LOGIC;
signal nETERM_int: STD_LOGIC;
signal nRDSACK0_int: STD_LOGIC;
signal nRDSACK1_int: STD_LOGIC;
signal nRAVEC_int: STD_LOGIC;
begin
nRAVEC <= nRAVEC_int;

---- simplified equations
--nRAVEC_PRE <= nAVEC or nAS040;
----nRAVEC_PRE <= nAVEC or nIACK;
--nRTERM <= nDSACK0_D and nDSACK1_D;
--nETERM <= ( (nRAVEC or nAVEC)
--        and (nRBERR or nBERR)
--        and (nDSACK0_D or nDSACK(0))
--        and (nDSACK1_D or nDSACK(1)) ) or nAS040;
----nRDSACK0 <= nDSACK(0) and nDSACK0_D and nPLSTERM and nPLSTERM_D and nRAVEC;
----nRDSACK1 <= nDSACK(1) and nDSACK1_D and nPLSTERM and nPLSTERM_D;
--nRDSACK0 <= nDSACK(0)
--         and nDSACK0_D
--         and nPLSTERM
--         and nPLSTERM_D;
----         and nRAVEC;
--nRDSACK1 <= nDSACK(1)
--         and nDSACK1_D
--         and nPLSTERM
--         and nPLSTERM_D;
----         and nRAVEC;

-- "original equations"

nRAVEC_PRE <= not(
       ( not(nAS040) and not(nAVEC) ) );
nRTERM <= not(
       ( not(nDSACK0_D) )
    or ( not(nDSACK1_D) ) );
nETERM <= nETERM_int;
nETERM_int <= not(
       ( not(nRAVEC_int) and not(nAS040) and not(nAVEC) )
    or ( not(nRBERR) and not(nBERR) and not(nAS040) )
    or ( not(nDSACK0_D) and not(nDSACK(0)) and not(nAS040) )
    or ( not(nDSACK1_D) and not(nAS040) and not(nDSACK(1)) ) );
--    or ( not(nRDSACK0_int) and not(nAS040) )
--    or ( not(nAS040) and not(nRDSACK1_int) ) );
--    or ( not(nDSACK(0) ) and not(nAS040) )
--    or ( not(nDSACK(1) ) and not(nAS040) ) );
nRDSACK0 <= nRDSACK0_int;
nRDSACK1 <= nRDSACK1_int;
nRDSACK0_int <= not(
       ( not(nDSACK(0)) )
    or ( not(nPLSTERM) )
    or ( not(nRAVEC_int) )
    or ( not(nDSACK0_D) )
    or ( not(nPLSTERM_D) ) 
    );
nRDSACK1_int <= not(
       ( not(nDSACK(1)) )
    or ( not(nPLSTERM) )
    or ( not(nDSACK1_D) )
    or ( not(nPLSTERM_D) ) 
    );

-- Original from A3640
--nETERM <= nETERM_int;
--nRAVEC_PRE <= not(
--       ( not(nAS040) and not(nAVEC) ) );
--nRTERM <= not(
--       ( not(nETERM_int) and not(nRAVEC_int) and not(nAS040) )
--    or ( not(nETERM_int) and not(nRBERR) and not(nAS040) )
--    or ( not(nDSACK1_D) and not(nETERM_int) and not(nAS040) )
--    or ( not(nDSACK0_D) and not(nETERM_int) and not(nAS040) ) );
--nETERM_int <= not(
--       ( not(nAS040) and not(nAVEC) )
--    or ( not(nBERR) )
--    or ( not(nDSACK(1)) )
--    or ( not(nDSACK(0)) ) );
--nRDSACK0 <= not(
--       ( not(nDSACK0_D) )
--    or ( not(nPLSTERM_D) )
--    or ( not(nRAVEC_int) ) );
--nRDSACK1 <= not(
--       ( not(nDSACK1_D) )
--    or ( not(nPLSTERM_D) ) );

   process(BCLK)
   begin
      if(BCLK'event and  BCLK='1') then
         if nBGACK='1' then
            nRAVEC_int <= nRAVEC_PRE;
            nPLSTERM_D <= nPLSTERM;
            nDSACK0_D <= nDSACK(0);
            nDSACK1_D <= nDSACK(1);
         else
            nRAVEC_int <= '1';
            nPLSTERM_D <= '1';
            nDSACK0_D <= '1';
            nDSACK1_D <= '1';
         end if;
      end if;
   end process;

end Behavioral;

