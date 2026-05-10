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
    Port ( --BCLK : in  STD_LOGIC;
--           R_W040 : in  STD_LOGIC;
           SLV0 : in  STD_LOGIC;
           SLV1 : in  STD_LOGIC;
           SLV2 : in  STD_LOGIC;
--           SLV3 : in  STD_LOGIC;
--           nCYCPEND : in  STD_LOGIC;
--           SIZ40 : in  STD_LOGIC_VECTOR (1 downto 0);
--           nPLSTERM : in  STD_LOGIC;
--           nRTERM : in  STD_LOGIC;
--           nTS : in  STD_LOGIC;
           nDS040 : out  STD_LOGIC;
           nAS040 : out  STD_LOGIC;
           nTEA : out  STD_LOGIC;
--           nTBI : out  STD_LOGIC;
--           n040RSTI : in  STD_LOGIC;
           nTA : out  STD_LOGIC
         );
end BUSTERM;

architecture Behavioral of BUSTERM is

-- Slave State Values
CONSTANT  S0: std_logic_vector(2 downto 0):= "000"; -- 0
CONSTANT  S1: std_logic_vector(2 downto 0):= "001"; -- 1
CONSTANT  S2: std_logic_vector(2 downto 0):= "010"; -- 2
CONSTANT  S3: std_logic_vector(2 downto 0):= "011"; -- 3
CONSTANT  S4: std_logic_vector(2 downto 0):= "100"; -- 4
CONSTANT  SB: std_logic_vector(2 downto 0):= "110"; -- 6

signal SLV_state: std_logic_vector(2 downto 0);
begin

SLV_state <= SLV2 & SLV1 & SLV0;

nTA    <= '0' when  SLV_state=S1                  else '1';
nTEA   <= '0' when  SLV_state=SB                  else '1';
nAS040 <= '0' when (SLV_state=S4 or SLV_state=S2) else '1';
nDS040 <= '0' when (SLV_state=S4 or SLV_state=S2) else '1';

end Behavioral;

