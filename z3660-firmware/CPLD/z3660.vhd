----------------------------------------------------------------------------------
--
-- Z3660 wo_FPGA
--
-- version info:
-- 0.1: Maximum frequency clock is 50MHz
--      Mapper is active if you select JP1 to ENABLE, but it will need something
--        like "BlizKick * CPUCARD" in S:Startup-Sequence
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
           nBB040 : in  STD_LOGIC;
           R_W040 : in  STD_LOGIC;
           TP7 : out  STD_LOGIC;
           TP8 : out  STD_LOGIC;
           TP9 : out  STD_LOGIC;
           TP10 : out  STD_LOGIC;
           TP11 : out  STD_LOGIC;
           TP12 : out  STD_LOGIC;
           TP13 : out  STD_LOGIC;
           TP15 : out  STD_LOGIC;
           TP16 : out  STD_LOGIC;
           TP17 : out STD_LOGIC;
           PCLK : in  STD_LOGIC;
           nTS : in  STD_LOGIC;
           nHLT : in  STD_LOGIC;
           nEMUL : in  STD_LOGIC;
           nLOCK : in  STD_LOGIC;
           nLOCKE : in  STD_LOGIC;
           nDSACK : in  STD_LOGIC_VECTOR (1 downto 0);
           nBR : in  STD_LOGIC;
           nCPURST : in  STD_LOGIC;
           nCIIN : in  STD_LOGIC;
           nBR040 : in  STD_LOGIC;
           nTA : out  STD_LOGIC;
           nTEA : out  STD_LOGIC;
           SIZ40 : in  STD_LOGIC_VECTOR (1 downto 0);
           nSTERM : in  STD_LOGIC;
           V_DETECTOR : in  STD_LOGIC;
           n040RSTI : out  STD_LOGIC;
           TP18 : out  STD_LOGIC;
           TP19 : out  STD_LOGIC;
           TM : in  STD_LOGIC_VECTOR (2 downto 0);
           TT : in  STD_LOGIC_VECTOR (1 downto 0);
           MAPROM : in  STD_LOGIC;
           n040CIOUT : in  STD_LOGIC;
           TP20 : out  STD_LOGIC;
           TP21 : out  STD_LOGIC;
           nIPL : in  STD_LOGIC_VECTOR (2 downto 0);
           nDS040 : out  STD_LOGIC;
           p040A : in  STD_LOGIC_VECTOR (31 downto 19);
           p040A3 : in  STD_LOGIC;
           p040A2 : in  STD_LOGIC;
           p040A1 : in  STD_LOGIC;
           p040A0 : in  STD_LOGIC;
           LEBUS : out  STD_LOGIC_VECTOR (7 downto 0);
           OEBUS : out  STD_LOGIC_VECTOR (7 downto 0);
           TP14 : out  STD_LOGIC;
           nCLKEN : out  STD_LOGIC;
           n060IPL : out  STD_LOGIC_VECTOR (2 downto 0);
           MA : inout  STD_LOGIC_VECTOR (26 downto 24);
           NU_3 : in  STD_LOGIC;
           nAS040 : out  STD_LOGIC;
           nBERR : in  STD_LOGIC;
           n040EMUL : out  STD_LOGIC;
           nTCI : out  STD_LOGIC;
           nTBI : out  STD_LOGIC;
           nAVEC040 : out  STD_LOGIC;
           nBG040 : out  STD_LOGIC;
           CPUCLK : out  STD_LOGIC;
           nR_W040 : out  STD_LOGIC;
           NU_7 : in  STD_LOGIC; -- no se puede usar!!!!
           NU_8 : in  STD_LOGIC;
           CLK90 : out  STD_LOGIC;
           FC : out  STD_LOGIC_VECTOR (2 downto 0);
           A : out  STD_LOGIC_VECTOR (3 downto 0);
           nBGACK : in  STD_LOGIC;
           BCLK : out  STD_LOGIC;
           SIZ : out  STD_LOGIC_VECTOR (1 downto 0);
           NU_2 : in  STD_LOGIC; -- es TBI que viene que la FPGA
           NU_1 : in  STD_LOGIC;
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

   COMPONENT MAPPER
   PORT(
         BCLK : in  STD_LOGIC;
         p040A : in  STD_LOGIC_VECTOR (31 downto 19);
         MAPROM : in  STD_LOGIC;
         R_W040 : in  STD_LOGIC;
         nPWRST : in  STD_LOGIC;
         MA : out  STD_LOGIC_VECTOR (26 downto 24);
         nBGACK040 : in  STD_LOGIC
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
         SLV0_out : out  STD_LOGIC;
         SLV1_out : out  STD_LOGIC;
         SLV2_out : out  STD_LOGIC;
         SLV3_out : out  STD_LOGIC;
         MAS0_out : out  STD_LOGIC;
         MAS1_out : out  STD_LOGIC;
         MAS2_out : out  STD_LOGIC;
         MAS3_out : out  STD_LOGIC;
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
         nBGACK040 : out  STD_LOGIC
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
         SLV0 : in  STD_LOGIC;
         SLV1 : in  STD_LOGIC;
         SLV2 : in  STD_LOGIC;
         SLV3 : in  STD_LOGIC;
         nRTERM : in  STD_LOGIC;
         nLSTERM : in  STD_LOGIC;
         R_W040 : in STD_LOGIC;
         LEBUS : out  STD_LOGIC_VECTOR (7 downto 0)
         );
   END COMPONENT;

   COMPONENT TERM
   PORT (
         BCLK : in  STD_LOGIC;
         nPLSTERM : in  STD_LOGIC;
         nRAVEC_out : out  STD_LOGIC;
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

signal nIACK: STD_LOGIC :='1';
signal nTEA_int,nTA_int,nTA_int2: STD_LOGIC :='1';
signal nBGACK040_int,nBG_int:  STD_LOGIC :='1';
signal nDS040_int,nAS040_int: STD_LOGIC :='0';
signal nCLKEN_int: STD_LOGIC :='0';
signal CPUCLK_int,CLK90_int: STD_LOGIC :='0';
signal nPWRST,nLSTERM: STD_LOGIC :='1';
signal nPLSTERM: STD_LOGIC :='1';
signal nRAVEC,nRTERM,nETERM,nRBERR,nRHALT,nRDSACK0,nRDSACK1: STD_LOGIC :='1';
signal MAS0,MAS1,MAS2,MAS3: STD_LOGIC :='0';
signal SLV0,SLV1,SLV2: STD_LOGIC :='0';
signal SLV3: STD_LOGIC :='0';
signal nCYCPEND: STD_LOGIC :='1';
signal nTCI_int: STD_LOGIC :='1';
signal nAVEC040_int: STD_LOGIC :='1';
signal nSBR030,nSBGACK030,nBGACK_D,nBR_D: STD_LOGIC :='1';
signal nRCIIN: STD_LOGIC :='1';
signal BCLK_int: STD_LOGIC :='0';
signal nTS_030,nTS0,nTS1: STD_LOGIC :='1';
signal FPGA_PRESENCE: STD_LOGIC :='1';
signal FPGA_RAM_SELECTED: STD_LOGIC :='0';
signal n040RSTI_int: STD_LOGIC :='1';

begin

   TP7 <= '0';
   TP8 <= '0';
   TP9 <= '0';
   TP10 <= '0';
   TP11 <= '0';
   TP12 <= '0';
   TP13 <= '0';
   TP14 <= '0';
   TP15 <= '1'; -- disable CPLD reset out
   TP16 <= '0';
   TP17 <= '0';
   TP18 <= '0';
   TP19 <= '0';
   TP20 <= '0';
   TP21 <= '0';

   nDS040 <= nDS040_int when nBGACK040_int='0' else '1';
   nAS040 <= nAS040_int when nBGACK040_int='0' else '1';
   nTA_int2 <= nTA_int when nBGACK040_int='0' else '1';
   nTA <= '0' when nTA_int2='0' else 'Z';
   nTEA <= nTEA_int when nBGACK040_int='0' else '1';
   nTCI <= nTCI_int;
   nAVEC040 <= nAVEC040_int;
   
   nR_W040 <= not R_W040;
   nBGACK040 <= nBGACK040_int; -- this is 244 buffer's triestate signal
   nBG <= nBG_int;
   nSBGACK030 <= nBGACK_D or nBGACK;
   nSBR030 <= nBR_D or nBR;

   n040RSTI <= n040RSTI_int;
   n040EMUL <= nEMUL or not(n040RSTI_int);

   nLSTERM <= nAS040_int or nPLSTERM;
   nPLSTERM <= nSTERM; -- it is really the same signal

   FPGA_RAM_SELECTED <= '1' when ((p040A(31)='0' and p040A(30)='0' and p040A(29)='0' and p040A(28)='0' and p040A(27)='1'))
--                              or ((p040A(31)='0' and p040A(30)='0' and p040A(29)='0' and p040A(28)='1' and p040A(27)='0'))
                                 else '0';
   FPGA_PRESENCE <= NU_7;
--   FPGA_PRESENCE<='1';

   process(FPGA_PRESENCE,FPGA_RAM_SELECTED,NU_2,nTS)--,nTS1)
   begin
      if(FPGA_PRESENCE='1') then
         if(FPGA_RAM_SELECTED='1') then
            nTS_030 <= '1';
         else
            nTS_030 <= nTS;--nTS1;
         end if;
         nTBI <= not(NU_2); -- NU_2 viene de la FPGA y es realmente TBI
      else
         nTBI <= '0';
         nTS_030 <= nTS;--nTS1;
      end if;
   end process;

   process(PCLK)
   begin
      if(PCLK'event and PCLK='1') then
         nTS0 <= nTS;
         nTS1 <= nTS0;
      end if;
   end process;

   process(PCLK)
   begin
      if(PCLK'event and PCLK='1') then
         BCLK_int<=not(BCLK_int);
         CPUCLK_int<=BCLK_int;
      end if;
      if(PCLK'event and PCLK='0') then
         CLK90_int <= not(BCLK_int);
         nCLKEN_int <= BCLK_int;
      end if;
   end process;

   BCLK<=BCLK_int;
   nCLKEN <= BCLK_int;--nCLKEN_int; delays on -10 CPLD seems too tight
   CPUCLK <= CPUCLK_int;
   CLK90 <= CLK90_int;
   

   process(BCLK_int)
   begin
      if(BCLK_int'event and BCLK_int='1') then
         nRBERR <= nBERR;
         nRHALT <= nHLT;
         if(nCPURST='0' or nPWRST='0') then
            n040RSTI_int <= '0';
         else
            n040RSTI_int <= '1';
         end if;
--         n040RSTI_int <= nCPURST;
         nBR_D <= nBR;
         nBGACK_D <= nBGACK;
         nRCIIN <= nCIIN;
      end if;
   end process;

   process(BCLK_int,n040RSTI_int)
   begin
      if(n040RSTI_int='0') then
         n060IPL <= "111";
--         n060IPL <= "011"; -- extra data write hold
      elsif(BCLK_int'event and  BCLK_int='1') then
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
         nDMACOE => nDMACOE,
         OEBUS => OEBUS
         );
--END OEBUS

--MAPPER
    mapper_unit: MAPPER PORT MAP (
         BCLK => BCLK_int,
         p040A => p040A,
         MAPROM => MAPROM,
         R_W040 => R_W040,
         nPWRST => nPWRST,
         MA => MA,
         nBGACK040 => nBGACK040_int
         );
--END MAPPER

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
         SLV0_out => SLV0,
         SLV1_out => SLV1,
         SLV2_out => SLV2,
         SLV3_out => SLV3,
         MAS0_out => MAS0,
         MAS1_out => MAS1,
         MAS2_out => MAS2,
         MAS3_out => MAS3,
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
         nBG040 => nBG040,
         nBG => nBG_int,
         nBGACK040 => nBGACK040_int
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
         SLV0 => SLV0,
         SLV1 => SLV1,
         SLV2 => SLV2,
         SLV3 => SLV3,
         nRTERM => nRTERM,
         nLSTERM => nLSTERM,
         R_W040 => R_W040,
         LEBUS => LEBUS
         );
--END LEBUS

--TERM
   term_unit: TERM PORT MAP (
         BCLK => BCLK_int,
         nPLSTERM => nPLSTERM,
         nRAVEC_out => nRAVEC,
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