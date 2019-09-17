=====================================
Accordo2 / M3 / FreeRTOS / STAudioLib
=====================================
Last update: 2016/09/14


This package contains the STAudioLib development environment (hence still under development!).
For this environment, we use A2EVB/M3/FreeRTOS connected to emIDE via j-link.

NOTE: We are running on the Cortex-M3 core because we boot directly via emIDE via j-link, however STAudio
      could also be compiled and run seemlessly on the Cortex-R4.


The package is composed of two parts:


- Emerald:      the DSP code of the audio effect library used by STAudioLib. (DSP Firware)
		(pre-compiled binaries are located into FreeRTOS_M3\Demo\STA10XX_M3_emIDE_GCC\dsp_fw)


- FreeRTOS_M3:  the ARM code of FreeRTOS/STAudioLib (Audio Effect Driver)
  |
  |_ \Source                   -> contains the FreeRTOS source
  |_ \Demo
     |_ \STA10XX_M3_emIDE_GCC  -> contains the STA10XX_M3.emP project file + DSP binaries
     |_ \Common
        |_ \STAudioLib         -> contains the STAudioLib (ARM) src + tests
           |_ \inc
           |_ \src
           |_ \tests



HW and SW Requirements
======================

  HW requirements:
  . A2EVB board 
      * REMAP switches MUST be set to 'off-off'
      * DBG_CF switches MUST be set to 'on-on' (since A2EVB v20)
  . SEGGER j-link
  . SD card

  ARM IDE
  . emIDE
  . SEGGER j-link driver

  DSP IDE
  . Emerald Development Environment (EME)



Setting up the A2EVB
=====================

     *************************************************************
  1) !!! SET REMAP SWITCHES TO 'OFF-OFF' and DBG_CF to 'ON-ON' !!!
     *************************************************************

     ... in order to boot from emIDE via j-link instead of internal NAND or LIVEBOOT

  2) Connect the j-link from JTAG to PC

  3) Connect a UART cable from the mini USB-DEBUG plug to a PC. This is to read the prints on the hyperterminal of the PC.

  4) Connect a speaker to the stereo lines: DAC_OUT0_L / DAC_OUT0_R

  5) Copy the song 'SD\James Bond Theme_48Khz_16bit_stereo.raw' into an SD card.
     And insert it into the SDMMC0 (below the board)


Compiling the DSP Firmware [optionnal]
========================================

  The DSP binary is already compiled for DSP0 and DSP1 (same binaries), 
  but if you want to recompile it:

  1) Open the 'accordo2.jxt' project into the Emerald Development Environment.

  2) Select the target DSP from DSP0, DSP1(actually same as DSP0) and DSP2:
  		step1: in compiler_switches.h, uncomment one of:
  			#define ACCORDO2_DSP0
  			#define ACCORDO2_DSP1
  			#define ACCORDO2_DSP2
  			
  		step2: in Project Option ->  Memory tab -> Configuration file, select one of:
  			memory_config_A2_DSP0.h
  			memory_config_A2_DSP1.h
  			memory_config_A2_DSP2.h	
  
  3) Click re-build the project
  	 output: 
  	 the output are 3 binary files located into Emerald\audio_programmable\_ede_build:
  	 - emerald.P: the Program file that will be loaded into the DSP PMEM
  	 - emerald.X: the X data  file that will be loaded into the DSP XMEM
  	 - emerald.Y: the Y data  file that will be loaded into the DSP YMEM

  4) install:
     Copy these 3 binary files into the FreeRTOS folder:
     FreeRTOS_M3\Demo\STA10XX_M3_emIDE_GCC\dsp_fw

    (note: you can use the update_dspX.bat files in this folder)


Compiling the STAudioLib and test applications
==============================================

  1) Open the 'STA10XX_M3.emP' project (located in FreeRTOS_M3\Demo\STA10XX_M3_emIDE_GCC)
     in the emIDE.

  2) In the 'main.c', function STAudioTestTask(), select the test to be compiled
     e.g. test_equilizer()


Running the test application
============================

  1) run the JLINK-SERVER

  2) from the emIDE, 'start' the DEBUG mode


  Some tests (test_loudness and test_equalizer) have user control via the Rotary and Keypad.
  Check their source code to see the keypad mapping.


