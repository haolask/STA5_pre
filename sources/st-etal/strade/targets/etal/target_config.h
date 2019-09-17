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
#define CONFIG_USE_ETAL

//#define CONFIG_BOARD_SIGLE_TUNER
//#undef  CONFIG_BOARD_DUAL_TUNER

#undef  CONFIG_BOARD_SIGLE_TUNER
#define  CONFIG_BOARD_DUAL_TUNER

#if (defined CONFIG_BOARD_SIGLE_TUNER)
    // single tuner board setup
    // Foreground
    // FM setup
    #define RADIO_MNGR_ETAL_FG_FM_PATH  ETAL_FE_HANDLE_1
    // DAB setup
    #define RADIO_MNGR_ETAL_FG_DAB_PATH ETAL_FE_HANDLE_1
    // AM setup
    #define RADIO_MNGR_ETAL_FG_AM_PATH  ETAL_FE_HANDLE_1

    // Background
    // FM setup
    #define RADIO_MNGR_ETAL_BG_FM_PATH  ETAL_FE_HANDLE_2
    // DAB setup
    #define RADIO_MNGR_ETAL_BG_DAB_PATH ETAL_FE_HANDLE_2
#endif // #if (defined CONFIG_BOARD_SIGLE_TUNER)

#if (defined CONFIG_BOARD_DUAL_TUNER)
    // single tuner board setup
    // Foreground
    // FM setup
    #define RADIO_MNGR_ETAL_FG_FM_PATH  ETAL_FE_HANDLE_1
    #define RADIO_MNGR_ETAL_FG_FM_PD
    // DAB setup
    #define RADIO_MNGR_ETAL_FG_DAB_PATH ETAL_FE_HANDLE_3
    // AM setup
    #define RADIO_MNGR_ETAL_FG_AM_PATH  ETAL_FE_HANDLE_1

    // Background
    #define RADIO_MNGR_ETAL_BG_SHARED
    // FM setup
    #define RADIO_MNGR_ETAL_BG_FM_PATH  ETAL_FE_HANDLE_2
    // DAB setup
    #define RADIO_MNGR_ETAL_BG_DAB_PATH ETAL_FE_HANDLE_3

#endif // #if (defined CONFIG_BOARD_DUAL_TUNER)


#endif // TARGET_CONFIG_H

// End of file
