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
           SLV0 : in  STD_LOGIC;
           SLV1 : in  STD_LOGIC;
           SLV2 : in  STD_LOGIC;
           SLV3 : in  STD_LOGIC;
           nRTERM : in  STD_LOGIC;
           nLSTERM : in  STD_LOGIC;
           R_W040 : in STD_LOGIC;
           LEBUS : out  STD_LOGIC_VECTOR (7 downto 0));
end LEBUS_component;

architecture Behavioral of LEBUS_component is
--signal NEG_LE:  STD_LOGIC;
--signal nSIG_LE:  STD_LOGIC;
signal MAS_state: STD_LOGIC_VECTOR(3 downto 0);
signal SLV_state: STD_LOGIC_VECTOR(3 downto 0);
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

-- Slave State Values
CONSTANT  S3: std_logic_vector(3 downto 0):= "0000";
CONSTANT  S0: std_logic_vector(3 downto 0):= "0100";
CONSTANT  S1: std_logic_vector(3 downto 0):= "1000";
CONSTANT  S2: std_logic_vector(3 downto 0):= "0001";
CONSTANT S2W: std_logic_vector(3 downto 0):= "0011";
CONSTANT S3W: std_logic_vector(3 downto 0):= "0010";
CONSTANT  SB: std_logic_vector(3 downto 0):= "0101";
CONSTANT SR0: std_logic_vector(3 downto 0):= "0111";
CONSTANT SR1: std_logic_vector(3 downto 0):= "0110";

begin

--   NEG_LE <= '0' when SLV_state=S3 or SLV_state=SR1 else '1'; -- ok???
--   nSIG_LE <= '0' when SLV0='0' and SLV1='0' and SLV3='0' else '1';
--   MAS_state <= MAS3 & MAS2 & MAS1 & MAS0;
--   SLV_state <= SLV3 & SLV2 & SLV1 & SLV0;
   LEBUS <= LEBUS_int;