Tested features / modules.
=========================
  - STAudioLib modules: ONLY the following modules have been validated:
	o MUX          -> test_mux.c
	o GAIN         -> test_gain.c
	o SMOOTH GAIN  -> test_gain_smooth.c
	o MIXER        -> test_mixer.c
	o BALANCE/FADE -> test_mixer_balance.c
	o LOUDNESS     -> test_loudness.c
	o EQUALIZER    -> test_equalizer
	o DELAY        -> test_delay.c
	o SINEWAVGEN   -> test_sinewaveGen.c
	o USERMODULE   -> test_userModule.c
	o DSP0 + DSP1  -> test_dsp0_dsp1.c
	o ADCmic       -> test_ADCmic.c
	o DCO DETECT   -> test_dco_4chmonozcf.c
	o CD-DEEMPHASIS-> test_cdddeemphasis.c
	o LIMITER      -> test_limiter2.c
	o CLIP LIMITER -> test_clip_limiter.c
	o COMPANDER    -> test_compander.c
	o PCMCHIME     -> test_pcmchime.c
	o POLYCHIME    -> test_polychime.c
	o BITSHIFTER   -> test_bitshifter.c
	o PEAK DETECTOR-> test_peak_detector.c
	
	
Current Limitations
===================

  - Front Panel: 
  	o no support yet for the Dotmatrix. Instead connect the A2EVB-UART(usb debug) to a PC and use an hyperterminal.
    o With the A2_EVB_V20 board, the keypad col2 is sometime not detected...

  - DSP2: DSP2 is only functional from CUT2.2 (thus the dsp2 binary should only be used from CUT2.2)
  
  - Not tested: 
    o TONE and LOUDNESS automatic modes
  

  
Bug report
==========

  PLEASE, report any bugs/strange behaviours to us (christophe.quarre@st.com), thanks!



Change Logs
===========
2015/02/25	STA v8 (needs DSP v8)
			. modified the dsp fw loading mechanism to support a different set of fw for the DSP2.
			. renamed the dsp binaries from emerald.P to dsp0.P ..
 
2015/05/04	STA v9 (needs DSP v9)
			. added DCO v1 (ref)
			
2015/05/26	STA v10 (needs DSP v10)
			. DCO v2 (increased max delay; improved test_dco_4chmonozcf.c)
			. TESTS: Introduced task for miniplayer
			. DSP2 fw provided
			
2015/06/01	STA v11 (needs DSP v11)
			. DCO v3 (add matching stats + selectable score window)
			
2015/06/10	STA v12 (needs DSP v11)	
			. STA_SinewaveGenPlay(): decreased amplitude step to +-0.1% full scale
			. STA_DCOSetParam: decreased winth and dcoffset step to +-0.1% full scale
			. test_dco_4chmonozcf: using timer to check for dc offset every sec.
			. FrontPanel: seems to have fixed col2/row2 missdetection issue on A2EVBv20

2015/06/25	STA v13 (needs DSP v11)	
			. added STA_SetDspISR() and STA_SetFSyncISR()
			. added getter for STA_DCO_SWZCF and STA_DCO_HWZCF 
			. test_dco_4chmonozcf: forward swZCF and hwZCF to GPIOs for analysis on oscillo.
			. added some missing volatile attributes...
			
2015/07/17  STA v14 (needs DSP v12)
			. add support for DMABUS transfers @CK1,CK2:
				- STA_DMAAddTransfer2()
				- STA_DMASetFSyncCK1(), STA_DMASetFSyncCK2()
			. fixed CD-DEEMPHASIS
			. test: added support for playing 16khz via LPFSRC
			. test: added test_usecase_ecnr.c for DSP-AHB-IO workaround (for asynchronous DSP mode)

2015/08/10  STA v15 (needs DSP v13)
			. fixed SMOOTH_GAIN mute issue (was not muting completely).

2015/11/12  STA v16 (needs DSP v14)
			. STA:     added 11 and 12 bands EQ
			. ECNRlib: fixed some timing issue
			. UIF:     UART3 Interface to control the ECNR (DSP2) and later the AUDIO

2015/12/14  STA v17 (needs DSP v15)
			. fixed Smooth Gain glitch

