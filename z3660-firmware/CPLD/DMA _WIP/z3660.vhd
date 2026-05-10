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
           PS_MIO_15 : inout  STD_LOGIC;
           INT6_ARM : in  STD_LOGIC;
           nINT6 : out  STD_LOGIC;
           nCEAB : out  STD_LOGIC;
           n040RSTO : in  STD_LOGIC;
           nSNOOP : out  STD_LOGIC;
           LEBUS_DMA : out  STD_LOGIC;
           RESET_OUT : out  STD_LOGIC;
           nTS : inout  STD_LOGIC;
           nEMUL : in  STD_LOGIC;
           nLOCK : in  STD_LOGIC;
           nLOCKE : in  STD_LOGIC;
           nDSACK : inout  STD_LOGIC_VECTOR (1 downto 0);
           nBR : in  STD_LOGIC;
           nCPURST : inout  STD_LOGIC;
           nCIIN : in  STD_LOGIC;
           nBR040 : in  STD_LOGIC;
           nTA : inout  STD_LOGIC;
           nTEA : out  STD_LOGIC;
           SIZ40 : inout  STD_LOGIC_VECTOR (1 downto 0);
           nSTERM : inout  STD_LOGIC;
           V_DETECTOR : in  STD_LOGIC;
           n040RSTI : out  STD_LOGIC;
           TM0 : inout  STD_LOGIC;
           TM1 : inout  STD_LOGIC;
           TM2 : inout  STD_LOGIC;
           TT : inout  STD_LOGIC_VECTOR (1 downto 0);
           J1 : in  STD_LOGIC;
           nIPL : in  STD_LOGIC_VECTOR (2 downto 0);
           nDS040 : inout  STD_LOGIC;
--           p040A : in  STD_LOGIC_VECTOR (31 downto 27);
           p040A : in  STD_LOGIC_VECTOR (31 downto 19);
           p040A3 : inout  STD_LOGIC;
           p040A2 : inout  STD_LOGIC;
           p040A1 : inout  STD_LOGIC;
           p040A0 : inout  STD_LOGIC;
           LEBUS : out  STD_LOGIC_VECTOR (7 downto 0);
           OEBUS : out  STD_LOGIC_VECTOR (7 downto 0);
           
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
           nCEBA : out  STD_LOGIC;
           FC : out  STD_LOGIC_VECTOR (2 downto 0);
           A : inout  STD_LOGIC_VECTOR (3 downto 0);
           nBGACK : in  STD_LOGIC;
           SIZ : inout  STD_LOGIC_VECTOR (1 downto 0);
           nTBI_FPGA : in  STD_LOGIC;
           nTS_FPGA : in  STD_LOGIC;
           nBG : out  STD_LOGIC;
           nBGACK040 : out  STD_LOGIC;
           nDMACOE : out  STD_LOGIC;
--           CPUCLK_RECVD : in  STD_LOGIC;
           CLK060 : in  STD_LOGIC;
           CPLDCLK : in  STD_LOGIC;
           nHLT : inout  STD_LOGIC--;

-- NOT USED BUT WIRED
--           nCLKEN : in  STD_LOGIC;
--           CPUCLK : in  STD_LOGIC;
--           CLK90 : in  STD_LOGIC;
--           FPGA_PRESENCE : in  STD_LOGIC;
--           p040A : in  STD_LOGIC_VECTOR (26 downto 19);
--           n040CIOUT : in  STD_LOGIC;
--           nBTT : in  STD_LOGIC;
--           nBGR040 : in  STD_LOGIC;
--           nINT2 : in  STD_LOGIC
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
         nS2W : in STD_LOGIC;
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
         p040A0 : in  STD_LOGIC;
         p040A1 : in  STD_LOGIC;
         SIZ40 : in  STD_LOGIC_VECTOR (1 downto 0);
         nETERM : in  STD_LOGIC;
         nRDSACK1 : in  STD_LOGIC;
         nRDSACK0 : in  STD_LOGIC;
         nRBERR : in  STD_LOGIC;
