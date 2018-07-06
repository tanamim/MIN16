-- cpu entity
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use ieee.numeric_std.all;

use work.util.all;

entity cpu is
	port (
		sysclk1   			 : in     std_ulogic;
		pc        			 : buffer std_ulogic_vector(15 downto 0);
		instr     			 : buffer std_ulogic_vector(15 downto 0);
		reset     			 : in     std_ulogic;
    mem_addressready : in     std_logic;
    mem_addressready_inv : in std_logic;
    mem_addr         : inout  std_logic_vector(20 downto 0);
    mem_data_read    : inout  std_logic_vector(31 downto 0);
    mem_data_write   : inout  std_logic_vector(31 downto 0);
    mem_rw           : out    std_logic;
    mem_sixteenbit   : out    std_logic;
    reg1_number      : out    std_ulogic_vector(3 downto 0);
    reg2_number      : out    std_ulogic_vector(3 downto 0);
    reg1_value       : out    std_ulogic_vector(15 downto 0);
    reg2_value       : out    std_ulogic_vector(15 downto 0);
    ALU_a            : out    std_ulogic_vector(15 downto 0);
    ALU_b            : out    std_ulogic_vector(15 downto 0);
    ALU_res          : out    std_ulogic_vector(15 downto 0);
    aluflags         : out    std_ulogic_vector(4 downto 0);
    reg_din          : out    std_ulogic_vector(15 downto 0);
    br_pc            : out    std_ulogic_vector(15 downto 0);
    j_pc             : out    std_ulogic_vector(15 downto 0);
    sq_pc            : out    std_ulogic_vector(15 downto 0);
    nw_pc            : out    std_ulogic_vector(15 downto 0);
    fsmStateCodeCpu  : out    std_ulogic_vector(3 downto 0)
	);
end;

architecture dataflow of cpu is
	signal counter  : std_ulogic_vector(15 downto 0) := "0000000000000000";
	signal branch_PC: std_ulogic_vector(15 downto 0);
	signal jump_PC  : std_ulogic_vector(15 downto 0);
	signal seq_PC   : std_ulogic_vector(15 downto 0);
	signal new_PC   : std_ulogic_vector(15 downto 0);
	signal loadPC   : std_ulogic;
	signal loadIR   : std_ulogic;
	signal clearPC  : std_ulogic;
	signal clearIR  : std_ulogic;
	signal regWrite : std_ulogic;
	signal reg1_num : std_ulogic_vector(3 downto 0);
	signal reg2_num : std_ulogic_vector(3 downto 0);
	signal regW_num : std_ulogic_vector(3 downto 0);
	signal reg1_out : std_ulogic_vector(15 downto 0);
	signal reg2_out : std_ulogic_vector(15 downto 0);
	signal ALU_offs : std_ulogic_vector(15 downto 0);
	signal ALU_imm  : std_ulogic_vector(15 downto 0);
	signal ALU_a_in : std_ulogic_vector(15 downto 0);
	signal ALU_b_in : std_ulogic_vector(15 downto 0);
	signal ALU_out  : std_ulogic_vector(15 downto 0) := x"0000";
	signal MUL_out  : std_ulogic_vector(15 downto 0) := x"0000";
	signal SHFT_out : std_ulogic_vector(15 downto 0) := x"0000";
	signal AMS_out  : std_ulogic_vector(15 downto 0);
	signal dataIn   : std_ulogic_vector(15 downto 0);
	signal ALUStatus: std_ulogic_vector(4 downto 0);
	-- Control Signals
	signal BranchOrJumpOrSeq  : std_ulogic_vector(1 downto 0);
	signal TargetOrRegister   : std_ulogic;
	signal MemAddrFromPCOrALU : std_ulogic;
	signal Reg1From9_6Or9_7   : std_ulogic;
	signal Reg2From5_2Or6_4   : std_ulogic;
	signal RegWFromReg1OrReg2 : std_ulogic;
	signal SignOrUnsign       : std_ulogic;
	signal ALUFstOpOffsOrReg1 : std_ulogic;
	signal ALUSndOpImmOrReg2  : std_ulogic;
	signal ALUFunctionCode    : std_ulogic_vector(3 downto 0);
	signal ALUOrMULOrShift    : std_ulogic_vector(1 downto 0);
	signal WriteRegFromALUOrPCOrMem : std_ulogic_vector(1 downto 0);

	type States is (Init, MemOP, Fetch, Decode, Execute, WriteBack, LoadStoreMemSet, LoadStoreMemOP, LoadWriteBack);
	signal currentState: States := Init;

