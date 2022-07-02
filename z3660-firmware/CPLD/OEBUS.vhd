----------------------------------------------------------------------------------
--
-- Z3660
-- 
-- OEBUS equations
--
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity OEBUS_component is
    Port ( BCLK : in  STD_LOGIC;
           MAS0 : in  STD_LOGIC;
           MAS1 : in  STD_LOGIC;
           MAS2 : in  STD_LOGIC;
           MAS3 : in  STD_LOGIC;
           SLV0 : in  STD_LOGIC;
           SLV1 : in  STD_LOGIC;
           SLV2 : in  STD_LOGIC;
           SLV3 : in  STD_LOGIC;
           R_W040 : in  STD_LOGIC;
           nRBERR : in  STD_LOGIC;
           nRDSACK0 : in  STD_LOGIC;
           nRDSACK1 : in  STD_LOGIC;
           nDMACOE : out  STD_LOGIC;
           OEBUS : out  STD_LOGIC_VECTOR (7 downto 0)
           );
end OEBUS_component;

architecture Behavioral of OEBUS_component is
signal nS2W:  STD_LOGIC :='1';
signal OEBUS_int: STD_LOGIC_VECTOR(7 downto 0);
--OEBUS
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
CONSTANT  S3: std_logic_vector(3 downto 0):= "0000"; -- 0
CONSTANT  S0: std_logic_vector(3 downto 0):= "0100"; -- 4
CONSTANT  S1: std_logic_vector(3 downto 0):= "1000"; -- 0
CONSTANT  S2: std_logic_vector(3 downto 0):= "0001"; -- 1
CONSTANT S2W: std_logic_vector(3 downto 0):= "0011"; -- 3
CONSTANT S3W: std_logic_vector(3 downto 0):= "0010"; -- 2
CONSTANT  SB: std_logic_vector(3 downto 0):= "0101"; -- 5
CONSTANT SR0: std_logic_vector(3 downto 0):= "0111"; -- 7
CONSTANT SR1: std_logic_vector(3 downto 0):= "0110"; -- 6

impure function byte return std_logic is begin
  return ((nRBERR and not(nRDSACK0) and nRDSACK1)); end byte;
impure function word return std_logic is begin
  return (nRBERR and nRDSACK0 and not(nRDSACK1)); end word;
impure function long return std_logic is begin
  return (nRBERR and not(nRDSACK0) and not(nRDSACK1)); end long;

begin

   nS2W <= not(SLV0 and SLV1 and not(SLV3));
   OEBUS <= OEBUS_int;

   process(BCLK)
   begin
      if(BCLK'event and  BCLK='1') then
--register
nDMACOE <= not(
        ( not(MAS0) and MAS1 and not(MAS2) and not(MAS3) and not(R_W040) ) );
--register
OEBUS_int(0) <= not(
        ( R_W040 and nRBERR and not(nRDSACK0) )
     or ( R_W040 and nRBERR and not(nRDSACK1) )
     or ( not(OEBUS_int(0)) and not(nS2W) and R_W040 )
     or ( MAS0 and not(MAS1) and not(MAS2) and not(R_W040) )    -- A F
     or ( MAS0 and not(MAS2) and not(MAS3) and not(R_W040) ) ); -- A K
--register
OEBUS_int(1) <= not(
        ( not(OEBUS_int(1)) and not(nS2W) and R_W040 )
     or ( MAS0 and MAS2 and MAS3 and not(R_W040) )
     or ( R_W040 and byte )
     or ( not(MAS0) and MAS1 and not(MAS2) and not(MAS3) and not(R_W040) ) );
--register
OEBUS_int(2) <= not(
        ( R_W040 and nRBERR and not(nRDSACK1) )
     or ( not(OEBUS_int(2)) and not(nS2W) and R_W040 )
     or ( MAS0 and MAS1 and MAS2 and not(R_W040) )
     or ( MAS0 and not(MAS1) and not(MAS2) and not(R_W040) )
     or ( MAS1 and not(MAS2) and not(MAS3) and not(R_W040) ) );
--register
OEBUS_int(3) <= not(
        ( not(OEBUS_int(3)) and not(nS2W) and R_W040 )
     or ( R_W040 and word )
     or ( R_W040 and byte )
     or ( MAS0 and MAS2 and not(MAS3) and not(R_W040) )            -- M N
--     or ( MAS0 and MAS1 and MAS2 and not(MAS3) and not(R_W040) )     -- M
     or ( not(MAS1) and MAS2 and not(MAS3) and not(R_W040) )         -- B D
     or ( MAS0 and MAS1 and not(MAS2) and MAS3 and not(R_W040) ) );  -- G
--register
OEBUS_int(4) <= not(
        ( MAS0 and not(R_W040) ) -- A B C     F G H   K   M
     or ( MAS1 and not(R_W040) ) --             G H J K L M N DC
     or ( MAS2 and not(R_W040) ) --   B C D E     H J     M N
     or ( not(nS2W) and not(OEBUS_int(4)) and R_W040 )
     or ( R_W040 and long ) );
--register
OEBUS_int(5) <= not(
        ( not(nS2W) and not(OEBUS_int(5)) and R_W040 )
     or ( not(MAS0) and MAS1 and MAS2 and not(R_W040) )
     or ( not(MAS0) and MAS2 and MAS3 and not(R_W040) )
     or ( R_W040 and byte ) );
--register
OEBUS_int(6) <= not(
        ( not(MAS0) and MAS2 and not(R_W040) )
     or ( not(MAS1) and MAS2 and not(R_W040) )
     or ( not(nS2W) and R_W040 and not(OEBUS_int(6)) )
     or ( R_W040 and word )
     or ( MAS0 and MAS1 and not(MAS2) and MAS3 and not(R_W040) ) );
--register
OEBUS_int(7) <= not(
        ( MAS0 and not(R_W040) )
     or ( MAS2 and not(R_W040) )
     or ( MAS1 and MAS3 and not(R_W040) )
     or ( not(nS2W) and R_W040 and not(OEBUS_int(7)) )
     or ( R_W040 and long ) );
      end if;
   end process;

end Behavioral;

