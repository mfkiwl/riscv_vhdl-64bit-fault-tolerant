-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     Implementation of Debug Support Unit (DSU) with AXI4 interface.
--! @details   DSU provides access to the internal CPU registers (CSRs) via
--!            'Rocket-chip' specific bus HostIO.
-----------------------------------------------------------------------------
--! 
--! @page dsu_link Debug Support Unit (DSU)
--! 
--! @par Overview
--! Debug Support Unit (DSU) was developed to interact with "RIVER" CPU
--! via its debug port interace. This bus provides access to all internal CPU
--! registers and states and may be additionally extended by request.
--! Run control functionality like 'run', 'halt', 'step' or 'breakpoints'
--! imlemented using proprietary algorithms and intend to simplify integration
--! with debugger application.
--!
--! Set of general registers and control registers (CSR) are described in 
--! RISC-V privileged ISA specification and also available for read and write
--! access via debug port.
--!
--! @note Take into account that CPU can have any number of
--! platform specific CSRs that usually not entirely documented.
--! 
--! @par Operation
--! DSU acts like a slave AMBA AXI4 device that is directly mapped into 
--! physical memory. Default address location for our implementation 
--! is 0x80020000. DSU directly transforms device offset address
--! into one of regions of the debug port:
--!    Region 1: CSR registers.
--!    Region 2: General set of registers.
--!    Region 3: Run control and debugging registers.
--!    Region 4: Local DSU region that doesn't send into debug port.
--!
--! @par Example:
--!     Bus transaction at address <em>0x80023C10</em>
--!     will be redirected to Debug port with CSR index <em>0x782</em>.

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
library commonlib;
use commonlib.types_common.all;
--! AMBA system bus specific library.
library ambalib;
--! AXI4 configuration constants.
use ambalib.types_amba4.all;
--! RIVER CPU specific library.
library riverlib;
--! RIVER CPU configuration constants.
use riverlib.river_cfg.all;
--! River top level with AMBA interface module declaration
use riverlib.types_river.all;

entity axi_dsu is
  generic (
    xaddr    : integer := 0;
    xmask    : integer := 16#fffff#
  );
  port 
  (
    clk    : in std_logic;
    nrst   : in std_logic;
    o_cfg  : out nasti_slave_config_type;
    i_axi  : in nasti_slave_in_type;
    o_axi  : out nasti_slave_out_type;
    o_irq  : out std_logic;
    o_soft_reset : out std_logic
  );
end;

architecture arch_axi_dsu of axi_dsu is

  constant xconfig : nasti_slave_config_type := (
     descrtype => PNP_CFG_TYPE_SLAVE,
     descrsize => PNP_CFG_SLAVE_DESCR_BYTES,
     irq_idx => 0,
     xaddr => conv_std_logic_vector(xaddr, CFG_NASTI_CFG_ADDR_BITS),
     xmask => conv_std_logic_vector(xmask, CFG_NASTI_CFG_ADDR_BITS),
     vid => VENDOR_GNSSSENSOR,
     did => GNSSSENSOR_DSU
  );
  constant CSR_MRESET : std_logic_vector(11 downto 0) := X"782";

type state_type is (reading, writting);

type registers is record
  bank_axi : nasti_slave_bank_type;
  --! Message multiplexer to form 128 request message of writting into CSR
  state         : state_type;
  addr16_sel : std_logic;
  waddr : std_logic_vector(11 downto 0);
  wdata : std_logic_vector(63 downto 0);
  rdata : std_logic_vector(63 downto 0);
  -- Soft reset via CSR 0x782 MRESET doesn't work
  -- so here I implement special register that will reset CPU via 'rst' input.
  -- Otherwise I cannot load elf-file while CPU is not halted.
  soft_reset : std_logic;
end record;

signal r, rin: registers;
begin

  comblogic : process(i_axi, r)
    variable v : registers;
    variable rdata : std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0);
  begin
    v := r;

    procedureAxi4(i_axi, xconfig, r.bank_axi, v.bank_axi);
    --! redefine value 'always ready' inserting waiting states.
    v.bank_axi.rwaitready := '0';

    if r.bank_axi.wstate = wtrans then
      -- 32-bits burst transaction
      v.addr16_sel := r.bank_axi.waddr(0)(16);
      v.waddr := r.bank_axi.waddr(0)(15 downto 4);
      if r.bank_axi.wburst = NASTI_BURST_INCR and r.bank_axi.wsize = 4 then
         if r.bank_axi.waddr(0)(2) = '1' then
             v.state := writting;
             v.wdata(63 downto 32) := i_axi.w_data(31 downto 0);
         else
             v.wdata(31 downto 0) := i_axi.w_data(31 downto 0);
         end if;
      else
         -- Write data on next clock.
         if i_axi.w_strb /= X"00" then
            v.wdata := i_axi.w_data;
         end if;
         v.state := writting;
      end if;
    end if;

    case r.state is
      when reading =>
           if r.bank_axi.rstate = rtrans then
               if r.bank_axi.raddr(0)(16) = '1' then
                  -- Control registers (not implemented)
                  v.bank_axi.rwaitready := '1';
               elsif r.bank_axi.raddr(0)(15 downto 4) = CSR_MRESET then
                  v.rdata(0) := r.soft_reset;
                  v.rdata(63 downto 1) := (others => '0');
               else
               end if;
           end if;
      when writting =>
           v.state := reading;
           if r.addr16_sel = '1' then
              -- Bank with control register (not implemented by CPU)
           elsif r.waddr = CSR_MRESET then
              -- Soft Reset
              v.soft_reset := r.wdata(0);
           end if;
      when others =>
    end case;

    if r.bank_axi.raddr(0)(2) = '0' then
       rdata(31 downto 0) := r.rdata(31 downto 0);
    else
       -- 32-bits aligned access (can be generated by MAC)
       rdata(31 downto 0) := r.rdata(63 downto 32);
    end if;
    rdata(63 downto 32) := r.rdata(63 downto 32);

    o_axi <= functionAxi4Output(r.bank_axi, rdata);

    rin <= v;
  end process;

  o_cfg  <= xconfig;
  o_soft_reset <= r.soft_reset;
  o_irq <= '0';


  -- registers:
  regs : process(clk, nrst)
  begin 
    if nrst = '0' then 
       r.bank_axi <= NASTI_SLAVE_BANK_RESET;
       r.state <= reading;
       r.addr16_sel <= '0';
       r.waddr <= (others => '0');
       r.wdata <= (others => '0');
       r.rdata <= (others => '0');
       r.soft_reset <= '0';
    elsif rising_edge(clk) then 
       r <= rin; 
    end if; 
  end process;
end;