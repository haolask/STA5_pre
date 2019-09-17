Build command lines
===================

For release built with SP_MIN as Secure Payload:
make PLAT=sta ARCH=aarch32 CROSS_COMPILE=<path to toolchain>/arm-none-eabi- AARCH32_SP=sp_min BL33=bl33.bin MEMORY_MAP_XML=<path to XML memory mapping description> plat_config all


For debug built with SP_MIN as Secure Payload:
make PLAT=sta ARCH=aarch32 CROSS_COMPILE=<path to toolchain>/arm-none-eabi- AARCH32_SP=sp_min DEBUG=1 BL33=bl33.bin MEMORY_MAP_XML=<path to XML memory mapping description> plat_config all

