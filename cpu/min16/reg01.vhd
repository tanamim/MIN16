-- 1bit register
library ieee;
use ieee.std_logic_1164.all;

entity reg01 is
  port (load, reset: in  std_ulogic;
        din:  in  std_ulogic;
        dout: out std_ulogic
        );
end entity reg01;

architecture dataflow of reg01 is
signal dbuf: std_ulogic;
begin
    process (load, reset)
    begin
        if (reset = '1') then
            dbuf <= '0';
        elsif (load'event and load = '1') then
            dbuf <= din;
        end if;
    end process;
    dout <= dbuf;
end architecture dataflow;