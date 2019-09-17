


/*-----------------------------------------------------------------------------
    defines	for DAB APP Request Message id
-----------------------------------------------------------------------------*/

/**
 * @brief startup request message id DAB_APP_STARTUP_REQID
 */
#define DAB_APP_STARTUP_REQID			((Tu16)0x4000)

/**
 * @brief instance hsm startup message id from main hsm to instance hsm DAB_APP_INST_HSM_STARTUP
 */
#define DAB_APP_INST_HSM_STARTUP		((Tu16)0x4001)

/**
 * @brief Station list update request DAB_APP_GETSTL_REQID
 */
#define DAB_APP_GETSTL_REQID			((Tu16)0x4002)

/**
 * @brief select DAB band request message id  DAB_APP_SELECT_DAB_REQID
 */
#define DAB_APP_SELECT_DAB_REQID		((Tu16)0x4003)

/**
  * @brief de-select DAB band request message id  DAB_APP_DESELECT_DAB_REQID
 */
#define DAB_APP_DESELECT_DAB_REQID		((Tu16)0x4004)

/**
 * @brief tune request message id  DAB_APP_PLAY_SEL_STN_REQID
 */
#define DAB_APP_PLAY_SEL_STN_REQID		((Tu16)0x4005)

/**
 * @brief shutdown request message id DAB_APP_SHUTDOWN_REQID
 */
#define DAB_APP_SHUTDOWN_REQID			((Tu16)0x4006)

/**
 * @brief instance hsm shutdown message id from main hsm to instance hsm DAB_APP_INST_HSM_SHUTDOWN
 */
#define DAB_APP_INST_HSM_SHUTDOWN		((Tu16)0x4007)

/**
 * @brief instance hsm service component seek message id from main hsm to instance hsm DAB_APP_SER_COMP_SEEK_REQID
 */
#define DAB_APP_SER_COMP_SEEK_REQID		((Tu16)0x4008)


#define DAB_APP_DAB_FM_LINKING_ENABLE_REQID			((Tu16)0x4009)

#define DAB_APP_CANCEL_REQID					((Tu16)0x400A)

#define DAB_APP_TUNEUPDOWN_REQID	((Tu16)0x400C)

#define	DAB_APP_FM_DAB_LINKING_PI	((Tu16)0x400D)

#define DAB_APP_INTERNAL_ANNO_MSG			((Tu16)0x400E)

#define DAB_APP_CANCEL_ANNO_REQID			((Tu16)0x400F)

#define DAB_APP_ANNO_CONFIG_REQID			((Tu16)0x4010)	

#define	DAB_APP_AF_TUNE_REQID				((Tu16)0x4011)

#define DAB_APP_DABTUNER_RESTART_REQID		((Tu16)0x4012)
#define	DAB_APP_ENG_MODE_REQID				((Tu16)0x4013)
#define DAB_APP_ACTIVATE_DEACTIVATE_REQID	((Tu16)0x4014)
#define DAB_APP_DAB_AF_SETTINGS_REQID		((Tu16)0x4015)
#define	DAB_APP_FACTORY_RESET_REQID			((Tu16)0x4016)
#define DAB_APP_MANUAL_TUNEBY_CHNAME_REQID	((Tu16)0x4017)
#define	DAB_APP_COMP_LIST_SORT_REQID	    ((Tu16)0x4018)
/*-----------------------------------------------------------------------------
	defines for DAB APP Response Message id
-----------------------------------------------------------------------------*/

/**
 * @brief Tuner Ctrl startup response message id DAB_APP_START_RESID
 */
#define DAB_APP_STARTUP_DONE_RESID		((Tu16)0x4500)

/**
 * @brief instance hsm start internal message id DAB_APP_INST_HSM_START_DONE
 */
#define DAB_APP_INST_HSM_START_DONE		((Tu16)0x4501)

/**
 * @brief Tuner Ctrl Station list update response message id DAB_APP_SCAN_RESID
 */
#define DAB_APP_SCAN_RESID				((Tu16)0x4502)

/**
 * @brief tune response message id DAB_APP_PLAY_SEL_STN_RESID
 */
#define DAB_APP_PLAY_SEL_STN_RESID		((Tu16)0x4503)

/**
 * @brief DAB is active band response message id DAB_APP_ACTIVATE_RESID
 */
#define DAB_APP_ACTIVATE_RESID			((Tu16)0x4504)

/**
 * @brief DAB is inactive band response message id DAB_APP_DEACTIVATE_RESID
 */
#define DAB_APP_DEACTIVATE_RESID		((Tu16)0x4505)

/**
  * @brief shutdown response message id DAB_APP_SHUTDOWN_RESID
 */
#define DAB_APP_SHUTDOWN_RESID			((Tu16)0x4506)

/**
 * @brief instance hsm shutdown internal message id DAB_APP_INST_HSM_SHUTDOWN_DONE
 */
#define DAB_APP_INST_HSM_SHUTDOWN_DONE	((Tu16)0x4507)

/**
 * @brief service component seek message id DAB_APP_SER_COMP_SEEK_RESID
 */
#define DAB_APP_SER_COMP_SEEK_RESID		((Tu16)0x4508)


#define DAB_APP_DAB_FM_LINKING_ENABLE_RESID			((Tu16)0x4509)

#define DAB_APP_SEEK_CANCEL_RESID		((Tu16)0x450A)