begin
	clearPC  <= reset;
	clearIR  <= reset;

  -- MUX
  reg1_num <= instr(9 downto 6) when Reg1From9_6Or9_7 = '0' else '0' & instr(9 downto 7);
  reg2_num <= instr(5 downto 2) when Reg2From5_2Or6_4 = '0' else '0' & instr(6 downto 4);
  regW_num <= x"4"     when instr(15 downto 10)  = "101001" else  -- JAL writes to $ra
						  reg1_num when RegWFromReg1OrReg2 = '0'      else 
  						reg2_num;

  ALU_offs <= std_ulogic_vector(resize(signed(instr(3 downto 0)), instr'length))(14 downto 0)&'0';
  ALU_imm  <= "0000000000"&instr(5 downto 0) when SignOrUnsign = '0' else std_ulogic_vector(resize(signed(instr(5 downto 0)), ALU_imm'length));

  ALU_a_in <= reg1_out when ALUFstOpOffsOrReg1 = '0' else ALU_offs; -- offs
  ALU_b_in <= ALU_offs when instr(15 downto 11) = "11001" else  -- SW, SB
  						reg2_out when ALUSndOpImmOrReg2  = '0' else 
  						ALU_imm;

  AMS_out  <= ALU_out  when ALUOrMULOrShift = "00" else
              MUL_out  when ALUOrMULOrShift = "01" else
              SHFT_out when ALUOrMULOrShift = "10" else 
              x"0000";

  dataIn   <= AMS_out  when WriteRegFromALUOrPCOrMem = "00" else
  						seq_PC   when WriteRegFromALUOrPCOrMem = "01" else
  						std_ulogic_vector(mem_data_read(15 downto 0)) when WriteRegFromALUOrPCOrMem = "10" else 
  						x"0000";

  jump_PC  <= "000000"&instr(9 downto 0) when TargetOrRegister = '1' else reg1_out;
  new_PC   <= branch_PC when BranchOrJumpOrSeq = "10" else
  	          jump_PC   when BranchOrJumpOrSeq = "01" else 
  	          seq_PC;

-- NOTE: IMPORTANT!!!
-- Memo: MemAddrFromPCOrALU is not switched by state = Init. Swich to ALU when Fst of LW, LB, SW, SB, Snd should be PC.
-- Memo: Load and Store has 2 memory operations: read register/read memory(Load), and read register and write memory(Store)
--
	mem_rw             <= '1'  when instr(15 downto 11) = "11001" and currentState = LoadStoreMemOP else '0'; -- SW, SB

	mem_sixteenbit     <= '0'  when instr(15 downto 10) = "110001" and currentState = LoadStoreMemOP else -- LB
												'0'  when instr(15 downto 10) = "110011" and currentState = LoadStoreMemOP else -- SB
	                      '1';

	MemAddrFromPCOrALU <= '1'  when instr(15 downto 12) = "1100" and currentState = LoadStoreMemOP else '0';  -- LW, LB, SW, SB

  mem_addr <= "00000"&std_logic_vector(pc) when MemAddrFromPCOrALU = '0' else "00000"&std_logic_vector(AMS_out); 
	
	mem_data_write <= x"0000"&std_logic_vector(reg2_out);  -- SW, SB


	-- Registers
	reg_PC:  reg16 port map (load=>loadPC, reset=>clearPC, din=>new_PC, dout=>pc);
	reg_IR:  reg16 port map (load=>loadIR, reset=>clearIR, din=>std_ulogic_vector(mem_data_read(15 downto 0)), dout=>instr);
	reg_Arr: entity work.regFile port map (
		regWrite => regWrite, 
		reset    => reset, 
		reg1_num => reg1_num, 
		reg2_num => reg2_num, 
		regW_num => regW_num, 
		dataIn   => dataIn, 
		reg1_out => reg1_out, 
		reg2_out => reg2_out
	);

  -- Adder for PC (sequential, branch)
  add0: addr16 port map (a=>pc, b=>x"0002", cin=>'0', q=>seq_PC, cout=>open);     
  add1: addr16 port map (a=>ALU_offs, b=>pc, cin=>'0', q=>branch_PC, cout=>open); 

  -- ALU, Multiplier, BarrelShifter
  alu0:  alu     port map (alucode=>ALUFunctionCode, rd=>ALU_a_in, rs=>ALU_b_in, result=>ALU_out,  flags=>ALUStatus);
  mul0:  mul     port map (alucode=>ALUFunctionCode, rd=>ALU_a_in, rs=>ALU_b_in, result=>MUL_out,  flags=>open);
  shft0: shifter port map (alucode=>ALUFunctionCode, rd=>ALU_a_in, rs=>ALU_b_in, result=>SHFT_out, flags=>open);

  -- Sequencer
  seq0: entity work.seq port map ( 
  	opfunc          				 =>	instr(15 downto 10), 
  	ALUStatus								 =>	ALUStatus,
  	BranchOrJumpOrSeq				 =>	BranchOrJumpOrSeq,
  	TargetOrRegister				 =>	TargetOrRegister,
  	--MemAddrFromPCOrALU			 =>	MemAddrFromPCOrALU,
  	--MemReadOrWrite					 =>	mem_rw,
  	--MemSixteenbit 					 =>	mem_sixteenbit,
  	Reg1From9_6Or9_7				 =>	Reg1From9_6Or9_7,
  	Reg2From5_2Or6_4				 =>	Reg2From5_2Or6_4,
  	RegWFromReg1OrReg2			 =>	RegWFromReg1OrReg2,
  	SignOrUnsign						 =>	SignOrUnsign,
  	ALUFstOpOffsOrReg1			 =>	ALUFstOpOffsOrReg1,
  	ALUSndOpImmOrReg2				 =>	ALUSndOpImmOrReg2,
  	ALUFunctionCode					 =>	ALUFunctionCode,
  	ALUOrMULOrShift					 =>	ALUOrMULOrShift,
  	WriteRegFromALUOrPCOrMem =>	WriteRegFromALUOrPCOrMem
  );


	FSM: process(sysclk1, reset, mem_addressready)
	begin
		if reset = '1' then
			currentState <= Init;

		elsif rising_edge(sysclk1) then
		 case currentState is
		 	when Init =>      -- (0) loadPC on falling_edge
		 		if mem_addressready = '1' then
	 				currentState <= MemOP;
		 		end if;

		 	when MemOP =>     -- (1)
		 		if mem_addressready = '0' then
		 			currentState <= Fetch;
		 		end if;

		 	when Fetch =>     -- (2) loadIR on falling_edge
	 			currentState <= Decode;
	 	
	 		when Decode =>    -- (3)
	 			if instr(15 downto 12) = "1100" then  -- LW, LB, SW, SB
	 				currentState <= LoadStoreMemSet;
	 			else
		 			currentState <= Execute;
	 			end if;

	 		when Execute =>   -- (4) regWrite on falling_edge
	 			--currentState <= WriteBack;
	 			currentState <= Init;

	 		-- NOTE: WriteBack may not needed. falling_edge of Execute can do the job.
	 		when WriteBack => -- (5) regWrite on falling_edge
	 			currentState <= Init;

	 		-- LW, LB, SW, SB
	 		when LoadStoreMemSet =>  -- (6)
	 			if mem_addressready = '1' then
	 				currentState <= LoadStoreMemOP;
	 			end if;

	 		when LoadStoreMemOP =>   -- (7)
	 			if mem_addressready = '0' then
		 			if instr(15 downto 11) = "11000" then -- LW, LB
		 				currentState <= LoadWriteBack;
		 			else
		 				currentState <= Init;
		 			end if;
		 		end if;

		 	when LoadWriteBack =>    -- (8) regWrite on falling_edge
		 		currentState <= Init;
		 	end case;

		 elsif falling_edge(sysclk1) then
		  case currentState is
		  	when Init =>   -- (0)
		  		loadPC   <= '1';
		  		loadIR   <= '0';
		  		regWrite <= '0';		  		
		  	when Fetch =>  -- (2)
		  		loadPC   <= '0';
		  		loadIR   <= '1';
		  		regWrite <= '0';
		  	when Execute => -- (4)
		  	--when WriteBack => -- (5)
		  		loadPC   <= '0';
		  		loadIR   <= '0';
		  		if WriteRegFromALUOrPCOrMem = "11" then
						regWrite <= '0';
					else
		  		  regWrite <= '1';
		  		end if;
		  	when LoadWriteBack => -- (8)
		  		loadPC   <= '0';
		  		loadIR   <= '0';
	  		  regWrite <= '1';
		  	when others =>
		  		loadPC   <= '0';
		  		loadIR   <= '0';		  	
		  		regWrite <= '0';
		 	end case;

		 end if;
	end process;

	-- Diagnostics
	reg1_number <= reg1_num;
	reg2_number <= reg2_num;
	reg1_value  <= reg1_out;
	reg2_value  <= reg2_out;
	ALU_a       <= ALU_a_in;
	ALU_b       <= ALU_b_in;
	ALU_res     <= ALU_out;
	aluflags    <= ALUStatus;
	reg_din     <= dataIn;
	br_pc       <= branch_PC;
	j_pc        <= jump_PC;
	sq_pc       <= seq_PC;
	nw_pc       <= new_PC;	

	with currentState select
		fsmStateCodeCpu <=	"0000" when Init,
		    					 		 	"0001" when MemOP,
		    								"0010" when Fetch,
		    								"0011" when Decode,
		    								"0100" when Execute,
		    								"0101" when WriteBack,
		    								"0110" when LoadStoreMemSet,
		    								"0111" when LoadStoreMemOP,
		    								"1000" when LoadWriteBack,
		    								"1111" when others;

end architecture dataflow;