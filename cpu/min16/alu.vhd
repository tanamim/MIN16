library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use ieee.numeric_std.all;

use work.util.all;

entity alu is
  port (alucode: in     std_ulogic_vector(3 downto 0);
        rd:      in     std_ulogic_vector(15 downto 0);
        rs:      in     std_ulogic_vector(15 downto 0);
        result:  buffer std_ulogic_vector(15 downto 0);
        flags:   out    std_ulogic_vector(4 downto 0)
       );
end entity alu;

architecture dataflow of alu is
  constant ALU_ADD:  std_ulogic_vector := "0000";
  constant ALU_SUB:  std_ulogic_vector := "0001";
  constant ALU_MUL:  std_ulogic_vector := "0010";
  constant ALU_SLT:  std_ulogic_vector := "0011";
  constant ALU_ADDU: std_ulogic_vector := "0100";
  constant ALU_SUBU: std_ulogic_vector := "0101";
  constant ALU_MULU: std_ulogic_vector := "0110";
  constant ALU_SLTU: std_ulogic_vector := "0111";
  constant ALU_AND:  std_ulogic_vector := "1000";
  constant ALU_OR:   std_ulogic_vector := "1001";
  constant ALU_XOR:  std_ulogic_vector := "1010";
  constant ALU_NOR:  std_ulogic_vector := "1011";
  constant ALU_SLL:  std_ulogic_vector := "1100";
  constant ALU_SRL:  std_ulogic_vector := "1101";
  constant ALU_SRA:  std_ulogic_vector := "1110"; 
  constant ALU_ROTL: std_ulogic_vector := "1111";

  signal res_ADD:    std_ulogic_vector(15 downto 0); 
  signal res_SUB:    std_ulogic_vector(15 downto 0); 
  signal res_MUL:    std_ulogic_vector(15 downto 0); -- TODO: hardware multiplier
  signal res_SLT:    std_ulogic_vector(15 downto 0);   
  signal res_SLL:    std_ulogic_vector(15 downto 0);
  signal res_SRL:    std_ulogic_vector(15 downto 0);
  signal res_SRA:    std_ulogic_vector(15 downto 0);
  signal res_ROTL:   std_ulogic_vector(15 downto 0); -- TODO: barrel shifter

  signal cf_ADD:     std_ulogic;   -- carry flag for ADD
  signal cf_SUB:     std_ulogic;   -- carry flag for SUB
  signal tmp_cf:     std_ulogic;

  constant zero_vec: std_ulogic_vector := "0000000000000000";
  constant zero:     std_ulogic := '0';
  constant one:      std_ulogic := '1';

