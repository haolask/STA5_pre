#ifndef DEFINES_H
#define DEFINES_H

// Return codes
#define OSAL_OK                                 ((int)0)
#define OSAL_ERROR                              ((int)-1)

// DAB frequency number
#define MAX_NB_DAB_FREQUENCIES                  41

// Volume defines
#define VOLUME_MAX_VALUE                        24
#define VOLUME_MIN_VALUE                        0

// Invalid values for generic data
#define INVALID_DATA                            ((unsigned int)0xFFFFFFFF)
#define INVALID_DATA_BYTE                       ((unsigned char)0xFF)
#define INVALID_DATA_U16                        ((unsigned short)0xFFFF)
#define INVALID_DATA_S16                        ((signed short)0xFFFF)

#define INVALID_SDATA                           ((signed int)-1)
#define DAB_FREQ_0_INDEX                        ((signed int)41)

#define INVALID_LONGLONG_DATA                   ((signed long long)-1)

// Invalid value frequency
#define INVALID_FREQUENCY                       INVALID_DATA

// Invalid value band
#define INVALID_BAND                            INVALID_DATA

// Invalid values for ensemble identifiers
#define INVALID_ENSEMBLE_ID                     ((unsigned int)0xFFFFFFFF)
#define INVALID_ECC                             ((unsigned char)0xFF)
#define INVALID_EID                             ((unsigned short)0xFFFF)

// Invalid service ID (valid for DAB, DRM and AM/FM)
#define INVALID_SERVICE_ID                      ((unsigned int)0xFFFFFFFF)

// Invalid DAB SCIDS
#define INVALID_SCS_ID                          ((unsigned char)0xFF)

// Invalid DRM stream
#define INVALID_STREAM                          INVALID_SCS_ID

#endif // DEFINES_H
