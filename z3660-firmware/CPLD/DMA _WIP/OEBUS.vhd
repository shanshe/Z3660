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
           nS2W : in  STD_LOGIC;
           R_W040 : in  STD_LOGIC;
           nRBERR : in  STD_LOGIC;
           nRDSACK0 : in  STD_LOGIC;
           nRDSACK1 : in  STD_LOGIC;
           nDMACOE : out  STD_LOGIC;
           OEBUS : out  STD_LOGIC_VECTOR (7 downto 0)
           );
end OEBUS_component;

architecture Behavioral of OEBUS_component is
signal MAS_state: STD_LOGIC_VECTOR(3 downto 0);
signal OEBUS_int: STD_LOGIC_VECTOR(7 downto 0);
--OEBUS   060     030
--  0   d31..24 d31..24    a
--  1   d23..16 d31..24    b2
--  2   d23..16 d23..16    b1 
--  3   d15.. 8 d31..24    c2

--  4   d15.. 8 d15.. 8    c1
--  5   d 7.. 0 d31..24    d3
--  6   d 7.. 0 d23..16    d2
--  7   d 7.. 0 d 7.. 0    d1

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

impure function byte return std_logic is begin
  return (nRBERR and not(nRDSACK0) and     nRDSACK1 ); end byte;
impure function word return std_logic is begin
  return (nRBERR and     nRDSACK0  and not(nRDSACK1)); end word;
impure function long return std_logic is begin
  return (nRBERR and not(nRDSACK0) and not(nRDSACK1)); end long;

begin

   MAS_state <= MAS3 & MAS2 & MAS1 & MAS0;
   OEBUS <= OEBUS_int;

   process(BCLK)
   begin
      if(RISING_EDGE(BCLK)) then
         if(R_W040='1') then -- READS
            nDMACOE <= '1';
            OEBUS_int(0) <= not(byte or word or long) and ( OEBUS_int(0) or nS2W );
            OEBUS_int(1) <= not(byte                ) and ( OEBUS_int(1) or nS2W );
            OEBUS_int(2) <= not(        word or long) and ( OEBUS_int(2) or nS2W );
            OEBUS_int(3) <= not(byte or word        ) and ( OEBUS_int(3) or nS2W );
            OEBUS_int(4) <= not(                long) and ( OEBUS_int(4) or nS2W );
            OEBUS_int(5) <= not(byte                ) and ( OEBUS_int(5) or nS2W );
            OEBUS_int(6) <= not(        word        ) and ( OEBUS_int(6) or nS2W );
            OEBUS_int(7) <= not(                long) and ( OEBUS_int(7) or nS2W );
         else                -- WRITES
            case (MAS_state) is
               when L =>
                  nDMACOE <= '0';
               when others =>
                  nDMACOE <= '1';
            end case;

            case (MAS_state) is
               when A  =>  OEBUS_int <= "01101010";
               when F  =>  OEBUS_int <= "01101010";
               when K  =>  OEBUS_int <= "01101010";

               when B  =>  OEBUS_int <= "00100111";
               when D  =>  OEBUS_int <= "00100111";
               when G  =>  OEBUS_int <= "00100111";

               when E  =>  OEBUS_int <= "00001111";
               when J  =>  OEBUS_int <= "00001111";
               when N  =>  OEBUS_int <= "00001111";

               when C  =>  OEBUS_int <= "00101101";
               when H  =>  OEBUS_int <= "01101001";
               when L  =>  OEBUS_int <= "11101001"; -- L -> DMA instead of OEBUS(7)

               when M  =>  OEBUS_int <= "01100011";

--               when I  =>  OEBUS_int <= "11111111";
--               when Z  =>  OEBUS_int <= "11111111";
--               when DC =>  OEBUS_int <= "01101111"; -- Don't Care state -> active low OEBUS 4 and 7
               when others =>
                           OEBUS_int <= "11111111";
            end case;
         end if;

      end if;
   end process;

