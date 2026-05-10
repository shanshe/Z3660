----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    09:26:33 08/20/2021 
-- Design Name: 
-- Module Name:    BUSCON - Behavioral 
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

entity BUSCON is
    Port ( BCLK : in  STD_LOGIC;
--           nBGACK040 : in  STD_LOGIC;
           p040A0 : in  STD_LOGIC;
           p040A1 : in  STD_LOGIC;
           SIZ40 : in  STD_LOGIC_VECTOR (1 downto 0);
           nETERM : in  STD_LOGIC;
           nRDSACK1 : in  STD_LOGIC;
           nRDSACK0 : in  STD_LOGIC;
           nRBERR : in  STD_LOGIC;
--           nRHALT : in  STD_LOGIC;
--           nRAVEC : in  STD_LOGIC;
--           nCYCPEND : in  STD_LOGIC;
           n040RSTI : in  STD_LOGIC;
           nLSTERM : in  STD_LOGIC;
           SLV0 : out  STD_LOGIC;
           SLV1 : out  STD_LOGIC;
           SLV2 : out  STD_LOGIC;
           SLV3 : out  STD_LOGIC;
           MAS0 : out  STD_LOGIC;
           MAS1 : out  STD_LOGIC;
           MAS2 : out  STD_LOGIC;
           MAS3 : out  STD_LOGIC;
--           nDS040 : out  STD_LOGIC;
--           nAS040 : out  STD_LOGIC;
--           R_W040 : in STD_LOGIC;
--           nTBI : out  STD_LOGIC;
--           nTEA : out  STD_LOGIC;
--           nTA : out  STD_LOGIC;
           nTS : in  STD_LOGIC
           );
end BUSCON;

architecture Behavioral of BUSCON is

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
CONSTANT  S0: std_logic_vector(2 downto 0):= "000"; -- 0
CONSTANT  S1: std_logic_vector(2 downto 0):= "001"; -- 1
CONSTANT  S2: std_logic_vector(2 downto 0):= "010"; -- 2
CONSTANT  S3: std_logic_vector(2 downto 0):= "011"; -- 3
CONSTANT  S4: std_logic_vector(2 downto 0):= "100"; -- 4
CONSTANT  SB: std_logic_vector(2 downto 0):= "110"; -- 6

--SIZ40 values
CONSTANT long: std_logic_vector(1 downto 0):= "00";
CONSTANT byte: std_logic_vector(1 downto 0):= "01";
CONSTANT word: std_logic_vector(1 downto 0):= "10";
CONSTANT line: std_logic_vector(1 downto 0):= "11";

CONSTANT addr0: std_logic_vector(1 downto 0):= "00";
CONSTANT addr1: std_logic_vector(1 downto 0):= "01";
CONSTANT addr2: std_logic_vector(1 downto 0):= "10";
CONSTANT addr3: std_logic_vector(1 downto 0):= "11";

signal MAS_state : STD_LOGIC_VECTOR (3 downto 0);
signal MAS_state_next : STD_LOGIC_VECTOR (3 downto 0);
signal SLV_state : STD_LOGIC_VECTOR (3 downto 0);
signal SLV_state_next : STD_LOGIC_VECTOR (3 downto 0);

impure function byteport return std_logic is begin
  return (nRBERR and not(nRDSACK0) and nRDSACK1); end byteport;
impure function wordport return std_logic is begin
  return (nRBERR and nRDSACK0 and not(nRDSACK1)); end wordport;
impure function longport return std_logic is begin
  return (nRBERR and not(nRDSACK0) and not(nRDSACK1)); end longport;
impure function noport return std_logic is begin
  return (nRBERR and nRDSACK0 and nRDSACK1); end noport;

signal SLV0_int: std_logic;
signal SLV1_int: std_logic;
signal SLV2_int: std_logic;
signal SLV3_int: std_logic;
signal MAS0_int: std_logic;
signal MAS1_int: std_logic;
signal MAS2_int: std_logic;
signal MAS3_int: std_logic;
signal ADDR : STD_LOGIC_VECTOR (1 downto 0);
--signal ASDS_state : STD_LOGIC_VECTOR (3 downto 0);

