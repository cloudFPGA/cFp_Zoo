--  *
--  *                       cloudFPGA
--  *     Copyright IBM Research, All Rights Reserved
--  *    =============================================
--  *     Created: Apr 2019
--  *     Authors: FAB, WEI, NGL
--  *
--  *     Description:
--  *       ROLE template for Themisto SRA
--  *

--******************************************************************************
--**  CONTEXT CLAUSE  **  FMKU60 ROLE(Flash)
--******************************************************************************
library IEEE;
use     IEEE.std_logic_1164.all;
use     IEEE.numeric_std.all;

library UNISIM; 
use     UNISIM.vcomponents.all;

-- library XIL_DEFAULTLIB;
-- use     XIL_DEFAULTLIB.all;


--******************************************************************************
--**  ENTITY  **  FMKU60 ROLE
--******************************************************************************

entity Role_Themisto is
  generic (
    gAxiIdWidth          : integer :=  8
  );
  port (

    --------------------------------------------------------
    -- SHELL / Global Input Clock and Reset Interface
    --------------------------------------------------------
    piSHL_156_25Clk                     : in    std_ulogic;
    piSHL_156_25Rst                     : in    std_ulogic;
    -- LY7 Enable and Reset
    piMMIO_Ly7_Rst                      : in    std_ulogic;
    piMMIO_Ly7_En                       : in    std_ulogic;

    ------------------------------------------------------
    -- SHELL / Role / Nts0 / Udp Interface
    ------------------------------------------------------
    ---- Input AXI-Write Stream Interface ----------
    siNRC_Udp_Data_tdata       : in    std_ulogic_vector( 63 downto 0);
    siNRC_Udp_Data_tkeep       : in    std_ulogic_vector(  7 downto 0);
    siNRC_Udp_Data_tvalid      : in    std_ulogic;
    siNRC_Udp_Data_tlast       : in    std_ulogic;
    siNRC_Udp_Data_tready      : out   std_ulogic;
    ---- Output AXI-Write Stream Interface ---------
    soNRC_Udp_Data_tdata       : out   std_ulogic_vector( 63 downto 0);
    soNRC_Udp_Data_tkeep       : out   std_ulogic_vector(  7 downto 0);
    soNRC_Udp_Data_tvalid      : out   std_ulogic;
    soNRC_Udp_Data_tlast       : out   std_ulogic;
    soNRC_Udp_Data_tready      : in    std_ulogic;
    -- Open Port vector
    poROL_Nrc_Udp_Rx_ports     : out    std_ulogic_vector( 31 downto 0);
    -- ROLE <-> NRC Meta Interface
    soROLE_Nrc_Udp_Meta_TDATA   : out   std_ulogic_vector( 63 downto 0);
    soROLE_Nrc_Udp_Meta_TVALID  : out   std_ulogic;
    soROLE_Nrc_Udp_Meta_TREADY  : in    std_ulogic;
    soROLE_Nrc_Udp_Meta_TKEEP   : out   std_ulogic_vector(  7 downto 0);
    soROLE_Nrc_Udp_Meta_TLAST   : out   std_ulogic;
    siNRC_Role_Udp_Meta_TDATA   : in    std_ulogic_vector( 63 downto 0);
    siNRC_Role_Udp_Meta_TVALID  : in    std_ulogic;
    siNRC_Role_Udp_Meta_TREADY  : out   std_ulogic;
    siNRC_Role_Udp_Meta_TKEEP   : in    std_ulogic_vector(  7 downto 0);
    siNRC_Role_Udp_Meta_TLAST   : in    std_ulogic;
      
    ------------------------------------------------------
    -- SHELL / Role / Nts0 / Tcp Interface
    ------------------------------------------------------
    ---- Input AXI-Write Stream Interface ----------
    siNRC_Tcp_Data_tdata       : in    std_ulogic_vector( 63 downto 0);
    siNRC_Tcp_Data_tkeep       : in    std_ulogic_vector(  7 downto 0);
    siNRC_Tcp_Data_tvalid      : in    std_ulogic;
    siNRC_Tcp_Data_tlast       : in    std_ulogic;
    siNRC_Tcp_Data_tready      : out   std_ulogic;
    ---- Output AXI-Write Stream Interface ---------
    soNRC_Tcp_Data_tdata       : out   std_ulogic_vector( 63 downto 0);
    soNRC_Tcp_Data_tkeep       : out   std_ulogic_vector(  7 downto 0);
    soNRC_Tcp_Data_tvalid      : out   std_ulogic;
    soNRC_Tcp_Data_tlast       : out   std_ulogic;
    soNRC_Tcp_Data_tready      : in    std_ulogic;
    -- Open Port vector
    poROL_Nrc_Tcp_Rx_ports     : out    std_ulogic_vector( 31 downto 0);
    -- ROLE <-> NRC Meta Interface
    soROLE_Nrc_Tcp_Meta_TDATA   : out   std_ulogic_vector( 63 downto 0);
    soROLE_Nrc_Tcp_Meta_TVALID  : out   std_ulogic;
    soROLE_Nrc_Tcp_Meta_TREADY  : in    std_ulogic;
    soROLE_Nrc_Tcp_Meta_TKEEP   : out   std_ulogic_vector(  7 downto 0);
    soROLE_Nrc_Tcp_Meta_TLAST   : out   std_ulogic;
    siNRC_Role_Tcp_Meta_TDATA   : in    std_ulogic_vector( 63 downto 0);
    siNRC_Role_Tcp_Meta_TVALID  : in    std_ulogic;
    siNRC_Role_Tcp_Meta_TREADY  : out   std_ulogic;
    siNRC_Role_Tcp_Meta_TKEEP   : in    std_ulogic_vector(  7 downto 0);
    siNRC_Role_Tcp_Meta_TLAST   : in    std_ulogic;
    
    
    --------------------------------------------------------
    -- SHELL / Mem / Mp0 Interface
    --------------------------------------------------------
    ---- Memory Port #0 / S2MM-AXIS ----------------   
    ------ Stream Read Command ---------
    soMEM_Mp0_RdCmd_tdata           : out   std_ulogic_vector( 79 downto 0);
    soMEM_Mp0_RdCmd_tvalid          : out   std_ulogic;
    soMEM_Mp0_RdCmd_tready          : in    std_ulogic;
    ------ Stream Read Status ----------
    siMEM_Mp0_RdSts_tdata           : in    std_ulogic_vector(  7 downto 0);
    siMEM_Mp0_RdSts_tvalid          : in    std_ulogic;
    siMEM_Mp0_RdSts_tready          : out   std_ulogic;
    ------ Stream Data Input Channel ---
    siMEM_Mp0_Read_tdata            : in    std_ulogic_vector(511 downto 0);
    siMEM_Mp0_Read_tkeep            : in    std_ulogic_vector( 63 downto 0);
    siMEM_Mp0_Read_tlast            : in    std_ulogic;
    siMEM_Mp0_Read_tvalid           : in    std_ulogic;
    siMEM_Mp0_Read_tready           : out   std_ulogic;
    ------ Stream Write Command --------
    soMEM_Mp0_WrCmd_tdata           : out   std_ulogic_vector( 79 downto 0);
    soMEM_Mp0_WrCmd_tvalid          : out   std_ulogic;
    soMEM_Mp0_WrCmd_tready          : in    std_ulogic;
    ------ Stream Write Status ---------
    siMEM_Mp0_WrSts_tdata           : in    std_ulogic_vector(  7 downto 0);
    siMEM_Mp0_WrSts_tvalid          : in    std_ulogic;
    siMEM_Mp0_WrSts_tready          : out   std_ulogic;
    ------ Stream Data Output Channel --
    soMEM_Mp0_Write_tdata           : out   std_ulogic_vector(511 downto 0);
    soMEM_Mp0_Write_tkeep           : out   std_ulogic_vector( 63 downto 0);
    soMEM_Mp0_Write_tlast           : out   std_ulogic;
    soMEM_Mp0_Write_tvalid          : out   std_ulogic;
    soMEM_Mp0_Write_tready          : in    std_ulogic; 
    
    --------------------------------------------------------
    -- SHELL / Mem / Mp1 Interface
    --------------------------------------------------------
    moMEM_Mp1_AWID                  : out   std_ulogic_vector(gAxiIdWidth-1 downto 0);
    moMEM_Mp1_AWADDR                : out   std_ulogic_vector(32 downto 0);
    moMEM_Mp1_AWLEN                 : out   std_ulogic_vector(7 downto 0);
    moMEM_Mp1_AWSIZE                : out   std_ulogic_vector(2 downto 0);
    moMEM_Mp1_AWBURST               : out   std_ulogic_vector(1 downto 0);
    --moMEM_Mp1_AWLOCK                : out   std_ulogic_vector(1 downto 0);
    --moMEM_Mp1_AWREGION              : out   std_ulogic_vector(3 downto 0);
    --moMEM_Mp1_AWCACHE               : out   std_ulogic_vector(3 downto 0);
    --moMEM_Mp1_AWPROT                : out   std_ulogic_vector(2 downto 0);
    --moMEM_Mp1_AWQOS                 : out   std_ulogic_vector(3 downto 0);
    moMEM_Mp1_AWVALID               : out   std_ulogic;
    moMEM_Mp1_AWREADY               : in    std_ulogic;
    moMEM_Mp1_WDATA                 : out   std_ulogic_vector(511 downto 0);
    moMEM_Mp1_WSTRB                 : out   std_ulogic_vector(63 downto 0);
    moMEM_Mp1_WLAST                 : out   std_ulogic;
    moMEM_Mp1_WVALID                : out   std_ulogic;
    moMEM_Mp1_WREADY                : in    std_ulogic;
    moMEM_Mp1_BID                   : in    std_ulogic_vector(gAxiIdWidth-1 downto 0);
    moMEM_Mp1_BRESP                 : in    std_ulogic_vector(1 downto 0);
    moMEM_Mp1_BVALID                : in    std_ulogic;
    moMEM_Mp1_BREADY                : out   std_ulogic;
    moMEM_Mp1_ARID                  : out   std_ulogic_vector(gAxiIdWidth-1 downto 0);
    moMEM_Mp1_ARADDR                : out   std_ulogic_vector(32 downto 0);
    moMEM_Mp1_ARLEN                 : out   std_ulogic_vector(7 downto 0);
    moMEM_Mp1_ARSIZE                : out   std_ulogic_vector(2 downto 0);
    moMEM_Mp1_ARBURST               : out   std_ulogic_vector(1 downto 0);
    --moMEM_Mp1_ARLOCK                : out   std_ulogic_vector(1 downto 0);
    --moMEM_Mp1_ARREGION              : out   std_ulogic_vector(3 downto 0);
    --moMEM_Mp1_ARCACHE               : out   std_ulogic_vector(3 downto 0);
    --moMEM_Mp1_ARPROT                : out   std_ulogic_vector(2 downto 0);
    --moMEM_Mp1_ARQOS                 : out   std_ulogic_vector(3 downto 0);
    moMEM_Mp1_ARVALID               : out   std_ulogic;
    moMEM_Mp1_ARREADY               : in    std_ulogic;
    moMEM_Mp1_RID                   : in    std_ulogic_vector(gAxiIdWidth-1 downto 0);
    moMEM_Mp1_RDATA                 : in    std_ulogic_vector(511 downto 0);
    moMEM_Mp1_RRESP                 : in    std_ulogic_vector(1 downto 0);
    moMEM_Mp1_RLAST                 : in    std_ulogic;
    moMEM_Mp1_RVALID                : in    std_ulogic;
    moMEM_Mp1_RREADY                : out   std_ulogic;

    ---- [APP_RDROL] -------------------
    -- to be use as ROLE VERSION IDENTIFICATION --
    poSHL_Mmio_RdReg                    : out   std_ulogic_vector( 15 downto 0);

    --------------------------------------------------------
    -- TOP : Secondary Clock (Asynchronous)
    --------------------------------------------------------
    piTOP_250_00Clk                     : in    std_ulogic;  -- Freerunning
    
    ------------------------------------------------
    -- SMC Interface
    ------------------------------------------------ 
    piFMC_ROLE_rank                      : in    std_logic_vector(31 downto 0);
    piFMC_ROLE_size                      : in    std_logic_vector(31 downto 0);
    
    poVoid                              : out   std_ulogic

  );
  
