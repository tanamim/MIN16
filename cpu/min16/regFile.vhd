-- register array entity
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use ieee.numeric_std.all;

use work.util.all;

entity regFile is
	port (
			regWrite : in  std_ulogic;  --  0 is read
			reset    : in  std_ulogic;
			reg1_num : in  std_ulogic_vector(3 downto 0);			
			reg2_num : in  std_ulogic_vector(3 downto 0);			
			regW_num : in  std_ulogic_vector(3 downto 0);			
			dataIn   : in  std_ulogic_vector(15 downto 0);
			reg1_out : out std_ulogic_vector(15 downto 0);
			reg2_out : out std_ulogic_vector(15 downto 0)
	);
end;

architecture dataflow of regFile is
	constant reg_r0: std_ulogic_vector := "0000000000000000";
	signal reg_at : std_ulogic_vector(15 downto 0);
	signal reg_sp : std_ulogic_vector(15 downto 0);
	signal reg_fp : std_ulogic_vector(15 downto 0);
	signal reg_ra : std_ulogic_vector(15 downto 0);
	signal reg_rb : std_ulogic_vector(15 downto 0);
	signal reg_rc : std_ulogic_vector(15 downto 0);
	signal reg_rd : std_ulogic_vector(15 downto 0);
	signal reg_s0 : std_ulogic_vector(15 downto 0);
	signal reg_s1 : std_ulogic_vector(15 downto 0);
	signal reg_t0 : std_ulogic_vector(15 downto 0);
	signal reg_t1 : std_ulogic_vector(15 downto 0);
	signal reg_hi : std_ulogic_vector(15 downto 0);
	signal reg_lo : std_ulogic_vector(15 downto 0);

	signal load_at: std_ulogic;
	signal load_sp: std_ulogic;
	signal load_fp: std_ulogic;
	signal load_ra: std_ulogic;
	signal load_rb: std_ulogic;
	signal load_rc: std_ulogic;
	signal load_rd: std_ulogic;
	signal load_s0: std_ulogic;
	signal load_s1: std_ulogic;
	signal load_t0: std_ulogic;
	signal load_t1: std_ulogic;
	signal load_hi: std_ulogic;
	signal load_lo: std_ulogic;

begin
  -- Read Register
  reg1_out <= reg_at when reg1_num=x"1" else
  						reg_sp when reg1_num=x"2" else
  						reg_fp when reg1_num=x"3" else
  						reg_ra when reg1_num=x"4" else
  						reg_rb when reg1_num=x"5" else
  						reg_rc when reg1_num=x"6" else
  						reg_rd when reg1_num=x"7" else
  						reg_s0 when reg1_num=x"8" else
  						reg_s1 when reg1_num=x"9" else
  						reg_t0 when reg1_num=x"a" else
  						reg_t1 when reg1_num=x"b" else
  						reg_hi when reg1_num=x"c" else
  						reg_lo when reg1_num=x"d" else
							reg_r0;

  reg2_out <= reg_at when reg2_num=x"1" else
  						reg_sp when reg2_num=x"2" else
  						reg_fp when reg2_num=x"3" else
  						reg_ra when reg2_num=x"4" else
  						reg_rb when reg2_num=x"5" else
  						reg_rc when reg2_num=x"6" else
  						reg_rd when reg2_num=x"7" else
  						reg_s0 when reg2_num=x"8" else
  						reg_s1 when reg2_num=x"9" else
  						reg_t0 when reg2_num=x"a" else
  						reg_t1 when reg2_num=x"b" else
  						reg_hi when reg2_num=x"c" else
  						reg_lo when reg2_num=x"d" else
							reg_r0;

	-- Write to Register
	load_at <= '1' when regW_num=x"1" and regWrite='1' else '0';
	load_sp <= '1' when regW_num=x"2" and regWrite='1' else '0';
	load_fp <= '1' when regW_num=x"3" and regWrite='1' else '0';
	load_ra <= '1' when regW_num=x"4" and regWrite='1' else '0';
	load_rb <= '1' when regW_num=x"5" and regWrite='1' else '0';
	load_rc <= '1' when regW_num=x"6" and regWrite='1' else '0';
	load_rd <= '1' when regW_num=x"7" and regWrite='1' else '0';
	load_s0 <= '1' when regW_num=x"8" and regWrite='1' else '0';
	load_s1 <= '1' when regW_num=x"9" and regWrite='1' else '0';
	load_t0 <= '1' when regW_num=x"a" and regWrite='1' else '0';
	load_t1 <= '1' when regW_num=x"b" and regWrite='1' else '0';
	load_hi <= '1' when regW_num=x"c" and regWrite='1' else '0';
	load_lo <= '1' when regW_num=x"d" and regWrite='1' else '0';

	-- General Purpose Registers
	at: reg16 port map (load=>load_at, reset=>reset, din=>dataIn, dout=>reg_at);
	sp: reg16 port map (load=>load_sp, reset=>reset, din=>dataIn, dout=>reg_sp);
	fp: reg16 port map (load=>load_fp, reset=>reset, din=>dataIn, dout=>reg_fp);
	ra: reg16 port map (load=>load_ra, reset=>reset, din=>dataIn, dout=>reg_ra);
	rb: reg16 port map (load=>load_rb, reset=>reset, din=>dataIn, dout=>reg_rb);
	rc: reg16 port map (load=>load_rc, reset=>reset, din=>dataIn, dout=>reg_rc);
	rd: reg16 port map (load=>load_rd, reset=>reset, din=>dataIn, dout=>reg_rd);
	s0: reg16 port map (load=>load_s0, reset=>reset, din=>dataIn, dout=>reg_s0);
	s1: reg16 port map (load=>load_s1, reset=>reset, din=>dataIn, dout=>reg_s1);
	t0: reg16 port map (load=>load_t0, reset=>reset, din=>dataIn, dout=>reg_t0);
	t1: reg16 port map (load=>load_t1, reset=>reset, din=>dataIn, dout=>reg_t1);
	hi: reg16 port map (load=>load_hi, reset=>reset, din=>dataIn, dout=>reg_hi);
	lo: reg16 port map (load=>load_lo, reset=>reset, din=>dataIn, dout=>reg_lo);
	
end architecture dataflow;