2015/12/21  STA v18 (needs DSP v16)
			. fixed LIMITER (some params remain to be tested)

2016/02/04  STA v19 (needs DSP v17)
			. improved EQ DP (Dual Precision): up to 16bands, reduced mips, reduced "pop" noise (implementing dual coefs buffer).
			. adding   EQ SP (Simple Precision): up to 16bands, fast biquads. Added in DSP fw but not yet integrated in STA

2016/02/12  STA v20 (needs DSP v17)
			. added EQ SP (Simple Precision)
			. added new APIS for EQ/LOUDNESS/BMT tunning:
				STA_SetFilterHalvedCoefs()
				STA_SetFilterHalvedCoefsAll()
				STA_GetFilterHalvedCoefs()
				STA_GetFilterHalvedCoefsAll()
			. added new APIs for updateSlot management (mainly for LIMITER and COMPANDER)
				STA_SetNumOfUpdateSlots() (default of 6 slots)
				STA_SetUpdateSlots() (default is slot 0)
				
2016/02/25  STA v21 (needs DSP v18)
			. added PCMCHIME (for clic-clac generator)
			. fixed COMPANDER (ST version)

2016/03/14  STA v22 (needs DSP v19)
			. added LINEAR gain

2016/03/21  STA v23 (needs DSP v20)
			. added CLIP LIMITER

2016/04/01  STA v24 (needs DSP v21)
			. added POLY-CHIME / BEEP
			. changed sinewaveGen mode (on/off) is no more used as duration (incl infinite) is set directly via 
			   STA_SinewaveGenPlay(), STA_SinewaveGenGen()

2016/04/07  STA v25 (needs DSP v22)
			. added STA_ChimeGenGetParam()
			. TODO: patch _STA_TCtoSmoothFactor with 1/eps in NO_FLOAT (for Polychime exp ramp time)

2016/04/13  STA v26 (needs DSP v22)
			. added All-pass filter (STA_BIQUAD_ALLP_2)
			. added #define STA_BIQ_GAIN_SCALE (=4)
			. fixed Compander set STA_COMP_ATTENUATION in float mode.

2016/04/22  STA v27 (needs DSP v22)
			. added STA_GainSetLinearGainsAndShifts() to implement bitshifter.
			. added STA_GetInfo()
			. removed STA_GetSamplingFrequency() (replaced by STA_GetInfo(STA_INFO_SAMPLING_FREQ))
			. impl  _STA_SetLinLimiter(),  _STA_GetInterpolatedValue() in NO_FLOAT
			
2016/05/10  STA v28 (needs DSP v22)
			. added STA Cmd parser (PC -> UART_IF -> STA Cmd parser -> STA) WORK-IN-PROGRESS
			. added some APIs for passing 'ch' instead of chMask. (for the STA Cmd Parser)
			. added STA_GetModuleInfo()
			. changed STA_BIQ_GAIN_SCALE = 10 (from 4)
			. fixed BIQ_Bypass() output shift
			. added STA_ChimeGenSetParam() / STA_ChimeGenGetParam() for PCMChime as well
			
2016/05/10  STA v29 (needs DSP v23)
			. fixed Mixer Ramps (with dsp MIXER2)

2016/05/27  STA v30 (needs DSP v23)	
			STA core:
			. added missing filter conv in Fixed (STA_NO_FLOAT)
			. added STA_MixerSetInGain() 
			OTHERS
			. added STA Preset Loader (WORK IN PROGRESS)
			. test_miniplayer with M5 preset (defined in st_gui_module_id_preset_M5.h)
			. added support for PC SAT v0.2
			
2016/06/08  STA v31 (needs DSP v23)	
			STA core:
			. fixed STA_MixerSetInGain() 
			. patched Biquad SP rescaling the bi coef to stay below 1 (to avoid crash with PC GUI)
			STA Preset Loader:
			. added STA_GetModuleName() and STA_GetModuleEntry()
			. moved M5 preset stuff in AudioPresets/M5
			STA cmd parser:
			. fixing STA Cmd parser for the PC SAT v0.3 (NO MORE COMPATIBLE WITH PC SAT v0.2)
			UIF
			. added GSM-A8 Authentication