end Role_Themisto;


-- *****************************************************************************
-- **  ARCHITECTURE  **  FLASH of ROLE 
-- *****************************************************************************

architecture Flash of Role_Themisto is

  constant cUSE_DEPRECATED_DIRECTIVES       : boolean := false;

  --============================================================================
  --  SIGNAL DECLARATIONS
  --============================================================================  


  -- signal EMIF_inv   : std_logic_vector(7 downto 0);

  -- I hate Vivado HLS 
  signal sReadTlastAsVector : std_logic_vector(0 downto 0);
  signal sWriteTlastAsVector : std_logic_vector(0 downto 0);
  signal sResetAsVector : std_logic_vector(0 downto 0);

  signal sMetaOutTlastAsVector_Udp : std_logic_vector(0 downto 0);
  signal sMetaInTlastAsVector_Udp  : std_logic_vector(0 downto 0);
  signal sMetaOutTlastAsVector_Tcp : std_logic_vector(0 downto 0);
  signal sMetaInTlastAsVector_Tcp  : std_logic_vector(0 downto 0);

  signal sUdpPostCnt : std_ulogic_vector(9 downto 0);
  signal sTcpPostCnt : std_ulogic_vector(9 downto 0);

  --signal sMemTestDebugOut : std_logic_vector(15 downto 0);
  
  --============================================================================
  --  VARIABLE DECLARATIONS
  --============================================================================  

  --===========================================================================
  --== COMPONENT DECLARATIONS
  --===========================================================================
  component UppercaseApplication is
    port (
      ------------------------------------------------------
      -- From SHELL / Clock and Reset
      ------------------------------------------------------
           ap_clk                      : in  std_logic;
           ap_rst_n                    : in  std_logic;
           ap_start                    : in  std_logic;

      -- rank and size
           piFMC_ROL_rank_V        : in std_logic_vector (31 downto 0);
      --piSMC_ROL_rank_V_ap_vld : in std_logic;
           piFMC_ROL_size_V        : in std_logic_vector (31 downto 0);
      --piSMC_ROL_size_V_ap_vld : in std_logic;
      --------------------------------------------------------
      -- From SHELL / Udp-Tcp Data Interfaces
      --------------------------------------------------------
           siSHL_This_Data_tdata     : in  std_logic_vector( 63 downto 0);
           siSHL_This_Data_tkeep     : in  std_logic_vector(  7 downto 0);
           siSHL_This_Data_tlast     : in  std_logic;
           siSHL_This_Data_tvalid    : in  std_logic;
           siSHL_This_Data_tready    : out std_logic;
      --------------------------------------------------------
      -- To SHELL / Udp-Tcp Data Interfaces
      --------------------------------------------------------
           soTHIS_Shl_Data_tdata     : out std_logic_vector( 63 downto 0);
           soTHIS_Shl_Data_tkeep     : out std_logic_vector(  7 downto 0);
           soTHIS_Shl_Data_tlast     : out std_logic;
           soTHIS_Shl_Data_tvalid    : out std_logic;
           soTHIS_Shl_Data_tready    : in  std_logic;
      -- NRC Meta and Ports
           siNrc_meta_TDATA          : in std_logic_vector (63 downto 0);
           siNrc_meta_TVALID         : in std_logic;
           siNrc_meta_TREADY         : out std_logic;
           siNrc_meta_TKEEP          : in std_logic_vector (7 downto 0);
           siNrc_meta_TLAST          : in std_logic_vector (0 downto 0);

           soNrc_meta_TDATA          : out std_logic_vector (63 downto 0);
           soNrc_meta_TVALID         : out std_logic;
           soNrc_meta_TREADY         : in std_logic;
           soNrc_meta_TKEEP          : out std_logic_vector (7 downto 0);
           soNrc_meta_TLAST          : out std_logic_vector (0 downto 0);

           poROL_NRC_Rx_ports_V        : out std_logic_vector (31 downto 0);
           poROL_NRC_Rx_ports_V_ap_vld : out std_logic;
    --------------------------------------------------------
    -- SHELL / Mem / Mp1 Interface / Start Component
    --------------------------------------------------------
    --m_axi_moMEM_Mp1_AWID                  : out   std_ulogic_vector(gAxiIdWidth-1 downto 0);
    m_axi_moMEM_Mp1_AWADDR                : out   std_ulogic_vector(63 downto 0);
    m_axi_moMEM_Mp1_AWLEN                 : out   std_ulogic_vector(7 downto 0);
    m_axi_moMEM_Mp1_AWSIZE                : out   std_ulogic_vector(2 downto 0);
    m_axi_moMEM_Mp1_AWBURST               : out   std_ulogic_vector(1 downto 0);
    m_axi_moMEM_Mp1_AWLOCK                : out   std_ulogic_vector(1 downto 0);
    m_axi_moMEM_Mp1_AWREGION              : out   std_ulogic_vector(3 downto 0);
    --m_axi_moMEM_Mp1_AWUSER                : out   std_ulogic_vector(0 downto 0);
    m_axi_moMEM_Mp1_AWCACHE               : out   std_ulogic_vector(3 downto 0);
    m_axi_moMEM_Mp1_AWPROT                : out   std_ulogic_vector(2 downto 0);
    m_axi_moMEM_Mp1_AWQOS                 : out   std_ulogic_vector(3 downto 0);
    m_axi_moMEM_Mp1_AWVALID               : out   std_ulogic;
    m_axi_moMEM_Mp1_AWREADY               : in    std_ulogic;
    m_axi_moMEM_Mp1_WDATA                 : out   std_ulogic_vector(511 downto 0);
    m_axi_moMEM_Mp1_WSTRB                 : out   std_ulogic_vector(63 downto 0);
    m_axi_moMEM_Mp1_WLAST                 : out   std_ulogic;
    --m_axi_moMEM_Mp1_WID                   : out   std_ulogic_vector(gAxiIdWidth-1 downto 0);
    --m_axi_moMEM_Mp1_WUSER                 : out   std_ulogic_vector(0 downto 0);
    m_axi_moMEM_Mp1_WVALID                : out   std_ulogic;
    m_axi_moMEM_Mp1_WREADY                : in    std_ulogic;
    --m_axi_moMEM_Mp1_BID                   : in    std_ulogic_vector(gAxiIdWidth-1 downto 0);
    --m_axi_moMEM_Mp1_BUSER                 : in    std_ulogic_vector(0 downto 0);
    m_axi_moMEM_Mp1_BRESP                 : in    std_ulogic_vector(1 downto 0);
    m_axi_moMEM_Mp1_BVALID                : in    std_ulogic;
    m_axi_moMEM_Mp1_BREADY                : out   std_ulogic;
    --m_axi_moMEM_Mp1_ARID                  : out   std_ulogic_vector(gAxiIdWidth-1 downto 0);
    m_axi_moMEM_Mp1_ARADDR                : out   std_ulogic_vector(63 downto 0);
    m_axi_moMEM_Mp1_ARLEN                 : out   std_ulogic_vector(7 downto 0);
    m_axi_moMEM_Mp1_ARSIZE                : out   std_ulogic_vector(2 downto 0);
    m_axi_moMEM_Mp1_ARBURST               : out   std_ulogic_vector(1 downto 0);
    m_axi_moMEM_Mp1_ARLOCK                : out   std_ulogic_vector(1 downto 0);
    m_axi_moMEM_mp1_ARREGION              : out   std_ulogic_vector(3 downto 0);
    --m_axi_moMEM_mp1_ARUSER                : out   std_ulogic_vector(0 downto 0);
    m_axi_moMEM_mp1_ARCACHE               : out   std_ulogic_vector(3 downto 0);
    m_axi_moMEM_mp1_ARPROT                : out   std_ulogic_vector(2 downto 0);
    m_axi_moMEM_mp1_ARQOS                 : out   std_ulogic_vector(3 downto 0);
    m_axi_moMEM_Mp1_ARVALID               : out   std_ulogic;
    m_axi_moMEM_Mp1_ARREADY               : in    std_ulogic;
    --m_axi_moMEM_Mp1_RID                   : in    std_ulogic_vector(gAxiIdWidth-1 downto 0);
    --m_axi_moMEM_Mp1_RUSER                 : in    std_ulogic_vector(0 downto 0);
    m_axi_moMEM_Mp1_RDATA                 : in    std_ulogic_vector(511 downto 0);
    m_axi_moMEM_Mp1_RRESP                 : in    std_ulogic_vector(1 downto 0);
    m_axi_moMEM_Mp1_RLAST                 : in    std_ulogic;
    m_axi_moMEM_Mp1_RVALID                : in    std_ulogic;
    m_axi_moMEM_Mp1_RREADY                : out   std_ulogic;
    lcl_mem0_v                            : in   std_ulogic_vector(63 downto 0);
    lcl_mem1_v                            : in   std_ulogic_vector(63 downto 0)
    --------------------------------------------------------
    -- SHELL / Mem / Mp1 Interface / End Component
    --------------------------------------------------------
         );
  end component UppercaseApplication;



  --===========================================================================
  --== FUNCTION DECLARATIONS  [TODO-Move to a package]
  --===========================================================================
  function fVectorize(s: std_logic) return std_logic_vector is
    variable v: std_logic_vector(0 downto 0);
  begin
    v(0) := s;
    return v;
  end fVectorize;

  function fScalarize(v: in std_logic_vector) return std_ulogic is
  begin
    assert v'length = 1
    report "scalarize: output port must be single bit!"
    severity FAILURE;
    return v(v'LEFT);
  end;


--################################################################################
--#                                                                              #
--#                          #####   ####  ####  #     #                         #
--#                          #    # #    # #   #  #   #                          #
--#                          #    # #    # #    #  ###                           #
--#                          #####  #    # #    #   #                            #
--#                          #    # #    # #    #   #                            #
--#                          #    # #    # #   #    #                            #
--#                          #####   ####  ####     #                            #
--#                                                                              #
--################################################################################

begin

  --poSHL_Mmio_RdReg <= sMemTestDebugOut when (unsigned(piSHL_Mmio_WrReg) /= 0) else 
  -- x"BEEF"; 
  -- to be use as ROLE VERSION IDENTIFICATION --
  poSHL_Mmio_RdReg <= x"BEEF";
  

  --################################################################################
  --#                                                                              #
  --#    #     #  #####    ######     #####                                        #
  --#    #     #  #    #   #     #   #     # #####   #####                         #
  --#    #     #  #     #  #     #   #     # #    #  #    #                        #
  --#    #     #  #     #  ######    ####### #####   #####                         #
  --#    #     #  #    #   #         #     # #       #                             #
  --#    #######  #####    #         #     # #       #                             #
  --#                                                                              #
  --################################################################################

  -- gUdpAppFlashDepre : if cUSE_DEPRECATED_DIRECTIVES generate --TODO

  --  begin 

  sMetaInTlastAsVector_Udp(0) <= siNRC_Role_Udp_Meta_TLAST;
  soROLE_Nrc_Udp_Meta_TLAST <=  sMetaOutTlastAsVector_Udp(0);

  UAF: UppercaseApplication
  port map (

             ------------------------------------------------------
             -- From SHELL / Clock and Reset
             ------------------------------------------------------
             ap_clk                      => piSHL_156_25Clk,
             ap_rst_n                    => (not piMMIO_Ly7_Rst),
             ap_start                    => piMMIO_Ly7_En,
            
             piFMC_ROL_rank_V         => piFMC_ROLE_rank,
             --piFMC_ROL_rank_V_ap_vld  => '1',
             piFMC_ROL_size_V         => piFMC_ROLE_size,
             --piFMC_ROL_size_V_ap_vld  => '1',
             --------------------------------------------------------
             -- From SHELL / Udp Data Interfaces
             --------------------------------------------------------
             siSHL_This_Data_tdata     => siNRC_Udp_Data_tdata,
             siSHL_This_Data_tkeep     => siNRC_Udp_Data_tkeep,
             siSHL_This_Data_tlast     => siNRC_Udp_Data_tlast,
             siSHL_This_Data_tvalid    => siNRC_Udp_Data_tvalid,
             siSHL_This_Data_tready    => siNRC_Udp_Data_tready,
             --------------------------------------------------------
             -- To SHELL / Udp Data Interfaces
             --------------------------------------------------------
             soTHIS_Shl_Data_tdata     => soNRC_Udp_Data_tdata,
             soTHIS_Shl_Data_tkeep     => soNRC_Udp_Data_tkeep,
             soTHIS_Shl_Data_tlast     => soNRC_Udp_Data_tlast,
             soTHIS_Shl_Data_tvalid    => soNRC_Udp_Data_tvalid,
             soTHIS_Shl_Data_tready    => soNRC_Udp_Data_tready, 

             siNrc_meta_TDATA          =>  siNRC_Role_Udp_Meta_TDATA    ,
             siNrc_meta_TVALID         =>  siNRC_Role_Udp_Meta_TVALID   ,
             siNrc_meta_TREADY         =>  siNRC_Role_Udp_Meta_TREADY   ,
             siNrc_meta_TKEEP          =>  siNRC_Role_Udp_Meta_TKEEP    ,
             siNrc_meta_TLAST          =>  sMetaInTlastAsVector_Udp,

             soNrc_meta_TDATA          =>  soROLE_Nrc_Udp_Meta_TDATA  ,
             soNrc_meta_TVALID         =>  soROLE_Nrc_Udp_Meta_TVALID ,
             soNrc_meta_TREADY         =>  soROLE_Nrc_Udp_Meta_TREADY ,
             soNrc_meta_TKEEP          =>  soROLE_Nrc_Udp_Meta_TKEEP  ,
             soNrc_meta_TLAST          =>  sMetaOutTlastAsVector_Udp,

             poROL_NRC_Rx_ports_V        => poROL_Nrc_Udp_Rx_ports,
            --poROL_NRC_Udp_Rx_ports_V_ap_vld => '1'
            --------------------------------------------------------
             -- SHELL / Mem / Mp1 Interface / Start in UAF
             --------------------------------------------------------     
             m_axi_moMEM_Mp1_ARADDR(32 DOWNTO 0)  => moMEM_Mp1_ARADDR,
             m_axi_moMEM_Mp1_ARADDR(63 DOWNTO 33) => open,
             m_axi_moMEM_Mp1_ARBURST      => moMEM_Mp1_ARBURST,
             m_axi_moMEM_Mp1_ARCACHE      => open, -- m_axi_card_mem0_arcache,
             --m_axi_moMEM_Mp1_ARID         => moMEM_Mp1_ARID( 0 DOWNTO 0),--SR# 10394170 : out   std_ulogic_vector(gAxiIdWidth-1 downto 0);
             m_axi_moMEM_Mp1_ARLEN        => moMEM_Mp1_ARLEN,
             m_axi_moMEM_Mp1_ARLOCK       => open, -- m_axi_card_mem0_arlock,
             m_axi_moMEM_Mp1_ARPROT       => open, -- m_axi_card_mem0_arprot,
             m_axi_moMEM_Mp1_ARQOS        => open, -- m_axi_card_mem0_arqos,
             m_axi_moMEM_Mp1_ARREADY      => moMEM_Mp1_ARREADY,
             m_axi_moMEM_Mp1_ARREGION     => open, -- m_axi_card_mem0_arregion,
             m_axi_moMEM_Mp1_ARSIZE       => moMEM_Mp1_ARSIZE,
             --m_axi_moMEM_Mp1_ARUSER       => open, -- m_axi_card_mem0_aruser,
             m_axi_moMEM_Mp1_ARVALID      => moMEM_Mp1_ARVALID,
             m_axi_moMEM_Mp1_AWADDR(32 DOWNTO 0)  => moMEM_Mp1_AWADDR,
             m_axi_moMEM_Mp1_AWADDR(63 DOWNTO 33) => open,
             m_axi_moMEM_Mp1_AWBURST      => moMEM_Mp1_AWBURST,
             m_axi_moMEM_Mp1_AWCACHE      => open, -- m_axi_card_mem0_awcache,
             --m_axi_moMEM_Mp1_AWID         => moMEM_Mp1_AWID(0 DOWNTO 0),--SR# 10394170 : out   std_ulogic_vector(gAxiIdWidth-1 downto 0);
             m_axi_moMEM_Mp1_AWLEN        => moMEM_Mp1_AWLEN,
             m_axi_moMEM_Mp1_AWLOCK       => open, -- m_axi_card_mem0_awlock,
             m_axi_moMEM_Mp1_AWPROT       => open, -- m_axi_card_mem0_awprot,
             m_axi_moMEM_Mp1_AWQOS        => open, -- m_axi_card_mem0_awqos,
             m_axi_moMEM_Mp1_AWREADY      => moMEM_Mp1_AWREADY,
             m_axi_moMEM_Mp1_AWREGION     => open, -- m_axi_card_mem0_awregion,
             m_axi_moMEM_Mp1_AWSIZE       => moMEM_Mp1_AWSIZE,
             --m_axi_moMEM_Mp1_AWUSER       => open, -- m_axi_card_mem0_awuser,
             m_axi_moMEM_Mp1_AWVALID      => moMEM_Mp1_AWVALID,
             --m_axi_moMEM_Mp1_BID          => moMEM_Mp1_BID(0 DOWNTO 0),--SR# 10394170 : in    std_ulogic_vector(gAxiIdWidth-1 downto 0);
             m_axi_moMEM_Mp1_BREADY       => moMEM_Mp1_BREADY,
             m_axi_moMEM_Mp1_BRESP        => moMEM_Mp1_BRESP,
             --m_axi_moMEM_Mp1_BUSER  m_axi_card_mem0_buser,
             m_axi_moMEM_Mp1_BVALID       => moMEM_Mp1_BVALID,
             m_axi_moMEM_Mp1_RDATA        => moMEM_Mp1_RDATA,
             --m_axi_moMEM_Mp1_RID          => moMEM_Mp1_RID(0 DOWNTO 0),--SR# 10394170 : in    std_ulogic_vector(gAxiIdWidth-1 downto 0);
             m_axi_moMEM_Mp1_RLAST        => moMEM_Mp1_RLAST,
             m_axi_moMEM_Mp1_RREADY       => moMEM_Mp1_RREADY,
             m_axi_moMEM_Mp1_RRESP        => moMEM_Mp1_RRESP,
             --m_axi_moMEM_Mp1_RUSER        => open, -- m_axi_card_mem0_ruser,
             m_axi_moMEM_Mp1_RVALID       => moMEM_Mp1_RVALID,
             m_axi_moMEM_Mp1_WDATA        => moMEM_Mp1_WDATA,
             --m_axi_moMEM_Mp1_WID          => open,
             m_axi_moMEM_Mp1_WLAST        => moMEM_Mp1_WLAST,
             m_axi_moMEM_Mp1_WREADY       => moMEM_Mp1_WREADY,
             m_axi_moMEM_Mp1_WSTRB        => moMEM_Mp1_WSTRB,
             --m_axi_moMEM_Mp1_WUSER        => open, -- m_axi_card_mem0_wuser,
             m_axi_moMEM_Mp1_WVALID       => moMEM_Mp1_WVALID,

             lcl_mem0_v                     => x"0000000000000000",
             lcl_mem1_v                     => x"8000000000000000"
             --------------------------------------------------------
             -- SHELL / Mem / Mp1 Interface / End in UAF
             --------------------------------------------------------
           );

  --end generate;
  
  
  --################################################################################
  --#                                                                              #
  --#    #######    ####   ######     #####                                        #
  --#       #      #       #     #   #     # #####   #####                         #
  --#       #     #        #     #   #     # #    #  #    #                        #
  --#       #     #        ######    ####### #####   #####                         #
  --#       #      #       #         #     # #       #                             #
  --#       #       ####   #         #     # #       #                             #
  --#                                                                              #
  --################################################################################

  -- gUdpAppFlashDepre : if cUSE_DEPRECATED_DIRECTIVES generate --TODO

  --  begin 

  sMetaInTlastAsVector_Tcp(0) <= siNRC_Role_Tcp_Meta_TLAST;
  soROLE_Nrc_Tcp_Meta_TLAST <=  sMetaOutTlastAsVector_Tcp(0);

-- auto excluding TAF             TAF: UppercaseApplication
-- auto excluding TAF             port map (
-- auto excluding TAF           
-- auto excluding TAF                        ------------------------------------------------------
-- auto excluding TAF                        -- From SHELL / Clock and Reset
-- auto excluding TAF                        ------------------------------------------------------
-- auto excluding TAF                        ap_clk                      => piSHL_156_25Clk,
-- auto excluding TAF                        ap_rst_n                    => (not piMMIO_Ly7_Rst),
-- auto excluding TAF                        ap_start                    => piMMIO_Ly7_En,
-- auto excluding TAF                     
-- auto excluding TAF                        piFMC_ROL_rank_V         => piFMC_ROLE_rank,
-- auto excluding TAF                        --piFMC_ROL_rank_V_ap_vld  => '1',
-- auto excluding TAF                        piFMC_ROL_size_V         => piFMC_ROLE_size,
-- auto excluding TAF                        --piFMC_ROL_size_V_ap_vld  => '1',
-- auto excluding TAF                        --------------------------------------------------------
-- auto excluding TAF                        -- From SHELL / Tcp Data Interfaces
-- auto excluding TAF                        --------------------------------------------------------
-- auto excluding TAF                        siSHL_This_Data_tdata     => siNRC_Tcp_Data_tdata,
-- auto excluding TAF                        siSHL_This_Data_tkeep     => siNRC_Tcp_Data_tkeep,
-- auto excluding TAF                        siSHL_This_Data_tlast     => siNRC_Tcp_Data_tlast,
-- auto excluding TAF                        siSHL_This_Data_tvalid    => siNRC_Tcp_Data_tvalid,
-- auto excluding TAF                        siSHL_This_Data_tready    => siNRC_Tcp_Data_tready,
-- auto excluding TAF                        --------------------------------------------------------
-- auto excluding TAF                        -- To SHELL / Tcp Data Interfaces
-- auto excluding TAF                        --------------------------------------------------------
-- auto excluding TAF                        soTHIS_Shl_Data_tdata     => soNRC_Tcp_Data_tdata,
-- auto excluding TAF                        soTHIS_Shl_Data_tkeep     => soNRC_Tcp_Data_tkeep,
-- auto excluding TAF                        soTHIS_Shl_Data_tlast     => soNRC_Tcp_Data_tlast,
-- auto excluding TAF                        soTHIS_Shl_Data_tvalid    => soNRC_Tcp_Data_tvalid,
-- auto excluding TAF                        soTHIS_Shl_Data_tready    => soNRC_Tcp_Data_tready, 
-- auto excluding TAF           
-- auto excluding TAF                        siNrc_meta_TDATA          =>  siNRC_Role_Tcp_Meta_TDATA    ,
-- auto excluding TAF                        siNrc_meta_TVALID         =>  siNRC_Role_Tcp_Meta_TVALID   ,
-- auto excluding TAF                        siNrc_meta_TREADY         =>  siNRC_Role_Tcp_Meta_TREADY   ,
-- auto excluding TAF                        siNrc_meta_TKEEP          =>  siNRC_Role_Tcp_Meta_TKEEP    ,
-- auto excluding TAF                        siNrc_meta_TLAST          =>  sMetaInTlastAsVector_Tcp,
-- auto excluding TAF           
-- auto excluding TAF                        soNrc_meta_TDATA          =>  soROLE_Nrc_Tcp_Meta_TDATA  ,
-- auto excluding TAF                        soNrc_meta_TVALID         =>  soROLE_Nrc_Tcp_Meta_TVALID ,
-- auto excluding TAF                        soNrc_meta_TREADY         =>  soROLE_Nrc_Tcp_Meta_TREADY ,
-- auto excluding TAF                        soNrc_meta_TKEEP          =>  soROLE_Nrc_Tcp_Meta_TKEEP  ,
-- auto excluding TAF                        soNrc_meta_TLAST          =>  sMetaOutTlastAsVector_Tcp,
-- auto excluding TAF           
-- auto excluding TAF                        poROL_NRC_Rx_ports_V        => poROL_Nrc_Tcp_Rx_ports
-- auto excluding TAF                      --poROL_NRC_Tcp_Rx_ports_V_ap_vld => '1'
-- auto excluding TAF                      );

  --end generate;

  --DEBUGING:
  --poROL_Nrc_Tcp_Rx_ports <= (others => '0');

  --################################################################################
  --  1st Memory Port dummy connections
  --################################################################################
    soMEM_Mp0_RdCmd_tdata   <= (others => '0');
    soMEM_Mp0_RdCmd_tvalid  <= '0';
    siMEM_Mp0_RdSts_tready  <= '0';
    siMEM_Mp0_Read_tready   <= '0';
    soMEM_Mp0_WrCmd_tdata   <= (others => '0');
    soMEM_Mp0_WrCmd_tvalid  <= '0';
    siMEM_Mp0_WrSts_tready  <= '0';
    soMEM_Mp0_Write_tdata   <= (others => '0');
    soMEM_Mp0_Write_tkeep   <= (others => '0');
    soMEM_Mp0_Write_tlast   <= '0';
    soMEM_Mp0_Write_tvalid  <= '0';
    

  --################################################################################
  --  2nd Memory Port dummy connections
  --################################################################################

  -- moMEM_Mp1_AWVALID <= '0';
  -- moMEM_Mp1_WVALID  <= '0';
  -- moMEM_Mp1_BREADY  <= '0';
  -- moMEM_Mp1_ARVALID <= '0';
  -- moMEM_Mp1_RREADY  <= '0';

end architecture Flash;