--         nRHALT : in  STD_LOGIC;
--         nRAVEC : in  STD_LOGIC;
--         nCYCPEND : in  STD_LOGIC;
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
--         R_W040 : in  STD_LOGIC;
--         nTBI : out  STD_LOGIC;
--         nTEA : out  STD_LOGIC;
--         nTA : out  STD_LOGIC;
         nTS : in  STD_LOGIC
         );
   END COMPONENT;

   COMPONENT BUSTERM
   PORT(
--         BCLK : in  STD_LOGIC;
--         R_W040 : in  STD_LOGIC;
         SLV0 : in  STD_LOGIC;
         SLV1 : in  STD_LOGIC;
         SLV2 : in  STD_LOGIC;
--         SLV3 : in  STD_LOGIC;
--         nCYCPEND : in  STD_LOGIC;
--         SIZ40 : in  STD_LOGIC_VECTOR (1 downto 0);
--         nPLSTERM : in  STD_LOGIC;
--         nRTERM : in  STD_LOGIC;
--         nTS : in  STD_LOGIC;
         nDS040 : out  STD_LOGIC;
         nAS040 : out  STD_LOGIC;
         nTEA : out  STD_LOGIC;
--         nTBI : out  STD_LOGIC;
--         n040RSTI : in  STD_LOGIC;
         nTA : out  STD_LOGIC
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
         nBG_ARM : out STD_LOGIC;
         nBOSS: in STD_LOGIC;
         nTEND: in STD_LOGIC
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
--         nBGACK040 : in  STD_LOGIC;
         nTS : in  STD_LOGIC;
         n040RSTI : in  STD_LOGIC;
         nTEND : out  STD_LOGIC;
--         nCYCPEND : out  STD_LOGIC;
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
         A : out  STD_LOGIC_VECTOR (3 downto 0)--;
--         nIACK : out  STD_LOGIC
         );
   END COMPONENT;
   COMPONENT LEBUS_component
   PORT (
         BCLK : in  STD_LOGIC;
         MAS0 : in  STD_LOGIC;
         MAS1 : in  STD_LOGIC;
         MAS2 : in  STD_LOGIC;
         MAS3 : in  STD_LOGIC;
         nRTERM   : in  STD_LOGIC;
         nLSTERM  : in  STD_LOGIC;
         LEBUS    : out STD_LOGIC_VECTOR (7 downto 0);
         DMA_BUSY : in  STD_LOGIC
         );
   END COMPONENT;

   COMPONENT TERM
   PORT (
         BCLK : in  STD_LOGIC;
         nBOSS : in STD_LOGIC;
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
--         nIACK: in STD_LOGIC;
         nETERM : out  STD_LOGIC;
         nRTERM : out  STD_LOGIC
         );
   END COMPONENT;

--signal nIACK: STD_LOGIC;
signal nTEA_int,nTA_int: STD_LOGIC :='1';
signal nBGACK040_int,nBG_int:  STD_LOGIC :='1';
signal nDS040_int,nAS040_int: STD_LOGIC :='1';
--signal CPUCLK_int,CLK90_int: STD_LOGIC :='0';
signal nLSTERM,nPLSTERM: STD_LOGIC;
signal nRAVEC,nRTERM,nETERM,nRBERR,nRDSACK0,nRDSACK1: STD_LOGIC;
--signal nRHALT: STD_LOGIC;
signal MAS0,MAS1,MAS2,MAS3: STD_LOGIC :='0';
signal SLV0,SLV1,SLV2,SLV3: STD_LOGIC :='0';
--signal nCYCPEND: STD_LOGIC :='1';
signal nTCI_int: STD_LOGIC :='1';
signal nAVEC040_int: STD_LOGIC :='1';
signal nSBR030,nSBGACK030,nBGACK_D,nBR_D: STD_LOGIC;
signal nRCIIN: STD_LOGIC;
signal nDMACOE_int: STD_LOGIC:='1';
signal OEBUS_int: STD_LOGIC_VECTOR (7 downto 0):="11111111";
signal LEBUS_int: STD_LOGIC_VECTOR (7 downto 0):="00000000";
--signal n040IPL: STD_LOGIC_VECTOR (2 downto 0);
signal nTS_030: STD_LOGIC:='1';
signal BCLK_int,CPLDCLK_int,BCLK_d,BCLK_m: STD_LOGIC :='0';

