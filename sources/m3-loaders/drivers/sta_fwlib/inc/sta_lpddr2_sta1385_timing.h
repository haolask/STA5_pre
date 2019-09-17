/**
 * Code generated
 * Selected PUBL: 
 * LPDDR2 Model: IS43/46LD16128B
 * 	Type:LPDDR2
 * 	Supplier:ISSI
 * 	Frequency:1066
 * 	Grade:sg-18
 * 	RL:6
 * 	WL:3
 * 	tCKmin (ns):2.5
 * 	tCKmax (ns):2.5
 * 	tRCD (ns):18
 * 	tRPab (ns):21
 * 	tRPpb (ns):18
 * 	tRC (ns):63
 * 	tRASmin (ns):42
 * 	tRASmax (ns):70
 * 	tFAW 1KB (ns):50
 * 	tFAW 2KB (ns):50
 * 	tRRD 1KB (ns):10
 * 	tRRD 2KB (ns):10
 * 	tRFC 1Gb (ns):130
 * 	tRFC 2Gb (ns):130
 * 	tRFC 4Gb (ns):130
 * 	tRFC 8Gb (ns):130
 * 	tWR (ns):15
 * 	tCCD (ns):5
 * 	tZQINIT (ns):1000
 * 	tZQCS (ns):90
 * 	tRTP (ns):7.5
 * 	tWTR (ns):7.5
 * 	tXP (ns):7.5
 * 	tCKE (ns):3
 * 	tCKESR (ns):15
 * 	tREFI (ns):3900
 * 	tCH:1.25
 * 	tCL:1.25
 * 	tZQCL  (ns):360
 * 	tZQRESET (ns):50
 * 	tDQSCK min (ns):2.5
 * 	tDQSCK max (ns):5.5
 * 	tMRW (ns):12.5
 * 	tMRR (ns):5
 * 	tXSR (ns):140
 * 	:
 * 	:
 * 	:
 * 	:
 * 	:
 * 	:
 * tCK: 2.56
 * SoC: sta1385
 * FXTAL: 26
 * NDIV: 30
 * ODF: 1
 * Burst Length: 8
*/
/* PUBL */
#define TIMING_PTR0 0x0021F354
		/*tDLLSRST 20*/
		/*tDLLLOCK 1997*/
		/*tITMSRST 8*/
#define TIMING_PTR1 0x013930B0
		/*tDINIT0 78000*/
		/*tDINIT1 39*/
#define TIMING_PTR2 0x030C10C2
		/*tDINIT2 4290*/
		/*tDINIT3 390*/
#define TIMING_DTPR0 0xB291896E
		/*tMRD 2*/
		/*tRTP 3*/
		/*tWTR 3*/
		/*tRP 9*/
		/*tRCD 8*/
		/*tRAS 17*/
		/*tRRD 4*/
		/*tRC 25*/
		/*tCCD 1*/
#define TIMING_DTPR1 0x193300A0
		/*tAOND/tAOFD 0*/
		/*tRTW 0*/
		/*tFAW 20*/
		/*tMOD 0*/
		/*tRTODT 0*/
		/*tRFC 51*/
		/*tDQSCKmin 1*/
		/*tDQSCKmax 3*/
#define TIMING_DTPR2 0x00018C37
		/*tXS 55*/
		/*tXP 3*/
		/*tCKE 3*/
		/*tDLLK 0*/
#define TIMING_MR1 0x00000083
		/*BL 3*/
		/*BT 0*/
		/*WC 0*/
		/*nWR 4*/
		/*reserved 0*/
#define TIMING_MR2 0x00000004
		/*RL/WL 4*/
/* uMCTL2 */
#define TIMING_INIT0 0x40270001
		/*pre_cke_x1024 1*/
		/*post_cke_x1024 39*/
		/*skip_dram_init 1*/
#define TIMING_INIT2 0x00000702
		/*min_stable_clock_x1 2*/
		/*idle_after_reset_x32 7*/
#define TIMING_INIT1 0x0000000A
		/*pre_ocd_x32 10*/
		/*dram_rstn_x1024 0*/
#define TIMING_INIT3 0x00830004
		/*emr 4*/
		/*mr 131*/
#define TIMING_INIT5 0x00070002
		/*max_auto_init_x1024 2*/
		/*dev_zqinit_x32 7*/
#define TIMING_DRAMTMG0 0x070A0D09
		/*t_ras_min 9*/
		/*t_ras_max 13*/
		/*t_faw 10*/
		/*wr2pre 7*/
#define TIMING_DRAMTMG1 0x0002030C
		/*t_rc 12*/
		/*rd2pre 3*/
		/*t_xp 2*/
#define TIMING_DRAMTMG2 0x02030606
		/*wr2rd 6*/
		/*rd2wr 6*/
		/*read_latency 3*/
		/*write_latency 2*/
#define TIMING_DRAMTMG3 0x00300000
		/*t_mod 0*/
		/*t_mrd 4*/
		/*t_mrw 3*/
#define TIMING_DRAMTMG4 0x04010205
		/*t_rp 5*/
		/*t_rrd 2*/
		/*t_ccd 1*/
		/*t_rcd 4*/
#define TIMING_DRAMTMG5 0x01010303
		/*t_cke 3*/
		/*t_ckesr 3*/
		/*t_cksre 1*/
		/*t_cksrx 1*/
#define TIMING_DRAMTMG6 0x01010003
		/*t_ckcsx 3*/
		/*t_ckdpdx 1*/
		/*t_ckdpde 1*/
#define TIMING_DRAMTMG7 0x00000101
		/*t_ckpsx 1*/
		/*t_ckpde 1*/
#define TIMING_DRAMTMG8 0x00000000
		/*t_xs_x32 0*/
		/*t_xs_dll_x32 0*/
		/*t_xs_abort_x32 0*/
		/*t_xs_fast_x32 0*/
#define TIMING_DRAMTMG14 0x0000001C
		/*t_xsr 28*/
#define TIMING_DFITMG0 0x2020001
		/*dfi_tphy_wrlat 1*/
		/*dfi_tphy_wrdata 0*/
		/*dfi_wrdata_use_dfi_phy_clk 0*/
		/*dfi_t_rddata_en 2*/
		/*dfi_rddata_use_dfi_phy_clk 0*/
		/*dfi_t_ctrl_delay 2*/
#define TIMING_DFITMG1 0x10202
		/*dfi_t_dram_clk_enable 2*/
		/*sdfi_t_dram_clk_disable 2*/
		/*dfi_t_wrdata_delay 1*/
		/*dfi_t_parin_lat 0*/
		/*dfi_t_cmd_lat 0*/
#define TIMING_ZQCTL0 0x40470012
		/*t_zq_short_nop 18*/
		/*t_zq_long_nop 71*/
		/*dis_mpsmx_zqcl 0*/
		/*zq_resistor_shared 0*/
		/*dis_srx_zqcl 1*/
		/*dis_auto_zq 0*/
