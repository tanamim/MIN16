-- sequencer entity
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use ieee.numeric_std.all;

use work.util.all;

entity seq is
	port (
			opfunc   : in std_ulogic_vector(5 downto 0);
			ALUStatus: in std_ulogic_vector(4 downto 0);
			BranchOrJumpOrSeq  : out std_ulogic_vector(1 downto 0);
			TargetOrRegister   : out std_ulogic;
			--MemAddrFromPCOrALU : out std_ulogic;
			--MemReadOrWrite     : out std_ulogic;
			--MemSixteenbit      : out std_ulogic;
			Reg1From9_6Or9_7   : buffer std_ulogic;
			Reg2From5_2Or6_4   : out std_ulogic;
			RegWFromReg1OrReg2 : out std_ulogic;
			SignOrUnsign       : out std_ulogic;
			ALUFstOpOffsOrReg1 : out std_ulogic;
			ALUSndOpImmOrReg2  : out std_ulogic;
			ALUFunctionCode    : out std_ulogic_vector(3 downto 0);
			ALUOrMULOrShift    : out std_ulogic_vector(1 downto 0);
			WriteRegFromALUOrPCOrMem : out std_ulogic_vector(1 downto 0)
	);
end;

architecture dataflow of seq is

begin
	-- Branch: 10, Jump: 01, Sequential: 00
	BranchOrJumpOrSeq  <= "10" when opfunc(5 downto 0) = "101100" and ALUStatus(4) = '1' else -- BEQ
												"10" when opfunc(5 downto 0) = "101101" and ALUStatus(4) = '0' else -- BNE
										 	  "01" when opfunc(5 downto 2) = "1010" else -- J, JAL, JR, JALR
											  "00";
  -- Target: 1								(J, JAL)
	TargetOrRegister   <= '1'  when opfunc(5 downto 1) = "10100" else '0';
	-- ALU: 1										(LW, LB, SW, SB)
	--MemAddrFromPCOrALU <= '1'  when opfunc(5 downto 2) = "1100"  else '0';
  -- Write: 1									(SW, SB)
	--MemReadOrWrite     <= '1'  when opfunc(5 downto 1) = "11001" else '0';
  -- 16bit: 1									(LB, SB)
	--MemSixteenbit      <= '0'  when opfunc(5 downto 0) = "110001" or opfunc(5 downto 0) = "110011" else '1';
  -- O-type: 1 								(BEQ, BNE, LW, LB, SW, SB)
	Reg1From9_6Or9_7   <= '1'  when opfunc(5 downto 2) = "1011" or opfunc(5 downto 2) = "1100" else '0';
	Reg2From5_2Or6_4   <= Reg1From9_6Or9_7;
  -- Write from Reg2: '1' 		(JALR)
	RegWFromReg1OrReg2 <= '1'  when opfunc(5 downto 0) = "101011" else '0';
	-- Signed Immediate: '1' 		(ADDI, SUBI, MULI, SLTI, SRAI)
	SignOrUnsign       <= '1'  when opfunc(5 downto 2) = "0100" or opfunc(5 downto 0) = "011110" else '0';
	-- Store/Load: 1            (LW, LB)
	ALUFstOpOffsOrReg1 <= '1'  when opfunc(5 downto 1) = "11000" else '0';	
	-- I-Type: '1' 							(ADDI, SUBI, MULI, SLTI, ADDIU, SUBIU, MULIU, SLTIU, ANDI, ORI, XORI, NORI, SLLI, SRLI, SRAI, ROTLI)
	-- opfunc(5 downto 4): "00" is ALU R-type, "01" is ALU I-type
	ALUSndOpImmOrReg2  <= '1'  when opfunc(5 downto 4) = "01" else '0'; 

	ALUFunctionCode    <= "0000" when opfunc(5 downto 2) = "1011" or opfunc(5 downto 4) = "11" else
 												opfunc(3 downto 0);

	-- non ALU: 11, Shift: 10, MUL: 01, other ALU: "00"
	ALUOrMULOrShift    <= "11" when opfunc(5 downto 2) = "1010" or opfunc(5 downto 2) = "1101" else -- non ALU, MUL, Shift
												"10" when opfunc(3 downto 0) = "1111" else -- ROT, ROTI
												"01" when opfunc[3] = '0' and opfunc(1 downto 0) = "10" else -- MUL, MULU, MULI, MULIU
												"00";
	-- ALUMULShift: "00", PC: "01", Memory: "10"
  WriteRegFromALUOrPCOrMem <= "00" when opfunc(5) = '0' else -- ALU, MUL, Shift
  												 		"01" when opfunc(5 downto 0) = "101001" or opfunc(5 downto 0) = "101011" else -- JAL, JALR
  												 		"10" when opfunc(5 downto 1) = "11000" else -- LW, LB
  												 		"11";  -- Don't write register
end architecture dataflow;