2016/06/14  STA v32 (needs DSP v24)	
			. fixed MIXER2 input ramps
			. added BITSHIFTER

2016/06/16  STA v33 (needs DSP v25)	
			. go back to MIXER1 (MIXER2 needs MIPS optimization)
			. added 9 inputs MIXER (for both versions 1 and 2)

2016/06/24  STA v34 (needs DSP v25)	
			. Fixed Dynamic Range Limiter tuning issue with Hysteresis parameter
			Precision increased from 1/10 dB to 1/100 dB
			New range [0:600] => from 0 dB up to 6.00 dB

2016/07/11  STA v35 (needs DSP v26)	
			. added Peak Detector

2016/09/14  STA v36 (needs DSP v27)	
			. fixed support for full DSP2 memory size
			. removed DCO from DSP bin to free a lot of PRAM
			. patched STA_MixerSetInGains() to force a minimal ramp if precison leads to local_inc = 0
			. patched Fixed version of _STA_TCtoSmoothFactor() for linux

2016/09/14  STA v37 (needs DSP v27)
			. added STA_GainGetGain()
			. changed PCMChime API to be aligned with polychime:
					- normal stop (finish the pcm): STA_ChimeGenSetParam(STA_CHIME_REPEAT_COUNT, 0)
					- hard stop: STA_PCMStop()

2016/09/22  STA v38 (needs DSP v27)
			. added STA_LimiterGetParam()
			. Limiter's dynamic gain and peak now exposed to outchannels (n) and (n+1)
			. rewrote _STA_db2lin() in float (improved accuracy for "small" gains)

2016/11/11  STA v39 (needs DSP v28)
			. optimized smooth mixer (MIXER3)
			. patched Limiter setparam to avoid issue when setting peak atk/rel times
			. more MISRA compliant

2016/12/05  STA v40 (needs DSP v28)
			. added STA_CHIME_POST_REPEAT_RAMPS (to enable/disable the post repeat ramps)
			. added STA_GainSetMaxGain() (to support up to +96 dB)
			. fixed glitch issue with smooth gain ramp.
			. MISRA compliant (zero lint message)
			
2017/01/05  STA v41 (needs DSP v29)
			. added STA_Enable(), STA_Disable() (only param is STA_FREE_CYCLE_COUNTER)
			. added STA_GetFreeCycles(), STA_PrintFreeCycles()
			. improved EQ_SP: added bshift to extend Gain range. Added Gain in LPF, HPF (disabled by default)

2017/03/27  STA v42 (needs DSP v30)
			. added SPECTRUM METER
			. added cookbook filters: PBF, LPF, HPF, Notch, Peaking, LowShelf, HighShelf 
			. added STA_SetFilterParam(), STA_SetFilterParams() 
			. added STA_MixerSetRelativeInGain(), STA_MixerSetAbsoluteInGain()
			. added getters
				- STA_GetFilterParam(), STA_GetFilterParams()
				- STA_MixerGetRelativeInGain(), STA_MixerGetAbsoluteInGain()
				- STA_GetModuleInfo() getters: STA_INFO_TYPE, STA_INFO_NUM_IN_CHANNELS, STA_INFO_NUM_OUT_CHANNELS,...
				- STA_SmoothGainGetRamp(), STA_GainGetMaxGain()
				- STA_DelayGetLength(), STA_DelayGetDelay()
				- STA_MuxGet()
			. moved OS-dependant APIs to separate .c (API_freertos.c...)

2017/05/12  STA v43 (needs DSP v31)
			. added STA_GainSetPolarity() (ONLY for smooth gain for now...)
			. added EQ 1 and 2 bands
			. replaced EQ_DP with EQ_MP (more optimized)
			. changed COMPANDER hysteresis's scale from 0.1 to 0.01 dB
			. _STA_lin2db now returns dB/100 instead of dB/10

2017/06/06  STA v44 (needs DSP v31)
			. Merged linux diff (DSP_BASE[], DSP_READ, DSP_WRITE, STA_WITH_DMABUS...)
			. added STA_WaitDspReady()
			. added STA_LoadDSP_ext()
			. moved internal DSP_READ, DSP_WRITE... to internal_user.h for USER_MODULE

