/**
 * Code generated
 * Selected PUB: PUBL 2.00
 * DDR Model: DDR3L-1866-107
 * 	Type:DDR3
 * 	Supplier:Micron
 * 	Frequency:1866
 * 	Grade:sg-107
 * 	CL:7
 * 	CWL:6
 * 	tCKmin (ns):1.875
 * 	tCKmax (ns):2.5
 * 	tRCD (ns):13.91
 * 	tRP (ns):13.91
 * 	tRC (ns):47.91
 * 	tRAS (ns):34
 * 	tFAW 1KB (ns):27
 * 	tFAW 2KB (ns):35
 * 	tRRD 1KB (ns):7.5
 * 	tRRD 2KB (ns):7.5
 * 	tRFC 1Gb (ns):110
 * 	tRFC 2Gb (ns):160
 * 	tRFC 4Gb (ns):260
 * 	tRFC 8Gb (ns):350
 * 	tDLLK (ns):960
 * 	tXSDLL (ns):960
 * 	tWR (ns):15
 * 	tCCD (ns):7.5
 * 	tMRD (ns):7.5
 * 	tMPRR (ns):1.88
 * 	tZQINIT (ns):960
 * 	tZQOPER (ns):480
 * 	tZQCS (ns):120
 * 	tCPDED (ns):1.875
 * 	tACTPDEN (ns):1.875
 * 	tPRPDEN (ns):1.875
 * 	tREFPDEN (ns):1.875
 * 	tWLMRD (ns):75
 * 	tWLDQSEN (ns):46.875
 * 	tWLOmin (ns):0
 * 	tWLOmax (ns):7.5
 * 	tRTP (ns):7.5
 * 	tWTR (ns):7.5
 * 	tMRD (ns):7.5
 * 	tMOD (ns):22.5
 * 	tXP (ns):6
 * 	tCKE (ns):5.625
 * 	tCKESR (ns):7.5
 * 	tCKESRE (ns):10
 * 	tCKESRX (ns):10
 * 	tREFI (ns):3900
 * tCK: 1.92
 * CL: 7
 * CWL: 6
 * AL: 0
 * SoC: sta1385
 * FXTAL: 26
 * NDIV: 40
 * ODF: 1
 * Burst Length: 8
 * Page size: 2KB
 * DQ width: 1
 * Density: 4Gb
*/
/* PUBL */
#define TIMING_PTR0 0x002299DA
		/*tDLLSRST 26*/
		/*tDLLLOCK 2663*/
		/*tITMSRST 8*/
#define TIMING_PTR1 0x046BF7A0
		/*tDINIT0 260000*/
		/*tDINIT1 141*/
#define TIMING_PTR2 0x04119640
		/*tDINIT2 104000*/
		/*tDINIT3 520*/
#define TIMING_DTPR0 0xB2928898
		/*tMRD 0*/
		/*tRTP 6*/
		/*tWTR 4*/
		/*tRP 8*/
		/*tRCD 8*/
		/*tRAS 18*/
		/*tRRD 4*/
		/*tRC 25*/
		/*tCCD 1*/
#define TIMING_DTPR1 0x00870090
		/*tAOND/tAOFD 0*/
		/*tRTW 0*/
		/*tFAW 18*/
		/*tMOD 0*/
		/*tRTODT 0*/
		/*tRFC 135*/
		/*tDQSCKmin 0*/
		/*tDQSCKmax 0*/
#define TIMING_DTPR2 0x0FA461F4
		/*tXS 500*/
		/*tXP 24*/
		/*tCKE 8*/
		/*tDLLK 500*/
#define TIMING_MR0 0x00000830
		/*BL 0*/
		/*CL bit 2 0*/
		/*BT 0*/
		/*CL bit 6:4 3*/
		/*TM 0*/
		/*DR 0*/
		/*WR 4*/
		/*PD 0*/
#define TIMING_MR1 0x00000006
		/*DE 0*/
		/*DIC bit 1 1*/
		/*RTT bit 2 1*/
		/*AL 0*/
		/*DIC bit 5 0*/
		/*RTT bit 6 0*/
		/*LEVEL 0*/
		/*RTT bit 9 0*/
		/*TDQS 0*/
		/*QOFF 0*/
#define TIMING_MR2 0x00000008
		/*PASR 0*/
		/*CWL 1*/
		/*ASR 0*/
		/*SRT 0*/
		/*RTTWR 0*/
#define TIMING_MR3 0x00000000
		/*MR3 0*/
		/* 0*/
/* uMCTL2 */
#define TIMING_INIT3 0x08300006
		/*emr 6*/
		/*mr 2096*/
#define TIMING_INIT4 0x00080000
		/*emr3 0*/
		/*emr2 8*/
#define TIMING_INIT5 0x00100000
		/*max_auto_init_x1024 0*/
		/*dev_zqinit_x32 16*/
#define TIMING_DRAMTMG0 0x080A0808
		/*t_ras_min 8*/
		/*t_ras_max 8*/
		/*t_faw 10*/
		/*wr2pre 8*/
#define TIMING_DRAMTMG1 0x0002020D
		/*t_rc 13*/
		/*rd2pre 2*/
		/*t_xp 2*/
#define TIMING_DRAMTMG2 0x03040407
		/*wr2rd 7*/
		/*rd2wr 4*/
		/*read_latency 4*/
		/*write_latency 3*/
#define TIMING_DRAMTMG3 0x00004006
		/*t_mod 6*/
		/*t_mrd 4*/
		/*t_mrw 0*/
#define TIMING_DRAMTMG4 0x04040205
		/*t_rp 5*/
		/*t_rrd 2*/
		/*t_ccd 4*/
		/*t_rcd 4*/
#define TIMING_DRAMTMG5 0x03030202
		/*t_cke 2*/
		/*t_ckesr 2*/
		/*t_cksre 3*/
		/*t_cksrx 3*/
#define TIMING_DRAMTMG8 0x00000F03
		/*t_xs_x32 3*/
		/*t_xs_dll_x32 15*/
		/*t_xs_abort_x32 0*/
		/*t_xs_fast_x32 0*/
#define TIMING_DFITMG0 0x03020101
		/*dfi_tphy_wrlat 1*/
		/*dfi_tphy_wrdata 1*/
		/*dfi_wrdata_use_sdr 0*/
		/*dfi_t_rddata_en 2*/
		/*dfi_rddata_use_sdr 0*/
		/*dfi_t_ctrl_delay 3*/
#define TIMING_ZQCTL0 0x407D0020
		/*t_zq_short_nop 32*/
		/*t_zq_long_nop 125*/
		/*dis_mpsmx_zqcl 0*/
		/*zq_resistor_shared 0*/
		/*dis_srx_zqcl 1*/
		/*dis_auto_zq 0*/