--   process(BCLK)
--   begin
--      if(BCLK'event and  BCLK='1') then
--      if(R_W040='1') then
--         if nRTERM='0' or nLSTERM='0' or (LEBUS_int(0)='1' and NEG_LE='0' and 
--                           (MAS_state=I or --rearm
--                            MAS_state=Z or
--                            MAS_state=DC or
--                            MAS_state=A or
--                            MAS_state=F or
--                            MAS_state=K)) or
--                            MAS_state=B or --maintain
--                            MAS_state=C or
--                            MAS_state=D or
--                            MAS_state=E or
--                            MAS_state=H then
--            LEBUS_int(0)<='1';
--         else 
--            LEBUS_int(0)<='0';
--         end if;
--         if nRTERM='0' or nLSTERM='0' or (LEBUS_int(1)='1' and NEG_LE='0' and 
--                           (MAS_state=I or
--                            MAS_state=Z or
--                            MAS_state=DC or
--                            MAS_state=C or
--                            MAS_state=H or
--                            MAS_state=L)) or
--                            MAS_state=D or
--                            MAS_state=E then
--            LEBUS_int(1) <= '1';
--         else
--            LEBUS_int(1) <= '0';
--         end if;
--         if nRTERM='0' or nLSTERM='0' or (LEBUS_int(2)='1' and NEG_LE='0' and 
--                           (MAS_state=I or
--                            MAS_state=Z or
--                            MAS_state=DC or
--                            MAS_state=A or
--                            MAS_state=F or
--                            MAS_state=L)) or
--                            MAS_state=B then
--            LEBUS_int(2) <= '1';
--         else
--            LEBUS_int(2) <= '0';
--         end if;
--         if nRTERM='0' or nLSTERM='0' or (LEBUS_int(3)='1' and NEG_LE='0' and 
--                           (MAS_state=I or
--                            MAS_state=Z or
--                            MAS_state=DC or
--                            MAS_state=B or
--                            MAS_state=D or
--                            MAS_state=G or
--                            MAS_state=M --or
--                            --MAS_state=DC
--                            )) or
----                            MAS_state=C or
--                            MAS_state=E or
----                            MAS_state=H or
--                            MAS_state=J then
--            LEBUS_int(3) <= '1';
--         else
--            LEBUS_int(3) <= '0';
--         end if;
--         if nRTERM='0' or nLSTERM='0' or (LEBUS_int(4)='1' and NEG_LE='0' and 
--                           (MAS_state=I or
--                            MAS_state=Z or
--                            MAS_state=DC or
--                            MAS_state=A or
--                            MAS_state=G or
--                            MAS_state=M)) then
--            LEBUS_int(4) <= '1';
--         else
--            LEBUS_int(4) <= '0';
--         end if;
--         if nRTERM='0' or nLSTERM='0' or (LEBUS_int(5)='1' and NEG_LE='0' and 
--                           (MAS_state=I or
--                            MAS_state=Z or
--                            MAS_state=DC or
--                            MAS_state=E or
--                            MAS_state=J or
--                            MAS_state=N)) then
--            LEBUS_int(5) <= '1';
--         else
--            LEBUS_int(5) <= '0';
--         end if;
--         if nRTERM='0' or nLSTERM='0' or (LEBUS_int(6)='1' and NEG_LE='0' and 
--                           (MAS_state=I or
--                            MAS_state=Z or
--                            MAS_state=DC or
--                            MAS_state=B or
--                            MAS_state=G or
--                            MAS_state=N --or
--                            --MAS_state=DC
--                            )) then
--            LEBUS_int(6) <= '1';
--         else
--            LEBUS_int(6) <= '0';
--         end if;
--         if nRTERM='0' or nLSTERM='0' or (LEBUS_int(7)='1' and NEG_LE='0' and 
--                           (MAS_state=I or
--                            MAS_state=Z or
--                            MAS_state=DC or
--                            MAS_state=A or
--                            MAS_state=N or
--                            MAS_state=G)) then
--            LEBUS_int(7) <= '1';
--         else
--            LEBUS_int(7) <= '0';
--         end if;
--         else
--            LEBUS_int<= "00000000";
--         end if;
--      end if;
--   end process;

--   process(BCLK)
--   begin
--      if(BCLK'event and  BCLK='1') then
--         if((nRTERM='0') or (nLSTERM='0')) then
--            LEBUS_int(0) <= '1';
--            LEBUS_int(1) <= '1';
--            LEBUS_int(2) <= '1';
--            LEBUS_int(3) <= '1';
--            LEBUS_int(4) <= '1';
--            LEBUS_int(5) <= '1';
--            LEBUS_int(6) <= '1';
--            LEBUS_int(7) <= '1';
--         else
--            case (MAS_state) is
--               when B | C | D | E | H =>
--                  LEBUS_int(0) <= '1';
--               when others =>
--                  LEBUS_int(0) <= '0';
--            end case;
--
--            case (MAS_state) is
--               when D | E =>
--                  LEBUS_int(1) <= '1';
--               when others =>
--                  LEBUS_int(1) <= '0';
--            end case;
--
--            case (MAS_state) is
--               when B =>
--                  LEBUS_int(2) <= '1';
--               when others =>
--                  LEBUS_int(2) <= '0';
--            end case;
--
--            case (MAS_state) is
--               when C | E | H | J =>
--                  LEBUS_int(3) <= '1';
--               when others =>
--                  LEBUS_int(3) <= '0';
--            end case;
--            LEBUS_int(4) <= '0';
--            LEBUS_int(5) <= '0';
--            LEBUS_int(6) <= '0';
--            LEBUS_int(7) <= '0';
--         end if;
--      end if;
--   end process;

----simplified equations
--   process(BCLK)
--   begin
--      if(BCLK'event and  BCLK='1') then
--         if((nRTERM='0') or (nLSTERM='0')) then
--            LEBUS_int(0) <= '1';
--            LEBUS_int(1) <= '1';
--            LEBUS_int(2) <= '1';
--            LEBUS_int(3) <= '1';
--            LEBUS_int(4) <= '1';
--            LEBUS_int(5) <= '1';
--            LEBUS_int(6) <= '1';
--            LEBUS_int(7) <= '1';
--         else
--            LEBUS_int(0) <= ( not(MAS1) and  MAS2 )
--                     or (  MAS0 and  MAS2 and  MAS3 );                   -- b c d e h
--            LEBUS_int(1) <= ( not(MAS0) and not(MAS1) and  MAS2 );           -- d e
--            LEBUS_int(2) <= (  MAS0 and not(MAS1) and  MAS2 and not(MAS3) ); -- b    
--            LEBUS_int(3) <= (  MAS2 and  MAS3 );                             -- c e h j
--            LEBUS_int(4) <= '0';
--            LEBUS_int(5) <= '0';
--            LEBUS_int(6) <= '0';
--            LEBUS_int(7) <= '0';
--         end if;
--      end if;
--   end process;

--LEBUS <= "00000000"; -- transparent bus
--LEBUS <= "11111111"; -- maintains what was previously on the bus
-- so... 1 = "maintain"
--       0 = "ream"

-- "original equations"
   process(BCLK)
   begin
      if(BCLK'event and  BCLK='1') then
--register
LEBUS_int(0) <= (
       ( not(nRTERM) )
    or ( not(nLSTERM) )
    or ( not(MAS1) and  MAS2 )
    or (  MAS0 and  MAS2 and  MAS3 ) );
--register
LEBUS_int(1) <= (
       ( not(nRTERM) )
    or ( not(nLSTERM) )
    or ( not(MAS0) and not(MAS1) and  MAS2 ) );
--register
LEBUS_int(2) <= (
       ( not(nRTERM) )
    or ( not(nLSTERM) )
    or (  MAS0 and not(MAS1) and  MAS2 and not(MAS3) ) );
--register
LEBUS_int(3) <= (
       ( not(nRTERM) )
    or ( not(nLSTERM) )
    or (  MAS2 and  MAS3 ) );
--register
LEBUS_int(4) <= (
       ( not(nRTERM) )
    or ( not(nLSTERM) ) );
--register
LEBUS_int(5) <= (
       ( not(nRTERM) )
    or ( not(nLSTERM) ) );
--register
LEBUS_int(6) <= (
       ( not(nRTERM) )
    or ( not(nLSTERM) ) );
--register
LEBUS_int(7) <= (
       ( not(nRTERM) )
    or ( not(nLSTERM) ) );
      end if;
   end process;

--   process(BCLK)
--   begin
--      if(BCLK'event and  BCLK='1') then
--original 3640
----register
--LEBUS_int(0) <= (
--       ( not(nRTERM) )
--    or ( not(nLSTERM) )
--    or (  MAS0 and  MAS1 and  MAS2 and  MAS3 )
--    or (  LEBUS_int(0) and  nSIG_LE and not(MAS1) and not(MAS2) )
--    or (  LEBUS_int(0) and  nSIG_LE and  MAS0 and  MAS1 and not(MAS2) and not(MAS3) )
--    or ( not(MAS1) and  MAS2 ) );
----register
--LEBUS_int(1) <= (
--       ( not(nRTERM) )
--    or ( not(nLSTERM) )
--    or (  nSIG_LE and  LEBUS_int(1) and not(MAS0) and not(MAS1) and not(MAS2) )
--    or (  nSIG_LE and  LEBUS_int(1) and  MAS0 and  MAS2 and  MAS3 )
--    or (  nSIG_LE and  LEBUS_int(1) and not(MAS0) and  MAS1 and not(MAS2) and not(MAS3) )
--    or ( not(MAS0) and not(MAS1) and  MAS2 ) );
----register
--LEBUS_int(2) <= (
--       ( not(nRTERM) )
--    or ( not(nLSTERM) )
--    or (  MAS0 and not(MAS1) and  MAS2 and not(MAS3) )
--    or (  nSIG_LE and  LEBUS_int(2) and not(MAS1) and not(MAS2) )
--    or (  nSIG_LE and not(MAS0) and  LEBUS_int(2) and  MAS1 and not(MAS2) and not(MAS3) ) );
----register
--LEBUS_int(3) <= (
--       ( not(nRTERM) )
--    or ( not(nLSTERM) )
--    or (  nSIG_LE and not(MAS0) and not(MAS1) and  LEBUS_int(3) and not(MAS2) )
--    or (  nSIG_LE and  MAS0 and  LEBUS_int(3) and  MAS2 and not(MAS3) )
--    or (  nSIG_LE and  MAS1 and  LEBUS_int(3) and not(MAS2) and  MAS3 )
--    or (  nSIG_LE and not(MAS0) and not(MAS1) and  LEBUS_int(3) and  MAS2 and not(MAS3) )
--    or (  MAS2 and  MAS3 ) );
----register
--LEBUS_int(4) <= (
--       ( not(nRTERM) )
--    or ( not(nLSTERM) )
--    or (  nSIG_LE and not(MAS0) and not(MAS1) and not(MAS2) and  LEBUS_int(4) )
--    or (  nSIG_LE and  MAS0 and not(MAS1) and not(MAS2) and  LEBUS_int(4) and not(MAS3) )
--    or (  nSIG_LE and  MAS0 and  MAS1 and not(MAS2) and  LEBUS_int(4) and  MAS3 )
--    or (  nSIG_LE and  MAS0 and  MAS1 and  MAS2 and  LEBUS_int(4) and not(MAS3) ) );
----register
--LEBUS_int(5) <= (
--       ( not(nRTERM) )
--    or ( not(nLSTERM) )
--    or (  nSIG_LE and not(MAS0) and not(MAS1) and not(MAS2) and  LEBUS_int(5) )
--    or (  nSIG_LE and not(MAS0) and  MAS1 and  MAS2 and  LEBUS_int(5) )
--    or (  nSIG_LE and not(MAS0) and not(MAS1) and  MAS2 and  MAS3 and  LEBUS_int(5) ) );
----register
--LEBUS_int(6) <= (
--       ( not(nRTERM) )
--    or ( not(nLSTERM) )
--    or (  nSIG_LE and not(MAS0) and not(MAS1) and not(MAS2) and  LEBUS_int(6) )
--    or (  nSIG_LE and  MAS0 and not(MAS1) and  MAS2 and not(MAS3) and  LEBUS_int(6) )
--    or (  nSIG_LE and  MAS1 and not(MAS2) and  MAS3 and  LEBUS_int(6) )
--    or (  nSIG_LE and not(MAS0) and  MAS1 and  MAS2 and not(MAS3) and  LEBUS_int(6) ) );
----register
--LEBUS_int(7) <= (
--       ( not(nRTERM) )
--    or ( not(nLSTERM) )
--    or (  nSIG_LE and not(MAS0) and not(MAS1) and not(MAS2) and  LEBUS_int(7) )
--    or (  nSIG_LE and  MAS0 and not(MAS1) and not(MAS2) and not(MAS3) and  LEBUS_int(7) )
--    or (  nSIG_LE and  MAS0 and  MAS1 and not(MAS2) and  MAS3 and  LEBUS_int(7) )
--    or (  nSIG_LE and not(MAS0) and  MAS1 and  MAS2 and not(MAS3) and  LEBUS_int(7) ) );
--
--
--
--      end if;
--   end process;

end Behavioral;

