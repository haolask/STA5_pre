#ifndef TARGET_CONFIG_H
#define TARGET_CONFIG_H

// Select between solution Qt stand-alone compilation and
// Qt + ETAL version. main differences are:
// - Stand-alone is launching TCP/IP and uses MW and CIS commands
// - ETAL rely on underlying ETAL library to do communication
//   with the IC
//#define CONFIG_USE_STANDALONE
//#undef CONFIG_USE_ETAL

#undef CONFIG_USE_STANDALONE
#undef CONFIG_USE_ETAL
#define CONFIG_USE_JASMIN

//#define CONFIG_BOARD_SIGLE_TUNER
//#undef  CONFIG_BOARD_DUAL_TUNER

#undef  CONFIG_BOARD_SIGLE_TUNER
#define  CONFIG_BOARD_DUAL_TUNER

#endif // TARGET_CONFIG_H

// End of file
