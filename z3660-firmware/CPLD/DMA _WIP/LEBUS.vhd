----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    12:38:30 08/20/2021 
-- Design Name: 
-- Module Name:    LEBUS - Behavioral 
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

entity LEBUS_component is
    Port ( BCLK : in  STD_LOGIC;
           MAS0 : in  STD_LOGIC;
           MAS1 : in  STD_LOGIC;
           MAS2 : in  STD_LOGIC;
           MAS3 : in  STD_LOGIC;
           nRTERM   : in  STD_LOGIC;
           nLSTERM  : in  STD_LOGIC;
           LEBUS    : out STD_LOGIC_VECTOR (7 downto 0);
           DMA_BUSY : in  STD_LOGIC);
end LEBUS_component;

architecture Behavioral of LEBUS_component is
signal MAS_state: STD_LOGIC_VECTOR(3 downto 0);
signal LEBUS_int: STD_LOGIC_VECTOR(7 downto 0);
--LEBUS
--  0   d31..24 d31..24
--  1   d23..16 d31..24
--  2   d23..16 d23..16
--  3   d15.. 8 d31..24

--  4   d15.. 8 d15.. 8
--  5   d 7.. 0 d31..24
--  6   d 7.. 0 d23..16
--  7   d 7.. 0 d 7.. 0

-- Master State Values
CONSTANT  I: std_logic_vector(3 downto 0):= "0000";
CONSTANT  A: std_logic_vector(3 downto 0):= "0001";
CONSTANT  B: std_logic_vector(3 downto 0):= "0101";
CONSTANT  C: std_logic_vector(3 downto 0):= "1101";
CONSTANT  D: std_logic_vector(3 downto 0):= "0100";
CONSTANT  E: std_logic_vector(3 downto 0):= "1100";
CONSTANT  F: std_logic_vector(3 downto 0):= "1001";
CONSTANT  G: std_logic_vector(3 downto 0):= "1011";
CONSTANT  H: std_logic_vector(3 downto 0):= "1111";
CONSTANT  J: std_logic_vector(3 downto 0):= "1110";
CONSTANT  K: std_logic_vector(3 downto 0):= "0011";
CONSTANT  L: std_logic_vector(3 downto 0):= "0010";
CONSTANT  M: std_logic_vector(3 downto 0):= "0111";
CONSTANT  N: std_logic_vector(3 downto 0):= "0110";
CONSTANT  Z: std_logic_vector(3 downto 0):= "1000";
CONSTANT DC: std_logic_vector(3 downto 0):= "1010";

begin

   MAS_state <= MAS3 & MAS2 & MAS1 & MAS0;
   LEBUS <= LEBUS_int;

----simplified equations
   process(BCLK,DMA_BUSY)
   begin
      if(DMA_BUSY='1') then
         LEBUS_int <= "00000000";
--         LEBUS_int <= "10010101";
      elsif(RISING_EDGE(BCLK)) then
         if((nRTERM='0') or (nLSTERM='0')) then
            LEBUS_int <= "11111111";
--            LEBUS_int <= "00000000";
         else
--            LEBUS_int(0) <= (               not(MAS1) and  MAS2               )  -- b c d e
--                         or (     MAS0                and  MAS2 and     MAS3  ); --   c     h
--            LEBUS_int(1) <= ( not(MAS0) and not(MAS1) and  MAS2               ); --     d e
--            LEBUS_int(2) <= (     MAS0  and not(MAS1) and  MAS2 and not(MAS3) ); -- b    
--            LEBUS_int(3) <= (                              MAS2 and     MAS3  ); --   c   e h j
--            LEBUS_int(4) <= '0';
--            LEBUS_int(5) <= '0';
--            LEBUS_int(6) <= '0';
--            LEBUS_int(7) <= '0';
            case (MAS_state) is
--               when I =>  LEBUS_int <= "00000000";
--               when A =>  LEBUS_int <= "00000000";
--               when F =>  LEBUS_int <= "00000000";
--               when G =>  LEBUS_int <= "00000000";
--               when K =>  LEBUS_int <= "00000000";
--               when L =>  LEBUS_int <= "00000000";
--               when M =>  LEBUS_int <= "00000000";
--               when N =>  LEBUS_int <= "00000000";
--               when Z  => LEBUS_int <= "00000000";
--               when DC => LEBUS_int <= "00000000";

               when B =>  LEBUS_int <= "00000101";
               when C =>  LEBUS_int <= "00001001";
               when D =>  LEBUS_int <= "00000011";
               when E =>  LEBUS_int <= "00001011";
               when H =>  LEBUS_int <= "00001001";
               when J =>  LEBUS_int <= "00001000";
               when others =>
                          LEBUS_int <= "00000000";
            end case;
         end if;
      end if;
   end process;

--LEBUS <= "00000000"; -- transparent bus
--LEBUS <= "11111111"; -- maintains what was previously on the bus
-- so... 1 = "maintain"
--       0 = "rearm"

end Behavioral;

