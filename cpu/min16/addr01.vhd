-- 1bit full adder
library ieee;
use ieee.std_logic_1164.all;

entity addr01 is
  port (a, b, cin: in  std_ulogic;
        q, cout:   out std_ulogic
        );
end entity addr01;

architecture rtl of addr01 is
begin
  q    <= (a xor b) xor cin;
  cout <= (a and b) or (b and cin) or (cin and a);
end architecture rtl;