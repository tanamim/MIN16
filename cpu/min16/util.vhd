-- utility package
library ieee;
use ieee.std_logic_1164.all;

package util is

  component addr01 is
    port (a, b, cin: in  std_ulogic;
          q, cout:   out std_ulogic
          );
  end component addr01;

  component addr04 is
    port (a, b:      in  std_ulogic_vector(3 downto 0);
          cin:       in  std_ulogic;
          q:         out std_ulogic_vector(3 downto 0);
          cout:      out std_ulogic
          );
    end component addr04;

  component addr08 is
    port (a, b:      in  std_ulogic_vector(7 downto 0);
          cin:       in  std_ulogic;
          q:         out std_ulogic_vector(7 downto 0);
          cout:      out std_ulogic
          );
    end component addr08;

  component addr16 is
    port (a, b:      in  std_ulogic_vector(15 downto 0);
          cin:       in  std_ulogic;
          q:         out std_ulogic_vector(15 downto 0);
          cout:      out std_ulogic
          );
  end component addr16;


  component reg01 is
    port (load, reset: in  std_ulogic;
          din:  in  std_ulogic;
          dout: out std_ulogic
          );
  end component reg01;

  component reg08 is
    port (load, reset: in  std_ulogic;
          din:  in  std_ulogic_vector(7 downto 0);
          dout: out std_ulogic_vector(7 downto 0)
          );
  end component reg08;

  component reg16 is
    port (
          load, reset: in  std_ulogic;
          din:  in  std_ulogic_vector(15 downto 0);
          dout: out std_ulogic_vector(15 downto 0)
          );
  end component reg16;


  component hexdisp is
    port (num: in     std_ulogic_vector(3 downto 0);
          hex: buffer std_ulogic_vector(6 downto 0)
          );
  end component hexdisp;


  component hexLED is
  port (val0: in     std_ulogic_vector(15 downto 0);
        val1: in     std_ulogic_vector(7 downto 0);
        val2: in     std_ulogic_vector(7 downto 0);
        hex0: out    std_ulogic_vector(6 downto 0);
        hex1: out    std_ulogic_vector(6 downto 0); 
        hex2: out    std_ulogic_vector(6 downto 0); 
        hex3: out    std_ulogic_vector(6 downto 0);
        hex4: out    std_ulogic_vector(6 downto 0);
        hex5: out    std_ulogic_vector(6 downto 0);
        hex6: out    std_ulogic_vector(6 downto 0);
        hex7: out    std_ulogic_vector(6 downto 0)
        );
  end component hexLED;


  component alu is
    port (alucode: in     std_ulogic_vector(3 downto 0);
          rd:      in     std_ulogic_vector(15 downto 0);
          rs:      in     std_ulogic_vector(15 downto 0);
          result:  out    std_ulogic_vector(15 downto 0);
          flags:   out    std_ulogic_vector(4 downto 0)
         );
  end component alu;


end util;