---- original equations
--   process(BCLK)
--   begin
--      if(RISING_EDGE(BCLK)) then
----register
--nDMACOE <= not(
--        ( not(R_W040) and not(MAS0) and MAS1 and not(MAS2) and not(MAS3) ) );
----register
--OEBUS_int(0) <= not(
--        (     R_W040  and not(nS2W) and not(OEBUS_int(0)))
--     or (     R_W040  and byte )
--     or (     R_W040  and word )
--     or (     R_W040  and long )
--     or ( not(R_W040) and MAS0 and not(MAS1) and not(MAS2)               )    -- A         F
--     or ( not(R_W040) and MAS0 and               not(MAS2) and not(MAS3) ) ); -- A                 K
----register
--OEBUS_int(1) <= not(
--        (     R_W040  and not(nS2W) and not(OEBUS_int(1)) )
--     or (     R_W040  and byte )
--     or ( not(R_W040) and MAS0               and     MAS2  and     MAS3 )     --     C         H
--     or ( not(R_W040) and not(MAS0) and MAS1 and not(MAS2) and not(MAS3) ) ); --                     L
----register
--OEBUS_int(2) <= not(
--        (     R_W040  and not(nS2W) and not(OEBUS_int(2)) )
--     or (     R_W040  and word )
--     or (     R_W040  and long )
--     or ( not(R_W040) and MAS0 and     MAS1  and     MAS2                )    --               H       M
--     or ( not(R_W040) and MAS0 and not(MAS1) and not(MAS2)               )    -- A         F
--     or ( not(R_W040) and              MAS1  and not(MAS2) and not(MAS3) ) ); --                   K L
----register
--OEBUS_int(3) <= not(
--        (     R_W040  and not(nS2W) and not(OEBUS_int(3)) )
--     or (     R_W040  and byte )
--     or (     R_W040  and word )
----     or ( not(R_W040) and MAS0 and MAS2 and not(MAS3) )                       --                       M N
--     or ( not(R_W040) and MAS0 and     MAS1  and     MAS2  and not(MAS3) )    --                       M
--     or ( not(R_W040) and          not(MAS1) and     MAS2  and not(MAS3) )    --   B   D
--     or ( not(R_W040) and MAS0 and     MAS1  and not(MAS2) and     MAS3  ) ); --             G
----register
--OEBUS_int(4) <= not(
--        (     R_W040  and not(nS2W) and not(OEBUS_int(4)) )
--     or (     R_W040  and long ) 
--     or ( not(R_W040) and MAS0                         )                      -- A B C     F G H   K   M
--     or ( not(R_W040) and          MAS1                )                      --             G H J K L M N DC
--     or ( not(R_W040) and                   MAS2       ) );                   --   B C D E     H J     M N
----register
--OEBUS_int(5) <= not(
--        (     R_W040  and not(nS2W) and not(OEBUS_int(5)) )
--     or (     R_W040  and byte )
--     or ( not(R_W040) and not(MAS0) and MAS1 and MAS2          )              --                 J       N
--     or ( not(R_W040) and not(MAS0)          and MAS2 and MAS3 ) );           --         E       J
----register
--OEBUS_int(6) <= not(
--        (     R_W040  and not(nS2W) and not(OEBUS_int(6)) )
--     or (     R_W040  and word )
--     or ( not(R_W040) and not(MAS0) and                   MAS2           )    --       D E       J       N
--     or ( not(R_W040) and               not(MAS1) and     MAS2           )    --   B C D E
--     or ( not(R_W040) and     MAS0  and     MAS1  and not(MAS2) and MAS3 ) ); --             G
----register
--OEBUS_int(7) <= not(
--        (     R_W040  and not(nS2W) and not(OEBUS_int(7)) )
--     or (     R_W040  and long )
--     or ( not(R_W040) and MAS0                            )                   -- A B C     F G H   K   M
--     or ( not(R_W040) and                   MAS2          )                   --   B C D E     H J     M N
--     or ( not(R_W040) and          MAS1 and          MAS3 ) );                --             G H J         DC
--      end if;
--   end process;

end Behavioral;