#define DAB_APP_START_ANNO_RESID		((Tu16)0x450C)

#define DAB_APP_STOP_ANNO_RESID			((Tu16)0x450D)

#define DAB_APP_CANCEL_ANNO_RESID		((Tu16)0x450E)

#define DAB_APP_ANNO_CONFIG_RESID		((Tu16)0x450F)

#define DAB_APP_CANCEL_RESID			((Tu16)0x4510)

#define DAB_APP_DABTUNER_RESTART_RESID	((Tu16)0x4511)
#define	DAB_APP_AF_LIST_RESID			((Tu16)0x4512)
#define DAB_APP_ACTIVATE_DEACTIVATE_RESID 	((Tu16)0x4513)

#define	DAB_APP_INST_HSM_DEACTIVATE_DONE	((Tu16)0x4514)

#define DAB_APP_DAB_AF_SETTINGS_RESID			((Tu16)0x4515)

#define	DAB_APP_DAB_DAB_STATUS_NOTIFYID		((Tu16)0x4516)

#define	DAB_APP_FACTORY_RESET_DONE_RESID	((Tu16)0x4517)
#define DAB_APP_ABORT_SCAN_RESID            ((Tu16)0x4518)
/*-----------------------------------------------------------------------------
	defines for DAB APP Notify Message id
-----------------------------------------------------------------------------*/

/**
 * @brief Station info update notification DAB_APP_STL_UPDATE_NOTIFYID
 */
#define DAB_APP_STL_UPDATE_NOTIFYID		((Tu16)0x4A00)

/**
 * @brief Current tuned station status info notification DAB_APP_STATUS_NOTIFYID
 */
#define DAB_APP_STATUS_NOTIFYID			((Tu16)0x4A01)

/**
 * @brief Current tuned station status info notification DAB_APP_FREQ_CHANGE_NOTIFYID
 */
#define DAB_APP_FREQ_CHANGE_NOTIFYID	((Tu16)0x4A02)

#define DAB_APP_PICODE_LIST_NOTIFYID	((Tu16)0x4A03)

#define DAB_APP_BESTPI_NOTIFYID						((Tu16)0x4A04)

#define DAB_APP_DAB_FM_LINKING_STATUS_NOTIFYID		((Tu16)0x4A05)

#define DAB_APP_PI_QUALITY_NOTIFYID					((Tu16)0x4A06)

#define DAB_APP_DAB_FM_BLENDING_STATUS_NOTIFYID		((Tu16)0x4A07)

#define DAB_APP_DAB_DLS_DATA_NOTIFYID				((Tu16)0x4A08)
#define DAB_APP_RECONFIG_NOTIFYID					((Tu16)0x4A09)

#define	DAB_APP_FM_DAB_LINKING_STATION_NOTIFYID		((Tu16)0x4A0A)

#define	DAB_APP_FM_DAB_LINKING_STATUS_NOTIFYID		((Tu16)0x4A0B)

#define DAB_APP_ANNO_NOTIFYID				        ((Tu16)0x4A0C)
#define DAB_APP_ANNO_STATION_INFO_NOTIFYID			((Tu16)0x4A0D)

#define	DAB_APP_DAB_FM_HARDLINKS_STATUS_NOTIFYID	((Tu16)0x4A0E)
#define	DAB_APP_ANNO_SIGNAL_LOSS_NOTIFYID			((Tu16)0x4A0F)

#define DAB_APP_COMPONENT_STATUS_NOTIFYID			((Tu16)0x4A20)

#define DAB_APP_AMFMTUNER_STATUS_NOTIFYID			((Tu16)0x4A21)			

#define DAB_APP_BACKGROUND_SCAN_START_NOTIFYID		((Tu16)0x4A22)
#define DAB_APP_STATIONNOTAVAIL_STRATERGY_STATUS_NOTIFYID	((Tu16)0x4A23)
#define DAB_APP_DAB_AF_LEARN_MEM_NOTIFYID			((Tu16)0x4A24)
#define DAB_APP_SIGNAL_LOST_NOTIFYID				((Tu16)0x4A25)
#define	DAB_APP_SIGNAL_STATUS_NOTIFYID				((Tu16)0x4A26)
#define DAB_APP_UPNOT_RECEIVED_NOTIFYID				((Tu16)0x4A27)
#define DAB_APP_FM_DAB_STOP_LINKING_NOTIFYID		((Tu16)0x4A28)
#define	DAB_APP_VERSION_NOTIFYID					((Tu16)0x4A29)
#define	DAB_APP_INIT_FMDAB_LINKING_NOTIFYID			((Tu16)0x4A2A)
#define	DAB_APP_GET_COMP_LIST_STATUS_NOTIFYID		((Tu16)0x4A2B)
#define DAB_APP_SYNCHRONISATION_NOTIFYID            ((Tu16)0x4A2C)
#define DAB_APP_AUTOSCAN_PLAY_STATION_NOTIFYID		((Tu16)0x4A2D)
#define DAB_APP_DAB_SLS_DATA_NOTIFYID				((Tu16)0x4A2E)
/*-----------------------------------------------------------------------------
	defines for HSM CID
-----------------------------------------------------------------------------*/

/**
 * @brief component id of instance hsm  DAB_APP_INST_HSM_CID
 */
#define DAB_APP_INST_HSM_CID			((Tu16)0xfe)