begin
SLV0 <= SLV0_int;
SLV1 <= SLV1_int;
SLV2 <= SLV2_int;
SLV3 <= SLV3_int;
MAS0 <= MAS0_int;
MAS1 <= MAS1_int;
MAS2 <= MAS2_int;
MAS3 <= MAS3_int;
--MAS3_int <= MAS_state(3);
--MAS2_int <= MAS_state(2);
--MAS1_int <= MAS_state(1);
--MAS0_int <= MAS_state(0);
--SLV3_int <= SLV_state(3);
--SLV2_int <= SLV_state(2);
--SLV1_int <= SLV_state(1);
--SLV0_int <= SLV_state(0);
MAS_state <= MAS3_int & MAS2_int & MAS1_int & MAS0_int;
SLV_state <= SLV3_int & SLV2_int & SLV1_int & SLV0_int;
MAS0_int <= MAS_state_next(0);
MAS1_int <= MAS_state_next(1);
MAS2_int <= MAS_state_next(2);
MAS3_int <= MAS_state_next(3);
SLV0_int <= SLV_state_next(0);
SLV1_int <= SLV_state_next(1);
SLV2_int <= SLV_state_next(2);
SLV3_int <= SLV_state_next(3);

--SLV_state <= SLV3_int & SLV2_int & SLV1_int & SLV0_int;
ADDR <= p040A1 & p040A0;

