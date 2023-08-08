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
           nBGACK040 : in  STD_LOGIC;
           p040A0 : in  STD_LOGIC;
           p040A1 : in  STD_LOGIC;
           SIZ40 : in  STD_LOGIC_VECTOR (1 downto 0);
           nETERM : in  STD_LOGIC;
           nRDSACK1 : in  STD_LOGIC;
           nRDSACK0 : in  STD_LOGIC;
           nRBERR : in  STD_LOGIC;
--           nRHALT : in  STD_LOGIC;
           nRAVEC : in  STD_LOGIC;
           nCYCPEND : in  STD_LOGIC;
           n040RSTI : in  STD_LOGIC;
           nLSTERM : in  STD_LOGIC;
           SLV0 : buffer  STD_LOGIC;
           SLV1 : buffer  STD_LOGIC;
           SLV2 : buffer  STD_LOGIC;
           SLV3 : buffer  STD_LOGIC;
           MAS0 : buffer  STD_LOGIC;
           MAS1 : buffer  STD_LOGIC;
           MAS2 : buffer  STD_LOGIC;
           MAS3 : buffer  STD_LOGIC;
--           nDS040 : out  STD_LOGIC;
--           nAS040 : out  STD_LOGIC;
           R_W040 : in STD_LOGIC;
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
CONSTANT  S3: std_logic_vector(3 downto 0):= "0000"; -- 0
CONSTANT  S0: std_logic_vector(3 downto 0):= "0100"; -- 4
CONSTANT  S1: std_logic_vector(3 downto 0):= "1000"; -- 0
CONSTANT  S2: std_logic_vector(3 downto 0):= "0001"; -- 1
CONSTANT S2W: std_logic_vector(3 downto 0):= "0011"; -- 3
CONSTANT S3W: std_logic_vector(3 downto 0):= "0010"; -- 2
CONSTANT  SB: std_logic_vector(3 downto 0):= "0101"; -- 5
CONSTANT SR0: std_logic_vector(3 downto 0):= "0111"; -- 7
CONSTANT SR1: std_logic_vector(3 downto 0):= "0110"; -- 6

--SIZ40 values
CONSTANT long: std_logic_vector(1 downto 0):= "00";
CONSTANT byte: std_logic_vector(1 downto 0):= "01";
CONSTANT word: std_logic_vector(1 downto 0):= "10";
CONSTANT line: std_logic_vector(1 downto 0):= "11";

CONSTANT addr0: std_logic_vector(1 downto 0):= "00";
CONSTANT addr1: std_logic_vector(1 downto 0):= "01";
CONSTANT addr2: std_logic_vector(1 downto 0):= "10";
CONSTANT addr3: std_logic_vector(1 downto 0):= "11";

CONSTANT     sidle: std_logic_vector(3 downto 0):= "0000"; -- Idle state, wait for TS assertion
CONSTANT    ts_rec: std_logic_vector(3 downto 0):= "0010"; -- Transfer start recognized
CONSTANT  rd_state: std_logic_vector(3 downto 0):= "1100"; -- Assert AS and DS
CONSTANT  wr_state: std_logic_vector(3 downto 0):= "1000"; -- Assert AS only
CONSTANT wait_term: std_logic_vector(3 downto 0):= "1101"; -- Wait for termination to occur 
CONSTANT cycle_end: std_logic_vector(3 downto 0):= "0001"; -- End of cycle go back to start
CONSTANT nRHALT: std_logic:='1';
--signal MAS_state : STD_LOGIC_VECTOR (3 downto 0);
--signal SLV_state : STD_LOGIC_VECTOR (3 downto 0);

impure function byteport return std_logic is begin
  return (nRBERR and not(nRDSACK0) and nRDSACK1); end byteport;
impure function wordport return std_logic is begin
  return (nRBERR and nRDSACK0 and not(nRDSACK1)); end wordport;
impure function longport return std_logic is begin
  return (nRBERR and not(nRDSACK0) and not(nRDSACK1)); end longport;
impure function noport return std_logic is begin
  return (nRBERR and nRDSACK0 and nRDSACK1); end noport;

--impure function bus_err return std_logic is begin
--  return (not(nRBERR) and nRHALT); end bus_err;
--impure function retry return std_logic is begin
--  return (not(nRBERR) and not(nRHALT)); end retry;
--impure function norm_wait return std_logic is begin
--  return (nRBERR and not(nRHALT)); end norm_wait;
--impure function norm_nowait return std_logic is begin
--  return (nRBERR and nRHALT); end norm_nowait;
--
--impure function final return std_logic is begin
--   if (   (longport='1' and (   MAS_state=A or
--                              MAS_state=F or
--                              MAS_state=G or
--                              MAS_state=K or
--                              MAS_state=L or
--                              MAS_state=M or
--                              MAS_state=N ))
--      or (wordport='1' and (    MAS_state=B or
--                              MAS_state=F or
--                              MAS_state=G or
--                              MAS_state=K or
--                              MAS_state=L or
--                              MAS_state=M or
--                              MAS_state=N ))
--      or (byteport='1' and (    MAS_state=E or
--                              MAS_state=H or
--                              MAS_state=J or
--                              MAS_state=K or
--                              MAS_state=L or
--                              MAS_state=M or
--                              MAS_state=N ))
--      or (nRAVEC='0' and nRBERR='1') ) then
--      return '1';
--   else
--      return '0';
--   end if;
--end final;
  