signal nTA_d: STD_LOGIC_VECTOR(1 downto 0):="00";
--signal nTEA_d: STD_LOGIC_VECTOR(3 downto 0):="0000";
signal nTEND: STD_LOGIC;

signal nBG_ARM: STD_LOGIC:='1';

signal nBR_ARM : STD_LOGIC:='1';
signal RESET_CPLD : STD_LOGIC:='0';

signal nCEAB_int: STD_LOGIC;
signal nCEBA_int: STD_LOGIC;
signal RAM_select: std_logic;
signal RTG_select: std_logic;
signal ROM_select: std_logic;
signal nTS_dma: std_logic;
signal nDMA_STERM: std_logic;
signal nDMA_DSACK: std_logic_vector(1 downto 0);
signal DMA_select: std_logic;
signal nBG040_int: std_logic;
signal LEBUS_DMA_int: std_logic;
signal n040RSTI_int: std_logic;
signal finish_DMA_cycle: std_logic_vector(2 downto 0):="111";
signal DMA_BUSY: std_logic :='0';
signal nBOSS: std_logic;
signal nBOSS_fake_jumper: std_logic := '0';
signal PS_MIO_8_last: std_logic := '0';
signal A_int: std_logic_vector(3 downto 0);
signal SIZ_int: std_logic_vector(1 downto 0);
signal FC_int: std_logic_vector(2 downto 0);
signal nS2W: STD_LOGIC;
signal nCPURST_d: STD_LOGIC;

TYPE dma_fsm IS (
            DMA_IDLE,
            DMA_START_TS,
            DMA_WAIT_TA,
            DMA_LATCH_BUS_AND_TERM
);
signal DMA_state: dma_fsm := DMA_IDLE;
attribute fsm_encoding : string;
attribute fsm_encoding of DMA_state : signal is "compact";

TYPE nts_fsm IS (
            NTS_IDLE,
            NTS_START,
            NTS_STRETCH,
            NTS_END
);
signal NTS_state: nts_fsm := NTS_IDLE;
attribute fsm_encoding of NTS_state : signal is "compact";

signal TM_out: std_logic_vector(2 downto 0);
signal TM_in: std_logic_vector(2 downto 0);
begin

-- BETA16: CPLD and 060 share the same clk (PCLK)
   BCLK_int    <= CPLDCLK; ---<--- BCLK  25 MHz
   CPLDCLK_int <= CLK060;  ---<--- PCLK 100 MHz
--BETA18: CPLD has its own clk (BCLK)
--   BCLK_int    <= CPUCLK_RECVD; ---<--- CPUCLK  25 MHz
--   CPLDCLK_int <= CPLDCLK;      ---<--- BCLK   100 Mhz

   DBG1 <= nBOSS_fake_jumper;--nIPL(0) and nIPL(1) and nIPL(2);--nCPURST; -- Debug: Estado del latch de reset del teclado
   DBG2 <= INT6_ARM;--n040RSTI_int;--nTA;-- and nTA;--CPLDCLK_int;--nDMA_STERM and nDMA_DSACK(0) and nDMA_DSACK(1);--'0' when SIZ40="11" else '1';--BCLK_int;--
--   DBG3 <= nTA;--nTS;--nTS_FPGA;--

   DBG3 <= nBOSS; -- this is not a debug signal anymore
   -- JUMPER J1 OFF -> J1 = '1' -> nBOSS = '0' (J1 has a pullup resistor)
   -- JUMPER J1 ON  -> J1 = '0' -> nBOSS = '1' --nBG_ARM

