library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity dht11_sa is
  generic(
    freq:    positive range 1 to 1000 -- Clock frequency (MHz)
  );
  port(
    clk:      in  std_ulogic;
    rst:      in  std_ulogic;                    -- Active high synchronous reset
    start:    in  std_ulogic;
    busy_bit: out std_ulogic;
    check_err:out std_ulogic;
    p_error:  out std_ulogic;
    data_in:  in  std_ulogic;
    data_drv: out std_ulogic;
    data:     out std_ulogic_vector(31 downto 0)
  );
end entity dht11_sa;

architecture rtl of dht11_sa is

  signal pe                : std_ulogic;
  signal b		   : std_ulogic;
  signal do                : std_ulogic_vector(39 downto 0);
  signal sipo_out_mux_in   : std_ulogic_vector(31 downto 0);
  signal checksum          : std_ulogic_vector(7 downto 0);
  signal srstn		   : std_ulogic;
begin
	
  srstn <= NOT(rst);

  u0: entity work.dht11_ctrl(rtl)
  generic map(
    freq => freq
  )
  port map(
    clk      => clk,
    srstn    => srstn,
    start    => start,
    data_in  => data_in,
    data_drv => data_drv,
    pe       => pe,
    b        => b,
    do       => do
  );

  sipo_out_mux_in <= do(39 DOWNTO 8);
  checksum <= do(7 DOWNTO 0);

  Checksum_controller: process(checksum, sipo_out_mux_in)
  variable sum: unsigned(7 DOWNTO 0);
  variable sum_int: std_ulogic_vector(7 downto 0);
  begin
    sum:= unsigned(sipo_out_mux_in(31 DOWNTO 24)) + unsigned(sipo_out_mux_in(23 DOWNTO 16)) + unsigned(sipo_out_mux_in(15 DOWNTO 8)) + unsigned(sipo_out_mux_in(7 DOWNTO 0));
    sum_int:= std_ulogic_vector(sum);
    if sum_int = checksum then
      check_err <= '0';
    else
      check_err <= '1';
    end if;
  end process Checksum_controller;

  busy_bit <= b;
  p_error <= pe;
  data(31 downto 0) <= sipo_out_mux_in(31 downto 0);

end architecture rtl;
