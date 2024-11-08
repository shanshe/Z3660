----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    21:47:04 08/17/2021 
-- Design Name: 
-- Module Name:    z3660 - Behavioral 
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
use IEEE.STD_LOGIC_UNSIGNED.ALL;
-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx primitives in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity z3660 is
    Port ( nAVEC : in  STD_LOGIC;
           nBB040 : inout  STD_LOGIC;
           R_W040 : in  STD_LOGIC;
           PS_MIO_8 : in  STD_LOGIC;
           PS_MIO_0 : out  STD_LOGIC;
           PS_MIO_9 : out  STD_LOGIC;
           PS_MIO_12 : out  STD_LOGIC;
           PS_MIO_13 : in  STD_LOGIC;
           PS_MIO_15 : out  STD_LOGIC;
           INT6_ARM : in  STD_LOGIC;
           nBTT : in  STD_LOGIC;
           nBGR040 : in  STD_LOGIC;
           nINT6 : out  STD_LOGIC;
           R_W040_out : out  STD_LOGIC;
           nINT2 : in  STD_LOGIC;
           n040RSTO : in  STD_LOGIC;
           nSNOOP : out  STD_LOGIC;
           LEBUS_DMA : out  STD_LOGIC;
           CPUCLK_RECVD : in  STD_LOGIC;
           RESET_OUT : out  STD_LOGIC;
           PCLK : in  STD_LOGIC;
           nTS : out  STD_LOGIC;
           nHLT : in  STD_LOGIC;
           nEMUL : in  STD_LOGIC;
           nLOCK : in  STD_LOGIC;
           nLOCKE : in  STD_LOGIC;
           nDSACK : inout  STD_LOGIC_VECTOR (1 downto 0);
           nBR : in  STD_LOGIC;
           nCPURST : in  STD_LOGIC;
           nCIIN : in  STD_LOGIC;
           nBR040 : in  STD_LOGIC;
           nTA : inout  STD_LOGIC;
           nTEA : out  STD_LOGIC;
           SIZ40 : inout  STD_LOGIC_VECTOR (1 downto 0);
           nSTERM : inout  STD_LOGIC;
           V_DETECTOR : in  STD_LOGIC;
           n040RSTI : out  STD_LOGIC;
           TM : inout  STD_LOGIC_VECTOR (2 downto 0);
           TT : inout  STD_LOGIC_VECTOR (1 downto 0);
           J1 : in  STD_LOGIC;
           n040CIOUT : in  STD_LOGIC;
           nIPL : in  STD_LOGIC_VECTOR (2 downto 0);
           nDS040 : inout  STD_LOGIC;
           p040A : in  STD_LOGIC_VECTOR (31 downto 19);
           p040A3 : inout  STD_LOGIC;
           p040A2 : inout  STD_LOGIC;
           p040A1 : inout  STD_LOGIC;
           p040A0 : inout  STD_LOGIC;
           LEBUS : out  STD_LOGIC_VECTOR (7 downto 0);
           OEBUS : out  STD_LOGIC_VECTOR (7 downto 0);
           
           nCLKEN : in  STD_LOGIC; -- be careful, output from FPGA
           n060IPL : out  STD_LOGIC_VECTOR (2 downto 0);
           DBG1 : out  STD_LOGIC;
           DBG2 : out  STD_LOGIC;
           DBG3 : out  STD_LOGIC;
           nAS040 : inout  STD_LOGIC;
           nBERR : in  STD_LOGIC;
           n040EMUL : out  STD_LOGIC;
           nTCI : out  STD_LOGIC;
           nTBI : out  STD_LOGIC;
           nAVEC040 : out  STD_LOGIC;
           nBG040 : out  STD_LOGIC;
           CPUCLK : in  STD_LOGIC;
           nR_W040_out : out  STD_LOGIC;
           FPGA_PRESENCE : in  STD_LOGIC;
           CLK90 : in  STD_LOGIC;
           FC : out  STD_LOGIC_VECTOR (2 downto 0);
           A : inout  STD_LOGIC_VECTOR (3 downto 0);
           nBGACK : in  STD_LOGIC;
           BCLK : in  STD_LOGIC;
           SIZ : inout  STD_LOGIC_VECTOR (1 downto 0);
           nTBI_FPGA : in  STD_LOGIC;
           nTS_FPGA : in  STD_LOGIC;
           nBG : out  STD_LOGIC;
           nBGACK040 : out  STD_LOGIC;
           nDMACOE : out  STD_LOGIC := '1'
         );
end z3660;