--   nBOSS <= (not J1) and nBG_ARM;
   nBOSS <= nBOSS_fake_jumper and nBG_ARM;

   process(CPLDCLK_int)
   begin
      if(RISING_EDGE(CPLDCLK_int)) then
         PS_MIO_8_last <= PS_MIO_8;
         if(RESET_CPLD='0') then
            if(PS_MIO_8='1' and PS_MIO_8_last='0') then
               nBOSS_fake_jumper <= PS_MIO_15;
            end if;
         end if;
      end if;
   end process;

   nS2W <= not(not(SLV3)          and SLV1 and SLV0);

   nBGACK040 <= '0'; -- nBGACK040_int and nBG_ARM and nBGACK; -- this signal is 244's buffer trisate

   nBG040   <= nBG040_int;
   n040RSTI <= n040RSTI_int;
   nTA <= --'Z' when nBGACK='0' else--nTA_int when BCLK_ratio='1' else
          '0' when nTA_d(1 downto 0)="10" else
          '0' when nTA_d(1 downto 0)="00" else
          '1' when nTA_d(1 downto 0)="01" else
          'Z';

   nTEA <= nTEA_int when nBGACK040_int='0' or nBG_ARM='0' else '1';

   RAM_select <= --'1' when( p040A(31 downto 28) = "0100"  ) else -- 0x40000000 - 0x4FFFFFFF 256 Mbyte 
                 -- Z3 RAM is supposed to be only at DaughterBoard ( and uses FCS, DSx, ... signals) so
                 -- it can't be used by DMA
                 -- CPU RAM can be accessed by DMA (AS, DS, etc signals)
                 '1' when( p040A(31 downto 27) = "00001" ) else -- 0x08000000 - 0x0FFFFFFF 128 Mbyte
                 '0';
   RTG_select <= '1' when( p040A(31 downto 27) = "00010" ) else -- 0x10000000 - 0x17FFFFFF 128 Mbyte
                 '0';

   ROM_select <= '1' when( p040A(31 downto 24) = "00001111" ) else -- 0x10000000 - 0x17FFFFFF 128 Mbyte
                 '0';

