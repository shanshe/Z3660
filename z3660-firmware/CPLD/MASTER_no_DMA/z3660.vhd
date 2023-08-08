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
           nBB040 : in  STD_LOGIC;
           R_W040 : in  STD_LOGIC;
           PS_MIO_8 : in  STD_LOGIC;
           PS_MIO_0 : out  STD_LOGIC;
           PS_MIO_9 : out  STD_LOGIC;
           PS_MIO_12 : out  STD_LOGIC;
           PS_MIO_13 : in  STD_LOGIC;
           PS_MIO_15 : out  STD_LOGIC;
           INT6 : in  STD_LOGIC;
           nBTT : in  STD_LOGIC;
           nBGR040 : in  STD_LOGIC;
           nINT6 : out  STD_LOGIC;
           R_W040_out : out  STD_LOGIC;
           TP3 : in  STD_LOGIC;
           n040RSTO : in  STD_LOGIC;
           TP5 : in  STD_LOGIC;
           TP6 : out  STD_LOGIC;
           TP7 : in  STD_LOGIC;
           RESET_OUT : out  STD_LOGIC;
           PCLK : in  STD_LOGIC;
           nTS : in  STD_LOGIC;                             ---------------------------
           nHLT : in  STD_LOGIC;
           nEMUL : in  STD_LOGIC;
           nLOCK : in  STD_LOGIC;
           nLOCKE : in  STD_LOGIC;
           nDSACK : in  STD_LOGIC_VECTOR (1 downto 0);      ---------------------------
           nBR : in  STD_LOGIC;
           nCPURST : in  STD_LOGIC;
           nCIIN : in  STD_LOGIC;
           nBR040 : in  STD_LOGIC;
           nTA : out  STD_LOGIC;                            ---------------------------
           nTEA : out  STD_LOGIC;
           SIZ40 : in  STD_LOGIC_VECTOR (1 downto 0);
           nSTERM : in  STD_LOGIC;
           V_DETECTOR : in  STD_LOGIC;
           n040RSTI : buffer  STD_LOGIC;
           TM : inout  STD_LOGIC_VECTOR (2 downto 0);
           TT : inout  STD_LOGIC_VECTOR (1 downto 0);
           J1 : in  STD_LOGIC;
           n040CIOUT : in  STD_LOGIC;
           nIPL : in  STD_LOGIC_VECTOR (2 downto 0);
           nDS040 : inout  STD_LOGIC;
           p040A : in  STD_LOGIC_VECTOR (31 downto 19);
           p040A3 : in  STD_LOGIC;
           p040A2 : in  STD_LOGIC;
           p040A1 : in  STD_LOGIC;
           p040A0 : in  STD_LOGIC;
           LEBUS : out  STD_LOGIC_VECTOR (7 downto 0);
           OEBUS : out  STD_LOGIC_VECTOR (7 downto 0);
           
           nCLKEN : in  STD_LOGIC; -- be careful, output from FPGA
           n060IPL : out  STD_LOGIC_VECTOR (2 downto 0);
           MA : in  STD_LOGIC_VECTOR (26 downto 24); -- not really used anymore
