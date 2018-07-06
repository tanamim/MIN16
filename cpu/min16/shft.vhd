library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use ieee.numeric_std.all;

use work.util.all;

entity shft is
  port (alucode: in     std_ulogic_vector(3 downto 0);
        rd:      in     std_ulogic_vector(15 downto 0);
        rs:      in     std_ulogic_vector(15 downto 0);
        result:  buffer std_ulogic_vector(15 downto 0);
        flags:   out    std_ulogic_vector(4 downto 0)
       );
end entity shft;

architecture dataflow of shft is
  constant zero_vec: std_ulogic_vector := "0000000000000000";
  constant ALU_ROTL: std_ulogic_vector := "1111";
  --signal   res_ROTL: std_ulogic_vector(15 downto 0);
  signal   rs4bit:   std_ulogic_vector(3 downto 0);

begin
  flags  <= "00000";  -- TO Be improved
  rs4bit <= rs(3 downto 0);

  -- ROTL "1111"
  result <= zero_vec when alucode /= ALU_ROTL else
            rd                                 when rs4bit = "0000" else
            rd(14 downto 0) & rd(15)           when rs4bit = "0001" else
            rd(13 downto 0) & rd(15 downto 14) when rs4bit = "0010" else
            rd(12 downto 0) & rd(15 downto 13) when rs4bit = "0011" else
            rd(11 downto 0) & rd(15 downto 12) when rs4bit = "0100" else
            rd(10 downto 0) & rd(15 downto 11) when rs4bit = "0101" else
            rd( 9 downto 0) & rd(15 downto 10) when rs4bit = "0110" else
            rd( 8 downto 0) & rd(15 downto  9) when rs4bit = "0111" else
            rd( 7 downto 0) & rd(15 downto  8) when rs4bit = "1000" else
            rd( 6 downto 0) & rd(15 downto  7) when rs4bit = "1001" else
            rd( 5 downto 0) & rd(15 downto  6) when rs4bit = "1010" else
            rd( 4 downto 0) & rd(15 downto  5) when rs4bit = "1011" else
            rd( 3 downto 0) & rd(15 downto  4) when rs4bit = "1100" else
            rd( 2 downto 0) & rd(15 downto  3) when rs4bit = "1101" else
            rd( 1 downto 0) & rd(15 downto  2) when rs4bit = "1110" else
            rd(0)           & rd(15 downto  1) when rs4bit = "1111";

  --rotateLeft: process(rd, rs)
  --  variable index_tmp:  integer;
  --  variable index_end: integer;
  --begin
  --  res_ROTL <= zero_vec;
  --  index_tmp := 15 - index_end;
  --  index_end := to_integer(unsigned(rs));
  --  for index in 15 downto 0 loop
  --    res_ROTL(index) <= rd(index_tmp);
  --    index_tmp := index_tmp - 1;
  --    if index = index_end then
  --      index_tmp := 15;
  --    end if;
  --  end loop;
  --end process rotateLeft;

end architecture dataflow;