--   DMA_select <= '1' when (nBGACK='0' and nBG_ARM = '1' and RAM_select='1' and nAS040='0') else '0';
   DMA_select <= '1' when (   (nBGACK='0' and nBOSS='0' and RAM_select='1')
                           or (               nBOSS='1' and (RAM_select='1' or
                                                             RTG_select='1' or
                                                             ROM_select='1') )
                          ) else '0';

   DMA_BUSY <= '0' when DMA_state=DMA_IDLE else '1';

   nCEBA_int <= not R_W040;
   nCEAB_int <=     R_W040;

   nCEBA  <= nCEAB_int  when DMA_BUSY='1' else nCEBA_int;
   nCEAB  <= nCEBA_int  when DMA_BUSY='1' else nCEAB_int;

   nTS    <= nTS_dma    when DMA_BUSY='1' else 'Z';
   SIZ40  <= SIZ        when DMA_BUSY='1' else "ZZ";
   p040A0 <= A(0)       when DMA_BUSY='1' else 'Z';
   p040A1 <= A(1)       when DMA_BUSY='1' else 'Z';
   p040A2 <= A(2)       when DMA_BUSY='1' else 'Z';
   p040A3 <= A(3)       when DMA_BUSY='1' else 'Z';
   nSTERM <= nDMA_STERM when DMA_BUSY='1' else 'Z';
   nDSACK <= nDMA_DSACK when DMA_BUSY='1' else "ZZ";
   nBB040 <= '0'        when DMA_BUSY='1' else 'Z';

   A      <= A_int      when nBGACK040_int='0' or nBG_ARM='0' else "ZZZZ";
   SIZ    <= SIZ_int    when nBGACK040_int='0' or nBG_ARM='0' else "ZZ";
   FC     <= FC_int     when nBGACK040_int='0' or nBG_ARM='0' else "ZZZ";
   nDS040 <= nDS040_int when nBGACK040_int='0' or nBG_ARM='0' else 'Z';
   nAS040 <= nAS040_int when nBGACK040_int='0' or nBG_ARM='0' else 'Z';

   nTBI      <= '0' when DMA_BUSY='1' else not(nTBI_FPGA);
   nTCI      <= '0' when DMA_BUSY='1' else nTCI_int;
   nAVEC040  <= --'0' when DMA_BUSY='1' else
                nAVEC040_int;

   LEBUS     <= "10010101" when LEBUS_DMA_int='1' else
                LEBUS_int;
   LEBUS_DMA <= LEBUS_DMA_int;
   OEBUS     <= "01101010" when DMA_BUSY='1' else OEBUS_int; -- DMA -> 32 bit transparent bus
   nDMACOE   <= '1'        when DMA_BUSY='1' else nDMACOE_int;

   nSNOOP    <= '0' when DMA_BUSY='1' or nBG_ARM='0' else 'Z';
   n040EMUL  <= nEMUL or not(n040RSTI_int);

   nINT6 <= '0' when INT6_ARM ='1' else 'Z';

   PS_MIO_0  <= not nIPL(0);
   PS_MIO_9  <= not nIPL(1);
   PS_MIO_12 <= not nIPL(2);
   PS_MIO_15 <= nBG_ARM when RESET_CPLD='1' else 'Z';

   nBR_ARM    <= PS_MIO_8;
   RESET_CPLD <= PS_MIO_13;

-- TT = "00" -> normal
-- TT = "01" -> MOVE16 Access
-- TT = "10" -> Alternate Logical Function Code Access, Debug Access
-- TT = "11" -> Acknowledge Access, Low-Power Stop Broadcast
-- TM = "000" -> Data Cache Push Access
-- TM = "001" -> User Data Access
-- TM = "101" -> Supervisor Data Access
--   TM(2 downto 0) <= "000" when nBG_ARM = '0' else "ZZZ";
--   TT(1 downto 0) <=  "00" when nBG_ARM = '0' else  "ZZ";
   TM0 <= TM_out(0);
   TM1 <= TM_out(1);
   TM2 <= TM_out(2);
   TM_in <= TM2 & TM1 & TM0;

   TM_out <= "000" when DMA_BUSY = '1' else--"001" when DMA_select='1' else
             "000" when nBG_ARM  = '0' else
             "ZZZ";
   TT(1 downto 0) <= "00" when DMA_BUSY = '1' else
                     "00" when nBG_ARM  = '0' else
                     "ZZ";
   process(CPLDCLK_int)
   begin
      if(RISING_EDGE(CPLDCLK_int)) then
         BCLK_m <= BCLK_int;
         BCLK_d <= BCLK_m;
      end if;
   end process;
-- DMA State Machine
   process(CPLDCLK_int,n040RSTI_int)
   begin
      -- asynchronous reset
      if(n040RSTI_int='0') then
         DMA_state<=DMA_IDLE;
         nTS_dma <= '1';
         LEBUS_DMA_int <= '0';
         nDMA_STERM <= '1';
         nDMA_DSACK <= "11";
      else
         if(RISING_EDGE(CPLDCLK_int)) then
            -- default values
            nTS_dma <= '1';
            nDMA_STERM <= '1';
            nDMA_DSACK <= "11";

            case (DMA_state) is
               when DMA_IDLE =>
                  LEBUS_DMA_int <= '0';
                  if (nAS040='0' and DMA_select='1') then
                     DMA_state <= DMA_START_TS;
                  end if;
