----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    09:10:21 08/20/2021 
-- Design Name: 
-- Module Name:    MAPPER - Behavioral 
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

entity MAPPER is
    Port ( BCLK : in  STD_LOGIC;
           p040A : in  STD_LOGIC_VECTOR (31 downto 19);
           MAPROM : in  STD_LOGIC;
           R_W040 : in  STD_LOGIC;
           nPWRST : in  STD_LOGIC;
           MA : out  STD_LOGIC_VECTOR (26 downto 24);
           nBGACK040 : in  STD_LOGIC);
end MAPPER;

architecture Behavioral of MAPPER is
signal nREMAP_MOBO_RAM: STD_LOGIC:= '1'; -- initially not remmaped (active low signal)

begin

   process(nBGACK040,p040A,nREMAP_MOBO_RAM,R_W040)
   begin
MA(26) <= (
        ( p040A(26) )
    or ( not(p040A(31)) and not(p040A(30)) and not(p040A(29)) and not(p040A(28)) and not(nREMAP_MOBO_RAM) and not(p040A(27)) and not(p040A(26)) and R_W040 and not(p040A(25)) and not(p040A(24)) and p040A(19) and p040A(23) and p040A(20) and p040A(22) and p040A(21) ) );
MA(25) <= (
        ( p040A(25) )
     or ( not(p040A(31)) and not(p040A(30)) and not(p040A(29)) and not(p040A(28)) and not(nREMAP_MOBO_RAM) and not(p040A(27)) and not(p040A(26)) and R_W040 and not(p040A(25)) and not(p040A(24)) and p040A(19) and p040A(23) and p040A(20) and p040A(22) and p040A(21) ) );
MA(24) <= (
        ( p040A(24) )
     or ( not(p040A(31)) and not(p040A(30)) and not(p040A(29)) and not(p040A(28)) and not(nREMAP_MOBO_RAM) and not(p040A(27)) and not(p040A(26)) and R_W040 and not(p040A(25)) and not(p040A(24)) and p040A(19) and p040A(23) and p040A(20) and p040A(22) and p040A(21) ) );
     else
         MA(26) <= 'H';
         MA(25) <= 'H';
         MA(24) <= 'H';
      end if;
   end process;

   process(BCLK)
   begin
      if(BCLK'event and  BCLK='1') then

--register
nREMAP_MOBO_RAM <= not(
        ( p040A(31) and not(p040A(30)) and not(p040A(29)) and not(p040A(28)) and not(p040A(27)) and not(p040A(26)) and not(R_W040) and not(p040A(25)) and not(MAPROM) and not(p040A(24)) and p040A(19) and p040A(23) and p040A(20) and p040A(22) and p040A(21) )
     or ( not(nREMAP_MOBO_RAM) and nPWRST ) );

     end if;
   end process;


end Behavioral;