architecture Behavioral of z3660 is

   COMPONENT OEBUS_component
   PORT(
         BCLK : in  STD_LOGIC;
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
   END COMPONENT;

   COMPONENT BUSCON
   PORT(
         BCLK : in  STD_LOGIC;
         nBGACK040 : in  STD_LOGIC;
         p040A0 : in  STD_LOGIC;
         p040A1 : in  STD_LOGIC;
         SIZ40 : in  STD_LOGIC_VECTOR (1 downto 0);
         nETERM : in  STD_LOGIC;
         nRDSACK1 : in  STD_LOGIC;
         nRDSACK0 : in  STD_LOGIC;
         nRBERR : in  STD_LOGIC;
--         nRHALT : in  STD_LOGIC;
         nRAVEC : in  STD_LOGIC;
         nCYCPEND : in  STD_LOGIC;
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
--         nDS040 : out  STD_LOGIC;
--         nAS040 : out  STD_LOGIC;
         R_W040 : in  STD_LOGIC;
--         nTBI : out  STD_LOGIC;
--         nTEA : out  STD_LOGIC;
--         nTA : out  STD_LOGIC;
         nTS : in  STD_LOGIC
         );
   END COMPONENT;

   COMPONENT BUSTERM
   PORT(
         BCLK : in  STD_LOGIC;
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
--         nTBI : out  STD_LOGIC;
         n040RSTI : in  STD_LOGIC;
         nTA : out  STD_LOGIC;
         nBGACK040 : in  STD_LOGIC
         );
   END COMPONENT;
   
   COMPONENT BCTL
   PORT(
         BCLK : in  STD_LOGIC;
         nSBGACK030 : in  STD_LOGIC;
         nSBR030 : in  STD_LOGIC;
         nBB040 : in  STD_LOGIC;
         nBR040 : in  STD_LOGIC;
         nLOCK : in  STD_LOGIC;
         nLOCKE : in  STD_LOGIC;
         n040RSTI : in  STD_LOGIC;
         nBG040 : out  STD_LOGIC;
         nBG : out  STD_LOGIC;
         nBGACK040 : out  STD_LOGIC;
         nBR_ARM : in STD_LOGIC;
         nBG_ARM : out STD_LOGIC
         );
   END COMPONENT;
   COMPONENT START
   PORT (
         BCLK : in  STD_LOGIC;
         nAS040 : in  STD_LOGIC;
         nRCIIN : in  STD_LOGIC;
         nTA : in  STD_LOGIC;
         nTEA : in  STD_LOGIC;
         nRAVEC : in  STD_LOGIC;
         nBGACK040 : in  STD_LOGIC;
         nTS : in  STD_LOGIC;
         n040RSTI : in  STD_LOGIC;
--         nTEND : out  STD_LOGIC;
         nCYCPEND : out  STD_LOGIC;
         nTCI : out  STD_LOGIC;
         nAVEC040 : out  STD_LOGIC
         );
   END COMPONENT;

   COMPONENT TAXLAT
   PORT (
         BCLK : in  STD_LOGIC;
         p040A0 : in  STD_LOGIC;
         p040A1 : in  STD_LOGIC;
         p040A2 : in  STD_LOGIC;
         p040A3 : in  STD_LOGIC;
         SIZ40 : in  STD_LOGIC_VECTOR (1 downto 0);
         TT : in  STD_LOGIC_VECTOR (1 downto 0);
         TM : in  STD_LOGIC_VECTOR (2 downto 0);
         MAS0 : in  STD_LOGIC;
         MAS1 : in  STD_LOGIC;
         MAS2 : in  STD_LOGIC;
         MAS3 : in  STD_LOGIC;
         FC : out  STD_LOGIC_VECTOR (2 downto 0);
         SIZ : out  STD_LOGIC_VECTOR (1 downto 0);
         A : out  STD_LOGIC_VECTOR (3 downto 0);
         nIACK : out  STD_LOGIC;
         nBGACK040 : in  STD_LOGIC
         );
   END COMPONENT;
   COMPONENT LEBUS_component
   PORT (
         BCLK : in  STD_LOGIC;
         MAS0 : in  STD_LOGIC;
         MAS1 : in  STD_LOGIC;
         MAS2 : in  STD_LOGIC;
         MAS3 : in  STD_LOGIC;
         nRTERM : in  STD_LOGIC;
         nLSTERM : in  STD_LOGIC;
         LEBUS : out  STD_LOGIC_VECTOR (7 downto 0)
         );
   END COMPONENT;

   COMPONENT TERM
   PORT (
         BCLK : in  STD_LOGIC;
         nBGACK : in STD_LOGIC;
         nPLSTERM : in  STD_LOGIC;
         nRAVEC : out  STD_LOGIC;
         nRBERR : in  STD_LOGIC;
--         nTEND : in  STD_LOGIC;
         nAS040 : in  STD_LOGIC;
         nAVEC : in  STD_LOGIC;
         nDSACK : in  STD_LOGIC_VECTOR (1 downto 0);
         nRDSACK1 : out  STD_LOGIC;
         nBERR : in  STD_LOGIC;
         nRDSACK0 : out  STD_LOGIC;
         nIACK: in STD_LOGIC;
         nETERM : out  STD_LOGIC;
         nRTERM : out  STD_LOGIC
         );
   END COMPONENT;

   COMPONENT RST
   PORT (
         BCLK : in  STD_LOGIC;
--         nAS040 : in  STD_LOGIC;
--         nPLSTERM : in  STD_LOGIC;
         nPORESET : in  STD_LOGIC;
--         nEMUL : in  STD_LOGIC;
--         n040RSTI : in  STD_LOGIC;
--         nLSTERM : out  STD_LOGIC;
         nPWRST : out  STD_LOGIC
--         n040EMUL : out  STD_LOGIC
         );
   END COMPONENT;

signal nIACK: STD_LOGIC;
signal nTEA_int,nTA_int: STD_LOGIC :='1';
signal nBGACK040_int,nBG_int:  STD_LOGIC :='1';
--signal nT1A,nERST: STD_LOGIC :='0';
signal nDS040_int,nAS040_int: STD_LOGIC :='1';
--signal CPUCLK_int,CLK90_int: STD_LOGIC :='0';
signal nPWRST,nLSTERM: STD_LOGIC;
signal nPLSTERM: STD_LOGIC;
signal nRAVEC,nRTERM,nETERM,nRBERR,nRHALT,nRDSACK0,nRDSACK1: STD_LOGIC;
signal MAS0,MAS1,MAS2,MAS3: STD_LOGIC :='0';
signal SLV0,SLV1,SLV2,SLV3: STD_LOGIC :='0';
signal nCYCPEND: STD_LOGIC :='1';
signal nTCI_int: STD_LOGIC :='1';
signal nAVEC040_int: STD_LOGIC :='1';
signal nSBR030,nSBGACK030,nBGACK_D,nBR_D: STD_LOGIC;
signal nRCIIN: STD_LOGIC;
signal nDMACOE_int: STD_LOGIC:='1';
signal OEBUS_int: STD_LOGIC_VECTOR (7 downto 0):="11111111";
signal LEBUS_int: STD_LOGIC_VECTOR (7 downto 0):="00000000";
--signal n040IPL: STD_LOGIC_VECTOR (2 downto 0);
signal nTS_030,nTS_030_s,nTS_030_s1,nTA_int2: STD_LOGIC:='1';
signal BCLK_int,BCLK_d,BCLK_m: STD_LOGIC :='0';

signal nTA_d: STD_LOGIC_VECTOR(3 downto 0):="0000";
signal nTEA_d: STD_LOGIC_VECTOR(3 downto 0):="0000";

signal reset_extended: STD_LOGIC:='1';
signal reset_counter: STD_LOGIC_VECTOR(4 downto 0):="00000";
signal nBG_ARM: STD_LOGIC:='1';

signal nBR_ARM : STD_LOGIC:='1';
signal RESET_CPLD : STD_LOGIC:='1';

signal R_W040_out_int: STD_LOGIC;
signal nR_W040_out_int: STD_LOGIC;
signal RAM_select: std_logic;
signal nTS_int: std_logic;
signal nTA_out: std_logic;
signal SIZ40_int: std_logic_vector(1 downto 0);
signal p040A0_int: std_logic;
signal p040A1_int: std_logic;
signal p040A2_int: std_logic;
signal p040A3_int: std_logic;
signal nDMA_STERM,nDMA_DSACK: std_logic;
signal DMA_select: std_logic;
signal nBG040_int: std_logic;
signal LEBUS_DMA_int: std_logic;
signal n040RSTI_int: std_logic;
signal finish_with_STERM: std_logic:='1';
signal DMA_enable: std_logic:='1';
signal DMA_BUSY: std_logic;

TYPE dma_fsm IS (
            DMA_IDLE,
            DMA_DELAY_TS,
            DMA_START_TS,
            DMA_START_TS1,
            DMA_START_TS2,
            DMA_WAIT_TA,
            DMA_WAIT_TA2,
            DMA_LATCH_BUS_AND_TERM,
            DMA_TERM_WITH_DSACK,
            DMA_TERM_WITH_DSACK2,
            DMA_TERM_DELAY,
            DMA_TERM_DELAY2
);
signal DMA_state: dma_fsm := DMA_IDLE;
attribute fsm_encoding : string;
attribute fsm_encoding of DMA_state : signal is "compact";

TYPE nts_fsm IS (
            NTS_IDLE,
            NTS_START,
            NTS_START_1,
            NTS_STRETCH,
            NTS_STRETCH2,
            NTS_END
);
signal NTS_state: nts_fsm := NTS_IDLE;
attribute fsm_encoding of NTS_state : signal is "compact";

begin

-- enable or disable DMA
   DMA_enable <= '1';
-- finish DMA with STERM
   finish_with_STERM <= '1';
-- finish DMA with DSACK
--   finish_with_STERM <= '0';

   DBG1 <= R_W040;--nTBI_FPGA when (R_W040='1') else '1';--PCLK;--nTS_030;--
   DBG2 <= nTS_int;--'0' when SIZ40="11" else '1';--BCLK_int;--
   DBG3 <= nTA;--nTS;--nTS_FPGA;--

   nBG040 <= nBG040_int;
   n040RSTI <= n040RSTI_int;
   nTA <= --'Z' when nBGACK='0' and DMA_enable='1' else--nTA_int when BCLK_ratio='1' else
          '0' when nTA_d(1 downto 0)="10" else
          '1' when nTA_d(1 downto 0)="0X" else
          'Z';

   nTEA <= nTEA_int when nBGACK040_int='0' else '1';

   nAVEC040 <= nAVEC040_int;
-- TT = "00" -> normal
-- TM = "000" -> Data Cache Push Access
-- TM = "001" -> User Data Access
-- TM = "101" -> Supervisor Data Access
--   TM(2 downto 0) <= "000" when nBG_ARM = '0' else "ZZZ";
--   TT(1 downto 0) <=  "00" when nBG_ARM = '0' else  "ZZ";

   RAM_select <= --'1' when( p040A(31 downto 28) = "0100"  ) else -- 0x40000000 - 0x4FFFFFFF 256 Mbyte 
                 -- Z3 RAM is supposed to be only at DaughterBoard ( and uses FCS, DSx, ... signals) so
                 -- it can't be used by DMA
                 -- CPU RAM can be accessed by DMA (AS, DS, etc signals)
                 '1' when( p040A(31 downto 27) = "00001" ) else -- 0x08000000 - 0x0FFFFFFF 128 Mbyte
--                 '1' when( p040A(31 downto 27) = "00010" ) else -- 0x10000000 - 0x17FFFFFF 128 Mbyte
                 '0';

   nR_W040_out_int <= not R_W040;
    R_W040_out_int <=     R_W040;

   DMA_select <= '1' when (nBGACK='0' and nBG_ARM = '1' and RAM_select='1' and DMA_enable='1' and nAS040='0') else '0';

   nR_W040_out <=  R_W040_out_int when DMA_select='1' else nR_W040_out_int;
   R_W040_out  <= nR_W040_out_int when DMA_select='1' else  R_W040_out_int;


   nTS    <= nTS_int    when DMA_BUSY='1' else 'Z';
   SIZ40  <= SIZ40_int  when DMA_BUSY='1' else "ZZ";
   p040A0 <= p040A0_int when DMA_BUSY='1' else 'Z';
   p040A1 <= p040A1_int when DMA_BUSY='1' else 'Z';
   p040A2 <= p040A2_int when DMA_BUSY='1' else 'Z';
   p040A3 <= p040A3_int when DMA_BUSY='1' else 'Z';

   nTS_030_s <= '1' when DMA_BUSY='1' else nTS_FPGA;
   nTBI      <= '0' when DMA_BUSY='1' else not(nTBI_FPGA);
   nTCI      <= '0' when DMA_BUSY='1' else nTCI_int;

   nDS040 <= nDS040_int when nBGACK040_int='0' else 'Z';
   nAS040 <= nAS040_int when nBGACK040_int='0' else 'Z';

   LEBUS     <= "00000000"    when DMA_BUSY='1' else LEBUS_int; -- don't use LEBA pins (will use LEAB pins with LEBUS_DMA)
   LEBUS_DMA <= LEBUS_DMA_int;-- when DMA_select='1' or DMA_BUSY='1' else '0';
   OEBUS     <= "01101010"    when DMA_BUSY='1' else OEBUS_int; -- 32 bit transparent bus
   nDMACOE   <= '1'           when DMA_BUSY='1' else nDMACOE_int;

   nSNOOP <= '0' when DMA_BUSY='1' or nBG_ARM='0' else 'Z';

   nBB040 <= '0' when DMA_BUSY='1' else 'Z';

   TM(2 downto 0) <= "000" when DMA_BUSY = '1' else--"001" when DMA_select='1' else
                     "000" when nBG_ARM = '0' else
                     "ZZZ";
   TT(1 downto 0) <= "00" when DMA_BUSY = '1' else
                     "00" when nBG_ARM = '0' else
                     "ZZ";

   nSTERM <= nDMA_STERM                when (DMA_BUSY='1' and finish_with_STERM='1') else 'Z';
   nDSACK <= (nDMA_DSACK & nDMA_DSACK) when (DMA_BUSY='1' and finish_with_STERM='0') else "ZZ";

   SIZ40_int <= "00" when SIZ = "11" else SIZ;
   p040A0_int <= A(0);
   p040A1_int <= A(1);
   p040A2_int <= A(2);
   p040A3_int <= A(3);

   DMA_BUSY <= '0' when DMA_state=DMA_IDLE else '1';
	
---- this works with A4091 but doesn't work with AA3000 scsi at 100 MHz
---- but it doesn't work with A4091 at 50 MHz
--   process(PCLK)
--   begin
--      if(PCLK'event and PCLK='1') then
--      -- DMA state machine
--         if(n040RSTI_int='0') then
--            DMA_state<=DMA_IDLE;
--            nTS_int <= '1';
--            LEBUS_DMA_int <= '0';
--            nDMA_STERM <= '1';
--            nDMA_DSACK <= '1';
--         else
--            BCLK_m <= BCLK_int;
--            BCLK_d <= BCLK_m;
--            nTS_int <= '1';
--            LEBUS_DMA_int <= '0';
--            nDMA_STERM <= '1';
--            nDMA_DSACK <= '1';
--            case (DMA_state) is
--               when DMA_IDLE =>
--                  if(BCLK_d = '1' and BCLK_m = '0') then
--                     if (DMA_select='1' and nAS040='0') then
--                        DMA_state<=DMA_DELAY_TS;
----                        DMA_state<=DMA_START_TS;
--                     end if;
--                  end if;
--               when DMA_DELAY_TS =>
--                  if(BCLK_d = '1' and BCLK_m = '0') then
--                     DMA_state<=DMA_START_TS;
--                  end if;
--               when DMA_START_TS =>
--                  nTS_int <= '0';
--                  DMA_state<=DMA_START_TS1;
--               when DMA_START_TS1 =>
--                  nTS_int <= '0';
--                  DMA_state<=DMA_START_TS2;
--                  if(nTA='0') then
--                     DMA_state<=DMA_LATCH_BUS_AND_TERM;
--							LEBUS_DMA_int <= '1';
--						end if;
--               when DMA_START_TS2 =>
--                  nTS_int <= '0';
--                  DMA_state<=DMA_WAIT_TA;
--                  if(nTA='0') then
--                     DMA_state<=DMA_LATCH_BUS_AND_TERM;
--							LEBUS_DMA_int <= '1';
--						end if;
--               when DMA_WAIT_TA =>
--                  if(nTA='0') then
--                     DMA_state<=DMA_LATCH_BUS_AND_TERM;
--							LEBUS_DMA_int <= '1';
--                  end if;
--               when DMA_LATCH_BUS_AND_TERM =>
--                  LEBUS_DMA_int <= '1';
--                  if finish_with_STERM='1' then
---- STERM
--                     nDMA_STERM <= '0';
--                     if(nAS040/='0') then
----                        DMA_state<=DMA_IDLE;
--								DMA_state<=DMA_TERM_DELAY2;
--                     end if;
--                  else
---- DSACK
--                     nDMA_DSACK <= '0';
--                     if(BCLK_d = '1' and BCLK_m = '0') then
--                        DMA_state<=DMA_TERM_WITH_DSACK;
--                     end if;
--                  end if;
--               when DMA_TERM_DELAY2 =>
--                  LEBUS_DMA_int <= '1';
--                  nDMA_STERM <= '0';
--                  if(BCLK_d = '0' and BCLK_m = '1') then
--                     DMA_state<=DMA_IDLE;
--                  end if;
--               when DMA_TERM_WITH_DSACK =>
--                  LEBUS_DMA_int <= '1';
--                  nDMA_DSACK <= '0';
--                  if(BCLK_d = '1' and BCLK_m = '0') then
--                     DMA_state<=DMA_TERM_WITH_DSACK2;
--                  end if;
--               when DMA_TERM_WITH_DSACK2 =>
--                  LEBUS_DMA_int <= '1';
--                  nDMA_DSACK <= '0';
--                  if(nAS040/='0') then
----                     DMA_state<=DMA_IDLE;
--                     DMA_state<=DMA_TERM_DELAY;
--                  end if;
--               when DMA_TERM_DELAY =>
--                  LEBUS_DMA_int <= '1';
--                  nDMA_DSACK <= '0';
--                  if(BCLK_d = '1' and BCLK_m = '0') then
--                     DMA_state<=DMA_IDLE;
--                  end if;
--               when others =>
--                  DMA_state<=DMA_IDLE;
--            end case;
--         end if;
--      end if;
--   end process;
---- end DMA

-- this works with AA3000 scsi but with some read/write errors
-- it doesn't detect A4091 at 50 MHz, and has a lot of errors in AA3000 at 50 MHz
-- it works with A4091 at 100 MHz
--   process(PCLK)
--   begin
--      if(PCLK'event and PCLK='1') then
--      -- DMA state machine
--         nTS_int <= '1';
--         LEBUS_DMA_int <= '0';
--         nDMA_STERM <= '1';
--         nDMA_DSACK <= '1';
--         if(n040RSTI_int='0') then
--            DMA_state<=DMA_IDLE;
--         else
--            BCLK_m <= BCLK_int;
--            BCLK_d <= BCLK_m;
--            case (DMA_state) is
--               when DMA_IDLE =>
--                  if(BCLK_d = '0' and BCLK_m = '1') then
--                     if (DMA_select='1' and nAS040='0') then
--                        DMA_state<=DMA_DELAY_TS;
--							end if;
--                  end if;
--               when DMA_DELAY_TS =>
--                  if(BCLK_d = '0' and BCLK_m = '1') then
--                     DMA_state<=DMA_START_TS1;
--                  end if;
--               when DMA_START_TS1 =>
--                  nTS_int <= '0';
--                  if(BCLK_d = '0' and BCLK_m = '1') then
--							DMA_state<=DMA_WAIT_TA;
--                  end if;
--               when DMA_WAIT_TA =>
--						if(nTA='0') then
--							DMA_state<=DMA_LATCH_BUS_AND_TERM;
--						end if;
--               when DMA_LATCH_BUS_AND_TERM =>
--                  LEBUS_DMA_int <= '1';
--						if(nAS040/='0') then
--							DMA_state<=DMA_IDLE;
--						end if;
--                  if finish_with_STERM='1' then
---- STERM
--                     nDMA_STERM <= '0';
--                  else
---- DSACK
--                     nDMA_DSACK <= '0';
--                  end if;
--               when others =>
--                  DMA_state<=DMA_IDLE;
--            end case;
--         end if;
--      end if;
--   end process;
----end DMA

-- this works with AA3000 scsi at 50 and 100 MHz
-- it doesn't detect A4091 at 50 MHz, but this maybe is a problem of my firebird...
-- it works with A4091 at 100 MHz
   process(PCLK)
   begin
      if(PCLK'event and PCLK='1') then
      -- DMA state machine
         nTS_int <= '1';
         LEBUS_DMA_int <= '0';
         nDMA_STERM <= '1';
         nDMA_DSACK <= '1';
         if(n040RSTI_int='0') then
            DMA_state<=DMA_IDLE;
         else
            BCLK_m <= BCLK_int;
            BCLK_d <= BCLK_m;
            case (DMA_state) is
               when DMA_IDLE =>
                  if(BCLK_d = '0' and BCLK_m = '1') then
                     if (DMA_select='1') then
                        DMA_state<=DMA_START_TS1;
							end if;
                  end if;
               when DMA_START_TS1 =>
                  nTS_int <= '0';
                  if(BCLK_d = '0' and BCLK_m = '1') then
							DMA_state<=DMA_WAIT_TA;
                  end if;
               when DMA_WAIT_TA =>
						if(nTA='0') then
							DMA_state<=DMA_LATCH_BUS_AND_TERM;
							LEBUS_DMA_int <= '1';
						end if;
               when DMA_LATCH_BUS_AND_TERM =>
                  LEBUS_DMA_int <= '1';
						if(nAS040/='0') then
							DMA_state<=DMA_IDLE;
						end if;
                  if finish_with_STERM='1' then
-- STERM
                     nDMA_STERM <= '0';
                  else
-- DSACK
                     nDMA_DSACK <= '0';
                  end if;
               when others =>
                  DMA_state<=DMA_IDLE;
            end case;
         end if;
      end if;
   end process;
--end DMA

-- This works with DMA scsi read ok, write errors) with 060 at PCLK=50 MHz (BCLK=25 MHz). It doesn't work at 100MHz
-- but it doesn't work with A4091 at 50 MHz or 100 MHz
--   process(BCLK_int)
--   begin
--      if(BCLK_int'event and BCLK_int='1') then
--      -- DMA state machine
--         if(n040RSTI_int='0') then
--            DMA_state<=DMA_IDLE;
--            nTS_int <= '1';
--            LEBUS_DMA_int <= '0';
--            nDMA_STERM <= '1';
--            nDMA_DSACK <= '1';
--         else
--            nTS_int <= '1';
--            LEBUS_DMA_int <= '0';
--            nDMA_STERM <= '1';
--            nDMA_DSACK <= '1';
--            case (DMA_state) is
--               when DMA_IDLE =>
--                     if (DMA_select='1' and nAS040='0') then
--                        DMA_state<=DMA_START_TS1;
--                  end if;
--               when DMA_START_TS1 =>
--                  nTS_int <= '0';
--                  DMA_state<=DMA_WAIT_TA;
--                  if(nTA='0') then
--                     DMA_state<=DMA_LATCH_BUS_AND_TERM;
--							LEBUS_DMA_int <= '1';
--                  end if;
--               when DMA_WAIT_TA =>
--                  if(nTA='0') then
--                     DMA_state<=DMA_LATCH_BUS_AND_TERM;
--							LEBUS_DMA_int <= '1';
--                  end if;
--               when DMA_LATCH_BUS_AND_TERM =>
--                  LEBUS_DMA_int <= '1';
--                  if finish_with_STERM='1' then
---- STERM
--                     nDMA_STERM <= '0';
--                     if(nAS040/='0') then
--                        DMA_state<=DMA_IDLE;
--                     end if;
--                  else
---- DSACK
--                     nDMA_DSACK <= '0';
--                     DMA_state<=DMA_TERM_WITH_DSACK;
--                  end if;
--               when DMA_TERM_WITH_DSACK =>
--                  LEBUS_DMA_int <= '1';
--                  nDMA_DSACK <= '0';
--                  DMA_state<=DMA_TERM_WITH_DSACK2;
--               when DMA_TERM_WITH_DSACK2 =>
--                  LEBUS_DMA_int <= '1';
--                  nDMA_DSACK <= '0';
--                  if(nAS040/='0') then
--                     DMA_state<=DMA_IDLE;
--                  end if;
--               when others =>
--                  DMA_state<=DMA_IDLE;
--            end case;
--         end if;
--      end if;
--   end process;
---- end DMA



   nBGACK040  <= '0' when NBG_ARM='1' else nBGACK040_int and nBG_int; -- this signal is 244's buffer trisate

   nBG        <= nBG_int;
   nSBGACK030 <= nBGACK_D or nBGACK;
   nSBR030    <= nBR_D or nBR;

   n040EMUL <= nEMUL or not(n040RSTI_int);

   nINT6 <= '0' when INT6_ARM ='1' else 'Z';

   nLSTERM <= --nAS040_int or 
              nPLSTERM;
   process(PCLK)
   begin
      if(PCLK'event and PCLK='1') then
--         if DMA_BUSY='1' then
--            nPLSTERM <= '1';
--         else
            nPLSTERM <= nSTERM; -- it is really the same signal
--         end if;
      end if;
   end process;

   PS_MIO_0  <= not nIPL(0);
   PS_MIO_9  <= not nIPL(1);
   PS_MIO_12 <= not nIPL(2);
   PS_MIO_15 <= nBG_ARM;

   nBR_ARM    <= PS_MIO_8;
   RESET_CPLD <= PS_MIO_13;

   process(PCLK)
   begin
      if(PCLK'event and PCLK='1') then
         if(n040RSTI_int='0') then
            nTS_state <= NTS_IDLE;
            nTS_030_s1 <= '1';
         else
            nTS_030_s1 <= '1';
            case (nTS_state) is
               when NTS_IDLE =>
                  if (nTS_030_s='0' and BCLK_int='0') then
                     nTS_state <= NTS_START;
                     nTS_030_s1 <= '0';
                  elsif (nTS_030_s='0' and BCLK_int='1') then
                     nTS_state <= NTS_START_1;
                     nTS_030_s1 <= '0';
                  end if;
               when NTS_START_1 =>
                  nTS_030_s1 <= '0';
                  if (BCLK_int='0') then
                     nTS_state <= NTS_START;
                  end if;
               when NTS_START =>
                  nTS_030_s1 <= '0';
                  if (BCLK_int='1') then
                     nTS_state <= NTS_STRETCH;
                  end if;
               when NTS_STRETCH =>
                  if (BCLK_int='0') then
                     nTS_state <= NTS_END;
                  end if;
               when NTS_END =>
                  if (BCLK_int='1') then
                     nTS_state <= NTS_IDLE;
                  end if;
               when others =>
                  nTS_state <= NTS_IDLE;
            end case;
         end if;
      end if;
   end process;

nTS_030 <= --nTS_FPGA and
           nTS_030_s1;

   process(PCLK)
   begin
      if(PCLK'event and PCLK='1') then
         if(n040RSTI_int='0') then
            nTA_d(3 downto 0) <= "1111";
            nTEA_d(3 downto 0) <= "1111";
         else
            nTA_d(3 downto 0) <= nTA_d(2 downto 0) & nTA_int;
            nTEA_d(3 downto 0) <= nTEA_d(2 downto 0) & nTEA_int;
         end if;
      end if;
   end process;

   BCLK_int<=BCLK;
   
   RESET_OUT <= RESET_CPLD; -- reset from FPGA side
--   process(BCLK_int)
--   begin
--      if(BCLK_int'event and BCLK_int='1') then
--         if (n040RSTO='0' or reset_counter/="00000") then
--            if (reset_counter="11111") then
--               reset_extended <= '1';
--               reset_counter <= "00000";
--            else
--               reset_counter <= reset_counter + 1;
--               reset_extended <= '0';
--            end if;
--         else
--            reset_counter <= "00000";
--         end if;
--      end if;
--   end process;
   process(BCLK_int)
   begin
      if(BCLK_int'event and BCLK_int='1') then
         nRBERR <= nBERR;
         nRHALT <= nHLT;
			if nCPURST='0'
			or nPWRST='0'
			or n040RSTO='0'
--			or reset_extended = '1'
			or RESET_CPLD='0' then
				n040RSTI_int<='0';
			else
				n040RSTI_int<='1';
			end if;
--         n040RSTI_int <= nCPURST; -- reset output <= reset input
         nBR_D <= nBR;
         nBGACK_D <= nBGACK;
         nRCIIN <= nCIIN;
      end if;
   end process;

--   process(BCLK_int,n040RSTI_int)
--   begin
--      if(n040RSTI_int='0') then
--         n060IPL <= "111";
----         n060IPL <= "011"; -- extra data write hold
--      elsif(BCLK_int'event and  BCLK_int='1') then
--         n060IPL <= nIPL;
--      end if;
--   end process;
   process(PCLK,n040RSTI_int)
   begin
      if(n040RSTI_int='0') then
         n060IPL <= "111";
--         n060IPL <= "011"; -- extra data write hold
      elsif(PCLK'event and  PCLK='1') then
         n060IPL <= nIPL;
      end if;
   end process;

--OEBUS
   oebus_unit: OEBUS_component PORT MAP (
         BCLK => BCLK_int,
         MAS0 => MAS0,
         MAS1 => MAS1,
         MAS2 => MAS2,
         MAS3 => MAS3,
         SLV0 => SLV0,
         SLV1 => SLV1,
         SLV2 => SLV2,
         SLV3 => SLV3,
         R_W040 => R_W040,
         nRBERR => nRBERR,
         nRDSACK0 => nRDSACK0,
         nRDSACK1 => nRDSACK1,
         nDMACOE => nDMACOE_int,
         OEBUS => OEBUS_int
         );
--END OEBUS

--BUSCON
   buscon_unit: BUSCON PORT MAP (
         BCLK => BCLK_int,
         nBGACK040 => nBGACK040_int,
         p040A0 => p040A0,
         p040A1 => p040A1,
         SIZ40 => SIZ40,
         nETERM => nETERM,
         nRDSACK1 => nRDSACK1,
         nRDSACK0 => nRDSACK0,
         nRBERR => nRBERR,
--         nRHALT => nRHALT,
         nRAVEC => nRAVEC,
         nCYCPEND => nCYCPEND,
         n040RSTI => n040RSTI_int,
         nLSTERM => nLSTERM,
         SLV0 => SLV0,
         SLV1 => SLV1,
         SLV2 => SLV2,
         SLV3 => SLV3,
         MAS0 => MAS0,
         MAS1 => MAS1,
         MAS2 => MAS2,
         MAS3 => MAS3,
--         nDS040 => nDS040_int,
--         nAS040 => nAS040_int,
         R_W040 => R_W040,
--         nTBI => nTBI,
--         nTEA => nTEA_int,
--         nTA => nTA_int,
         nTS => nTS_030
         );
--END BUSCON

--BUSTERM
   busterm_unit: BUSTERM PORT MAP (
         BCLK => BCLK_int,
         R_W040 => R_W040,
         SLV0 => SLV0,
         SLV1 => SLV1,
         SLV2 => SLV2,
         SLV3 => SLV3,
         nCYCPEND => nCYCPEND,
         SIZ40 => SIZ40,
         nPLSTERM => nPLSTERM,
         nRTERM => nRTERM,
         nTS => nTS_030,
         nDS040 => nDS040_int,
         nAS040 => nAS040_int,
         nTEA => nTEA_int,
--         nTBI => nTBI,
         n040RSTI => n040RSTI_int,
         nTA => nTA_int,
         nBGACK040 => nBGACK040_int
         );
--END BUSTERM

--BCTL
   bctl_unit: BCTL PORT MAP (
         BCLK => BCLK_int,
         nSBGACK030 => nSBGACK030,
         nSBR030 => nSBR030,
         nBB040 => nBB040,
         nBR040 => nBR040,
         nLOCK => nLOCK,
         nLOCKE => nLOCKE,
         n040RSTI => n040RSTI_int,
         nBG040 => nBG040_int,
         nBG => nBG_int,
         nBGACK040 => nBGACK040_int,
         nBR_ARM => nBR_ARM,
         nBG_ARM => nBG_ARM
         );
--END BCTL

--START
   start_unit: START PORT MAP (
         BCLK => BCLK_int,
         nAS040 => nAS040_int,
         nRCIIN => nRCIIN,
         nTA => nTA_int,
         nTEA => nTEA_int,
         nRAVEC => nRAVEC,
         nBGACK040 => nBGACK040_int,
         nTS => nTS_030,
         n040RSTI => n040RSTI_int,
--         nTEND => nTEND,
         nCYCPEND => nCYCPEND,
         nTCI => nTCI_int,
         nAVEC040 => nAVEC040_int
         );
--END START

--TAXLAT
   taxlat_unit: TAXLAT PORT MAP (
         BCLK => BCLK_int,
         p040A0 => p040A0,
         p040A1 => p040A1,
         p040A2 => p040A2,
         p040A3 => p040A3,
         SIZ40 => SIZ40,
         TT => TT,
         TM => TM,
         MAS0 => MAS0,
         MAS1 => MAS1,
         MAS2 => MAS2,
         MAS3 => MAS3,
         FC => FC,
         SIZ => SIZ,
         A => A,
         nIACK => nIACK,
         nBGACK040 => nBGACK040_int
         );
--END TAXLAT

--LEBUS
   lebus_unit: LEBUS_component PORT MAP (
         BCLK => BCLK_int,
         MAS0 => MAS0,
         MAS1 => MAS1,
         MAS2 => MAS2,
         MAS3 => MAS3,
         nRTERM => nRTERM,
         nLSTERM => nLSTERM,
         LEBUS => LEBUS_int
         );
--END LEBUS

--TERM
   term_unit: TERM PORT MAP (
         BCLK => BCLK_int,
         nBGACK => nBGACK,
         nPLSTERM => nPLSTERM,
         nRAVEC => nRAVEC,
         nRBERR => nRBERR,
--         nTEND => nTEND,
         nAS040 => nAS040_int,
         nAVEC => nAVEC,
         nDSACK => nDSACK,
         nRDSACK1 => nRDSACK1,
         nBERR => nBERR,
         nRDSACK0 => nRDSACK0,
         nIACK => nIACK,
         nETERM => nETERM,
         nRTERM => nRTERM
         );
--END TERM

--RST
   rst_unit: RST PORT MAP (
         BCLK => BCLK_int,
--         nAS040 => nAS040_int,
--         nPLSTERM => nPLSTERM,
         nPORESET => V_DETECTOR,
--         nEMUL => nEMUL,
--         n040RSTI => n040RSTI_int,
--         nLSTERM => nLSTERM,
         nPWRST => nPWRST
--         n040EMUL => n040EMUL
         );
--END RST

end Behavioral;