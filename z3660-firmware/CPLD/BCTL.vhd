----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    10:22:16 08/20/2021 
-- Design Name: 
-- Module Name:    BCTL - Behavioral 
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

entity BCTL is
    Port ( BCLK : in  STD_LOGIC;
           nSBGACK030 : in  STD_LOGIC;
           nSBR030 : in  STD_LOGIC;
           nBB040 : in  STD_LOGIC;
           nBR040 : in  STD_LOGIC;
           nLOCK : in  STD_LOGIC;
           nLOCKE : in  STD_LOGIC;
           n040RSTI : in  STD_LOGIC;
           nBG040 : out  STD_LOGIC;
           nBG : out  STD_LOGIC;
           nBGACK040 : out  STD_LOGIC);
end BCTL;

architecture Behavioral of BCTL is
signal bus_state: STD_LOGIC_VECTOR(3 downto 0);
signal bus_state_c: STD_LOGIC_VECTOR(3 downto 0);
-- State Values
CONSTANT B00: std_logic_vector(3 downto 0):= "1111";
CONSTANT B01: std_logic_vector(3 downto 0):= "1001";
CONSTANT B02: std_logic_vector(3 downto 0):= "1101";
CONSTANT B03: std_logic_vector(3 downto 0):= "1100";
CONSTANT B04: std_logic_vector(3 downto 0):= "0101";
CONSTANT B05: std_logic_vector(3 downto 0):= "1011";
CONSTANT B06: std_logic_vector(3 downto 0):= "0011";
CONSTANT B07: std_logic_vector(3 downto 0):= "0110";
CONSTANT B08: std_logic_vector(3 downto 0):= "0111";
CONSTANT B09: std_logic_vector(3 downto 0):= "1010";
CONSTANT B10: std_logic_vector(3 downto 0):= "1110";

begin

