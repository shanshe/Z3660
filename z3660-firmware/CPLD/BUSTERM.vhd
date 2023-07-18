----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    09:52:29 08/20/2021 
-- Design Name: 
-- Module Name:    BUSTERM - Behavioral 
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
     or ( not(SLV0) and SLV1 and not(SLV2)  ) );
nDS040_int <= not(
        ( not(SLV0) and not(SLV1) and SLV2 )
     or ( not(SLV0) and SLV1 and not(SLV2)  ) );

end Behavioral;