impure function first_term_sample return std_logic is begin
  return (not(nRDSACK0) or not(nRDSACK1) or not(nRBERR) or not(nRAVEC)); end first_term_sample;

--signal ADDR : STD_LOGIC_VECTOR (1 downto 0);
--signal ASDS_state : STD_LOGIC_VECTOR (3 downto 0);
begin
--MAS3 <= MAS_state(3);
--MAS2 <= MAS_state(2);
--MAS1 <= MAS_state(1);
--MAS0 <= MAS_state(0);--SLV3 <= SLV_state(3);
--SLV2 <= SLV_state(2);
--SLV1 <= SLV_state(1);
--SLV0 <= SLV_state(0);
--MAS_state <= MAS3 & MAS2 & MAS1 & MAS0;
--SLV_state <= SLV3 & SLV2 & SLV1 & SLV0;
--ADDR <= p040A1 & p040A0;
--   nTBI <= '0';-- when SLV_state=S2 and SIZ40=line else '1';

--   nTA <= '0' when SLV_state=S2 else '1';
--   nTEA <= '0' when SLV_state=SB else '1';
--   nAS040 <= '0' when SLV_state=S1 or SLV_state=S0 else '1';
--   nDS040 <= '0' when (SLV_state=S1 or SLV_state=S0) and (R_W040='1') else '1';

