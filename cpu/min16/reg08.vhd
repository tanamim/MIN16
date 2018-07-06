-- 8bit register
library ieee;
use ieee.std_logic_1164.all;

entity reg08 is
  port (load, reset: in  std_ulogic;
        din:  in  std_ulogic_vector(7 downto 0);
        dout: out std_ulogic_vector(7 downto 0)
        );
end entity reg08;

architecture dataflow of reg08 is
signal dbuf: std_ulogic_vector(7 downto 0);
begin
    process (load, reset)
    begin
        if (reset = '1') then
            dbuf <= "00000000";
        elsif (load'event and load = '1') then
            dbuf <= din;
        end if;
    end process;
    dout <= dbuf;
end architecture dataflow;