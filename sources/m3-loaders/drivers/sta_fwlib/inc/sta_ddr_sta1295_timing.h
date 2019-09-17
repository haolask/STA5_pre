/**
 * Code generated
 * Selected PUB: PUB 3.15
 * DDR Model: DDR3L-1600-125
 * 	Type:DDR3
 * 	Supplier:Micron
 * 	Frequency:1600
 * 	Grade:sg-125
 * 	CL:10
 * 	CWL:7
 * 	tCKmin (ns):1.5
 * 	tCKmax (ns):1.875
 * 	tRCD (ns):13.75
 * 	tRP (ns):13.75
 * 	tRC (ns):48.75
 * 	tRAS (ns):35
 * 	tFAW 1KB (ns):30
 * 	tFAW 2KB (ns):40
 * 	tRRD 1KB (ns):6
 * 	tRRD 2KB (ns):7.5
 * 	tRFC 1Gb (ns):110
 * 	tRFC 2Gb (ns):160
 * 	tRFC 4Gb (ns):260
 * 	tRFC 8Gb (ns):350
 * 	tDLLK (ns):768
 * 	tXSDLL (ns):768
 * 	tWR (ns):15
 * 	tCCD (ns):6
 * 	tMRD (ns):6
 * 	tMPRR (ns):1.50
 * 	tZQINIT (ns):768
 * 	tZQOPER (ns):384
 * 	tZQCS (ns):96
 * 	tCPDED (ns):1.5
 * 	tACTPDEN (ns):1.5
 * 	tPRPDEN (ns):1.5
 * 	tREFPDEN (ns):1.5
 * 	tWLMRD (ns):60
 * 	tWLDQSEN (ns):37.5
 * 	tWLOmin (ns):0
 * 	tWLOmax (ns):7.5
 * 	tRTP (ns):7.5
 * 	tWTR (ns):7.5
 * 	tMRD (ns):6
 * 	tMOD (ns):18
 * 	tXP (ns):6
 * 	tCKE (ns):5
 * 	tCKESR (ns):6.5
 * 	tCKESRE (ns):10
 * 	tCKESRX (ns):10
 * 	tREFI (ns):3900
 * tCK: 1.52
 * CL: 10
 * CWL: 7
 * AL: 8
 * SoC: sta1295
 * FXTAL: 24
 * NDIV: 55
 * ODF: 2
 * Burst Length: 8
 * Page size: 2KB
 * DQ width: 1
 * Density: 4Gb
*/
/* PUB */
#define TIMING_PTR0 0x06803410
		/*tPHYRST 16*/
		/*tPLLGS 208*/
		/*tPLLPD 52*/
#define TIMING_PTR1 0x1450009C
		/*tPLLRST 156*/
		/*tPLLLOCK 5200*/
#define TIMING_PTR2 0x00083DEF
		/*tCALON 15*/
		/*tCALS 15*/
		/*tCALH 15*/
		/*tWLDLYS 16*/
#define TIMING_PTR3 0x0B350910
		/*tDINIT0 330000*/
		/*tDINIT1 179*/
#define TIMING_PTR4 0x0A5203A0
		/*tDINIT2 132000*/
		/*tDINIT3 660*/
#define TIMING_DTPR0 0x85579955
		/*tRTP 5*/
		/*tWTR 5*/
		/*tRP 9*/
		/*tRCD 9*/
		/*tRAS 23*/
		/*tRRD 5*/
		/*tRC 33*/
#define TIMING_DTPR1 0x1A756360
		/*tMRD 0*/
		/*tMOD 0*/
		/*tFAW 27*/
		/*tRFC 172*/
		/*tWLMRD 39*/
		/*tWLO 6*/
		/*tAOND/tAOFD 0*/
#define TIMING_DTPR2 0x8FDAC1FB
		/*tXS 507*/
		/*tXP 16*/
		/*tCKE 5*/
		/*tDLLK 507*/
		/*tRTODT 0*/
		/*tRTW 0*/
		/*tCCD 1*/
#define TIMING_MR0 0x00000A60
		/*BL 0*/
		/*CL bit 2 0*/
		/*BT 0*/
		/*CL bit 6:4 6*/
		/*TM 0*/
		/*DR 0*/
		/*WR 5*/
		/*PD 0*/
#define TIMING_MR1 0x00000094
		/*DE 0*/
		/*DIC bit 1 0*/
		/*RTT bit 2 1*/
		/*AL 2*/
		/*DIC bit 5 0*/
		/*RTT bit 6 0*/
		/*LEVEL 1*/
		/*RTT bit 9 0*/
		/*TDQS 0*/
		/*QOFF 0*/
#define TIMING_MR2 0x00000210
		/*PASR 0*/
		/*CWL 2*/
		/*ASR 0*/
		/*SRT 0*/
		/*reserved 0*/
		/*RTTWR 1*/
#define TIMING_MR3 0x00000000
		/*MPRLOC 0*/
		/*MPR 0*/
#define TIMING_PGCR2 0x00F0087E
		/*tREFPRD 2174*/
		/*NOBUB 0*/
		/*FXDLAT 0*/
		/*DTPMXTMR 15*/
		/*SHRAC 0*/
		/*ACPDDC 0*/
		/*LPMSTRC0 0*/
		/*DYNACPDD 0*/
/* uMCTL2 */
#define TIMING_INIT3 0x0A600094
		/*emr 148*/
		/*mr 2656*/
#define TIMING_INIT4 0x02100000
		/*emr3 0*/
		/*emr2 528*/
#define TIMING_INIT5 0x00100000
		/*max_auto_init_x1024 0*/
		/*dev_zqinit_x32 16*/
#define TIMING_DRAMTMG0 0x0E0E0B0B
		/*t_ras_min 11*/
		/*t_ras_max 11*/
		/*t_faw 14*/
		/*wr2pre 14*/
#define TIMING_DRAMTMG1 0x00020711
		/*t_rc 17*/
		/*rd2pre 7*/
		/*t_xp 2*/
#define TIMING_DRAMTMG2 0x08090508
		/*wr2rd 8*/
		/*rd2wr 5*/
		/*read_latency 9*/
		/*write_latency 8*/
#define TIMING_DRAMTMG3 0x00003006
		/*t_mod 6*/
		/*t_mrd 3*/
		/*t_mrw 0*/
#define TIMING_DRAMTMG4 0x01030306
		/*t_rp 6*/
		/*t_rrd 3*/
		/*t_ccd 3*/
		/*t_rcd 1*/
#define TIMING_DRAMTMG5 0x04040303
		/*t_cke 3*/
		/*t_ckesr 3*/
		/*t_cksre 4*/
		/*t_cksrx 4*/
#define TIMING_DRAMTMG8 0x00000C03
		/*t_xs_x32 3*/
		/*t_xs_dll_x32 12*/
		/*t_xs_abort_x32 0*/
		/*t_xs_fast_x32 0*/
#define TIMING_DFITMG0 0x03070106
		/*dfi_tphy_wrlat 6*/
		/*dfi_tphy_wrdata 1*/
		/*dfi_wrdata_use_sdr 0*/
		/*dfi_t_rddata_en 7*/
		/*dfi_rddata_use_sdr 0*/
		/*dfi_t_ctrl_delay 3*/
#define TIMING_ZQCTL0 0x407F0020
		/*t_zq_short_nop 32*/
		/*t_zq_long_nop 127*/
		/*dis_mpsmx_zqcl 0*/
		/*zq_resistor_shared 0*/
		/*dis_srx_zqcl 1*/
		/*dis_auto_zq 0*/
