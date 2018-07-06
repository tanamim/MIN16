-- display a hex character from 4bit num
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity hexdisp is
  port (
        num: in     std_ulogic_vector(3 downto 0);
        hex: buffer std_ulogic_vector(6 downto 0)
        );
end entity hexdisp;

architecture dataflow of hexdisp is
  constant hex_0: std_ulogic_vector := "1000000";
  constant hex_1: std_ulogic_vector := "1111001";
  constant hex_2: std_ulogic_vector := "0100100";
  constant hex_3: std_ulogic_vector := "0110000";
  constant hex_4: std_ulogic_vector := "0011001";
  constant hex_5: std_ulogic_vector := "0010010";
  constant hex_6: std_ulogic_vector := "0000010";
  constant hex_7: std_ulogic_vector := "1111000";
  constant hex_8: std_ulogic_vector := "0000000";
  constant hex_9: std_ulogic_vector := "0011000";
  constant hex_a: std_ulogic_vector := "0001000";
  constant hex_b: std_ulogic_vector := "0000011";
  constant hex_c: std_ulogic_vector := "1000110";
  constant hex_d: std_ulogic_vector := "0100001";
  constant hex_e: std_ulogic_vector := "0000110";
  constant hex_f: std_ulogic_vector := "0001110";

  subtype hexvec is std_ulogic_vector(6 downto 0);
  type disp is array (0 to 15) of hexvec;
  constant hexarray: disp := (
    hex_0, hex_1, hex_2, hex_3, hex_4, hex_5, hex_6, hex_7, 
    hex_8, hex_9, hex_a, hex_b, hex_c, hex_d, hex_e, hex_f
    );

begin
  hex <= hexarray(to_integer(unsigned(num)));
end architecture dataflow;

