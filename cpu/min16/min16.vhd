-- min16 top level entity
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use ieee.numeric_std.all;
use work.util.all;

library cscie93;

-- This file should be used for the DE2-115 board ONLY

entity min16 is
  port (
      -- CLOCK
      clk50mhz : in std_logic;
      -- PS/2 PORT
      ps2_clk : in std_logic;
      ps2_data : in std_logic;
      -- LCD
      lcd_en : out std_logic;
      lcd_on : out std_logic;
      lcd_rs : out std_logic;
      lcd_rw : out std_logic;
      lcd_db : inout std_logic_vector(7 downto 0);
      -- RS232
      rs232_rxd : in std_logic;
      rs232_txd : out std_logic;
      rs232_cts : out std_logic;
      -- SSRAM interface
      sram_dq : inout std_logic_vector (15 downto 0);
      sram_addr : out std_logic_vector(19 downto 0);
      sram_ce_N : out std_logic;
      sram_oe_N : out std_logic;
      sram_we_N : out std_logic;
      sram_ub_N : out std_logic;
      sram_lb_N : out std_logic;
      -- PUSH Button
      key0:  in   std_ulogic;
      key1:  in   std_ulogic;
      key2:  in   std_ulogic;
      key3:  in   std_ulogic;
      -- SLIDE SWITCh
      sw0:   in   std_ulogic;
      sw1:   in   std_ulogic;
      sw2:   in   std_ulogic;
      sw3:   in   std_ulogic;
      sw4:   in   std_ulogic;
      sw5:   in   std_ulogic;
      sw6:   in   std_ulogic;
      sw7:   in   std_ulogic;
      sw8:   in   std_ulogic;
      sw9:   in   std_ulogic;
      sw10:  in   std_ulogic;
      sw11:  in   std_ulogic;
      sw12:  in   std_ulogic;
      sw13:  in   std_ulogic;
      --sw14:  in   std_ulogic;
      sw15:  in   std_ulogic;
      sw16:  in   std_ulogic;
      sw17:  in   std_ulogic;
      -- LED Green
      LEDg0: out  std_ulogic;  -- zero flag
      LEDg1: out  std_ulogic;  -- sign flag
      LEDg2: out  std_ulogic;  -- carry flag
      LEDg3: out  std_ulogic;  -- overflow flag
      LEDg4: out  std_ulogic;  -- equal flag
      -- LED Red
      --LEDr0: out  std_ulogic; 
      --LEDr1: out  std_ulogic; 
      --LEDr2: out  std_ulogic; 
      --LEDr3: out  std_ulogic; 
      -- HEX DISPLAY
      hex0:  out  std_ulogic_vector(6 downto 0);  -- 7 segment digit
      hex1:  out  std_ulogic_vector(6 downto 0); 
      hex2:  out  std_ulogic_vector(6 downto 0); 
      hex3:  out  std_ulogic_vector(6 downto 0);
      hex4:  out  std_ulogic_vector(6 downto 0);
      hex5:  out  std_ulogic_vector(6 downto 0);
      hex6:  out  std_ulogic_vector(6 downto 0);
      hex7:  out  std_ulogic_vector(6 downto 0)
  ); 
end;

