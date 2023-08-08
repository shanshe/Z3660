----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    09:52:29 08/20/2021 
-- Design Name: 
-- Module Name:    BUSTERM - Behavioral 
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

entity BUSTERM is
    Port ( BCLK : in  STD_LOGIC;
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
--           nTBI : out  STD_LOGIC;
           n040RSTI : in  STD_LOGIC;
           nTA : out  STD_LOGIC;
           nBGACK040 : in  STD_LOGIC
         );
end BUSTERM;

architecture Behavioral of BUSTERM is
signal IO2,IO5: STD_LOGIC:='1';
signal nDS040_int,nAS040_int: STD_LOGIC:='1';

-- Slave State Values
CONSTANT   S3: std_logic_vector(2 downto 0):= "000";
CONSTANT   S2: std_logic_vector(2 downto 0):= "001";
CONSTANT   S0: std_logic_vector(2 downto 0):= "010";
CONSTANT  S3W: std_logic_vector(2 downto 0):= "011";
CONSTANT   S1: std_logic_vector(2 downto 0):= "100";
CONSTANT   SB: std_logic_vector(2 downto 0):= "101";
CONSTANT S3W2: std_logic_vector(2 downto 0):= "110";
CONSTANT  SR0: std_logic_vector(2 downto 0):= "111";
--signal SLV_state: std_logic_vector(2 downto 0);
begin
--SLV_state <= SLV2 & SLV1 & SLV0;
--
--nTA <= '0' when SLV_state=S2 else '1'; -- moved to SLV state machine  (buscon)
--
--nTEA <= '0' when SLV_state=SB else '1'; -- moved to SLV state machine (buscon)
--
--nAS040_int <= '0' when SLV_state=S1 or SLV_state=S0 else '1';
--
--nDS040_int <= '0' when (SLV_state=S1 or SLV_state=S0) and (R_W040='1') else '1';
--
--nTBI <= '0' when SLV_state=S2 and SIZ40="11" else '1';

-- original equations without IO1
--nTA <= not(
--        ( SLV0 and not(SLV1) and not(SLV2) ) );
--nTEA <= not(
--        ( not(SLV0) and SLV1 and SLV2 ) );
--nAS040_int <= not(
--        ( not(SLV0) and not(SLV1) and SLV2 )
--     or ( not(SLV0) and SLV1 and not(SLV2) ) );
--nDS040_int <= not(
--        ( not(SLV0) and not(SLV1) and SLV2 )
--     or ( not(SLV0) and SLV1 and not(SLV2) ) );

--nTBI <= '0';

-- another way to do the same thing...
--   process(BCLK,n040RSTI)
--   begin
--      if(n040RSTI='0') then
--         nDS040_int <= '1';
--         nAS040_int <= '1';
--      elsif(BCLK'event and  BCLK='1') then
--         if(nBGACK040='0') then
--nAS040_int <= not(
--        ( not(SLV0) and not(SLV1) and SLV2 )
--     or ( not(SLV0) and SLV1 and not(SLV2) ) );
--nDS040_int <= not(
--        ( not(SLV0) and not(SLV1) and SLV2 )
--     or ( not(SLV0) and SLV1 and not(SLV2) ) );
--         else
--            nAS040_int <= '1';
--            nDS040_int <= '1';
--         end if;
--      end if;
--   end process;
     
------ original equations
nDS040 <= nDS040_int;
nAS040 <= nAS040_int;
nTA <= not(
        ( SLV0 and not(SLV1) and not(SLV2) ) );
nTEA <= not(
        ( not(SLV0) and SLV1 and SLV2 ) );
nAS040_int <= not(
        ( not(SLV0) and not(SLV1) and SLV2 )
--     or ( not(SLV0) and SLV1 and not(SLV2) and IO1 ) );
     or ( not(SLV0) and SLV1 and not(SLV2)  ) );
nDS040_int <= not(
        ( not(SLV0) and not(SLV1) and SLV2 )
--     or ( not(SLV0) and SLV1 and not(SLV2) and IO1 ) );
     or ( not(SLV0) and SLV1 and not(SLV2)  ) );
-- ???
--nTBI <= '0';

---- original from 3640
--nDS040 <= nDS040_int;
--nAS040 <= nAS040_int;
--nTA <= not(
--        ( SLV0 and not(SLV1) and not(SLV2) and not(SLV3) and not(IO5) and nAS040_int and nDS040_int and IO2 ) );
--nTBI <= not(
--        ( SLV0 and not(SLV1) and not(SLV2) and not(SLV3) and not(IO5) and nAS040_int and SIZ40(1) and nDS040_int and IO2 and SIZ40(0) ) );
--nTEA <= not(
--        ( not(SLV0) and SLV1 and SLV2 and not(SLV3) ) );
--   process(BCLK,n040RSTI)
--   begin
--      if(n040RSTI='0') then
--         IO2 <= '0';
--         IO5 <= '0';
--         nDS040_int <= '1';
--         nAS040_int <= '1';
--      elsif(BCLK'event and  BCLK='1') then
----register
--IO5 <= not(
--        ( SLV0 and n040RSTI and SLV1 and not(IO5) and nAS040_int and nDS040_int and IO2 )
--     or ( n040RSTI and SLV1 and not(SLV2) and not(IO5) and nAS040_int and nDS040_int and IO2 )
--     or ( IO5 and not(nAS040_int) and IO2 )
--     or ( not(IO5) and not(nAS040_int) and not(nDS040_int) and not(nPLSTERM) and IO2 )
--     or ( not(IO5) and not(nAS040_int) and not(nDS040_int) and IO2 and not(nRTERM) )
--     or ( not(IO5) and not(nAS040_int) and not(nDS040_int) and nPLSTERM and IO2 and nRTERM )
--     or ( n040RSTI and SLV3 and not(IO5) and nAS040_int and nDS040_int and IO2 )
--     or ( n040RSTI and not(SLV1) and SLV2 and not(IO5) and nAS040_int and nDS040_int and IO2 )
--     or ( SLV0 and n040RSTI and SLV2 and not(IO5) and nAS040_int and nDS040_int and IO2 ) );
----register
--nAS040_int <= not(
--        ( not(SLV0) and n040RSTI and not(SLV1) and not(SLV2) and not(SLV3) and not(IO5) and nAS040_int and nDS040_int and IO2 )
--     or ( n040RSTI and IO5 and nAS040_int and nDS040_int and not(IO2) )
--     or ( IO5 and not(nAS040_int) and nPLSTERM and IO2 )
--     or ( not(IO5) and not(nAS040_int) and not(nDS040_int) and nPLSTERM and IO2 and nRTERM ) );
----register
--nDS040_int <= not(
--        ( R_W040 and not(SLV0) and n040RSTI and not(SLV1) and not(SLV2) and not(SLV3) and not(IO5) and nAS040_int and nDS040_int and IO2 )
--     or ( R_W040 and n040RSTI and IO5 and nAS040_int and nDS040_int and not(IO2) )
--     or ( IO5 and not(nAS040_int) and nPLSTERM and IO2 )
--     or ( not(IO5) and not(nAS040_int) and not(nDS040_int) and nPLSTERM and IO2 and nRTERM ) );
----register
--IO2 <= not(
--        ( not(nBGACK040) and IO5 and nAS040_int and nDS040_int and IO2 and not(nTS) )
--     or ( not(nBGACK040) and IO5 and not(nCYCPEND) and nAS040_int and nDS040_int and IO2 ) );
--      end if;
--   end process;

end Behavioral;