--nBG040 <= '0' when (bus_state=B07 or bus_state=B08 or bus_state=B10) else '1';
nBG040 <= not(
        ( not(bus_state(3)) and bus_state(2) and bus_state(1) ) );


   process(BCLK,n040RSTI)
   begin
      if(n040RSTI='0') then
         bus_state<= B00;
         bus_state_c <= B00;
         nBG <= '1';
         nBGACK040 <= '1';
      elsif(BCLK'event and  BCLK='1') then
         bus_state <= bus_state_c;

--register
nBGACK040 <= not(
        ( nSBGACK030 and nSBR030 and bus_state(3) and bus_state(2) and bus_state(0) and bus_state(1) )
     or ( bus_state(3) and not(bus_state(0)) and bus_state(1) )
     or ( nSBGACK030 and nSBR030 and not(bus_state(3)) and bus_state(2) and bus_state(0) and not(bus_state(1)) )
     or ( not(bus_state(3)) and bus_state(2) and bus_state(1) ) );
--register
nBG <= not(
        ( nSBGACK030 and not(nSBR030) and bus_state(3) and bus_state(2) and bus_state(0) )
     or ( bus_state(3) and not(bus_state(2)) and bus_state(0) )
     or ( not(nSBR030) and not(bus_state(3)) and bus_state(2) and bus_state(0) and not(bus_state(1)) )
     or ( not(nSBR030) and not(bus_state(3)) and not(bus_state(2)) and bus_state(0) and bus_state(1) ) );
--register
bus_state_c(3) <= not(
        ( not(nSBGACK030) and bus_state(3) and bus_state(2) and bus_state(0) and bus_state(1) )
     or ( not(nBB040) and not(nBR040) and bus_state(3) and bus_state(2) and not(bus_state(0)) and bus_state(1) )
     or ( nSBGACK030 and nSBR030 and not(nBR040) and bus_state(3) and bus_state(2) and bus_state(0) and bus_state(1) )
     or ( bus_state(3) and bus_state(2) and not(bus_state(0)) and not(bus_state(1)) )
     or ( not(nSBGACK030) and nSBR030 and not(bus_state(3)) and bus_state(2) and bus_state(0) and not(bus_state(1)) )
     or ( bus_state(3) and not(bus_state(2)) and bus_state(0) and bus_state(1) )
     or ( not(nSBGACK030) and not(nSBR030) and not(bus_state(3)) and not(bus_state(2)) and bus_state(0) and bus_state(1) )
     or ( not(bus_state(3)) and bus_state(2) and not(bus_state(0)) and bus_state(1) )
     or ( nSBR030 and not(bus_state(3)) and bus_state(2) and bus_state(0) and bus_state(1) )
     or ( not(nSBR030) and not(nLOCK) and nLOCKE and not(bus_state(3)) and bus_state(2) and bus_state(0) and bus_state(1) )
     or ( not(nBB040) and not(nLOCK) and nLOCKE and bus_state(3) and not(bus_state(2)) and not(bus_state(0)) and bus_state(1) ) );
--register
bus_state_c(2) <= not(
        ( not(nBB040) and nBR040 and bus_state(3) and bus_state(2) and not(bus_state(0)) and bus_state(1) )
     or ( nSBGACK030 and not(nSBR030) and bus_state(3) and bus_state(2) and bus_state(0) and bus_state(1) )
     or ( not(nSBR030) and not(bus_state(3)) and bus_state(2) and bus_state(0) and not(bus_state(1)) )
     or ( bus_state(3) and not(bus_state(2)) and bus_state(0) and bus_state(1) )
     or ( not(nSBGACK030) and not(nSBR030) and not(bus_state(3)) and not(bus_state(2)) and bus_state(0) and bus_state(1) )
     or ( not(nSBR030) and not(nLOCK) and not(nLOCKE) and not(bus_state(3)) and bus_state(2) and bus_state(0) and bus_state(1) )
     or ( not(nSBR030) and nLOCK and not(bus_state(3)) and bus_state(2) and bus_state(0) and bus_state(1) )
     or ( not(nBB040) and nLOCK and bus_state(3) and not(bus_state(2)) and not(bus_state(0)) and bus_state(1) )
     or ( not(nBB040) and not(nLOCK) and not(nLOCKE) and bus_state(3) and not(bus_state(2)) and not(bus_state(0)) and bus_state(1) ) );
--register
bus_state_c(1) <= not(
        ( not(nSBGACK030) and bus_state(3) and bus_state(2) and bus_state(0) )
     or ( nSBGACK030 and not(nSBR030) and bus_state(3) and bus_state(2) and bus_state(0) )
     or ( nSBGACK030 and not(nSBR030) and not(bus_state(3)) and not(bus_state(2)) and bus_state(0) and bus_state(1) )
     or ( bus_state(3) and not(bus_state(2)) and bus_state(0) and not(bus_state(1)) )
     or ( nSBGACK030 and nSBR030 and bus_state(3) and bus_state(2) and bus_state(0) and not(bus_state(1)) )
     or ( bus_state(3) and bus_state(2) and not(bus_state(0)) and not(bus_state(1)) )
     or ( not(nSBGACK030) and nSBR030 and not(bus_state(3)) and bus_state(2) and bus_state(0) and not(bus_state(1)) )
     or ( nSBR030 and not(bus_state(3)) and not(bus_state(2)) and bus_state(0) and bus_state(1) ) );
--register
bus_state_c(0) <= not(
        ( not(nBB040) and nBR040 and bus_state(3) and bus_state(2) and not(bus_state(0)) and bus_state(1) )
     or ( nSBGACK030 and nSBR030 and not(nBR040) and bus_state(3) and bus_state(2) and bus_state(0) and bus_state(1) )
     or ( not(nSBGACK030) and bus_state(3) and bus_state(2) and bus_state(0) and not(bus_state(1)) )
     or ( nSBGACK030 and nSBR030 and bus_state(3) and bus_state(2) and bus_state(0) and not(bus_state(1)) )
     or ( nSBR030 and not(bus_state(3)) and not(bus_state(2)) and bus_state(0) and bus_state(1) )
     or ( not(nSBR030) and not(nLOCK) and not(nLOCKE) and not(bus_state(3)) and bus_state(2) and bus_state(0) and bus_state(1) )
     or ( not(nSBR030) and nLOCK and not(bus_state(3)) and bus_state(2) and bus_state(0) and bus_state(1) )
     or ( nBB040 and bus_state(3) and not(bus_state(2)) and not(bus_state(0)) and bus_state(1) )
     or ( not(nBB040) and nLOCK and bus_state(3) and not(bus_state(2)) and not(bus_state(0)) and bus_state(1) )
     or ( not(nBB040) and not(nLOCK) and not(nLOCKE) and bus_state(3) and not(bus_state(2)) and not(bus_state(0)) and bus_state(1) ) );

      end if;
   end process;

end Behavioral;