architecture default of min16 is
    attribute chip_pin : string;
    attribute chip_pin of clk50mhz :  signal is "Y2";
    attribute chip_pin of ps2_clk :   signal is "G6";
    attribute chip_pin of ps2_data :  signal is "H5";
    attribute chip_pin of lcd_on :    signal is "L5";
    attribute chip_pin of lcd_en :    signal is "L4";
    attribute chip_pin of lcd_rw :    signal is "M1";
    attribute chip_pin of lcd_rs :    signal is "M2";
    attribute chip_pin of lcd_db :    signal is "M5,M3,K2,K1,K7,L2,L1,L3";
    attribute chip_pin of rs232_rxd : signal is "G12";
    attribute chip_pin of rs232_txd : signal is "G9";
    attribute chip_pin of rs232_cts : signal is "G14";
    attribute chip_pin of sram_dq :   signal is "AG3,AF3,AE4,AE3,AE1,AE2,AD2,AD1,AF7,AH6,AG6,AF6,AH4,AG4,AF4,AH3";
    attribute chip_pin of sram_addr : signal is "T8,AB8,AB9,AC11,AB11,AA4,AC3,AB4,AD3,AF2,T7,AF5,AC5,AB5,AE6,AB6,AC7,AE7,AD7,AB7";
    attribute chip_pin of sram_ce_N : signal is "AF8";
    attribute chip_pin of sram_oe_N : signal is "AD5";
    attribute chip_pin of sram_we_N : signal is "AE8";
    attribute chip_pin of sram_ub_N : signal is "AC4";
    attribute chip_pin of sram_lb_N : signal is "AD4";

    attribute chip_pin of key0:  signal is "M23";  -- rightmost push button
    attribute chip_pin of key1:  signal is "M21";  
    attribute chip_pin of key2:  signal is "N21";  
    attribute chip_pin of key3:  signal is "R24";

    attribute chip_pin of sw0:   signal is "AB28"; -- rightmost switch
    attribute chip_pin of sw1:   signal is "AC28";
    attribute chip_pin of sw2:   signal is "AC27";
    attribute chip_pin of sw3:   signal is "AD27";
    attribute chip_pin of sw4:   signal is "AB27";
    attribute chip_pin of sw5:   signal is "AC26";
    attribute chip_pin of sw6:   signal is "AD26";
    attribute chip_pin of sw7:   signal is "AB26";
    attribute chip_pin of sw8:   signal is "AC25";
    attribute chip_pin of sw9:   signal is "AB25";
    attribute chip_pin of sw10:  signal is "AC24";
    attribute chip_pin of sw11:  signal is "AB24";
    attribute chip_pin of sw12:  signal is "AB23";
    attribute chip_pin of sw13:  signal is "AA24";
    --attribute chip_pin of sw14:  signal is "AA23";
    --attribute chip_pin of sw15:  signal is "AA22";
    attribute chip_pin of sw16:  signal is "Y24";
    attribute chip_pin of sw17:  signal is "Y23";

    attribute chip_pin of LEDg0: signal is "E21";  -- rightmost LED Green
    attribute chip_pin of LEDg1: signal is "E22";
    attribute chip_pin of LEDg2: signal is "E25";
    attribute chip_pin of LEDg3: signal is "E24";
    attribute chip_pin of LEDg4: signal is "H21";
  
    --attribute chip_pin of LEDr0: signal is "G19";  -- rightmost LED Red
    --attribute chip_pin of LEDr1: signal is "F19";
    --attribute chip_pin of LEDr2: signal is "E19";
    --attribute chip_pin of LEDr3: signal is "F21";

    attribute chip_pin of hex0:  signal is "H22 , J22 , L25 , L26 , E17 , F22 , G18 ";
    attribute chip_pin of hex1:  signal is "U24 , U23 , W25 , W22 , W21 , Y22 , M24 ";
    attribute chip_pin of hex2:  signal is "W28 , W27 , Y26 , W26 , Y25 , AA26, AA25";
    attribute chip_pin of hex3:  signal is "Y19 , AF23, AD24, AA21, AB20, U21 , V21 ";
    attribute chip_pin of hex4:  signal is "AE18, AF19, AE19, AH21, AG21, AA19, AB19";
    attribute chip_pin of hex5:  signal is "AH18, AF18, AG19, AH19, AB18, AC18, AD18";
    attribute chip_pin of hex6:  signal is "AC17, AA15, AB15, AB17, AA16, AB16, AA17";
    attribute chip_pin of hex7:  signal is "AA14, AG18, AF17, AH17, AG17, AE17, AD17";

    constant CHARIO_CONTROL_PORT : std_logic_vector(20 downto 0) := "00000" & X"FF00";
    constant SERIAL_CHARIO_DATA_PORT : std_logic_vector(20 downto 0) := "00000" & X"FF04";
    constant PS2_LCD_CHARIO_DATA_PORT : std_logic_vector(20 downto 0) := "00000" & X"FF08";
    constant mem_thirtytwobit  : std_logic := '0';
    constant zero_hex:  std_ulogic_vector(3 downto 0) := "0000";

    signal mem_addr          : std_logic_vector(20 downto 0);
    signal mem_data_write    : std_logic_vector(31 downto 0);
    signal mem_rw            : std_logic := '0';
    signal mem_sixteenbit    : std_logic := '1';  -- default is 16 bit
    signal mem_addressready  : std_logic := '0';
    signal mem_addressready_inv  : std_logic := '0';
    signal mem_reset         : std_logic := '0';
    signal clock_hold        : std_logic := '0';
    signal clock_step        : std_logic := '0';
    signal clock_divide_limit: std_logic_vector(19 downto 0) := "00000000000000000000";
    signal mem_suspend       : std_logic := '0';

    signal mem_data_read     : std_logic_vector(31 downto 0);
    signal mem_dataready_inv : std_logic;
    signal serial_character_ready: std_logic;

    signal clk1:      std_ulogic;
    signal result:    std_ulogic_vector(15 downto 0);
    signal pc:        std_ulogic_vector(15 downto 0);
    signal instr:     std_ulogic_vector(15 downto 0);

    -- Diagnostics
    signal fsmStateCodeCpu   : std_ulogic_vector(3 downto 0);
    signal fsmStateCodeMem   : std_ulogic_vector(1 downto 0);
    signal reg1_number       : std_ulogic_vector(3 downto 0);
    signal reg2_number       : std_ulogic_vector(3 downto 0);
    signal reg1_value        : std_ulogic_vector(15 downto 0);
    signal reg2_value        : std_ulogic_vector(15 downto 0);
    signal ALU_a             : std_ulogic_vector(15 downto 0);
    signal ALU_b             : std_ulogic_vector(15 downto 0);
    signal ALU_res           : std_ulogic_vector(15 downto 0);
    signal aluflags          : std_ulogic_vector(4 downto 0);
    signal reg_din           : std_ulogic_vector(15 downto 0);
    signal branch_pc         : std_ulogic_vector(15 downto 0);
    signal jump_pc           : std_ulogic_vector(15 downto 0);
    signal seq_pc            : std_ulogic_vector(15 downto 0);
    signal new_pc            : std_ulogic_vector(15 downto 0);

