----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    10:22:16 08/20/2021 
-- Design Name: 
-- Module Name:    BCTL - Behavioral 
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

entity BCTL is
    Port ( BCLK : in  STD_LOGIC;
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
           nBG_ARM : out STD_LOGIC);
end BCTL;

architecture Behavioral of BCTL is
TYPE bus_state_fsm IS (
            B00,
            B01,
            B02,
            B03,
            B04,
            B05,
            B06,
            B07,
            B08,
            B09,
            B10,
            B11
);
signal bus_state: bus_state_fsm := B00;

attribute fsm_encoding : string;
attribute fsm_encoding of bus_state : signal is "compact";

begin
   process(bus_state)
   begin
      case (bus_state) is
         when B07 | B08 =>
            nBG040 <= '0';
         when others =>
            nBG040 <= '1';
      end case;
      case (bus_state) is
         when B01 | B02 | B03 | B04 | B05 | B06 =>
            nBGACK040<='1';
         when others =>
            nBGACK040<='0';
      end case;
      case (bus_state) is
         when B01 | B02 | B05 | B06 =>
            nBG<='0';
         when others =>
            nBG<='1';
      end case;
      case (bus_state) is
         when B11 =>
            nBG_ARM<='0';
         when others =>
            nBG_ARM<='1';
      end case;
   end process;
   process(BCLK,n040RSTI)
   begin
      if(BCLK'event and BCLK='1') then
         if(n040RSTI='0') then
            bus_state <= B00;
         else
            case (bus_state) is
               when B00 =>
                  if (nBR_ARM='0') then         bus_state <= B11;
                  else
                     if (nSBGACK030='0') then   bus_state <= B04;
                     else
                        if (nSBR030='0') then   bus_state <= B01;
                        else
                           if (nBR040='0') then bus_state <= B07;
                           else                 bus_state <= B00;
                           end if;
                        end if;
                     end if;
                  end if;
               when B01 =>                      bus_state <= B02;
               when B02 =>
                  if(nSBGACK030='0') then       bus_state <= B03;
                  else
                     if(nSBR030='0') then       bus_state <= B02;
                     else                       bus_state <= B03;
                     end if;
                  end if;
               when B03 =>                      bus_state <= B04;
               when B04 =>
                  if(nSBR030='0') then          bus_state <= B05;
                  else
                     if(nSBGACK030='0') then    bus_state <= B04;
                     else                       bus_state <= B00;
                     end if;
                  end if;
               when B05 =>                      bus_state <= B06;
               when B06 =>
                  if (nSBR030='1') then         bus_state <= B03;
                  else
                     if (nSBGACK030='0') then   bus_state <= B06;
                     else                       bus_state <= B02;
                     end if;
                  end if;
               when B07 =>                      bus_state <= B08;
               when B08 =>
                  if(nSBR030='0' or nBR_ARM='0') then
                     if(nLOCK='1') then         bus_state <= B09;
                     else
                        if(nLOCKE='0') then     bus_state <= B09;
                        else                    bus_state <= B08;
                        end if;
                     end if;
                  else                          bus_state <= B08;
                  end if;
               when B09 =>
                  if (nBB040='0') then
                     if(nLOCK='1') then         bus_state <= B09;
                     else
                        if(nLOCKE='0') then     bus_state <= B09;
                        else                    bus_state <= B08;
                        end if;
                     end if;
                  else                          bus_state <= B10;
                  end if;
               when B10 =>
                  if(nBB040='0') then
                     if(nBR040='0') then        bus_state <= B08;
                     else                       bus_state <= B09;
                     end if;
                  else                          bus_state <= B00;
                  end if;
               when B11 =>
                  if(nBR_ARM='1') then          bus_state <= B00;
                  else                          bus_state <= B11;
                  end if;
               when others =>                   bus_state <= B00;
            end case;
         end if;
      end if;
   end process;
end Behavioral;

