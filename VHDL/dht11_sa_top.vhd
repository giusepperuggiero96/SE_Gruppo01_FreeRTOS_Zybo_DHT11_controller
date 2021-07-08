library unisim;
use unisim.vcomponents.all;


library ieee;
use ieee.std_logic_1164.all;

entity dht11_sa_top is
  generic(
    freq:    positive range 1 to 1000 -- Clock frequency (MHz)
  );
  port(
    clk:      in    std_ulogic;
    rst:      in    std_ulogic; -- Active high synchronous reset
    start:    in    std_ulogic;
    busy_bit: out   std_ulogic;
    check_err:out   std_ulogic;
    p_error:  out   std_ulogic;
    data:     inout std_logic;
    data_reg: out   std_ulogic_vector(31 downto 0) 
  );
end entity dht11_sa_top;

architecture rtl of dht11_sa_top is

  signal data_in:  std_ulogic;
  signal data_drv: std_ulogic;
  signal data_drvn: std_ulogic;

begin

  u0: entity work.dht11_sa(rtl)
  generic map(
    freq => freq
  )
  port map(
    clk       => clk,
    rst       => rst,
    start     => start,
    busy_bit  => busy_bit,
    check_err => check_err,
    p_error   => p_error,
    data(31 downto 0)=> data_reg(31 downto 0),
    data_in  => data_in,
    data_drv => data_drv
  );

  u1 : iobuf
    generic map (
      drive => 12,
      iostandard => "lvcmos33",
      slew => "slow")
    port map (
      o  => data_in,
      io => data,
      i  => '0',
      t  => data_drvn
  );

data_drvn <= not data_drv;

end architecture rtl;
