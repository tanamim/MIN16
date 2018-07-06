-- 16bit adder
library ieee;
use ieee.std_logic_1164.all;

use work.util.all;

entity addr16 is
  port (a, b:      in  std_ulogic_vector(15 downto 0);
        cin:       in  std_ulogic;
        q:         out std_ulogic_vector(15 downto 0);
        cout:      out std_ulogic
        );
end entity addr16;

architecture dataflow of addr16 is
  signal tmp_cout: std_ulogic;
begin
  add0: addr08 port map (a(7 downto 0), b(7 downto 0), cin, q(7 downto 0), tmp_cout);
  add1: addr08 port map (a(15 downto 8), b(15 downto 8), tmp_cout, q(15 downto 8), cout);
end architecture dataflow;