begin
    mem : cscie93.memory_controller port map (
                clk50mhz => clk50mhz,
                mem_addr           => mem_addr,           -- in std_logic_vector(20 downto 0);
                mem_data_write     => mem_data_write,     -- in std_logic_vector(31 downto 0);
                mem_rw             => mem_rw,             -- in std_logic
                mem_sixteenbit     => mem_sixteenbit,     -- in std_logic
                mem_thirtytwobit   => mem_thirtytwobit,   -- in std_logic
                mem_addressready   => mem_addressready,   -- in std_logic
                mem_reset          => not key3,           -- in std_logic
                ps2_clk  => ps2_clk,
                ps2_data => ps2_data,
                clock_hold         => sw16,               -- in std_logic
                clock_step         => not key2,           -- in std_logic
                clock_divide_limit => clock_divide_limit, -- in std_logic_vector(19 downto 0)
                mem_suspend        => sw17,               -- in std_logic_vector(19 downto 0)
                lcd_en => lcd_en,
                lcd_on => lcd_on,
                lcd_rs => lcd_rs,
                lcd_rw => lcd_rw,
                lcd_db => lcd_db,
                mem_data_read      => mem_data_read,      -- out std_logic_vector(31 downto 0)
                mem_dataready_inv  => mem_dataready_inv,  -- out std_logic
                sysclk1            => clk1,               -- out std_logic
                sysclk2            => open,               -- out std_logic
                rs232_rxd => rs232_rxd,
                rs232_txd => rs232_txd,
                rs232_cts => rs232_cts,
            sram_dq   => sram_dq,
            sram_addr => sram_addr,
            sram_ce_N => sram_ce_N,
            sram_oe_N => sram_oe_N,
            sram_we_N => sram_we_N,
            sram_ub_N => sram_ub_N,
            sram_lb_N => sram_lb_N,
                serial_character_ready => serial_character_ready, -- out std_logic
                ps2_character_ready    => open                    -- out std_logic
            );


  -- clock speed definition from switch
  process (clock_divide_limit, sw1, sw2)
  begin
    if sw2 = '1' then
      clock_divide_limit <= "11100000000000000000";
    elsif sw1 = '1' then
      clock_divide_limit <= "00010000000000000000";
    else
      clock_divide_limit <= "00000000000000000010";  -- 1 or 0 did't work.
    end if;
  end process;

  --
  -- Diagnostics Pin Assignments
  --

  -- key0: cpu reset
  -- key1: reg1 or reg2, ALU_a or ALU_b
  -- key2: clock_step
  -- key3: mem_reset 

  -- sw0:  pc/instr
  -- sw1:  slow clock
  -- sw2:  slower clock
  -- sw3:  result <= mem_addr
  -- sw4:  result <= mem_data_read
  -- sw5:  result <= reg1 or reg2 value
  -- sw6:  result <= ALU_a or ALU_b
  -- sw7:  result <= ALU_res
  -- sw8:  result <= reg_din
  -- sw9:  result <= branch_pc
  -- sw10: result <= jump_pc
  -- sw11: result <= seq_pc
  -- sw12: result <= new_pc
  -- sw13: result <= mem_data_write

  -- sw16: clock_hold
  -- sw17: mem_suspend


  -- result for HEX display
  process (
      result, pc, sw0, sw3, sw4, sw5, sw6, sw7, sw8, sw9, sw10, sw11, sw12, sw13, 
      key1, ALU_a, ALU_b, ALU_res, reg_din, branch_pc, jump_pc, seq_pc, new_pc, 
      instr, mem_addr, mem_data_read, reg1_value, reg2_value
    )
  begin
    if sw0 = '1' then
      result <= pc;
    elsif sw3 = '1' then
      result <= std_ulogic_vector(mem_addr(15 downto 0));
    elsif sw4 = '1' then
      result <= std_ulogic_vector(mem_data_read(15 downto 0));
    elsif sw5 = '1' and key1 = '1' then
      result <= reg1_value;
    elsif sw5 = '1' and key1 = '0' then
      result <= reg2_value;
    elsif sw6 = '1' and key1 = '1' then
      result <= ALU_a;
    elsif sw6 = '1' and key1 = '0' then
      result <= ALU_b;
    elsif sw7 = '1' then
      result <= ALU_res;
    elsif sw8 = '1' then
      result <= reg_din;
    elsif sw9 = '1' then
      result <= branch_pc;
    elsif sw10 = '1' then
      result <= jump_pc;
    elsif sw11 = '1' then
      result <= seq_pc;
    elsif sw12 = '1' then
      result <= new_pc;
    elsif sw13 = '1' then
      result <= std_ulogic_vector(mem_data_write(15 downto 0));
    else
      result <= instr;
    end if;
  end process;

  -- memory IO
  memio: entity work.memio port map (
      clk1             => clk1,      
      reset            => not key0,
      mem_addr         => mem_addr,
      mem_data_write   => mem_data_write,
      mem_rw           => mem_rw,
      mem_sixteenbit   => mem_sixteenbit,
      mem_addressready => mem_addressready,
      mem_data_read    => mem_data_read,
      mem_dataready_inv=> mem_dataready_inv,
      serial_character_ready => serial_character_ready,
      fsmStateCodeMem  => fsmStateCodeMem
  );

  -- Instantiate your CPU entity here!
  cpu : entity work.cpu port map (
      sysclk1   => clk1,
      pc        => pc,
      instr     => instr,
      reset     => not key0,
      mem_addressready => mem_addressready,
      mem_addressready_inv => mem_addressready_inv,
      mem_addr         => mem_addr,
      mem_data_read    => mem_data_read,
      mem_data_write   => mem_data_write,
      mem_rw           => mem_rw,
      mem_sixteenbit   => mem_sixteenbit,
      reg1_number      => reg1_number,
      reg2_number      => reg2_number,
      reg1_value       => reg1_value,
      reg2_value       => reg2_value,
      ALU_a            => ALU_a,
      ALU_b            => ALU_b,
      ALU_res          => ALU_res,
      aluflags         => aluflags,
      reg_din          => reg_din,
      br_pc            => branch_pc,
      j_pc             => jump_pc,
      sq_pc            => seq_pc,
      nw_pc            => new_pc,
      fsmStateCodeCpu  => fsmStateCodeCpu
  );

  -- flags to LED Green
  LEDg0 <= aluflags(0);
  LEDg1 <= aluflags(1);
  LEDg2 <= aluflags(2);
  LEDg3 <= aluflags(3);
  LEDg4 <= aluflags(4);

  -- 7 segment LED mapping
  h0: hexdisp port map (result(3 downto 0), hex0);
  h1: hexdisp port map (result(7 downto 4), hex1);
  h2: hexdisp port map (result(11 downto 8), hex2);
  h3: hexdisp port map (result(15 downto 12), hex3);
  h4: hexdisp port map (reg2_number, hex4);
  h5: hexdisp port map (reg1_number, hex5);
  h6: hexdisp port map ("00" & fsmStateCodeMem, hex6);
  h7: hexdisp port map (fsmStateCodeCpu, hex7);

end architecture default;