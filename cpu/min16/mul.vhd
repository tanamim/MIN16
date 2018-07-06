-- mul entity
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use ieee.numeric_std.all;

use work.util.all;

entity mul is
  port (alucode: in     std_ulogic_vector(3 downto 0);
        rd:      in     std_ulogic_vector(15 downto 0);
        rs:      in     std_ulogic_vector(15 downto 0);
        result:  buffer std_ulogic_vector(15 downto 0);
        flags:   out    std_ulogic_vector(4 downto 0)
       );
end entity mul;

architecture dataflow of mul is
  constant ALU_MUL:  std_ulogic_vector := "0010";
  signal   res_MUL:  std_ulogic_vector(15 downto 0);

begin
  flags <= "00000";  -- TO Be improved

  -- MUL (To Be implemented using logic gates)
  multiply: process(alucode, rd, rs)
  begin
    if alucode = ALU_MUL then
      res_MUL <= std_ulogic_vector(resize(signed(rd) * signed(rs), res_MUL'length));
    else
      res_MUL <= std_ulogic_vector(resize(unsigned(rd) * unsigned(rs), res_MUL'length));
    end if;
  end process multiply;

end architecture dataflow;