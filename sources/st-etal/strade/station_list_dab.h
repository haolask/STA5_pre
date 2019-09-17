
#ifndef STATION_LIST_DAB_H_
#define STATION_LIST_DAB_H_

#include "target_config.h"

#if (defined CONFIG_USE_STANDALONE)
    #include "stm_types.h"
#endif // #if (defined CONFIG_USE_STANDALONE)

#if (defined CONFIG_USE_ETAL) || (defined CONFIG_USE_JASMIN)
    #include "etal_types.h"
#endif // #if (defined CONFIG_USE_ETAL)



#include "station_list.h"

struct ServiceComponentInformation
{
    bool partTimeService;
    bool newService;
    bool currentlyOffAir;
    bool removed;
    unsigned int transferSid;
    unsigned int transferEid;

    unsigned char second;
    unsigned char minute;
    unsigned char hour;
    unsigned char date;

    unsigned char changeFLags;
};

struct DeltaTimeInfo
{
    tU8 minute;
    tU8 hours;

    unsigned int reboot;
};

struct ServiceTuneInformation
{
    unsigned int failedTuneCount;

    DeltaTimeInfo deltaTimeInfo;
};

////////////////////////////////////////////////////////////////////////////////
/// C++ <RadioServiceDab> class                                              ///
////////////////////////////////////////////////////////////////////////////////
class RadioServiceDab : public virtual RadioService
{
    public:
        RadioServiceDab (std::string name, unsigned int id, unsigned int freq, unsigned int band,
                         unsigned int ensId, std::string ensName, short preset = -1) :
            RadioService (name, id, freq, band, preset)
        {
            ensembleUniqueId = ensId;
            ensembleName = ensName;

            serviceComponentInformation = {}; // Clear the structure

            serviceTuneInformation = {}; // Clear the structure
        }

        ~RadioServiceDab () { }

        unsigned int GetEnsembleId () const
        {
            return ensembleUniqueId;
        }

        std::string GetEnsembleName () const
        {
            return ensembleName;
        }

        void SetEnsembleName (std::string& ensName)
        {
            ensembleName = ensName;
        }

    private:
        // DAB additional information
        unsigned int ensembleUniqueId;

        std::string ensembleName;

        // Information from FIG 0/20 (mandatory to properly build a service list)
        ServiceComponentInformation serviceComponentInformation;

        // Information regarding the aging of the service
        ServiceTuneInformation serviceTuneInformation;
};

////////////////////////////////////////////////////////////////////////////////
/// C++ <StationListDab> class                                               ///
////////////////////////////////////////////////////////////////////////////////
class StationListDab : public StationList<RadioServiceDab>
{
    public:
        StationListDab () : StationList () { }

        ~StationListDab () { }

        //friend std::istream& operator>>(std::istream& in, StationListDab& cm);
        //friend std::ostream& operator<<(std::ostream& out, const StationListDab& cm);

        void Test ();

        bool RemoveServiceByEnsemble (unsigned int ensembleId);

        bool InsertDabService (std::string name, unsigned int id, unsigned int freq, unsigned int band,
                               unsigned int ensId, std::string ensName, short preset = -1)
        {
            RadioServiceDab* radioService = new RadioServiceDab (name, id, freq, band, ensId, ensName, preset);

            return InsertService (*radioService);
        }

        void SetEnsembleName (unsigned int id, std::string& ensembleName);

    protected:
        virtual std::ostream& Print (std::ostream &os) const
        {
            //return StationList<RadioServiceDab>::Print (os);

            std::list<RadioServiceDab>::const_iterator it;
            unsigned int tmp;
            short tmpPreset;

            // Save data: it shall be done same order as 'Load'
            for (it = radioServiceList.begin (); it != radioServiceList.end (); ++it)
            {
                // Service name
                os.write (it->GetServiceName ().c_str(), NAME_STRINGS_CHAR_LEN);

                // Service ID
                tmp = it->GetId ();
                os.write ((const char*)&tmp, sizeof (unsigned int));

                // Frequency
                tmp = it->GetFrequency ();
                os.write ((const char*)&tmp, sizeof (unsigned int));

                // Band
                tmp = it->GetBand();
                os.write ((const char*)&tmp, sizeof (unsigned int));

                // Preset number
                tmpPreset = it->GetPresetNumber ();
                os.write ((const char*)&tmpPreset, sizeof (short));

                // Ensemble name
                os.write (it->GetEnsembleName ().c_str(), NAME_STRINGS_CHAR_LEN);

                // Ensemble ID
                tmp = it->GetEnsembleId ();
                os.write ((const char*)&tmp, sizeof (unsigned int));
            }

            return os;
        }

        // Load data: it shall be done same order as save ('Print')
        virtual std::istream& Load (std::istream& in)
        {
            //return StationList<RadioServiceDab>::Load (in);

            char tmpServiceName[NAME_STRINGS_CHAR_LEN + 1]; // 16 chars + termination
            char tmpEnsembleName[NAME_STRINGS_CHAR_LEN + 1]; // 16 chars + termination
            unsigned int tmpServiceId, tmpServiceFreq, tmpServiceBand, tmpEnsembleId;
            short tmpPreset;

            while (!in.eof ())
            {
                // Service name
                in.read ((char*)tmpServiceName, NAME_STRINGS_CHAR_LEN);
                tmpServiceName[NAME_STRINGS_CHAR_LEN] = '\0';

                // Service ID
                in.read ((char*)&tmpServiceId, sizeof(unsigned int));

                // Frequency
                in.read ((char*)&tmpServiceFreq, sizeof(unsigned int));

                // Band
                in.read ((char*)&tmpServiceBand, sizeof(unsigned int));

                // Preset number
                in.read ((char*)&tmpPreset, sizeof(short));

                // Ensemble name
                in.read ((char*)tmpEnsembleName, NAME_STRINGS_CHAR_LEN);
                tmpEnsembleName[NAME_STRINGS_CHAR_LEN] = '\0';

                // Ensemble ID
                in.read ((char*)&tmpEnsembleId, sizeof(unsigned int));

                // Create the radio service entry
                RadioServiceDab radioService (std::string (tmpServiceName), tmpServiceId, tmpServiceFreq,
                        tmpServiceBand, tmpEnsembleId, std::string (tmpEnsembleName), tmpPreset);

                // Insert the new service in the list
                InsertService (dynamic_cast<RadioServiceDab&>(radioService));
            }

            return in;
        }
};

#endif // STATION_LIST_DAB_HPP_
