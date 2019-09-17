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
  __ESRAM_M3_end       = ORIGIN(ESRAM_M3) + LENGTH(ESRAM_M3);
  __ESRAM_AP_start     = ORIGIN(ESRAM_AP);
  __ESRAM_AP_end       = ORIGIN(ESRAM_AP) + LENGTH(ESRAM_AP);
  __FLASH_start        = ORIGIN(FLASH_CACHE);
  __FLASH_end          = ORIGIN(FLASH_CACHE) + LENGTH(FLASH_CACHE);
  __DRAM_start         = ORIGIN(DRAM);

  __STACKSIZE__        = 1K;
  __SIGNATURE_LENGTH__ = 2K; /* FIXME must be updated to the max sign length */

#ifndef XIP
  #define MEM_CODE      ESRAM_M3 /* Code in M3 ESRAM */
  #define START_OF_CODE __ESRAM_M3_start
  #define START_OF_DATA __text_end__
  /*
   * Code and data are put at end of M3 ESRAM to be able to shadow M3 OS
   * into start of M3 ESRAM
   */
  __ESRAM_M3_CODE_OFFSET = 170K;
  __ESRAM_M3_CODE_START = __ESRAM_M3_start + __ESRAM_M3_CODE_OFFSET;
  __M3_SOFTWARE_END = ESRAM_M3_XL_SOFTWARE_BASE + ESRAM_M3_XL_SOFTWARE_SIZE;

  __MAX_XL_SIZE_TO_SHADOW__ = 128K; /* Size of a NAND block */
#else
  #define MEM_CODE      FLASH_CACHE /* Code in Flash */
  #define START_OF_CODE __FLASH_start
  #define START_OF_DATA __ESRAM_M3_start
#endif

  __vectors_load_start__ = ALIGN(START_OF_CODE, 256);
  .vectors __vectors_load_start__ : AT(__vectors_load_start__)
  {
    __vectors_start__ = .;
    *(.vectors .vectors.*)
    __vectors_end__ = .;
  } > MEM_CODE
  __vectors_load_end__ = __vectors_load_start__ + SIZEOF(.vectors);

#ifndef XIP
  . = ASSERT(__vectors_end__ < __ESRAM_M3_CODE_START, "error: .vectors is too large");
#endif

  __sysconfig_init_load_start__ = ALIGN(__vectors_load_end__ , 4);
  .sysconfig_init __sysconfig_init_load_start__ : AT(__sysconfig_init_load_start__)
  {
   __sysconfig_init_start__ = .;
   *(.sysconfig_init)
   __sysconfig_init_end__ = .;
  } > MEM_CODE
  __sysconfig_init_load_end__ = __sysconfig_init_load_start__ + SIZEOF(.sysconfig_init);

  __roinitdata_load_start__ = ALIGN(__sysconfig_init_load_end__ , 4);
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

  __rodata_load_start__ = ALIGN(__init_load_end__ , 4);
#ifndef XIP
  . = ASSERT(__init_end__ < __ESRAM_M3_CODE_START , "error: .init is too large");
  .rodata ALIGN(__ESRAM_M3_CODE_START, 4) : AT(__rodata_load_start__)
#else
  .rodata ALIGN(__init_end__, 4) : AT(__rodata_load_start__)
#endif
  {
    __rodata_start__ = .;
    *(.rodata .rodata.* .gnu.linkonce.r.*)
    *(.note.gnu.build-id .note.gnu.build-id.*)
    __rodata_end__ = .;
  } > MEM_CODE
  __rodata_load_end__ = __rodata_load_start__ + SIZEOF(.rodata);

  __rodataexidx_load_start__ = ALIGN(__rodata_load_end__ , 4);
  .rodataexidx ALIGN(__rodata_end__ , 4) : AT(__rodataexidx_load_start__)
  {
    __rodataexidx_start__ = .;
    *(.ARM.exidx .ARM.exidx.*)
    __rodataexidx_end__ = .;
  } > MEM_CODE
  __rodataexidx_load_end__ = __rodataexidx_load_start__ + SIZEOF(.rodataexidx);

  __text_load_start__ = ALIGN(__rodataexidx_load_end__ , 4);
  .text ALIGN(__rodataexidx_end__ , 4) : AT(__text_load_start__)
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
  . = ASSERT(__text_load_end__ < __FLASH_end , "error: .text is too large to fit in FLASH_CACHE memory segment");
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
  . = ASSERT(__data_end__ + 32 < __M3_SOFTWARE_END , "error: .data is too large to fit in RAM memory segment");
  . = ASSERT(__data_load_end__ + 32 < __ESRAM_M3_start + __MAX_XL_SIZE_TO_SHADOW__ , "error: secondary code size is too large to fit in one NAND block (128KB)");

  /* Add a gap after text & data end for signature and aligned it with 512 */
  __bss_start_addr__ = ALIGN(__data_end__ + __SIGNATURE_LENGTH__, 512);
#else
  . = ASSERT(__data_load_end__ < (__FLASH_end - 32) , "error: .data is too large to fit in FLASH_CACHE memory segment");

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
#else
  . = ASSERT(__stack_end__ < __ESRAM_M3_end, "error: .stack is too large to fit in RAM memory segment");
#endif

  /* M3 Shared data
   * Attention!!!: It must be persistent for next M3 boot stages */
  .shared_m3_data ALIGN(ESRAM_M3_SHARED_DATA_BASE, 4) (NOLOAD) :
  {
   __shared_m3_data_start__ = .;
   *(.shared_m3_data)
   __shared_m3_data_end__ = .;
  } > ESRAM_M3
  . = ASSERT(SIZEOF(.shared_m3_data) < ESRAM_M3_SHARED_DATA_SIZE , "error: .shared_m3_data is too large");

  /* System config (runtime M3 TOC) is put at end of M3 ESRAM,
   * Attention!!!: It must be persistent for next M3 boot stages */
  .sysconfig ALIGN(ESRAM_M3_RUNTIME_TOC_BASE, 4) (NOLOAD) :
  {
   __sysconfig_start__ = .;
   *(.sysconfig)
   __sysconfig_end__ = .;
  } > ESRAM_M3
  . = ASSERT(SIZEOF(.sysconfig) <= ESRAM_M3_RUNTIME_TOC_SIZE , "error: .sysconfig is too large");

#ifdef ESRAM_A7_C3_PROGRAMS_BASE
  .c3_programs ALIGN(ESRAM_A7_C3_PROGRAMS_BASE, 4) (NOLOAD) :
  {
    __c3_programs_start__ = .;
    *(.c3_programs)
    __c3_programs_end__ = .;
  } > ESRAM_AP
  . = ASSERT(SIZEOF(.c3_programs) <= ESRAM_A7_C3_PROGRAMS_SIZE , "error: .c3_programs is too large");
#endif

  .shared_data DDRAM_SHARED_DATA_BASE (NOLOAD) :
  {
    __shared_data_start__ = .;
    *(.shared_data)
    __shared_data_end__ = .;
  } > DRAM
  . = ASSERT(SIZEOF(.shared_data) <= DDRAM_SHARED_DATA_SIZE , "error: .shared_data is too large");
}

