#ifndef RADIO_STORAGE_TYPES_H
#define RADIO_STORAGE_TYPES_H

#include <QMetaType>

#include <string> // std::string

#include "common.h"

#define STATION_NAME_LEN                16
#define ENSEMBLE_NAME_LEN               16

enum State
{
    ACTIVE_STATE,
    STOPPED_STATE
};

enum SystemMode
{
    MODE_AM,
    MODE_FM,
    MODE_DAB,
    MODE_DRM,
    MODE_HD
};

enum CountryTy
{
    COUNTRY_EU,
    COUNTRY_US,
    COUNTRY_JP,
    COUNTRY_EE,
    COUNTRY_CHI,
    COUNTRY_CAN,
    COUNTRY_KOR
};

enum BandTy
{
    BAND_AM,
    BAND_FM,
    BAND_DAB3,
    BAND_SW,
    BAND_MW,
    BAND_LW,
    BAND_DRM30,
    BAND_DRM_PLUS,
    BAND_WX
};

struct RadioPanelData
{
    bool                    isRadioOn;
    bool                    isHwConnectionOk;
    bool                    isGoodLevel;
    int                     ensembleIndex;
    EnsembleTableTy         ensembleTable;
    ServiceListTy           list;
    int                     serviceIndex;
    int                     currPresetIndex;
};

struct DynamicData
{
    bool            radioMute;     // Audio Mute(true)/Unmute(false) status
    State           radioState;    // RadioActive / Radio Stopped
    unsigned int    qualityLevel;   // Radio Receiver FIC/BER
    unsigned int    signalStrength; // Radio Receiver FieldStrength
};

struct PersistentData
{
    BandTy          band;          // Radio Tuned Band
    unsigned int    frequency;     // Radio Tuned Frequency
    unsigned int    radioVolume;   // Radio Volume
    SystemMode      radioMode;     // Radio selected Mode
    char            serviceName[STATION_NAME_LEN];   // Station name serviceName[16 chars];

    unsigned int    ensembleId;    // Ensemble ID (null in FM)
    unsigned int    serviceId;     // Service ID, in FM it is station PI (program Identification)

    char            ensembleName[ENSEMBLE_NAME_LEN];   // Station Ensemble Name (Null in FM) [16 chars]

    unsigned int    frequencyFm;   // Last FM tuned frequency
    unsigned int    frequencyAm;   // Last AM tuned frequency
    unsigned int    frequencyDab;  // Last DAB tuned frequency
    CountryTy       country;       // Radio Country

    // Setup variables
    bool            slsDisplayEn;
    bool            afCheckEn;
    bool            dabFmServiceFollowingEn;
    bool            dabDabServiceFollowingEn;
    bool            rdsTaEnabled;
    bool            rdsRegionalEn;
    bool            rdsEonEn;
    bool            globalServiceListEn;
    bool            displayUnsupportedServiceEn;
    bool            alphabeticOrder;
    bool            testScreenIsActive;
    unsigned char   ptySelected;

    // Signature
    unsigned int    signature;
};

Q_DECLARE_METATYPE(PersistentData)

struct RadioTuneAnswer
{
    unsigned int freq;
    BandTy band;
};

Q_DECLARE_METATYPE(RadioTuneAnswer)

struct RadioStatusCurrentTune
{
    DynamicData     dynamicData;
    PersistentData  persistentData;
};

struct RadioStatusGlobal
{
    RadioStatusCurrentTune radioStatus;
    RadioPanelData panelData;
};

#endif // RADIO_STORAGE_TYPES_H

// End of file
