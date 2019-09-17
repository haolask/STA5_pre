
#ifndef STATION_LIST_FM_H_
#define STATION_LIST_FM_H_

#include "target_config.h"

#if (defined CONFIG_USE_STANDALONE)
    #include "stm_types.h"
#endif // #if (defined CONFIG_USE_STANDALONE)

#if (defined CONFIG_USE_ETAL)
    #include "etal_types.h"
#endif // #if (defined CONFIG_USE_ETAL)

#include "station_list.h"

////////////////////////////////////////////////////////////////////////////////
/// C++ <RadioServiceFm> class                                               ///
////////////////////////////////////////////////////////////////////////////////
class RadioServiceFm : public virtual RadioService
{
    public:
        RadioServiceFm (std::string name, unsigned int id, unsigned int freq, unsigned int band, short preset = -1) :
            RadioService (name, id, freq, band, preset)
        {

        }

        ~RadioServiceFm () { }
};

////////////////////////////////////////////////////////////////////////////////
/// C++ <StationListFm> class                                                ///
////////////////////////////////////////////////////////////////////////////////
class StationListFm : public virtual StationList<RadioServiceFm>
{
    public:
        StationListFm () : StationList () { }

        ~StationListFm () { }

        //friend std::istream& operator>>(std::istream& in, StationListFm& cm);
        //friend std::ostream& operator<<(std::ostream& out, const StationListFm& cm);

        void Test ();

        bool InsertFmService (std::string name, unsigned int id, unsigned int freq, unsigned int band, short preset = -1)
        {
            RadioServiceFm* radioService = new RadioServiceFm (name, id, freq, band, preset);

            return InsertService (*radioService);
        }

    protected:
        virtual std::ostream& Print (std::ostream &os) const
        {
            // Save data: it shall be done same order as 'Load'
            return StationList<RadioServiceFm>::Print (os);
        }

        virtual std::istream& Load (std::istream& in)
        {
            //return StationList<RadioServiceDab>::Load (in);

            char tmpServiceName[NAME_STRINGS_CHAR_LEN + 1]; // 16 chars + termination
            unsigned int tmpServiceId, tmpServiceFreq, tmpServiceBand;
            short tmpPreset;

            // Load data: it shall be done same order as save ('Print')
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

                // Preset
                in.read ((char*)&tmpPreset, sizeof(short));

                // Create the radio service entry
                RadioServiceFm radioService (std::string (tmpServiceName), tmpServiceId,
                        tmpServiceFreq, tmpServiceBand, tmpPreset);

                // Insert the new service in the list
                InsertService (dynamic_cast<RadioServiceFm&>(radioService));
            }

            return in;
        }
};

#endif // STATION_LIST_FM_H_
