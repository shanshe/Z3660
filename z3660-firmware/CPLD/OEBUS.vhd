----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    08:31:38 08/20/2021 
-- Design Name: 
-- Module Name:    OEBUS - Behavioral 
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
--signal MAS_state: STD_LOGIC_VECTOR(3 downto 0);
--signal SLV_state: STD_LOGIC_VECTOR(3 downto 0);
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

--impure function byte return boolean is begin
--  return ((nRBERR and not(nRDSACK0) and nRDSACK1))='1'; end byte;
--impure function word return boolean is begin
--  return (nRBERR and nRDSACK0 and not(nRDSACK1))='1'; end word;
--impure function long return boolean is begin
--  return (nRBERR and not(nRDSACK0) and not(nRDSACK1))='1'; end long;
impure function byte return std_logic is begin
  return ((nRBERR and not(nRDSACK0) and nRDSACK1)); end byte;
impure function word return std_logic is begin
  return (nRBERR and nRDSACK0 and not(nRDSACK1)); end word;
impure function long return std_logic is begin
  return (nRBERR and not(nRDSACK0) and not(nRDSACK1)); end long;

begin

--	n2SW <= SLV0 or (SLV1 and SLV2) or (not(SLV1) and not(SLV2)) or not(SLV0) or not(SLV1);
--	n2SW <= SLV3 or not(SLV0) or not(SLV1);
	nS2W <= not(SLV0 and SLV1 and not(SLV3));
--	MAS_state <= MAS3 & MAS2 & MAS1 & MAS0;
--	SLV_state <= SLV3 & SLV2 & SLV1 & SLV0;
	OEBUS <= OEBUS_int;