begin
  -- ADD
  a0: addr16 port map (a=>rd, b=>rs, cin=>zero, q=>res_ADD, cout=>cf_ADD);

  -- SUB
  a1: addr16 port map (a=>rd, b=>(not rs), cin=>one, q=>res_SUB, cout=>cf_SUB);

  -- MUL (To be implemented in mul.vhd)
  multiply: process(alucode, rd, rs)
  begin
    if alucode = ALU_MUL then
      res_MUL <= std_ulogic_vector(resize(signed(rd) * signed(rs), res_MUL'length));
    else
      res_MUL <= std_ulogic_vector(resize(unsigned(rd) * unsigned(rs), res_MUL'length));
    end if;
  end process multiply;

  -- SLT
  setLessThan: process(alucode, rd, rs, res_SUB)
  begin
    res_SLT <= zero_vec;
    if alucode = ALU_SLT then
      if rd(15) = rs(15) then
        res_SLT <= "000000000000000"&res_SUB(15);
      else
        res_SLT <= "000000000000000"&rd(15);
      end if;
    elsif alucode = ALU_SLTU then
      if rd(15) = res_SUB(15) then
        res_SLT <= x"0000";
      else 
        res_SLT <= x"0001";
      end if;
    end if;
  end process setLessThan;

  -- SLL "1100"
  shiftLeftLogical: process(rd, rs)
    variable index_tmp: integer;
  begin
    res_SLL <= zero_vec;
    index_tmp := 15 - to_integer(unsigned(rs));
    for index in 15 downto 0 loop
      res_SLL(index) <= rd(index_tmp);
      index_tmp := index_tmp - 1;
    end loop;
  end process shiftLeftLogical;

  -- SRL "1101"
  shiftRightLogical: process(rd, rs)
    variable index_tmp: integer;
    variable index_end: integer;
  begin
    res_SRL <= zero_vec;
    index_tmp := 15 - index_end;
    index_end := to_integer(unsigned(rs));
    for index in 15 downto 0 loop
      res_SRL(index_tmp) <= rd(index);
      if index = index_end then
        exit;
      end if;
      index_tmp := index_tmp - 1;
    end loop;
  end process shiftRightLogical;

  -- SRA "1110"
  shiftRightArithmetic: process(rd, rs)
    variable index_tmp: integer;
    variable index_end: integer;
  begin
    res_SRA <= rd;
    index_tmp := 14 - index_end;
    index_end := to_integer(unsigned(rs));
    for index in 14 downto 0 loop
      res_SRA(index_tmp) <= rd(index);
      if index = index_end then
        exit;
      end if;
      index_tmp := index_tmp - 1;
    end loop;
  end process shiftRightArithmetic;

  -- ROTL "1111" (To be implemented in shft.vhd)
  rotateLeft: process(rd, rs)
    variable index_tmp:  integer;
    variable index_end: integer;
  begin
    res_ROTL <= zero_vec;
    index_tmp := 15 - index_end;
    index_end := to_integer(unsigned(rs));
    for index in 15 downto 0 loop
      res_ROTL(index) <= rd(index_tmp);
      index_tmp := index_tmp - 1;
      if index = index_end then
        index_tmp := 15;
      end if;
    end loop;
  end process rotateLeft;

  -- Switching
  process(alucode, rd, rs, res_ADD, res_SUB, res_MUL, res_SLT, res_SLL, res_SRL, res_SRA, res_ROTL, cf_ADD, cf_SUB)
    variable var_cf: std_ulogic;
  begin
    var_cf := '0';
    case alucode is
      when "0000" => result <= res_ADD; var_cf := cf_ADD;
      when "0001" => result <= res_SUB; var_cf := cf_SUB;
      when "0010" => result <= res_MUL;
      when "0011" => result <= res_SLT;
      when "0100" => result <= res_ADD; var_cf := cf_ADD;
      when "0101" => result <= res_SUB; var_cf := cf_SUB;
      when "0110" => result <= res_MUL;
      when "0111" => result <= res_SLT;
      when "1000" => result <= rd and rs;
      when "1001" => result <= rd or rs;
      when "1010" => result <= rd xor rs;
      when "1011" => result <= not (rd or rs);      
      when "1100" => result <= res_SLL;
      when "1101" => result <= res_SRL;
      when "1110" => result <= res_SRA;
      when "1111" => result <= res_ROTL;
    end case;
    tmp_cf <= var_cf;
  end process;

  -- flag determination (cf is done already)
  -- TODO: Overflow and Carry flag logic must be reviewed later. Too Hard to debug with hardware.
  flagDetermine: process(rd, rs, result, tmp_cf)
    variable tmp_sf: std_ulogic;
    variable tmp_of: std_ulogic;
    variable tmp_zf: std_ulogic;
    variable tmp_eq: std_ulogic;
  begin
    tmp_sf := result(15);
    tmp_of := (rd(15) and rs(15) and (not result(15))) or ((not rd(15)) and (not rs(15)) and (result(15)));
    if to_integer(unsigned(result)) = 0 then
      tmp_zf := '1';
    else
      tmp_zf := '0';
    end if;

    if rd = rs then
      tmp_eq := '1';
    else
      tmp_eq := '0';
    end if;  

    flags <= tmp_eq&tmp_of&tmp_cf&tmp_sf&tmp_zf; 
  end process flagDetermine;

end architecture dataflow;