-- motorola way
--   process(BCLK,n040RSTI)
--   begin
--      if(n040RSTI='0') then
--         MAS_state <= I;
--      elsif(BCLK'event and  BCLK='1') then
--
--         case (MAS_state) is
--            when I =>
--                  if (nTS='1') then                               MAS_state <= I;
--               elsif (nTS='0' and SIZ40=line) then                MAS_state <= A;
--               elsif (nTS='0' and SIZ40=long) then                MAS_state <= A;
--               elsif (nTS='0' and SIZ40=word and ADDR=addr0) then MAS_state <= F;
--               elsif (nTS='0' and SIZ40=word and ADDR=addr2) then MAS_state <= G;
--               elsif (nTS='0' and SIZ40=byte and ADDR=addr0) then MAS_state <= K;
--               elsif (nTS='0' and SIZ40=byte and ADDR=addr1) then MAS_state <= L;
--               elsif (nTS='0' and SIZ40=byte and ADDR=addr2) then MAS_state <= M;
--               elsif (nTS='0' and SIZ40=byte and ADDR=addr3) then MAS_state <= N;
--               elsif (nTS='0' and SIZ40=word and ADDR=addr1) then MAS_state <= I; -- impossible case
--               elsif (nTS='0' and SIZ40=word and ADDR=addr3) then MAS_state <= I; -- impossible case
--               else                                               MAS_state <= I;
--               end if;
--            when A =>
--                  if (not(SLV_state=S1)) then                  MAS_state <= A;
--               elsif (SLV_state=S1 and retry='1') then         MAS_state <= A;
--               elsif (SLV_state=S1 and bus_err='1') then       MAS_state <= Z;
--               elsif (SLV_state=S1 and byteport='1') then      MAS_state <= C;
--               elsif (SLV_state=S1 and wordport='1') then      MAS_state <= B;
--               elsif (SLV_state=S1 and longport='1') then      MAS_state <= I;
--               elsif (SLV_state=S1 and noport='1') then        MAS_state <= I; -- impossible case
--               else                                            MAS_state <= I;
--               end if;
--            when B =>
--                  if (not(SLV_state=S1)) then                  MAS_state <= B;
--               elsif (SLV_state=S1 and retry='1') then         MAS_state <= B;
--               elsif (SLV_state=S1 and bus_err='1') then       MAS_state <= Z;
--               elsif (SLV_state=S1 and byteport='1') then      MAS_state <= Z;
--               elsif (SLV_state=S1 and wordport='1') then      MAS_state <= I;
--               elsif (SLV_state=S1 and longport='1') then      MAS_state <= Z;
--               elsif (SLV_state=S1 and noport='1') then        MAS_state <= I; -- impossible case
--               else                                            MAS_state <= I;
--               end if;
--            when C =>
--                  if (not(SLV_state=S1)) then                  MAS_state <= C;
--               elsif (SLV_state=S1 and retry='1') then         MAS_state <= C;
--               elsif (SLV_state=S1 and bus_err='1') then       MAS_state <= Z;
--               elsif (SLV_state=S1 and byteport='1') then      MAS_state <= D;
--               elsif (SLV_state=S1 and wordport='1') then      MAS_state <= Z;
--               elsif (SLV_state=S1 and longport='1') then      MAS_state <= Z;
--               elsif (SLV_state=S1 and noport='1') then        MAS_state <= I; -- impossible case
--               else                                            MAS_state <= I;
--               end if;
--            when D =>
--                  if (not(SLV_state=S1)) then                  MAS_state <= D;
--               elsif (SLV_state=S1 and retry='1') then         MAS_state <= D;
--               elsif (SLV_state=S1 and bus_err='1') then       MAS_state <= Z;
--               elsif (SLV_state=S1 and byteport='1') then      MAS_state <= E;
--               elsif (SLV_state=S1 and wordport='1') then      MAS_state <= Z;
--               elsif (SLV_state=S1 and longport='1') then      MAS_state <= Z;
--               elsif (SLV_state=S1 and noport='1') then        MAS_state <= I; -- impossible case
--               else                                            MAS_state <= I;
--               end if;
--            when E =>
--                  if (not(SLV_state=S1)) then                  MAS_state <= E;
--               elsif (SLV_state=S1 and retry='1') then         MAS_state <= E;
--               elsif (SLV_state=S1 and bus_err='1') then       MAS_state <= Z;
--               elsif (SLV_state=S1 and byteport='1') then      MAS_state <= I;
--               elsif (SLV_state=S1 and wordport='1') then      MAS_state <= Z;
--               elsif (SLV_state=S1 and longport='1') then      MAS_state <= Z;
--               elsif (SLV_state=S1 and noport='1') then        MAS_state <= I; -- impossible case
--               else                                            MAS_state <= I;
--               end if;
--            when F =>
--                  if (not(SLV_state=S1)) then                  MAS_state <= F;
--               elsif (SLV_state=S1 and retry='1') then         MAS_state <= F;
--               elsif (SLV_state=S1 and bus_err='1') then       MAS_state <= Z;
--               elsif (SLV_state=S1 and byteport='1') then      MAS_state <= H;
--               elsif (SLV_state=S1 and wordport='1') then      MAS_state <= I;
--               elsif (SLV_state=S1 and longport='1') then      MAS_state <= I;
--               elsif (SLV_state=S1 and noport='1') then        MAS_state <= I; -- impossible case
--               else                                            MAS_state <= I;
--               end if;
--            when G =>
--                  if (not(SLV_state=S1)) then                  MAS_state <= G;
--               elsif (SLV_state=S1 and retry='1') then         MAS_state <= G;
--               elsif (SLV_state=S1 and bus_err='1') then       MAS_state <= Z;
--               elsif (SLV_state=S1 and byteport='1') then      MAS_state <= J;
--               elsif (SLV_state=S1 and wordport='1') then      MAS_state <= I;
--               elsif (SLV_state=S1 and longport='1') then      MAS_state <= I;
--               elsif (SLV_state=S1 and noport='1') then        MAS_state <= I; -- impossible case
--               else                                            MAS_state <= I;
--               end if;
--            when H =>
--                  if (not(SLV_state=S1)) then                  MAS_state <= H;
--               elsif (SLV_state=S1 and retry='1') then         MAS_state <= H;
--               elsif (SLV_state=S1 and bus_err='1') then       MAS_state <= Z;
--               elsif (SLV_state=S1 and byteport='1') then      MAS_state <= I;
--               elsif (SLV_state=S1 and wordport='1') then      MAS_state <= Z;
--               elsif (SLV_state=S1 and longport='1') then      MAS_state <= Z;
--               elsif (SLV_state=S1 and noport='1') then        MAS_state <= I; -- impossible case
--               else                                            MAS_state <= I;
--               end if;
--            when J =>
--                  if (not(SLV_state=S1)) then                  MAS_state <= J;
--               elsif (SLV_state=S1 and retry='1') then         MAS_state <= J;
--               elsif (SLV_state=S1 and bus_err='1') then       MAS_state <= Z;
--               elsif (SLV_state=S1 and byteport='1') then      MAS_state <= I;
--               elsif (SLV_state=S1 and wordport='1') then      MAS_state <= Z;
--               elsif (SLV_state=S1 and longport='1') then      MAS_state <= Z;
--               elsif (SLV_state=S1 and noport='1') then        MAS_state <= I; -- impossible case
--               else                                            MAS_state <= I;
--               end if;
--            when K =>
--                  if (not(SLV_state=S1)) then                  MAS_state <= K;
--               elsif (SLV_state=S1 and retry='1') then         MAS_state <= K;
--               elsif (SLV_state=S1 and bus_err='1') then       MAS_state <= Z;
--               elsif (SLV_state=S1 and byteport='1') then      MAS_state <= I;
--               elsif (SLV_state=S1 and wordport='1') then      MAS_state <= I;
--               elsif (SLV_state=S1 and longport='1') then      MAS_state <= I;
--               elsif (SLV_state=S1 and noport='1') then        MAS_state <= I; -- impossible case
--               else                                            MAS_state <= I;
--               end if;
--            when L =>
--                  if (not(SLV_state=S1)) then                  MAS_state <= L;
--               elsif (SLV_state=S1 and retry='1') then         MAS_state <= L;
--               elsif (SLV_state=S1 and bus_err='1') then       MAS_state <= Z;
--               elsif (SLV_state=S1 and byteport='1') then      MAS_state <= I;
--               elsif (SLV_state=S1 and wordport='1') then      MAS_state <= I;
--               elsif (SLV_state=S1 and longport='1') then      MAS_state <= I;
--               elsif (SLV_state=S1 and noport='1') then        MAS_state <= I; -- impossible case
--               else                                            MAS_state <= I;
--               end if;
--            when M =>
--                  if (not(SLV_state=S1)) then                  MAS_state <= M;
--               elsif (SLV_state=S1 and retry='1') then         MAS_state <= M;
--               elsif (SLV_state=S1 and bus_err='1') then       MAS_state <= Z;
--               elsif (SLV_state=S1 and byteport='1') then      MAS_state <= I;
--               elsif (SLV_state=S1 and wordport='1') then      MAS_state <= I;
--               elsif (SLV_state=S1 and longport='1') then      MAS_state <= I;
--               elsif (SLV_state=S1 and noport='1') then        MAS_state <= I; -- impossible case
--               else                                            MAS_state <= I;
--               end if;
--            when N =>
--                  if (not(SLV_state=S1)) then                  MAS_state <= N;
--               elsif (SLV_state=S1 and retry='1') then         MAS_state <= N;
--               elsif (SLV_state=S1 and bus_err='1') then       MAS_state <= Z;
--               elsif (SLV_state=S1 and byteport='1') then      MAS_state <= I;
--               elsif (SLV_state=S1 and wordport='1') then      MAS_state <= I;
--               elsif (SLV_state=S1 and longport='1') then      MAS_state <= I;
--               elsif (SLV_state=S1 and noport='1') then        MAS_state <= I; -- impossible case
--               else                                            MAS_state <= I;
--               end if;
--            when others =>                                     MAS_state <= I;
--         end case;
------nAS040 <= not(ASDS_state(3));
------nDS040 <= not(ASDS_state(2));
----         case (ASDS_state) is
----            when sidle =>
----nAS040 <= '1';
----nDS040 <= '1';
----               if( nTS='0') then ASDS_state <= ts_rec;
----               else              ASDS_state <= sidle;
----               end if;
----            when ts_rec =>
----nAS040 <= '1';
----nDS040 <= '1';
----               if(R_W040='1') then                    ASDS_state <= rd_state;
----               else                                   ASDS_state <= wr_state;
----               end if;
----            when rd_state =>
----nAS040 <= '0';
----nDS040 <= '0';
----               if(nLSTERM='1') then                   ASDS_state <= wait_term;
----               else                                   ASDS_state <= cycle_end;
----               end if;
----            when wr_state =>
----nAS040 <= '0';
----nDS040 <= '0';
----               if(nLSTERM='1') then                   ASDS_state <= wait_term;
----               else                                   ASDS_state <= cycle_end;
----               end if;
----            when wait_term =>
----nAS040 <= '0';
----nDS040 <= '0';
----               if(nLSTERM='0' or nETERM='0') then     ASDS_state <= cycle_end;
----               else                                   ASDS_state <= wait_term;
----               end if;
----            when cycle_end =>
----nAS040 <= '1';
----nDS040 <= '1';
----               if(SLV_state=S2 or SLV_state=SB) then  ASDS_state <= sidle;
----               else
----                  if(not(SLV_state=SR1 or SLV_state=S3)) then ASDS_state <= cycle_end;
----                  else
----                     if(R_W040='1') then              ASDS_state <= rd_state;
----                     else                             ASDS_state <= wr_state;
----                     end if;
----                  end if;
----               end if;
----            when others =>
----nAS040 <= '1';
----nDS040 <= '1';
----               ASDS_state <= sidle;
----         end case;
--
--         case (SLV_state) is
--            when S3 =>
--                  if (nTS='0') then                       SLV_state <= S0;
--               elsif (MAS_state=I) then                   SLV_state <= S3;
--               elsif (MAS_state=Z) then                   SLV_state <= SB;
--               else                                       SLV_state <= S0;
--               end if;
--            when S0 =>
--                  if (first_term_sample='1' and nETERM='0' and nLSTERM='1') then SLV_state <= S1;
--               elsif (first_term_sample='0' and nETERM='0' and nLSTERM='1') then SLV_state <= S0;
--               elsif (nETERM='1' and nLSTERM='1') then                           SLV_state <= S0;
--               elsif (nLSTERM='0') then                                          SLV_state <= S1;
--               end if;
--            when S1 =>
--                  if (bus_err='1') then                   SLV_state <= SB;
--               elsif (retry='1') then                     SLV_state <= SR0;
--               elsif (norm_wait='1' and final='0') then   SLV_state <= S3W;
--               elsif (norm_wait='1' and final='1') then   SLV_state <= S2W;
--               elsif (norm_nowait='1' and final='0') then SLV_state <= S3;
--               elsif (norm_nowait='1' and final='1') then SLV_state <= S2;
--               end if;
--            when S2 =>
--                                                          SLV_state <= S3;
--            when S2W =>
--               if (nRHALT='1') then                       SLV_state <= S2;
--               else                                       SLV_state <= S2W;
--               end if;
--            when S3W =>
--               if (MAS_state=Z or MAS_state=DC) then      SLV_state <= SB;
--               else
--                  if(nRHALT='0') then                     SLV_state <= S3W;
--                  else                                    SLV_state <= S3;
--                  end if;
--               end if;
--            when SR0 =>
--               if (MAS_state=Z or MAS_state=DC) then      SLV_state <= SB;
--               else
--                  if(nRHALT='0') then                     SLV_state <= SR0;
--                  else                                    SLV_state <= SR1;
--                  end if;
--               end if;
--            when SR1 =>
--                                                          SLV_state <= S0;
--            when SB =>
--                                                          SLV_state <= S3;
--            when others => -- must be copy of S1
--                  if (bus_err='1') then                   SLV_state <= SB;
--               elsif (retry='1') then                     SLV_state <= SR0;
--               elsif (norm_wait='1' and final='0') then   SLV_state <= S3W;
--               elsif (norm_wait='1' and final='1') then   SLV_state <= S2W;
--               elsif (norm_nowait='1' and final='0') then SLV_state <= S3;
--               elsif (norm_nowait='1' and final='1') then SLV_state <= S2;
--               end if;
--         end case;
--      end if;
--   end process;

--SLV3 <= not(
--        ( not(SLV1) and SLV2 and not(SLV0) )
--     or ( SLV1 and not(SLV2) and not(SLV0) ) );
--   process(BCLK,n040RSTI)
--   begin
--      if(n040RSTI='0') then
--         nDS040 <= '1';
--         nAS040 <= '1';
--      elsif(BCLK'event and  BCLK='1') then
--         if(nBGACK040='0') then
--nAS040 <= not(
--        ( not(SLV0) and not(SLV1) and SLV2 )
--     or ( not(SLV0) and SLV1 and not(SLV2) ) );
--nDS040 <= not(
--        ( not(SLV0) and not(SLV1) and SLV2 )
--     or ( not(SLV0) and SLV1 and not(SLV2) ) );
--         else
--            nAS040 <= '1';
--            nDS040 <= '1';
--         end if;
--      end if;
--   end process;

--nAS040 <= not(
--        ( not(SLV0) and not(SLV1) and SLV2 )
--     or ( not(SLV0) and SLV1 and not(SLV2) ) );
--nDS040 <= not(
--        ( not(SLV0) and not(SLV1) and SLV2 )
--     or ( not(SLV0) and SLV1 and not(SLV2) ) );
--
--nTA <= not(
--        ( SLV0 and not(SLV1) and not(SLV2) ) );
--nTEA <= not(
--        ( not(SLV0) and SLV1 and SLV2 ) );

-- original from A3660
SLV3 <= not(
        ( not(SLV1) and SLV2 and not(SLV0) )
     or ( SLV1 and not(SLV2) and not(SLV0) ) );
   process(BCLK,n040RSTI)
   begin
      if(n040RSTI='0') then
         MAS0 <= '0';
         MAS1 <= '0';
         MAS2 <= '0';
         MAS3 <= '0';
      elsif(BCLK'event and  BCLK='1') then

--register
MAS1 <= (
        ( MAS1 and MAS2 and not(SLV0) )
     or ( MAS1 and not(MAS3) and not(SLV0) )
     or ( MAS1 and MAS0 and not(SLV0) and n040RSTI )
     or ( MAS0 and not(MAS2) and MAS3 and SLV0 and byteport )
     or ( not(nTS) and not(MAS1) and not(MAS0) and not(MAS2) and SIZ40(0) and not(MAS3) and not(SIZ40(1)) )
     or ( not(nTS) and not(MAS1) and not(p040A0) and not(MAS0) and p040A1 and not(MAS2) and not(SIZ40(0)) and not(MAS3) and SIZ40(1) ) );
--register
MAS0 <= (
        ( MAS0 and not(SLV0) and n040RSTI )
     or ( MAS1 and MAS0 and MAS2 and not(SLV0) )
     or ( MAS0 and not(MAS2) and not(MAS3) and not(SLV0) )
     or ( not(nTS) and not(MAS1) and not(p040A0) and not(MAS0) and not(MAS2) and not(MAS3) )
     or ( not(MAS1) and MAS0 and not(MAS2) and not(MAS3) and wordport )
     or ( not(MAS1) and MAS0 and not(MAS2) and SLV0 and byteport )
     or ( not(nTS) and not(MAS1) and not(MAS0) and not(MAS2) and SIZ40(0) and not(MAS3) and SIZ40(1) )
     or ( not(nTS) and not(MAS1) and not(MAS0) and not(MAS2) and not(SIZ40(0)) and not(MAS3) and not(SIZ40(1)) ) );
--register
MAS2 <= (
        ( MAS1 and MAS2 and not(SLV0) )
     or ( MAS2 and not(SLV0) and n040RSTI )
     or ( not(MAS1) and MAS0 and not(MAS2) and SLV0 and byteport )
     or ( not(MAS1) and MAS0 and MAS3 and SLV0 and byteport )
     or ( MAS0 and not(MAS2) and MAS3 and SLV0 and byteport )
     or ( not(MAS1) and MAS0 and not(MAS2) and not(MAS3) and SLV0 and wordport )
     or ( not(MAS1) and not(MAS0) and MAS2 and not(MAS3) and SLV0 and byteport )
     or ( not(nTS) and not(MAS1) and not(MAS0) and p040A1 and not(MAS2) and SIZ40(0) and not(MAS3) and not(SIZ40(1)) ) );
--register
MAS3 <= (
        ( MAS0 and SLV0 and not(nRBERR) )
     or ( MAS2 and SLV0 and not(nRBERR) )
     or ( MAS1 and not(MAS3) and SLV0 and not(nRBERR) )
     or ( MAS1 and MAS2 and MAS3 and not(SLV0) )
     or ( MAS0 and MAS3 and not(SLV0) and n040RSTI )
     or ( MAS2 and MAS3 and not(SLV0) and n040RSTI )
     or ( not(MAS1) and MAS0 and not(MAS2) and nRDSACK1 and SLV0 and not(nRDSACK0) )
     or ( MAS0 and not(MAS2) and MAS3 and nRDSACK1 and SLV0 and not(nRDSACK0) )
     or ( not(MAS1) and not(MAS0) and MAS2 and not(MAS3) and nRDSACK1 and SLV0 and not(nRDSACK0) )
     or ( not(nTS) and not(MAS1) and not(p040A0) and not(MAS0) and not(MAS2) and not(SIZ40(0)) and not(MAS3) and SIZ40(1) ) );
end if;
end process;
   process(BCLK,n040RSTI)
   begin
      if(n040RSTI='0') then
         SLV0 <= '0';
         SLV1 <= '0';
         SLV2 <= '0';
--         SLV3 <= '0';
      elsif(BCLK'event and  BCLK='1') then

--register
SLV1 <= (
        ( SLV1 and not(SLV2) and SLV0 and not(nRBERR) )
     or ( MAS0 and not(SLV1) and not(SLV2) and not(SLV0) )
     or ( MAS1 and not(SLV1) and not(SLV2) and not(SLV0) )
     or ( MAS2 and not(SLV1) and not(SLV2) and not(SLV0) )
     or ( MAS3 and not(SLV1) and not(SLV2) and not(SLV0) )
     or ( not(nTS) and not(SLV1) and not(SLV2) and not(SLV0) )
     or ( not(SLV1) and not(nETERM) and SLV2 and nRDSACK1 and not(SLV0) and nRDSACK0 )
     or ( not(MAS1) and MAS0 and not(MAS2) and not(SLV1) and not(nETERM) and nRDSACK1 and not(SLV0) )
     or ( not(MAS1) and MAS0 and MAS3 and not(SLV1) and not(nETERM) and nRDSACK1 and not(SLV0) )
     or ( MAS0 and not(MAS2) and MAS3 and not(SLV1) and not(nETERM) and nRDSACK1 and not(SLV0) )
     or ( not(MAS1) and MAS0 and not(MAS2) and not(MAS3) and not(SLV1) and not(nETERM) and not(SLV0) and nRDSACK0 )
     or ( not(MAS1) and not(MAS0) and MAS2 and not(MAS3) and not(SLV1) and not(nETERM) and nRDSACK1 and not(SLV0) ) );
--register
SLV2 <= (
        ( SLV1 and not(SLV2) and not(nRBERR) )
     or ( SLV1 and not(SLV2) and not(SLV0) )
     or ( not(SLV1) and nETERM and SLV2 and not(SLV0) and nLSTERM ) );
--register
SLV0 <= (
        ( not(SLV1) and SLV2 and not(SLV0) and nRBERR and not(nLSTERM) )
     or ( not(SLV1) and not(nETERM) and SLV2 and not(SLV0) and nRBERR )
     or ( not(SLV1) and not(nETERM) and SLV2 and nRDSACK1 and not(SLV0) and nRDSACK0 )
     or ( not(MAS1) and MAS0 and not(MAS2) and not(SLV1) and not(nETERM) and SLV2 and nRDSACK1 and not(SLV0) )
     or ( not(MAS1) and MAS0 and MAS3 and not(SLV1) and not(nETERM) and SLV2 and nRDSACK1 and not(SLV0) )
     or ( MAS0 and not(MAS2) and MAS3 and not(SLV1) and not(nETERM) and SLV2 and nRDSACK1 and not(SLV0) )
     or ( not(MAS1) and MAS0 and not(MAS2) and not(MAS3) and not(SLV1) and not(nETERM) and SLV2 and not(SLV0) and nRDSACK0 )
     or ( not(MAS1) and not(MAS0) and MAS2 and not(MAS3) and not(SLV1) and not(nETERM) and SLV2 and nRDSACK1 and not(SLV0) ) );
      end if;
   end process;

--Original from A3640
--   process(BCLK,n040RSTI)
--   begin
--      if(n040RSTI='0') then
--         SLV0 <= '0';
--         SLV1 <= '0';
--         SLV2 <= '0';
--         SLV3 <= '0';
--         MAS0 <= '0';
--         MAS1 <= '0';
--         MAS2 <= '0';
--         MAS3 <= '0';
--      elsif(BCLK'event and  BCLK='1') then
----register
--MAS1 <= (
--        ( MAS1 and MAS0 and MAS2 and not(MAS3) and not(SLV3) )
--     or ( not(nBGACK040) and not(MAS1) and not(p040A0) and not(MAS0) and p040A1 and not(MAS2) and not(SIZ40(0)) and not(MAS3) and SIZ40(1) and not(nCYCPEND) )
--     or ( not(nTS) and not(nBGACK040) and not(MAS1) and not(p040A0) and not(MAS0) and p040A1 and not(MAS2) and not(SIZ40(0)) and not(MAS3) and SIZ40(1) )
--     or ( MAS1 and not(MAS2) and not(MAS3) and not(SLV3) )
--     or ( MAS1 and not(MAS0) and MAS2 and not(SLV3) )
--     or ( not(nBGACK040) and not(MAS1) and not(MAS0) and not(MAS2) and SIZ40(0) and not(MAS3) and not(SIZ40(1)) and not(nCYCPEND) )
--     or ( not(nTS) and not(nBGACK040) and not(MAS1) and not(MAS0) and not(MAS2) and SIZ40(0) and not(MAS3) and not(SIZ40(1)) )
--     or ( MAS0 and not(MAS2) and MAS3 and nRDSACK1 and not(nRDSACK0) and SLV3 and nRBERR )
--     or ( MAS1 and MAS0 and MAS3 and not(SLV3) ) );
----register
--MAS0 <= (
--        ( not(nBGACK040) and not(MAS1) and not(MAS0) and not(MAS2) and SIZ40(0) and not(MAS3) and SIZ40(1) and not(nCYCPEND) )
--     or ( not(nTS) and not(nBGACK040) and not(MAS1) and not(MAS0) and not(MAS2) and SIZ40(0) and not(MAS3) and SIZ40(1) )
--     or ( not(nBGACK040) and not(MAS1) and not(MAS0) and not(MAS2) and not(SIZ40(0)) and not(MAS3) and not(SIZ40(1)) and not(nCYCPEND) )
--     or ( not(nTS) and not(nBGACK040) and not(MAS1) and not(MAS0) and not(MAS2) and not(SIZ40(0)) and not(MAS3) and not(SIZ40(1)) )
--     or ( not(nBGACK040) and not(MAS1) and not(p040A0) and not(MAS0) and not(MAS2) and not(SIZ40(0)) and not(MAS3) and SIZ40(1) and not(nCYCPEND) )
--     or ( not(nTS) and not(nBGACK040) and not(MAS1) and not(p040A0) and not(MAS0) and not(MAS2) and not(SIZ40(0)) and not(MAS3) and SIZ40(1) )
--     or ( not(nBGACK040) and not(MAS1) and not(p040A0) and not(MAS0) and not(MAS2) and SIZ40(0) and not(MAS3) and not(SIZ40(1)) and not(nCYCPEND) )
--     or ( not(nTS) and not(nBGACK040) and not(MAS1) and not(p040A0) and not(MAS0) and not(MAS2) and SIZ40(0) and not(MAS3) and not(SIZ40(1)) )
--     or ( not(MAS1) and MAS0 and not(MAS2) and not(MAS3) and not(nRDSACK1) and nRDSACK0 and SLV3 and nRBERR )
--     or ( MAS0 and not(SLV3) )
--     or ( not(MAS1) and MAS0 and not(MAS2) and nRDSACK1 and not(nRDSACK0) and SLV3 and nRBERR ) );
----register
--MAS2 <= (
--        ( MAS2 and not(SLV3) )
--     or ( not(nBGACK040) and not(MAS1) and not(MAS0) and p040A1 and not(MAS2) and SIZ40(0) and not(MAS3) and not(SIZ40(1)) and not(nCYCPEND) )
--     or ( not(nTS) and not(nBGACK040) and not(MAS1) and not(MAS0) and p040A1 and not(MAS2) and SIZ40(0) and not(MAS3) and not(SIZ40(1)) )
--     or ( not(MAS1) and MAS0 and not(MAS2) and nRDSACK1 and not(nRDSACK0) and SLV3 and nRBERR )
--     or ( not(MAS1) and MAS0 and not(MAS2) and not(MAS3) and not(nRDSACK1) and nRDSACK0 and SLV3 and nRBERR )
--     or ( MAS1 and MAS0 and not(MAS2) and MAS3 and nRDSACK1 and not(nRDSACK0) and SLV3 and nRBERR )
--     or ( not(MAS1) and MAS0 and MAS2 and MAS3 and nRDSACK1 and not(nRDSACK0) and SLV3 and nRBERR )
--     or ( not(MAS1) and not(MAS0) and MAS2 and not(MAS3) and nRDSACK1 and not(nRDSACK0) and SLV3 and nRBERR ) );
----register
--MAS3 <= (
--        ( MAS1 and MAS0 and MAS2 and not(MAS3) and SLV3 and not(nRBERR) )
--     or ( not(nBGACK040) and not(MAS1) and not(p040A0) and not(MAS0) and not(MAS2) and not(SIZ40(0)) and not(MAS3) and SIZ40(1) and not(nCYCPEND) )
--     or ( not(nTS) and not(nBGACK040) and not(MAS1) and not(p040A0) and not(MAS0) and not(MAS2) and not(SIZ40(0)) and not(MAS3) and SIZ40(1) )
--     or ( not(MAS1) and MAS0 and not(MAS3) and SLV3 and not(nRBERR) )
--     or ( not(MAS1) and MAS0 and not(MAS2) and nRDSACK1 and not(nRDSACK0) and SLV3 and nRBERR )
--     or ( MAS1 and not(MAS2) and not(MAS3) and SLV3 and not(nRBERR) )
--     or ( MAS2 and MAS3 and SLV3 and not(nRBERR) )
--     or ( not(MAS0) and MAS2 and not(MAS3) and SLV3 and not(nRBERR) )
--     or ( not(MAS1) and not(MAS0) and MAS2 and not(MAS3) and nRDSACK1 and not(nRDSACK0) and SLV3 and nRBERR )
--     or ( MAS0 and not(MAS2) and MAS3 and not(SLV3) )
--     or ( MAS0 and not(MAS2) and MAS3 and SLV3 and not(nRBERR) )
--     or ( MAS1 and MAS0 and not(MAS2) and MAS3 and nRDSACK1 and not(nRDSACK0) and SLV3 and nRBERR )
--     or ( MAS2 and MAS3 and not(SLV3) ) );
----register
--SLV1 <= (
--        ( not(SLV1) and not(SLV2) and not(SLV0) and SLV3 and not(nRBERR) )
--     or ( not(MAS1) and not(MAS0) and not(MAS2) and MAS3 and not(SLV1) and not(SLV2) and not(SLV0) and not(SLV3) ) );
----register
--SLV2 <= (
--        ( not(SLV1) and not(SLV2) and not(SLV0) and SLV3 and not(nRBERR) )
--     or ( not(MAS1) and not(MAS0) and not(MAS2) and MAS3 and not(SLV1) and not(SLV2) and not(SLV0) and not(SLV3) )
--     or ( MAS2 and not(SLV1) and not(SLV2) and not(SLV0) and not(SLV3) )
--     or ( MAS1 and not(SLV1) and not(SLV2) and not(SLV0) and not(SLV3) )
--     or ( MAS0 and not(SLV1) and not(SLV2) and not(SLV0) and not(SLV3) )
--     or ( not(SLV1) and SLV2 and nRDSACK1 and not(SLV0) and nRDSACK0 and not(SLV3) and nRBERR and nLSTERM )
--     or ( not(SLV1) and nETERM and SLV2 and not(SLV0) and not(SLV3) and nLSTERM ) );
----register
--SLV0 <= (
--        ( MAS1 and MAS0 and MAS2 and not(MAS3) and not(SLV1) and not(SLV2) and not(nRDSACK1) and not(SLV0) and not(nRDSACK0) and SLV3 and nRBERR )
--     or ( not(MAS0) and MAS2 and MAS3 and not(SLV1) and not(SLV2) and nRDSACK1 and not(SLV0) and not(nRDSACK0) and SLV3 and nRBERR )
--     or ( MAS1 and MAS0 and MAS2 and not(SLV1) and not(SLV2) and nRDSACK1 and not(SLV0) and not(nRDSACK0) and SLV3 and nRBERR )
--     or ( MAS1 and not(MAS2) and not(MAS3) and not(SLV1) and not(SLV2) and nRDSACK1 and not(SLV0) and not(nRDSACK0) and SLV3 and nRBERR )
--     or ( MAS1 and not(MAS0) and MAS2 and not(MAS3) and not(SLV1) and not(SLV2) and not(SLV0) and not(nRDSACK0) and SLV3 and nRBERR )
--     or ( MAS0 and MAS2 and not(MAS3) and not(SLV1) and not(SLV2) and not(nRDSACK1) and not(SLV0) and nRDSACK0 and SLV3 and nRBERR )
--     or ( MAS0 and not(MAS2) and MAS3 and not(SLV1) and not(SLV2) and not(nRDSACK1) and not(SLV0) and nRDSACK0 and SLV3 and nRBERR )
--     or ( MAS1 and not(MAS2) and not(MAS3) and not(SLV1) and not(SLV2) and not(nRDSACK1) and not(SLV0) and nRDSACK0 and SLV3 and nRBERR )
--     or ( MAS1 and not(MAS0) and MAS2 and not(MAS3) and not(SLV1) and not(SLV2) and not(nRDSACK1) and not(SLV0) and nRDSACK0 and SLV3 and nRBERR )
--     or ( MAS1 and not(MAS0) and not(MAS2) and not(MAS3) and not(SLV1) and not(SLV2) and not(nRDSACK1) and not(SLV0) and not(nRDSACK0) and SLV3 and nRBERR )
--     or ( MAS0 and not(MAS2) and not(SLV1) and not(SLV2) and not(nRDSACK1) and not(SLV0) and not(nRDSACK0) and SLV3 and nRBERR ) );
----register
--SLV3 <= (
--        ( not(SLV1) and SLV2 and not(SLV0) and not(SLV3) and not(nLSTERM) )
--     or ( not(SLV1) and not(nETERM) and SLV2 and not(SLV0) and not(nRDSACK0) and not(SLV3) and nLSTERM )
--     or ( not(SLV1) and not(nETERM) and SLV2 and not(nRDSACK1) and not(SLV0) and not(SLV3) and nLSTERM )
--     or ( not(SLV1) and not(nETERM) and SLV2 and not(SLV0) and not(SLV3) and not(nRBERR) and nLSTERM ) );
--
--      end if;
--   end process;

end Behavioral;