--	process(BCLK)
--	begin
--		if(BCLK'event and  BCLK='1') then
--			if(R_W040='1') then       -- read cycle
--				nDMACOE <= '1';
--				if ((byte or word or long) and SLV_state=S1) or (OEBUS_int(0)='0' and SLV_state=S2W) then
--					OEBUS_int(0) <= '0';
--				else
--					OEBUS_int(0) <= '1';
--				end if;
--				if ((byte                ) and SLV_state=S1) or (OEBUS_int(1)='0' and SLV_state=S2W) then
--					OEBUS_int(1) <= '0';
--				else
--					OEBUS_int(1) <= '1';
--				end if;
--				if ((word or long        ) and SLV_state=S1) or (OEBUS_int(2)='0' and SLV_state=S2W) then
--					OEBUS_int(2) <= '0';
--				else
--					OEBUS_int(2) <= '1';
--				end if;
--				if ((byte or word        ) and SLV_state=S1) or (OEBUS_int(3)='0' and SLV_state=S2W) then
--					OEBUS_int(3) <= '0';
--				else
--					OEBUS_int(3) <= '1';
--				end if;
--				if ((long                ) and SLV_state=S1) or (OEBUS_int(4)='0' and SLV_state=S2W) then
--					OEBUS_int(4) <= '0';
--				else
--					OEBUS_int(4) <= '1';
--				end if;
--				if ((byte                ) and SLV_state=S1) or (OEBUS_int(5)='0' and SLV_state=S2W) then
--					OEBUS_int(5) <= '0';
--				else
--					OEBUS_int(5) <= '1';
--				end if;
--				if ((word                ) and SLV_state=S1) or (OEBUS_int(6)='0' and SLV_state=S2W) then
--					OEBUS_int(6) <= '0';
--				else
--					OEBUS_int(6) <= '1';
--				end if;
--				if ((long                ) and SLV_state=S1) or (OEBUS_int(7)='0' and SLV_state=S2W) then
--					OEBUS_int(7) <= '0';
--				else
--					OEBUS_int(7) <= '1';
--				end if;
--			else                      -- write cycle
--				if(SLV_state=S1) then
--					OEBUS_int <= "11111111";
--					nDMACOE <= '1';
--				else
--				case (MAS_state) is
--					when I =>  OEBUS_int <= "11111111";
--					when A =>  OEBUS_int <= "01101010";
--					when B =>  OEBUS_int <= "00100111";
--					when C =>  OEBUS_int <= "00101101";
--					when D =>  OEBUS_int <= "00100111";
--					when E =>  OEBUS_int <= "00001111";
--					when F =>  OEBUS_int <= "01101010";
--					when G =>  OEBUS_int <= "00100111";
--					when H =>  OEBUS_int <= "01101001";
--					when J =>  OEBUS_int <= "00001111";
--					when K =>  OEBUS_int <= "01101010";
--					when L =>  OEBUS_int <= "11101001"; -- L -> DMA instead of OEBUS(7)
--					when M =>  OEBUS_int <= "01100011";
--					when N =>  OEBUS_int <= "00001111";
--					when Z  => OEBUS_int <= "11111111";
--					when DC => OEBUS_int <= "01101111"; -- Don't Care state -> active low OEBUS 4 and 7
--					when others =>
--					           OEBUS_int <= "11111111"; -- there are no "others" XD
--				end case;
--				
--				case (MAS_state) is
--					when L =>
--						nDMACOE <= '0';
--					when others =>
--						nDMACOE <= '1';
--				end case;
--				
--				end if;
--			end if;
--		end if;
--	end process;

--	process(BCLK)
--	begin
--		if(BCLK'event and  BCLK='1') then
--			if(R_W040='1') then       -- read cycle
--				nDMACOE <= '1';
--				OEBUS_int(0) <= not(byte or word or long);
--				OEBUS_int(1) <= not(byte);
--				OEBUS_int(2) <= not(word or long);
--				OEBUS_int(3) <= not(byte or word);
--				OEBUS_int(4) <= not(long);
--				OEBUS_int(5) <= not(byte);
--				OEBUS_int(6) <= not(word);
--				OEBUS_int(7) <= not(long);
--			else                      -- write cycle
--				case (MAS_state) is
--					when L =>
--						nDMACOE <= '0';
--					when others =>
--						nDMACOE <= '1';
--				end case;
--				case (MAS_state) is
--					when A | F | K =>
--						OEBUS_int(0) <= '0';
--					when others =>
--						OEBUS_int(0) <= '1';
--				end case;
--				case (MAS_state) is
--					when C | H | L =>
--						OEBUS_int(1) <= '0';
--					when others =>
--						OEBUS_int(1) <= '1';
--				end case;
--				case (MAS_state) is
--					when A | F | H | K | L | M =>
--						OEBUS_int(2) <= '0';
--					when others =>
--						OEBUS_int(2) <= '1';
--				end case;
--				case (MAS_state) is
--					when B | D | G | M =>
--						OEBUS_int(3) <= '0';
--					when others =>
--						OEBUS_int(3) <= '1';
--				end case;
--				case (MAS_state) is
--					when A | B | C | D | E | F | G | H | J | K | L | M | N | DC =>
--						OEBUS_int(4) <= '0';
--					when others =>
--						OEBUS_int(4) <= '1';
--				end case;
--				case (MAS_state) is
--					when E | J | N =>
--						OEBUS_int(5) <= '0';
--					when others =>
--						OEBUS_int(5) <= '1';
--				end case;
--				case (MAS_state) is
--					when B | C | D | E | G | J | N =>
--						OEBUS_int(6) <= '0';
--					when others =>
--						OEBUS_int(6) <= '1';
--				end case;
--				case (MAS_state) is
--					when A | B | C | D | E | F | G | H | J | K | M | N | DC =>
--						OEBUS_int(7) <= '0';
--					when others =>
--						OEBUS_int(7) <= '1';
--				end case;
--			end if;
--
--		end if;
--	end process;
--
--
--	process(BCLK)
--	begin
--		if(BCLK'event and  BCLK='1') then
--			if(R_W040='1') then
--				nDMACOE <= '1';
--			else
--				case (MAS_state) is
--					when L =>
--						nDMACOE <= '0';
--					when others =>
--						nDMACOE <= '1';
--				end case;
--			end if;
--
--			if(R_W040='1') then
--				OEBUS_int(0) <= not(byte or word or long)
--				and ( OEBUS_int(0) or nS2W );
--			else
--				case (MAS_state) is
--					when A | F | K =>
--						OEBUS_int(0) <= '0';
--					when others =>
--						OEBUS_int(0) <= '1';
--				end case;
--			end if;
--			if(R_W040='1') then
--				OEBUS_int(1) <= not(byte)
--				and ( OEBUS_int(1) or nS2W );
--			else
--				case (MAS_state) is
--					when C | H | L =>
--						OEBUS_int(1) <= '0';
--					when others =>
--						OEBUS_int(1) <= '1';
--				end case;
--			end if;
--			if(R_W040='1') then
--				OEBUS_int(2) <= not(word or long)
--				and ( OEBUS_int(2) or nS2W );
--			else
--				case (MAS_state) is
--					when A | F | H | K | L | M =>
--						OEBUS_int(2) <= '0';
--					when others =>
--						OEBUS_int(2) <= '1';
--				end case;
--			end if;
--			if(R_W040='1') then
--				OEBUS_int(3) <= not(byte or word)
--				and ( OEBUS_int(3) or nS2W );
--			else
--				case (MAS_state) is
--					when B | D | G | M =>
--						OEBUS_int(3) <= '0';
--					when others =>
--						OEBUS_int(3) <= '1';
--				end case;
--			end if;
--			if(R_W040='1') then
--				OEBUS_int(4) <= not(long)
--				and ( OEBUS_int(4) or nS2W );
--			else
--				case (MAS_state) is
--					when A | B | C | D | E | F | G | H | J | K | L | M | N | DC =>
--						OEBUS_int(4) <= '0';
--					when others =>
--						OEBUS_int(4) <= '1';
--				end case;
--			end if;
--			if(R_W040='1') then
--				OEBUS_int(5) <= not(byte)
--				and ( OEBUS_int(5) or nS2W );
--			else
--				case (MAS_state) is
--					when E | J | N =>
--						OEBUS_int(5) <= '0';
--					when others =>
--						OEBUS_int(5) <= '1';
--				end case;
--			end if;
--			if(R_W040='1') then
--				OEBUS_int(6) <= not(word)
--				and ( OEBUS_int(6) or nS2W );
--			else
--				case (MAS_state) is
--					when  B | C | D | E | G | J | N =>
--						OEBUS_int(6) <= '0';
--					when others =>
--						OEBUS_int(6) <= '1';
--				end case;
--			end if;
--			if(R_W040='1') then
--				OEBUS_int(7) <= not(long)
--				and ( OEBUS_int(7) or nS2W );
--			else
--				case (MAS_state) is
--					when A | B | C | D | E | F | G | H | J | K | M | N | DC =>
--						OEBUS_int(7) <= '0';
--					when others =>
--						OEBUS_int(7) <= '1';
--				end case;
--			end if;
--
--		end if;
--	end process;

-- original equations
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
--     or ( MAS0 and MAS2 and not(MAS3) and not(R_W040) )            -- M N
     or ( MAS0 and MAS1 and MAS2 and not(MAS3) and not(R_W040) )     -- M
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

---- original from 3640
----register
--nDMACOE <= not(
--        ( not(MAS0) and MAS1 and not(MAS2) and not(MAS3) and not(SLV3) and not(R_W040) ) );
----register
--OEBUS_int(0) <= not(
--        ( MAS0 and not(MAS1) and not(MAS2) and not(MAS3) and not(SLV3) and not(R_W040) )
--     or ( MAS0 and not(MAS1) and not(MAS2) and MAS3 and not(SLV3) and not(R_W040) )
--     or ( MAS0 and MAS1 and not(MAS2) and not(MAS3) and not(SLV3) and not(R_W040) )
--     or ( not(OEBUS_int(0)) and not(nS2W) and R_W040 )
--     or ( SLV3 and R_W040 and nRBERR and not(nRDSACK0) and nRDSACK1 )
--     or ( SLV3 and R_W040 and nRBERR and nRDSACK0 and not(nRDSACK1) )
--     or ( SLV3 and R_W040 and nRBERR and not(nRDSACK0) and not(nRDSACK1) ) );
----register
--OEBUS_int(1) <= not(
--        ( MAS0 and not(MAS1) and MAS2 and MAS3 and not(SLV3) and not(R_W040) )
--     or ( MAS0 and MAS1 and MAS2 and MAS3 and not(SLV3) and not(R_W040) )
--     or ( not(MAS0) and MAS1 and not(MAS2) and not(MAS3) and not(SLV3) and not(R_W040) )
--     or ( not(OEBUS_int(1)) and not(nS2W) and R_W040 )
--     or ( SLV3 and R_W040 and nRBERR and not(nRDSACK0) and nRDSACK1 ) );
----register
--OEBUS_int(2) <= not(
--        ( MAS0 and not(MAS1) and not(MAS2) and not(MAS3) and not(SLV3) and not(R_W040) )
--     or ( MAS0 and not(MAS1) and not(MAS2) and MAS3 and not(SLV3) and not(R_W040) )
--     or ( MAS0 and MAS1 and MAS2 and MAS3 and not(SLV3) and not(R_W040) )
--     or ( MAS0 and MAS1 and not(MAS2) and not(MAS3) and not(SLV3) and not(R_W040) )
--     or ( not(MAS0) and MAS1 and not(MAS2) and not(MAS3) and not(SLV3) and not(R_W040) )
--     or ( MAS0 and MAS1 and MAS2 and not(MAS3) and not(SLV3) and not(R_W040) )
--     or ( not(OEBUS_int(2)) and not(nS2W) and R_W040 )
--     or ( SLV3 and R_W040 and nRBERR and nRDSACK0 and not(nRDSACK1) )
--     or ( SLV3 and R_W040 and nRBERR and not(nRDSACK0) and not(nRDSACK1) ) );
----register
--OEBUS_int(3) <= not(
--        ( not(MAS0) and not(MAS1) and MAS2 and not(MAS3) and not(SLV3) and not(R_W040) )
--     or ( MAS0 and MAS1 and not(MAS2) and MAS3 and not(SLV3) and not(R_W040) )
--     or ( MAS0 and MAS1 and MAS2 and not(MAS3) and not(SLV3) and not(R_W040) )
--     or ( MAS0 and not(MAS1) and MAS2 and not(MAS3) and not(SLV3) and not(R_W040) )
--     or ( not(OEBUS_int(3)) and not(nS2W) and R_W040 )
--     or ( SLV3 and R_W040 and nRBERR and not(nRDSACK0) and nRDSACK1 )
--     or ( SLV3 and R_W040 and nRBERR and nRDSACK0 and not(nRDSACK1) ) );
----register
--OEBUS_int(4) <= not(
--        ( MAS2 and not(SLV3) and not(R_W040) )
--     or ( MAS0 and not(SLV3) and not(R_W040) )
--     or ( MAS1 and not(SLV3) and not(R_W040) )
--     or ( not(nS2W) and not(OEBUS_int(4)) and R_W040 )
--     or ( SLV3 and R_W040 and nRBERR and not(nRDSACK0) and not(nRDSACK1) ) );
----register
--OEBUS_int(5) <= not(
--        ( not(MAS0) and not(MAS1) and MAS2 and MAS3 and not(SLV3) and not(R_W040) )
--     or ( not(MAS0) and MAS1 and MAS2 and MAS3 and not(SLV3) and not(R_W040) )
--     or ( not(MAS0) and MAS1 and MAS2 and not(MAS3) and not(SLV3) and not(R_W040) )
--     or ( not(nS2W) and not(OEBUS_int(5)) and R_W040 )
--     or ( SLV3 and R_W040 and nRBERR and not(nRDSACK0) and nRDSACK1 ) );
----register
--OEBUS_int(6) <= not(
--        ( MAS0 and not(MAS1) and MAS2 and MAS3 and not(SLV3) and not(R_W040) )
--     or ( MAS0 and not(MAS1) and MAS2 and not(MAS3) and not(SLV3) and not(R_W040) )
--     or ( not(MAS0) and not(MAS1) and MAS2 and not(MAS3) and not(SLV3) and not(R_W040) )
--     or ( not(MAS0) and not(MAS1) and MAS2 and MAS3 and not(SLV3) and not(R_W040) )
--     or ( MAS0 and MAS1 and not(MAS2) and MAS3 and not(SLV3) and not(R_W040) )
--     or ( not(MAS0) and MAS1 and MAS2 and MAS3 and not(SLV3) and not(R_W040) )
--     or ( not(MAS0) and MAS1 and MAS2 and not(MAS3) and not(SLV3) and not(R_W040) )
--     or ( not(nS2W) and R_W040 and not(OEBUS_int(6)) )
--     or ( SLV3 and R_W040 and nRBERR and nRDSACK0 and not(nRDSACK1) ) );
----register
--OEBUS_int(7) <= not(
--        ( MAS1 and MAS3 and not(SLV3) and not(R_W040) )
--     or ( MAS0 and not(SLV3) and not(R_W040) )
--     or ( MAS2 and not(SLV3) and not(R_W040) )
--     or ( not(nS2W) and R_W040 and not(OEBUS_int(6)) )
--     or ( SLV3 and R_W040 and nRBERR and not(nRDSACK0) and not(nRDSACK1) ) );
--
--		end if;
--	end process;

end Behavioral;

