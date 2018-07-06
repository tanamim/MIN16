-- 8bit adder
library ieee;
use ieee.std_logic_1164.all;

use work.util.all;

entity addr08 is
  port (a, b:      in  std_ulogic_vector(7 downto 0);
        cin:       in  std_ulogic;
        q:         out std_ulogic_vector(7 downto 0);
        cout:      out std_ulogic
        );
end entity addr08;

architecture dataflow of addr08 is
  signal tmp_cout: std_ulogic;
begin
  add0: addr04 port map (a(3 downto 0), b(3 downto 0), cin, q(3 downto 0), tmp_cout);
  add1: addr04 port map (a(7 downto 4), b(7 downto 4), tmp_cout, q(7 downto 4), cout);
end architecture dataflow;