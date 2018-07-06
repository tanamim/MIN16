-- 4bit adder
library ieee;
use ieee.std_logic_1164.all;

use work.util.all;

entity addr04 is
  port (a, b:      in  std_ulogic_vector(3 downto 0);
        cin:       in  std_ulogic;
        q:         out std_ulogic_vector(3 downto 0);
        cout:      out std_ulogic
        );
end entity addr04;

architecture dataflow of addr04 is
signal couts: std_ulogic_vector(3 downto 0);
begin
  add0: addr01 port map (a(0), b(0), cin,      q(0), couts(0));
  add1: addr01 port map (a(1), b(1), couts(0), q(1), couts(1));
  add2: addr01 port map (a(2), b(2), couts(1), q(2), couts(2));
  add3: addr01 port map (a(3), b(3), couts(2), q(3), couts(3));
  cout <= couts(3);
end architecture dataflow;