--                  if(SIZ="00") then           -- long word
--                     finish_DMA_cycle<="110";
--                  elsif(SIZ="01") then        -- byte
--                     finish_DMA_cycle<="101"; 
--                  elsif(SIZ="10") then        -- word
--                     finish_DMA_cycle<="011";
--                  else                        -- line or 3 bytes???
                     finish_DMA_cycle<="001";
--                  end if;
               when DMA_START_TS =>
                  LEBUS_DMA_int <= '0';
                  nTS_dma <= '0';
                  if(nTA='0') then
                     DMA_state <= DMA_WAIT_TA;
--                     LEBUS_DMA_int <= '1';
                  end if;
               when DMA_WAIT_TA =>
                  LEBUS_DMA_int <= '0';
                  if(nTA/='0') then
                     DMA_state <= DMA_LATCH_BUS_AND_TERM;
                     LEBUS_DMA_int <= '1';
                  end if;
--                  nDMA_STERM <= finish_DMA_cycle(0);
--                  nDMA_DSACK <= finish_DMA_cycle(2 downto 1);
               when DMA_LATCH_BUS_AND_TERM =>
                  LEBUS_DMA_int <= '1';
                  nDMA_STERM <= finish_DMA_cycle(0);
                  nDMA_DSACK <= finish_DMA_cycle(2 downto 1);
                  if(nAS040/='0' or nDS040/='0') then
                     DMA_state <= DMA_IDLE;
                     nDMA_STERM <= '1';
                     nDMA_DSACK <= "11";
                     LEBUS_DMA_int <= '0';
                  end if;
               when others =>
                  DMA_state <= DMA_IDLE;
            end case;
         end if;
      end if;
   end process;
--end DMA

   nBG        <= nBG_int;
   nSBGACK030 <= nBGACK_D or nBGACK;
   nSBR030    <= nBR_D    or nBR;
   nLSTERM    <= nPLSTERM;-- or nAS040_int;

   process(BCLK_int)
   begin
      if(RISING_EDGE(BCLK_int)) then
         nBR_D    <= nBR;
         nBGACK_D <= nBGACK;
         nRBERR   <= nBERR;
--         nRHALT   <= nHLT; -- not used
         nRCIIN   <= nCIIN;
      end if;
   end process;
   process(BCLK_int)
   begin
      if(RISING_EDGE(BCLK_int)) then
         nPLSTERM <= nSTERM;
      end if;
   end process;

-- TS stretcher
   process(CPLDCLK_int,n040RSTI_int)
   begin
      if(n040RSTI_int='0') then
         nTS_state <= NTS_IDLE;
         nTS_030 <= '1';
      else
         if(RISING_EDGE(CPLDCLK_int)) then
            nTS_030 <= '1';
            case (nTS_state) is
               when NTS_IDLE =>
                  if (nTS_FPGA='0' and nBOSS='0') then
                     nTS_030 <= '0'; -- forward the nTS signal to 030 (makes 060 read at full speed)
                     nTS_state <= NTS_START;
                  end if;
               when NTS_START =>
                  nTS_030 <= '0';
                  if(BCLK_d = '0' and BCLK_m = '1') then
                     nTS_state <= NTS_STRETCH;
                     nTS_030 <= '0';
                  end if;
               when NTS_STRETCH =>
                  nTS_030 <= '0';
                  if(BCLK_d = '0' and BCLK_m = '1') then
                     nTS_state <= NTS_END;
                  end if;
               when NTS_END =>
                  nTS_030 <= '1';
                  if (
--                    BCLK_d = '0' and BCLK_m = '1' and
                    nTS_FPGA='1'
                       ) then
--                     nTS_030 <= '1';
                     nTS_state <= NTS_IDLE;
                  end if;
               when others =>
                  nTS_state <= NTS_IDLE;
            end case;
         end if;
      end if;
   end process;

   process(CPLDCLK_int,n040RSTI_int)
   begin
      if(n040RSTI_int='0') then
         nTA_d(1 downto 0) <= "11";
