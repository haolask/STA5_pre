///
//! \file          presets.h
//! \brief         Class used to manage radio presets list
//! \author        Alberto Saviotti
//!
//! Project        STRaDe
//! Sw component   HMI
//!
//! Copyright (c)  STMicroelectronics
//!
//! History
//! Date           | Modification               | Author
//! 20180130       | Initial version            | Alberto Saviotti
///

#ifndef PRESETS_H
#define PRESETS_H

#include "defines.h"
#include "station_list.h"

//! \brief        <i><b> Class for radio presets management </b></i>
//!
//! This class derives from RadioService one and add a xspecific constructor to store a preset.
//!
//! \remark This class uses virtual inheritance in order to avoid duplication of base class members due to the
//!         diamond inheritance implemented.
class RadioServicePreset : public virtual RadioService
{
    public:
        RadioServicePreset(std::string name, unsigned int id, unsigned int freq, unsigned int band, short preset) :
            RadioService(name, id, freq, band, preset)
        { }

        ~RadioServicePreset() { }
};

//! \brief        <i><b> Class for radio presets list management </b></i>
//!
//! This class implements the radio presets list and it inherits from the StationList template class specializing
//! it for RadioServicePreset storage.
//!
//! \remark This class shall not be directly used by the application, use \see Presets instead.
class PresetList : public StationList <RadioServicePreset>
{
    public:
        PresetList() : StationList()
        { }

        ~PresetList() { }

    protected:
        virtual std::istream&Load(std::istream&in)
        {
            char tmpServiceName[16];
            unsigned int tmpServiceId, tmpServiceFreq, tmpServiceBand;
            short tmpPreset;

            while (!in.eof())
            {
                in.read((char *)tmpServiceName, 16);
                in.read((char *)&tmpServiceId, sizeof (unsigned int));
                in.read((char *)&tmpServiceFreq, sizeof (unsigned int));
                in.read((char *)&tmpServiceBand, sizeof (unsigned int));
                in.read((char *)&tmpPreset, sizeof (short));

                RadioServicePreset radioService(std::string(tmpServiceName), tmpServiceId,
                                                tmpServiceFreq, tmpServiceBand, tmpPreset);

                InsertService(dynamic_cast <RadioServicePreset&> (radioService));
            }

            return in;
        }
};

//! \brief        <i><b> Class for presets management </b></i>
//!
//! This class is ment for preset management and it shall be used by the application in order to load and save preset
//! data at power on and power down.
//!
//! \remark This class implements the Singleton design patter in order to allow a single instance used by all modules
//!         that needs to accedd the presets. Singleton implementation assures that only one object is created
//!         and no more.
class Presets : public PresetList
{
    public:
        // To create the object instance function is used (Singleton pattern)
        static Presets * Instance();

        RadioServicePreset GetPreset(short presetToGet)
        {
            std::list <RadioServicePreset>::iterator itPresets;
            RadioServicePreset emptyPreset("Empty", INVALID_SERVICE_ID, INVALID_FREQUENCY, INVALID_BAND, 0);

            for (itPresets = radioServiceList.begin(); itPresets != radioServiceList.end(); itPresets++)
            {
                if (presetToGet == itPresets->GetPresetNumber())
                {
                    RadioServicePreset preset(itPresets->GetServiceName(),
                                              itPresets->GetId(),
                                              itPresets->GetFrequency(),
                                              itPresets->GetBand(),
                                              itPresets->GetPresetNumber());

                    return preset;
                }
            }

            return emptyPreset;
        }

        bool SetPreset(short presetToUse, std::string name, unsigned int id, unsigned int freq, unsigned int band)
        {
            std::list <RadioServicePreset>::iterator itPresets;
            RadioServicePreset newPreset(name, id, freq, band, presetToUse);
            bool addedPreset;

            for (itPresets = radioServiceList.begin(); itPresets != radioServiceList.end(); itPresets++)
            {
                if (presetToUse == itPresets->GetPresetNumber())
                {
                    // We already have this preset number, replace it
                    RemoveService(itPresets->GetId());

                    break;
                }
            }

            // We do not have the preset number already stored: store a new one
            addedPreset = InsertService(newPreset);

            return addedPreset;
        }

    private:
        // Let the constructor being private, Instance has to be called instead
        Presets() : PresetList()
        { }

        ~Presets()
        { }

        static Presets * presets; // Declared here for Singleton

    public:
        Presets(Presets const&) = delete; // Make copy constructor private, we do not want copies

        void operator=(Presets const&) = delete; // Make assignment operator private, we do not want copies
};

#endif // PRESETS_H

// End of file
