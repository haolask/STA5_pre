
#ifndef STATION_LIST_GLOBAL_H_
#define STATION_LIST_GLOBAL_H_

#include "target_config.h"

#if (defined CONFIG_USE_STANDALONE)
    #include "stm_types.h"
#endif // #if (defined CONFIG_USE_STANDALONE)

#if (defined CONFIG_USE_ETAL)
    #include "etal_types.h"
#endif // #if (defined CONFIG_USE_ETAL)

#include "common.h"
#include "radio_storage_types.h"
#include "station_list_dab.h"
#include "station_list_fm.h"

enum ServiceType
{
    SERVICE_TYPE_INVALID = -1,
    SERVICE_TYPE_UNKNOWN = 1,
    SERVICE_TYPE_FM = 2,
    SERVICE_TYPE_AM = 3,
    SERVICE_TYPE_DAB = 4
};

struct AlternateFrequencyInfo
{
    ServiceType type;

    unsigned int frequency;
};

////////////////////////////////////////////////////////////////////////////////
/// C++ <StationListGlobal> class                                            ///
////////////////////////////////////////////////////////////////////////////////
class StationListGlobal
{
    public:
        // To create the object instance function is used (Singleton pattern)
        static StationListGlobal* Instance ();

        int GetClassId ();

        void Test ();
        int LoadStationsList (int);
        RadioStatusGlobal GetTunedService(RadioStatusGlobal);

        void SetDebugInitList ();

        bool CheckIfServiceIsPresentInFm (unsigned int serviceId);

        bool CheckIfServiceIsPresentInDab (unsigned int serviceId, unsigned int ensembleId);


        StationListDab* stationListDab; // This list will store the DAB services

        StationListFm* stationListFm; // This list will store the AM/FM services


    private:
        // Let the constructor being private, Instance has to be called instead
        StationListGlobal ()
        {
            stationListDab = new StationListDab;

            stationListFm = new StationListFm;
        }

        ~StationListGlobal ()
        {
            delete stationListDab;

            delete stationListFm;
        }

        static StationListGlobal* stationListGlobal; // Declared here for Singleton

    public:
        StationListGlobal (StationListGlobal const&) = delete; // Make copy constructor private, we do not want copies

        void operator = (StationListGlobal const&) = delete; // Make assignment operator private, we do not want copies
};

#endif // STATION_LIST_GLOBAL_H_
