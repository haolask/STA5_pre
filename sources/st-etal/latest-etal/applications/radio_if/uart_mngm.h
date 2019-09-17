/****************************************************************************/
/*                                                                          */
/* Project:              ADR3 Control application                           */
/* Filename:             RS3232.H                                           */
/* Programmer:           Alessandro Vaghi                                   */
/* Date:                 Mar 16th 2010                                      */
/* CPU:                  PC                                                 */
/* Type:                 C code                                             */
/* Scope:                This is the windows specific serial port routines  */
/* Functions:                                                               */
/*                                                                          */
/****************************************************************************/

extern tVoid* CommParameterSetup (tU32 ComId, tU32 BaudRateBps);

extern tVoid CommPortClose (tVoid *deviceHandle);

#if defined(CONFIG_HOST_OS_WIN_VISUALSTUDIO) || defined(CONFIG_HOST_OS_WIN32_MINGW)

extern tU32 CommTransmitData (tVoid *deviceHandle, tUChar *WriteBuffer, tU16 BytesToSend);

extern tU32 CommReceiveData (tVoid *deviceHandle, tUChar *ReadBuffer, tU32 BytesToRead, tU32 RxTimeoutMs);

#elif defined(CONFIG_HOST_OS_LINUX) || defined(CONFIG_HOST_OS_TKERNEL) || defined(CONFIG_HOST_OS_WIN32)

extern tS32 CommTransmitData (tVoid *deviceHandle, tUChar *WriteBuffer, tU16 BytesToSend);

extern tS32 CommReceiveData (tVoid *deviceHandle, tUChar *ReadBuffer, tS32 BytesToRead, tS32 RxTimeoutMs);

#endif

// End of file
