//!
//!  \file 		 utility.h
//!  \brief 	 <i><b> Utility functions interface</b></i>
//!  \details Utility functions interface.
//!  \author 	Alberto Saviotti
//!

// This must be defined if we provide a LogString functionality
#define UTILITY_LOG_FNCT_PROVIDED

// LOG defines
#define LOG_MASK_UTILITY                         0x01
#define LOG_MASK_INTERLAYER                      0x02
#define LOG_MASK_DEVICE_STATUS                   0x04
#define LOG_MASK_DEVICE_LOW_LEVEL                0x08
#define LOG_MASK_TCP_IP                          0x10
#define LOG_MASK_UART                            0x20

// MAX file name
#define LOG_MAX_FILE_NAME						 256

extern tU32 UTILITY_GetProcessTimeMs (tVoid);
extern tS64 UTILITY_GetTime (tVoid);
extern tBool UTILITY_CheckTimeout (tS64 startTime, tS64 thrVal);
extern tVoid StringToUpper (tChar *StringToConvert);
extern tVoid StringToUpperLen (tChar *StringToConvert, tU16 BytesNum);
extern tU32 GetBELongFromBuffer (tU8 *Buffer);
extern tU16 GetBEWordFromBuffer (tU8 *Buffer);
extern tVoid PutBELongOnBuffer (tU8 *Buffer, tU32 LongToPut);
extern tVoid PutBEWordOnBuffer (tU8 *Buffer, tU16 WordToPut);
extern tVoid OpenLogSession (tVoid);
extern tVoid CloseLogSession (tVoid);
extern tVoid LogString (tChar *StringToLog, tU32 LogId);
extern tVoid LogStringAndFlush (tChar *StringToLog, tU32 LogId);
extern tVoid LogStringMaxSize (tChar *StringToLog, tU32 LogId, tU32 maxSize);
extern tVoid LogStringDataChannelFile (tU8 *DataToSend, tS32 BytesNum);
extern tVoid FixLogData (tVoid);
extern tChar *LogBufferPointer (tVoid);
extern tVoid SetLogFile (tChar *fileNamePtr);
extern tVoid SetLogMask (tU32 newMask);
extern tVoid SetLogFlag (tBool newLogFlag);
extern tU32 ConvertBaseBin2UInt (tU8 *baseBin, size_t baseLen, tBool LittleEndian);
extern tU32 /*@alt void@*/ ConvertBaseUInt2Bin (tU32 baseUInt, tU8 *baseBin, tS32 baseLen, tBool LittleEndian);
extern tVoid OpenSaveData (tVoid);
extern tVoid SaveData (tChar *strToLog, int length);
extern tVoid CloseSaveData (tVoid);
// End of file
