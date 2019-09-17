#include "hmi_if_extern.h"

#define TUNER_0		0
#define TUNER_1		1

void radioNotificationHdlr(HMIIF_IDataObject* pData);
void displayActivityStatus(DWORD data);
void displayBandInfo(DWORD data);
void displayDataServiceInfo(DWORD data);

DWORD activeBand;
DWORD data = 0;
IObjectList *StnList;
HMIIF_IDataObject *StnListObj;
IObjectList *DataService;
HMIIF_IDataObject *DataServiceObj;
int StnListCount;
int i;
INT payloadSize;