--         nTEA_d(3 downto 0) <= "1111";
      else
         if(RISING_EDGE(CPLDCLK_int)) then
            nTA_d(1 downto 0) <= nTA_d(0) & nTA_int;
--            nTEA_d(3 downto 0) <= nTEA_d(2 downto 0) & nTEA_int;
         end if;
      end if;
   end process;

   nHLT    <= '0' when n040RSTI_int='0' else 'Z';
   RESET_OUT <= n040RSTI_int;
   nCPURST <= '0' when (n040RSTI_int='0' and nBOSS = '1') else 'Z';

   process(BCLK_int)
   begin
      if(RISING_EDGE(BCLK_int)) then
         nCPURST_d <= nCPURST;
         if (nCPURST='0' and nCPURST_d='1' and nBOSS = '1') or
            (nCPURST='0'                   and nBOSS = '0') or
            V_DETECTOR='0' or
            n040RSTO='0' or
            RESET_CPLD='0' then
            n040RSTI_int <= '0';
         else
            n040RSTI_int <= '1';
         end if;
      end if;
   end process;

   process(CPLDCLK_int,n040RSTI_int)
   begin
      if(n040RSTI_int='0') then
         n060IPL <= "111";
--         n060IPL <= "011"; -- extra data write hold
      else
         if(RISING_EDGE(CPLDCLK_int)) then
            n060IPL <= nIPL;
         end if;
      end if;
   end process;

--OEBUS
   oebus_unit: OEBUS_component PORT MAP (
         BCLK => BCLK_int,
         MAS0 => MAS0,
         MAS1 => MAS1,
         MAS2 => MAS2,
         MAS3 => MAS3,
         nS2W => nS2W,
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
         p040A0 => p040A0,
         p040A1 => p040A1,
         SIZ40 => SIZ40,
         nETERM => nETERM,
         nRDSACK1 => nRDSACK1,
         nRDSACK0 => nRDSACK0,
         nRBERR => nRBERR,
--         nRHALT => nRHALT,
--         nRAVEC => nRAVEC,
--         nCYCPEND => nCYCPEND,
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
--         R_W040 => R_W040,
--         nTBI => nTBI,
--         nTEA => nTEA_int,
--         nTA => nTA_int,
         nTS => nTS_030
         );
--END BUSCON

--BUSTERM
   busterm_unit: BUSTERM PORT MAP (
--         BCLK => BCLK_int,
--         R_W040 => R_W040,
         SLV0 => SLV0,
         SLV1 => SLV1,
         SLV2 => SLV2,
--         SLV3 => SLV3,
--         nCYCPEND => nCYCPEND,
--         SIZ40 => SIZ40,
--         nPLSTERM => nPLSTERM,
--         nRTERM => nRTERM,
--         nTS => nTS_030,
         nDS040 => nDS040_int,
         nAS040 => nAS040_int,
         nTEA => nTEA_int,
--         nTBI => nTBI,
--         n040RSTI => n040RSTI_int,
         nTA => nTA_int
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
         nBG_ARM => nBG_ARM,
         nBOSS => nBOSS,
         nTEND => nTEND
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
--         nBGACK040 => nBGACK040_int,
         nTS => nTS_030,
         n040RSTI => n040RSTI_int,
         nTEND => nTEND,
--         nCYCPEND => nCYCPEND,
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
         TM => TM_in,
         MAS0 => MAS0,
         MAS1 => MAS1,
         MAS2 => MAS2,
         MAS3 => MAS3,
         FC => FC_int,
         SIZ => SIZ_int,
         A => A_int--,
--         nIACK => nIACK
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
         LEBUS => LEBUS_int,
         DMA_BUSY => DMA_BUSY
         );
--END LEBUS

--TERM
   term_unit: TERM PORT MAP (
         BCLK => BCLK_int,
         nBOSS => nBOSS,
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
--         nIACK => nIACK,
         nETERM => nETERM,
         nRTERM => nRTERM
         );
--END TERM

end Behavioral;