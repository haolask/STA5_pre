#include "sta_mem_map.h"

MEMORY
{
  ESRAM_M3 (wx)   : ORIGIN = ESRAM_M3_BASE,  LENGTH = ESRAM_M3_SIZE
  ESRAM_AP (wx)   : ORIGIN = ESRAM_A7_BASE,  LENGTH = ESRAM_A7_SIZE
  FLASH_CACHE (x) : ORIGIN = 0x30000000, LENGTH = 4M
  DRAM (wx)       : ORIGIN = DDRAM_BASE,  LENGTH = DDRAM_SIZE
}

ENTRY(reset_handler)

SECTIONS
{
  __ESRAM_M3_start     = ORIGIN(ESRAM_M3);
  __ESRAM_AP_start     = ORIGIN(ESRAM_AP);
  __ESRAM_AP_end       = ORIGIN(ESRAM_AP) + LENGTH(ESRAM_AP);
  __DRAM_start         = ORIGIN(DRAM);
  __FLASH_start        = ORIGIN(FLASH_CACHE);
  __FLASH_end          = ORIGIN(FLASH_CACHE) + LENGTH(FLASH_CACHE);
  __STACKSIZE__        = 512;

  __SIGNATURE_LENGTH__ = 2K; /* FIXME must be updated to the max sign length */

#ifndef XIP
  #define MEM_CODE      ESRAM_M3 /* Code in M3 ESRAM */
  #define START_OF_CODE __ESRAM_M3_start
  #define START_OF_DATA __text_end__
  __M3_SOFTWARE_END = ESRAM_M3_XL_SOFTWARE_BASE + ESRAM_M3_XL_SOFTWARE_SIZE;

  #if defined(ATF) && !defined(BOOT_M3OS_FROM_M3XL)
    /* No constraint with ATF, M3 OS is read by ATF not M3 XL */
    __MAX_XL_SIZE_TO_SHADOW__ = LENGTH(ESRAM_M3);
  #else
    /* Size max, resident M3 xloader which loads M3 OS start there */
    __MAX_XL_SIZE_TO_SHADOW__ = 190K;
  #endif
#else
  #define MEM_CODE      FLASH_CACHE /* Code in Flash */
  #define START_OF_CODE __FLASH_start + 256K /* Size max of M3 Xloader */
  #define START_OF_DATA __ESRAM_M3_start
#endif

#ifndef MINI_OS
#ifdef ESRAM_A7_M3_CODE_BASE
  /*
   * M3 OS code is split in 2 parts.
   * Place here only non time critical code in this section .text2
   * AP_ESRAM has longer access time compare to M3_ESRAM
   * but on TC3P cut2 and higher we able to use instruction cache through
   * flash cache
   */
#ifdef USE_FLASHCACHE
  .text2 (__FLASH_start + ALIGN(ESRAM_A7_M3_CODE_BASE, 4) - ESRAM_A7_BASE) : AT(ALIGN(ESRAM_A7_M3_CODE_BASE, 4))
#else
  .text2 ALIGN(ESRAM_A7_M3_CODE_BASE, 4) :
#endif
  {
    __text2_start__ = .;
	printf-stdarg.*(.text .text.* .rodata .rodata.*)
	trace.*(.text .text.* .rodata .rodata.*)
	FreeRTOS_CLI.*(.text .text.* .rodata .rodata.*)
	sta_wdt*.*(.text .text.* .rodata .rodata.*) /* Watchdogs */
	sta_ccc_test.*(.text .text.* .rodata .rodata.*) /* C3 tests */
	sta_rvc_test.*(.text .text.* .rodata .rodata.*) /* RVC tests */
	*_cli*.*(.text .text.* .rodata .rodata.*) /* Command Line Interface */
	etal*.*(.text .text.* .rodata .rodata.*) /* Radio middleware */
    __text2_end__ = .;
  }
  . = ASSERT(SIZEOF(.text2) <= ESRAM_A7_M3_CODE_SIZE , "error: .text2 is too large to fit in AP ESRAM memory segment");
#endif /* ESRAM_A7_M3_CODE_BASE */
#endif /* MINI_OS */

  __vectors_load_start__ = ALIGN(START_OF_CODE , 256);
  .vectors __vectors_load_start__ : AT(__vectors_load_start__)
  {
    __vectors_start__ = .;
    *(.vectors .vectors.*)
    __vectors_end__ = .;
  } > MEM_CODE
  __vectors_load_end__ = __vectors_load_start__ + SIZEOF(.vectors);

  __roinitdata_load_start__ = ALIGN(__vectors_load_end__ , 4);
  .roinitdata __roinitdata_load_start__ : AT(__roinitdata_load_start__)
  {
	  __roinitdata_start__ = .;
	  *(.roinitdata)
	  __roinitdata_end__ = .;
  } > MEM_CODE
  __roinitdata_load_end__ = __roinitdata_load_start__ + SIZEOF(.roinitdata);

  __init_load_start__ = ALIGN(__roinitdata_load_end__ , 4);
  .init __init_load_start__ : AT(__init_load_start__)
  {
    __init_start__ = .;
    *(.init .init.*)
    *(.fini)
    __init_end__ = .;
  } > MEM_CODE
  __init_load_end__ = __init_load_start__ + SIZEOF(.init);

  /* rodata are gathered into another section for HSM tests vectors */
#ifdef EHSM_TEST
  __rodataexidx_load_start__ = ALIGN(__init_load_end__ , 4);

  .rodataexidx ALIGN(__init_load_end__ , 4) : AT(__rodataexidx_load_start__)
#else
  __rodata_load_start__ = ALIGN(__init_load_end__ , 4);
  .rodata __rodata_load_start__ : AT(__rodata_load_start__)
  {
    __rodata_start__ = .;
    *(.rodata .rodata.* .gnu.linkonce.r.*)
    *(.note.gnu.build-id .note.gnu.build-id.*)
    __rodata_end__ = .;
  } > MEM_CODE
  __rodata_load_end__ = __rodata_load_start__ + SIZEOF(.rodata);

  __rodataexidx_load_start__ = ALIGN(__rodata_load_end__ , 4);

  .rodataexidx __rodataexidx_load_start__ : AT(__rodataexidx_load_start__)
#endif
  {
    __rodataexidx_start__ = .;
    __exidx_start = .;
    *(.ARM.exidx .ARM.exidx.*)
    __exidx_end = .;
    __rodataexidx_end__ = .;
  } > MEM_CODE
  __rodataexidx_load_end__ = __rodataexidx_load_start__ + SIZEOF(.rodataexidx);

  __text_load_start__ = ALIGN(__rodataexidx_load_end__ , 4);
  .text __text_load_start__ : AT(__text_load_start__)
  {
    __text_start__ = .;
    *(.text .text.*)
    *(.glue_7t .glue_7 .gnu.linkonce.t.* .gcc_except_table .ARM.extab* .gnu.linkonce.armextab.*)
    __text_end__ = .;
  } > MEM_CODE
  __text_load_end__ = __text_load_start__ + SIZEOF(.text);

#ifndef XIP
  . = ASSERT(__text_end__ < __M3_SOFTWARE_END , "error: .text is too large to fit in RAM memory segment");
#else
  . = ASSERT(__text_end__ < __FLASH_end , "error: .text is too large to fit in FLASH_CACHE memory segment");
#endif

  __data_load_start__ = ALIGN(__text_load_end__ , 4);
  .data ALIGN(START_OF_DATA , 4) : AT(__data_load_start__)
  {
    __data_start__ = .;
    *(.data .data.* .gnu.linkonce.d.*)
    __data_end__ = .;
  } > ESRAM_M3
  __data_load_end__ = __data_load_start__ + SIZEOF(.data);

  __hash_start__ = __data_load_end__;

#ifndef XIP
  . = ASSERT(__data_load_end__ < __ESRAM_M3_start + __MAX_XL_SIZE_TO_SHADOW__ , "error: secondary code size is too large (192KB Max)");
  . = ASSERT(__data_end__ < __M3_SOFTWARE_END , "error: .data is too large to fit in RAM memory segment");

  /* Add a gap after text & data end for signature and aligned it with 512 */
  __bss_start_addr__ = ALIGN(__data_end__ + __SIGNATURE_LENGTH__, 512);
#else
  . = ASSERT(__data_load_end__ < __FLASH_end , "error: .data is too large to fit in FLASH_CACHE memory segment");
  __bss_start_addr__ = ALIGN(__data_end__, 4);
#endif
 .bss __bss_start_addr__ (NOLOAD) :
  {
    __bss_start__ = .;
    *(.bss .bss.* .gnu.linkonce.b.*) *(COMMON)
    __bss_end__ = .;
  } > ESRAM_M3

#ifndef XIP
  . = ASSERT(__bss_end__ < __M3_SOFTWARE_END , "error: .bss is too large to fit in RAM memory segment");
#endif

  .stack ALIGN(__bss_end__ , 4) (NOLOAD) :
  {
    __stack_start__ = .;
    *(.stack .stack.*)
    . = ALIGN(MAX(__stack_start__ + __STACKSIZE__ , .), 4);
    __stack_end__ = .;
  } > ESRAM_M3

#ifndef XIP
  . = ASSERT(__stack_end__ < __M3_SOFTWARE_END, "error: .stack is too large to fit in RAM memory segment");
#endif

  /* M3 Shared data
   * Attention!!!: It must be persistent for next M3 boot stages */
  .shared_m3_data ALIGN(ESRAM_M3_SHARED_DATA_BASE, 4) (NOLOAD) :
  {
   __shared_m3_data_start__ = .;
   *(.shared_m3_data)
   __shared_m3_data_end__ = .;
  } > ESRAM_M3
  . = ASSERT(SIZEOF(.shared_m3_data) <= ESRAM_M3_SHARED_DATA_SIZE , "error: .shared_m3_data is too large");

  /* System config (TOC) is put at end of M3 ESRAM */
  .sysconfig ALIGN(ESRAM_M3_RUNTIME_TOC_BASE, 4) (NOLOAD) :
  {
   __sysconfig_start__ = .;
   *(.sysconfig)
   __sysconfig_end__ = .;
  } > ESRAM_M3
  . = ASSERT(SIZEOF(.sysconfig) <= ESRAM_M3_RUNTIME_TOC_SIZE , "error: .sysconfig is too large");


  MBOX ALIGN(DDRAM_APP_OS_MAILBOXES_BASE, 4) (NOLOAD) :
  {
	  __MBOX_start__ = .;
	  . = . + DDRAM_APP_OS_MAILBOXES_SIZE;
	  __MBOX_end__ = .;
  } > DRAM

#ifdef ATF
#if defined(ESRAM_A7_ATF_TRUSTED_ZONE_BASE)
  MBOX_SEC ALIGN(ESRAM_A7_ATF_IPC_BASE, 4) (NOLOAD) :
  {
	  __MBOX_SEC_start__ = .;
	  . = . + ESRAM_A7_ATF_IPC_SIZE;
	  __MBOX_SEC_end__ = .;
  } > ESRAM_AP
#elif defined(DDRAM_ATF_IPC_BASE)
  MBOX_SEC ALIGN(DDRAM_ATF_IPC_BASE, 4) (NOLOAD) :
  {
	  __MBOX_SEC_start__ = .;
	  . = __MBOX_SEC_start__ + DDRAM_ATF_IPC_SIZE;
	  __MBOX_SEC_end__ = .;
  }
#endif
#endif /* ATF */

#ifdef ESRAM_A7_C3_PROGRAMS_BASE
  .c3_programs ALIGN(ESRAM_A7_C3_PROGRAMS_BASE, 4) (NOLOAD) :
  {
    __c3_programs_start__ = .;
    *(.c3_programs)
    __c3_programs_end__ = .;
  }
  . = ASSERT(SIZEOF(.c3_programs) <= ESRAM_A7_C3_PROGRAMS_SIZE , "error: .c3_programs is too large");
#endif

#ifdef ESRAM_A7_KS_MBX_BUFFER_BASE
  .ks_mbx_buffer ALIGN(ESRAM_A7_KS_MBX_BUFFER_BASE, 4) (NOLOAD) :
  {
    __ks_mbx_buffer_start__ = .;
    *(.ks_mbx_buffer)
    __ks_mbx_buffers_end__ = .;
  } > ESRAM_AP
  . = ASSERT(SIZEOF(.ks_mbx_buffer) <= ESRAM_A7_KS_MBX_BUFFER_SIZE , "error: .ks_mbx_buffer is too large");
#endif

  .shared_data DDRAM_SHARED_DATA_BASE (NOLOAD) :
  {
    __shared_data_start__ = .;
    *(.shared_data)
    __shared_data_end__ = .;
  } > DRAM
  . = ASSERT(SIZEOF(.shared_data) <= DDRAM_SHARED_DATA_SIZE , "error: .shared_data is too large");

  .heap ALIGN(DDRAM_M3_OS_HEAP_BASE , 4) (NOLOAD) :
  {
    __heap_start__ = .;
    *(.heap .heap.*)
    . = ALIGN(MAX(__heap_start__ + DDRAM_M3_OS_HEAP_SIZE , .), 4);
    __heap_end__ = .;
  } > DRAM

}

#ifdef EHSM_TEST
SECTIONS
{
  /* Allocate area for HSM tests vectors at the end of TEE area */
  __EHSM_SIZE     = 6M;
  __EHSM_start    = DDRAM_ATF_TEE_BASE + DDRAM_ATF_TEE_SIZE - __EHSM_SIZE;
  __EHSM_end      = __EHSM_start + __EHSM_SIZE;

  __rodata_load_start__ = ALIGN(__EHSM_start , 4);
  .rodata ALIGN(__EHSM_start , 4) : AT(__rodata_load_start__)
  {
    __rodata_start__ = .;
    *(.rodata .rodata.* .gnu.linkonce.r.*)
    *(.note.gnu.build-id .note.gnu.build-id.*)
    __rodata_end__ = .;
  } > DRAM
  __rodata_load_end__ = __rodata_load_start__ + SIZEOF(.rodata);

  . = ASSERT(__rodata_end__ < __EHSM_end , "error: .rodata is too large to fit in RAM memory segment");
}
#endif