--           NU_3 : out  STD_LOGIC;
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
         SLV0 : buffer  STD_LOGIC;
         SLV1 : buffer  STD_LOGIC;
         SLV2 : buffer  STD_LOGIC;
         SLV3 : buffer  STD_LOGIC;
         MAS0 : buffer  STD_LOGIC;
         MAS1 : buffer  STD_LOGIC;
         MAS2 : buffer  STD_LOGIC;
         MAS3 : buffer  STD_LOGIC;
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
         nRAVEC : buffer  STD_LOGIC;
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
signal nDS040_int,nAS040_int: STD_LOGIC :='0';
--signal CPUCLK_int,CLK90_int: STD_LOGIC :='0';
signal nPWRST,nLSTERM: STD_LOGIC;
signal nPLSTERM: STD_LOGIC;
signal nRAVEC,nRTERM,nETERM,nRBERR,nRHALT,nRDSACK0,nRDSACK1: STD_LOGIC;
signal MAS0,MAS1,MAS2,MAS3: STD_LOGIC :='0';
signal SLV0,SLV1,SLV2: STD_LOGIC :='0';
signal SLV3: STD_LOGIC :='0';
signal nCYCPEND: STD_LOGIC :='1';
signal nTCI_int: STD_LOGIC :='1';
signal nAVEC040_int: STD_LOGIC :='1';
signal nSBR030,nSBGACK030,nBGACK_D,nBR_D: STD_LOGIC;
signal nRCIIN: STD_LOGIC;
signal nDMACOE_int: STD_LOGIC;
signal OEBUS_int: STD_LOGIC_VECTOR (7 downto 0);
signal LEBUS_int: STD_LOGIC_VECTOR (7 downto 0);
--signal n040IPL: STD_LOGIC_VECTOR (2 downto 0);
signal nTS_030,nTS_030_s,nTS_030_s1,nTA_int2,nTA_int3: STD_LOGIC:='1';
signal nTS_state: STD_LOGIC_VECTOR(3 downto 0):="0000";
signal BCLK_int: STD_LOGIC :='0';

signal nTA_d: STD_LOGIC_VECTOR(3 downto 0):="0000";
signal nTEA_d: STD_LOGIC_VECTOR(3 downto 0):="0000";

signal reset_extended: STD_LOGIC:='1';
signal reset_counter: STD_LOGIC_VECTOR(8 downto 0):="000000000";
signal nBG_ARM: STD_LOGIC:='1';

signal nBR_ARM : STD_LOGIC:='1';
signal RESET_CPLD : STD_LOGIC:='1';

--signal OEBUS_OUT: STD_LOGIC;
--signal nTS_RAM: STD_LOGIC :='1';
--signal nTEND:  STD_LOGIC;

-- Master State Values
CONSTANT  I: std_logic_vector(3 downto 0):= "0000";
CONSTANT  A1: std_logic_vector(3 downto 0):= "0001";
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
--CONSTANT   S3: std_logic_vector(2 downto 0):= "000";
--CONSTANT   S2: std_logic_vector(2 downto 0):= "001";
--CONSTANT   S0: std_logic_vector(2 downto 0):= "010";
--CONSTANT  S3W: std_logic_vector(2 downto 0):= "011";
--CONSTANT   S1: std_logic_vector(2 downto 0):= "100";
--CONSTANT   SB: std_logic_vector(2 downto 0):= "101";
--CONSTANT S3W2: std_logic_vector(2 downto 0):= "110";
--CONSTANT  SR0: std_logic_vector(2 downto 0):= "111";

--signal MAS_state: STD_LOGIC_VECTOR(3 downto 0);
--signal SLV_state: STD_LOGIC_VECTOR(2 downto 0);
--signal contador: STD_LOGIC_VECTOR(1 downto 0);
signal FPGA_RAM_SELECTED: STD_LOGIC;
signal nTA_up,nTEA_up: STD_LOGIC;

signal nTA_out: std_logic;

begin
--   MAS_state <= MAS3 & MAS2 & MAS1 & MAS0;
--   SLV_state <= SLV2 & SLV1 & SLV0;

--   TP8  <= '1' when (nRDSACK0='0' and nRDSACK1='0') else '0'; -- longport
--   TP9  <= '1' when (nRDSACK0='1' and nRDSACK1='0') else '0'; -- wordport
--   TP10 <= '1' when (nRDSACK0='0' and nRDSACK1='1') else '0'; -- byteport
--   TP11 <= '1' when SLV_state=S1 else '0';
--   TP12 <= '1' when SLV_state=SB else '0';
--   TP13 <= '1' when SLV_state=s3W2 else '0';
--   TP14 <= '1' when SLV_state=SR0 else '0';
--   TP15 <= SLV3;
--   TP16 <= '1' when MAS_state=J else '0';
--   TP18 <= '1' when MAS_state=L else '0';
--   TP19 <= '1' when MAS_state=M else '0';
--   TP20 <= '1' when MAS_state=N else '0';
--   TP21 <= '1' when MAS_state=Z else '0';
   
   
   nDMACOE <= nDMACOE_int;
   OEBUS <= OEBUS_int;
   LEBUS <= LEBUS_int;

