-- memory io entity
library ieee;
use ieee.std_logic_1164.all;

entity memio is
	port (
		clk1   : in  std_logic;
		reset  : in  std_logic;
        mem_addr         : inout std_logic_vector(20 downto 0);
        mem_data_write   : inout std_logic_vector(31 downto 0);
        mem_rw           : inout std_logic;
        mem_sixteenbit   : inout std_logic;
        mem_addressready : out   std_logic;
        mem_data_read    : inout std_logic_vector(31 downto 0);
        mem_dataready_inv: in    std_logic;
        serial_character_ready: in std_logic;
        fsmStateCodeMem  : out   std_ulogic_vector(1 downto 0)  -- DEBUG
	);
end;

architecture dataflow of memio is

	type States is (MemWait, MemSet, MemReadWrite);
	signal currentState: States := MemWait;

begin

	FSM: process(clk1, reset, mem_dataready_inv)
	begin
		if reset = '1' then
			currentState <= MemWait;
		elsif rising_edge(clk1) then
			case currentState is
				when MemWait =>
					if mem_dataready_inv = '1' then
						currentState <= MemSet;
					end if;		

				when MemSet =>
					mem_addressready <= '1';
					if mem_dataready_inv = '0' then
						currentState <= MemReadWrite;
					end if;

				when MemReadWrite =>
					mem_addressready <= '0';
					currentState <= MemWait;
			end case;
		end if;
	end process;


	-- Diagnostics
	with currentState select
		fsmStateCodeMem <= "00" when MemWait,
		    							 "01" when MemSet,
		    							 "10" when MemReadWrite,
		    							 "11" when others;

end architecture dataflow;