2017/06/21  STA v45 (needs DSP v31)
			. Misra 2004 (with PClint 9.00L)
			. fixed: not reset XIN[127]
			
2017/06/26  STA v46 (needs DSP v31)
			. more Misra 2004 (with PClint 9.00L)
			
2017/07/04  STA v47 (needs DSP v31)
			. merged diff from linux STA
			. added get STA_GetModuleInfo(,STA_INFO_MAX_DSP_CYCLES) and STA_GetMaxDspCycleCost(STA_Dsp core)
			. added STA_Disconnect()
			. added STA_AddModule2(), STA_AddUserModule2()
			. added STA_GetModuleName()  (renaming the previous one STA_GetPresetModuleName())
			. fixed STA_ClipLimiterSetParams forcing convergence even if CLIP_SIG is not updated
			. STA_Connect() checks that the new connection does not already exist
			. STA_DeleteModule() checks if 'module' or '*module' is a STAModule, ID or name

2017/07/13  STA v47 bis (needs DSP v32)
			. fixed a missing saturate instruction in DSP mixer
			
2017/10/09  STA v48 (needs DSP v33)
			. added STA_GetDspMemCost()
			. added STA_ReconnectFrom(), STA_ReconnectTo()
			. added STA_CALL_TRACE to enable STA tracing
			. added more getters in STA_Parser
			. Chime
			  - added STA_RAMP_SMP ramp, static ramp in samples (for short precise adjustment up to 5 msec)
			  - added STA_CHIME_MASTER_CHIME param to synchronise polychimes

2017/12/11  STA v49 (needs DSP v34)
			. POLYCHIME: repeat synchro patch
			. SPECMETER: 
				- STA_SpectrumMeterSetDecayFactor(): fixed a missing x10 scaling factor to the decayFactor
					=> /!\  IMPACT ON PREVIOUS TUNING OF THE DECAY FACTOR
				- STA_SpectrumMeterGetPeaks(): patch to return MUTE when detect a flat signal.
				

2018/01/04  STA v50 (needs DSP v35)
			. Modified DSP code to avoid bug: rsr followed by forbidden instruction loopblock
			. Modified STA and M3 code loading DSP firmware to avoid bug: rsr followed by uninitialized memory
			. Fixed SW Limiter and Clip Limiter dB ranges

2018/02/22  STA v51 (needs DSP v35)
			. Added STA_INFO_XDATA and STA_INFO_YDATA for Linux suspend/resume
			. Fixed Muxer interface comments for STA_MuxSet and STA_MuxSetOutChannel
			. Fixed Delay interface comments to set delay length
			. Fixed Tone filter type for treble band

2018/03/08  STA v52 (needs DSP v36)
			. increased STA_MAX_MUX_INS from 16 to 18
			. fixed getModule() crash bug calling with ID instead of STAModule. (TODO: check if crash with 'by name')

2018/05/03  STA v53 (needs DSP v36)
			. Fixed MIXER output gain to reach positive values up to +24dB (improved auto test coverage)
			. Merged STA modifications from Linux team
			. Modified STA_Init, STA_BuildFlow and STA_Stop behavior to let DSPs which are managed by 3rd parties

2018/07/11  STA v54 (needs DSP v37)
			. Merged STA modif from Linux
			. Updated SpectrumMeter cycles
			. Merged Header copyrights from TKernel
			. Fixed an hypothetic division by zero with linear ramps (linear gain and input mixer)
			. Fixed linear gain pop noise, updated linear gain cycles
			. Fixed exponential gain pop noise, updated exponential gain cycles

2018/07/16  STA v55 (needs DSP v38)
			. Fixed mixer output exponential gain pop noise, updated mixer cycles

2018/08/16  STA v56 (needs DSP v38)
			. Fixed lose of chime ramps when some have 0 ms duration
			. Added STA_GetDspMaxMem to get the maximum available X and Y memory in a given DSP core
			. Updated License to GPL version 2.0 for Linux

2018/11/30  STA v57 (needs DSP v39)
			. Fixed exponential gain, systematic little pop noise on down ramps, updated exponential gain cycles
			. Merged warning fixes from Linux
			. few MISRA fixes
			. few Coverity fixes
			. Peak Detector fix, improved automatic tests
			. activated YMEM automatic tests