TP6 <= '0';

   nDS040 <= nDS040_int when nBGACK040_int='0' else 'Z';
   nAS040 <= nAS040_int when nBGACK040_int='0' else 'Z';

   --nTA <= nTA_int3 when nBG_ARM='0' else nTA_int2;
   nTA <= '0' when nTA_out= '0' else 'Z';
   process(PCLK)
   begin
      if(PCLK'event and PCLK='0') then
         if(n040RSTI='0') then
            nTA_out <= '1';
         else
            if(nBG_ARM='1') then
               if nTA_int3='0' then
                  nTA_out <= '0';
               else
                  nTA_out <= '1';
               end if;
            else
               if nTA_d(1 downto 0)="10" then
                  nTA_out <= '0';
               else
                  nTA_out <= '1';
               end if;
            end if;
         end if;
      end if;
   end process;
   nTEA <= nTEA_int when nBGACK040_int='0' else '1';
--   nTEA <= '0' when nTEA_d(2 downto 0)="110" else
----           '1' when nTEA_d(2 downto 0)="100" else
--          nTEA_up;
   nTEA_up <= --'1' when nBG_ARM='0' else
--   'Z';
   '1';
   
   nTCI <= nTCI_int;
   nAVEC040 <= nAVEC040_int;
-- TT = "00" -> normal
-- TM = "000" -> Data Cache Push Access
-- TM = "001" -> User Data Access
-- TM = "101" -> Supervisor Data Access
   TM(2 downto 0) <= "000" when nBG_ARM = '0' else "ZZZ";
   TT(1 downto 0) <=  "00" when nBG_ARM = '0' else  "ZZ";
      
   nR_W040_out <= not R_W040;
   R_W040_out  <= R_W040;
   nBGACK040   <= nBGACK040_int; -- this signal is 244's buffer trisate
   
--   nR_W040_out<= '0' when (R_W040='1' and nBGACK040_int = '0') or
--                          (R_W040='0' and nBGACK040_int = '1')
--              else '1';
--   R_W040_out <= '1' when (R_W040='1' and nBGACK040_int = '0') or
--                          (R_W040='0' and nBGACK040_int = '1')
--              else '0';
--   nBGACK040  <= nBGACK040_int and nBG_int; -- this signal is 244's buffer trisate

   nBG        <= nBG_int;
   nSBGACK030 <= nBGACK_D or nBGACK;
   nSBR030    <= nBR_D or nBR;

   n040EMUL <= nEMUL or not(n040RSTI);
   
   nINT6 <= '0' when INT6 ='1' else 'Z';
   
   nLSTERM <= nAS040_int or nPLSTERM;
   process(PCLK)
   begin
      if(PCLK'event and PCLK='1') then
         nPLSTERM <= nSTERM; -- it is really the same signal
      end if;
   end process;

   PS_MIO_0  <= not nIPL(0);
   PS_MIO_9  <= not nIPL(1);
   PS_MIO_12 <= not nIPL(2);
   PS_MIO_15  <= nBG_ARM;

   nBR_ARM    <= PS_MIO_8;
   RESET_CPLD <= PS_MIO_13;

   nTS_030_s <= nTS_FPGA;
   nTBI <= not(nTBI_FPGA);

   process(PCLK)
   begin
      if(PCLK'event and PCLK='1') then
         if(n040RSTI='0') then
            nTS_state<="0000";
            nTS_030_s1 <= '1';
         else
         case (nTS_state) is
            when "0000" =>
               nTS_030_s1 <= '1';
               if (nTS_030_s='0') then
                  nTS_state <= "0001";
--               if (BCLK_int='0') then
--                  nTS_state <= "0010";
--               end if;
                  nTS_030_s1 <= '0';
               end if;
            when "0001" =>
               nTS_030_s1 <= '0';
               if (BCLK_int='0') then
                  nTS_state <= "0010";
               end if;
            when "0010" =>
               nTS_030_s1 <= '0';
               if (BCLK_int='1') then
                  nTS_state <= "0011";
--                  nTS_030_s1 <= '1';
            end if;
            when "0011" =>
               nTS_030_s1 <= '0';
               if (BCLK_int='0') then
                  nTS_state <= "0100";
--                  nTS_030_s1 <= '1';
               end if;
            when "0100" =>
               nTS_030_s1 <= '1';
               if (BCLK_int='1') then
                  nTS_state <= "0000";
--                  nTS_030_s1 <= '1';
               end if;
            when others =>
               nTS_state <= "0000";
               nTS_030_s1 <= '1';
         end case;
         end if;
      end if;
   end process;
--   nTS_030 <= '1' when nTS_030_s1='1' and nTS_030_s='1' else '0'; -- stretch nTS pulse
   
--   nTS_030 <= '0' when (nTS_state="0001" or nTS_state="0010") else '1';

   nTS_030 <= nTS_030_s1;

   process(PCLK)
   begin
      if(PCLK'event and PCLK='1') then
         if(n040RSTI='0') then
            nTA_d(3 downto 0) <= "1111";
            nTEA_d(3 downto 0) <= "1111";
         else
            nTA_d(3 downto 0) <= nTA_d(2 downto 0) & nTA_int;
            nTEA_d(3 downto 0) <= nTEA_d(2 downto 0) & nTEA_int;
         end if;
      end if;
   end process;
   nTA_int3<='0' when nTA_d(1 downto 0)="10" else '1';
--   nTA_int3<='0' when nTA_d(1 downto 0)="10" or nTA_d(2 downto 0)="100" else '1';
   
--   nTA_int3<=nTA_int; -- Esto funciona perfectamente con PCLK a 50 MHz

   BCLK_int<=BCLK;
   
   RESET_OUT <= RESET_CPLD; -- reset from FPGA side
   process(BCLK_int)
   begin
      if(BCLK_int'event and BCLK_int='1') then
         if (n040RSTO='0' or reset_counter/="000000000") then
            if (reset_counter(8 downto 0)="100000000") then
               reset_extended <= '1';
               reset_counter(8 downto 0) <= "000000000";
            else
               reset_counter <= reset_counter + 1;
               reset_extended <= '0';
            end if;
         else
            reset_counter(8 downto 0) <= "000000000";
         end if;
      end if;
   end process;
   process(BCLK_int)
   begin
      if(BCLK_int'event and BCLK_int='1') then
         nRBERR <= nBERR;
         nRHALT <= nHLT;
         if(nCPURST='0' or RESET_CPLD='0'
         --or n040RSTO='0' -- this needs a board fix
         or reset_extended='0'
         --or nPWRST='0' -- it needs R2 to be soldered...
         ) then
            n040RSTI<='0';
         else
            n040RSTI<='1';
         end if;
--         n040RSTI <= nCPURST; -- reset output <= reset input
         nBR_D <= nBR;
         nBGACK_D <= nBGACK;
         nRCIIN <= nCIIN;
      end if;
   end process;
--latch 373
--   process(BCLK_int,nBR,nBGACK,nCIIN)
--   begin
--      if(BCLK_int='1') then
--         nBR_D <= nBR;
--         nBGACK_D <= nBGACK;
--         nRCIIN <= nCIIN;
--      end if;
--   end process;


--   process(BCLK_int,n040RSTI)
--   begin
--      if(n040RSTI='0') then
--         n060IPL <= "111";
----         n060IPL <= "011"; -- extra data write hold
--      elsif(BCLK_int'event and  BCLK_int='1') then
--         n060IPL <= nIPL;
--      end if;
--   end process;
   process(PCLK,n040RSTI)
   begin
      if(n040RSTI='0') then
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
         n040RSTI => n040RSTI,
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
         n040RSTI => n040RSTI,
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
         n040RSTI => n040RSTI,
         nBG040 => nBG040,
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
         n040RSTI => n040RSTI,
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
         LEBUS => LEBUS_int
         );
--END LEBUS

--TERM
   term_unit: TERM PORT MAP (
         BCLK => BCLK_int,
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
--         n040RSTI => n040RSTI,
--         nLSTERM => nLSTERM,
         nPWRST => nPWRST
--         n040EMUL => n040EMUL
         );
--END RST

end Behavioral;