--SLV_state_next(3) <= not(
--        (     SLV2_int  and not(SLV1_int) and not(SLV0_int) )
--     or ( not(SLV2_int) and     SLV1_int  and not(SLV0_int) ) );
--   nS2W <= not(not(SLV3)          and SLV1 and SLV0);
SLV_state_next(3) <= '0' when (SLV_state(2 downto 0)=S4 or SLV_state(2 downto 0)=S2) else '1';
   process(BCLK,n040RSTI)
   begin
      if(n040RSTI='0') then
         MAS_state_next <= I;
      elsif(RISING_EDGE(BCLK)) then

        case (MAS_state) is
            when I =>
                  if (nTS='1') then                               MAS_state_next <= I;
               elsif (nTS='0' and SIZ40=line) then                MAS_state_next <= A;
               elsif (nTS='0' and SIZ40=long) then                MAS_state_next <= A;
               elsif (nTS='0' and SIZ40=word and ADDR=addr0) then MAS_state_next <= F;
               elsif (nTS='0' and SIZ40=word and ADDR=addr2) then MAS_state_next <= G;
               elsif (nTS='0' and SIZ40=byte and ADDR=addr0) then MAS_state_next <= K;
               elsif (nTS='0' and SIZ40=byte and ADDR=addr1) then MAS_state_next <= L;
               elsif (nTS='0' and SIZ40=byte and ADDR=addr2) then MAS_state_next <= M;
               elsif (nTS='0' and SIZ40=byte and ADDR=addr3) then MAS_state_next <= N;
               else                                               MAS_state_next <= I;
               end if;
            when A =>
                  if (SLV0_int='0') then                          MAS_state_next <= A;
               elsif (SLV0_int='1' and nRBERR='0') then           MAS_state_next <= Z;
               elsif (SLV0_int='1' and byteport='1') then         MAS_state_next <= C;
               elsif (SLV0_int='1' and wordport='1') then         MAS_state_next <= B;
               elsif (SLV0_int='1' and longport='1') then         MAS_state_next <= I;
               else                                               MAS_state_next <= I;
               end if;
            when B =>
                  if (SLV0_int='0') then                          MAS_state_next <= B;
               elsif (SLV0_int='1' and nRBERR='0') then           MAS_state_next <= Z;
               else                                               MAS_state_next <= I;
               end if;
            when C =>
                  if (SLV0_int='0') then                          MAS_state_next <= C;
               elsif (SLV0_int='1' and nRBERR='0') then           MAS_state_next <= Z;
               elsif (SLV0_int='1' and byteport='1') then         MAS_state_next <= D;
               else                                               MAS_state_next <= I;
               end if;
            when D =>
                  if (SLV0_int='0') then                          MAS_state_next <= D;
               elsif (SLV0_int='1' and nRBERR='0') then           MAS_state_next <= Z;
               elsif (SLV0_int='1' and byteport='1') then         MAS_state_next <= E;
               else                                               MAS_state_next <= I;
               end if;
            when E =>
                  if (SLV0_int='0') then                          MAS_state_next <= E;
               elsif (SLV0_int='1' and nRBERR='0') then           MAS_state_next <= Z;
               elsif (SLV0_int='1' and byteport='1') then         MAS_state_next <= I;
               else                                               MAS_state_next <= I;
               end if;
            when F =>
                  if (SLV0_int='0') then                          MAS_state_next <= F;
               elsif (SLV0_int='1' and nRBERR='0') then           MAS_state_next <= Z;
               elsif (SLV0_int='1' and byteport='1') then         MAS_state_next <= H;
               else                                               MAS_state_next <= I;
               end if;
            when G =>
                  if (SLV0_int='0') then                          MAS_state_next <= G;
               elsif (SLV0_int='1' and nRBERR='0') then           MAS_state_next <= Z;
               elsif (SLV0_int='1' and byteport='1') then         MAS_state_next <= J;
               else                                               MAS_state_next <= I;
               end if;
            when H =>
                  if (SLV0_int='0') then                          MAS_state_next <= H;
               elsif (SLV0_int='1' and nRBERR='0') then           MAS_state_next <= Z;
               elsif (SLV0_int='1' and byteport='1') then         MAS_state_next <= I;
               else                                               MAS_state_next <= I;
               end if;
            when J =>
                  if (SLV0_int='0') then                          MAS_state_next <= J;
               elsif (SLV0_int='1' and nRBERR='0') then           MAS_state_next <= Z;
               elsif (SLV0_int='1' and byteport='1') then         MAS_state_next <= I;
               else                                               MAS_state_next <= I;
               end if;
            when K =>
                  if (SLV0_int='0') then                          MAS_state_next <= K;
               elsif (SLV0_int='1' and nRBERR='0') then           MAS_state_next <= Z;
               else                                               MAS_state_next <= I;
               end if;
            when L =>
                  if (SLV0_int='0') then                          MAS_state_next <= L;
               elsif (SLV0_int='1' and nRBERR='0') then           MAS_state_next <= Z;
               else                                               MAS_state_next <= I;
               end if;
            when M =>
                  if (SLV0_int='0') then                          MAS_state_next <= M;
               elsif (SLV0_int='1' and nRBERR='0') then           MAS_state_next <= Z;
               else                                               MAS_state_next <= I;
               end if;
            when N =>
                  if (SLV0_int='0') then                          MAS_state_next <= N;
               elsif (SLV0_int='1' and nRBERR='0') then           MAS_state_next <= Z;
               else                                               MAS_state_next <= I;
               end if;
            when others =>
                                                                  MAS_state_next <= I;
        end case;
      end if;
   end process;
   process(BCLK,n040RSTI)
   begin
      if(n040RSTI='0') then
         SLV_state_next(2 downto 0) <= S0;
      elsif(RISING_EDGE(BCLK)) then

         case (SLV_state(2 downto 0)) is
            when S0 =>
                  if(nTS='0') then                                SLV_state_next(2 downto 0) <= S2;
               elsif(not(MAS_state = I)) then                     SLV_state_next(2 downto 0) <= S2;
               else                                               SLV_state_next(2 downto 0) <= S0;
               end if;
            when S1 =>
                                                                  SLV_state_next(2 downto 0) <= S0;
            when S2 =>
                                                                  SLV_state_next(2 downto 0) <= S4;
            when S3 =>
                  if(nRBERR='0') then                             SLV_state_next(2 downto 0) <= SB;
               else                                               SLV_state_next(2 downto 0) <= S0;
               end if;
            when S4 =>
               if(nETERM='1' and nLSTERM='1') then                SLV_state_next(2 downto 0) <= S4;
               elsif( ( nETERM='0' and
                        nRDSACK1='1' and nRDSACK0='1' ) ) or
                      ( nETERM='0' and nRDSACK0='1' and
                        (MAS_state=A )                  ) or
                      ( nETERM='0' and nRDSACK1='1' and
                        (MAS_state=A or MAS_state=C or
                         MAS_state=D or MAS_state=F or
                         MAS_state=G )                  ) then    SLV_state_next(2 downto 0) <= S3;
               elsif(nRBERR='1' and
                    (nETERM='0' or nLSTERM='0') ) then            SLV_state_next(2 downto 0) <= S1;
               else
                                                                  SLV_state_next(2 downto 0) <= S0;
               end if;
            when SB =>
                                                                  SLV_state_next(2 downto 0) <= S0;
            when others =>
                                                                  SLV_state_next(2 downto 0) <= S0;
         end case;
      end if;
   end process;

end Behavioral;

