#include <iostream>

#include <QtGlobal> // For Q_UNUSED

#include "station_list_global.h"

StationListGlobal * StationListGlobal::stationListGlobal = NULL;

StationListGlobal * StationListGlobal::Instance()
{
    if (!stationListGlobal)
    {
        stationListGlobal = new StationListGlobal;
    }

    return stationListGlobal;
}

int StationListGlobal::GetClassId()
{
    return (long)StationListGlobal::stationListGlobal;
}

bool StationListGlobal::CheckIfServiceIsPresentInFm(unsigned int serviceId)
{
    std::list<RadioServiceFm>::iterator itFm;

    for (itFm = stationListFm->begin(); itFm != stationListFm->end(); itFm++)
    {
        if (serviceId == itFm->GetId())
        {
            // We found the service
            return true;
        }
    }

    // We didn't find the service
    return false;
}

bool StationListGlobal::CheckIfServiceIsPresentInDab(unsigned int serviceId, unsigned int ensembleId)
{
    std::list<RadioServiceDab>::iterator itDab;

    for (itDab = stationListDab->begin(); itDab != stationListDab->end(); itDab++)
    {
        if (serviceId == itDab->GetId() && ensembleId == itDab->GetEnsembleId())
        {
            // We found the service
            return true;
        }
    }

    // We didn't find the service
    return false;
}

void StationListGlobal::Test()
{
    std::list<RadioServiceFm>::iterator itFm;
    std::list<RadioServiceDab>::iterator itDab;

    ///
    // Step 1: Create the list and check services inside the list, 2 FM and 3 DAB services shall be reported
    ///
    // Insert a new entry in the service list
    stationListFm->InsertFmService("*Radio 1", 0x5201, 88000, BAND_FM, -1);

    // Insert a new entry in the service list
    stationListFm->InsertFmService("RTL102.5", 0x5218, 102500, BAND_FM, -1);

    // Insert a new entry in the service list
    stationListFm->InsertFmService("R ITALIA", 0x5220, 106700, BAND_FM, -1);

    // Insert a new entry in the service list
    stationListDab->InsertDabService("Radio Rai 3", 0x5203, 225000, BAND_DAB3, 0xE05203, "Rai Mux", -1);

    // Insert a new entry in the service list
    stationListDab->InsertDabService("Radio Rai 4", 0x5204, 225000, BAND_DAB3, 0xE05203, "Rai Mux", -1);

    // Remove a service
    stationListFm->RemoveService(0x5218);

    // Add a duplicated entry on different system (DAB frequency added to existing FM station)
    stationListDab->InsertDabService("Radio Rai 1", 0x5201, 225000, BAND_DAB3, 0xE05201, "Rai Mux", -1);

    // Set service to tuned
    stationListDab->SetTunedStatus(0x5201, true);

    // Check number of FM services
    cout << "STEP 1 - Number of FM services: " << stationListFm->GetNumberOfServices() << endl;

    // Print available services for FM
    for (itFm = stationListFm->begin(); itFm != stationListFm->end(); itFm++)
    {
        cout << "STEP 1 - Service (AM/FM) : " <<  itFm->GetServiceName() << endl;

        if (itFm->IsTuned())
        {
            cout << "         (TUNED)" << endl;
        }
    }

    // Check number of DAB services
    cout << "STEP 1 - Number of DAB services: " << stationListDab->GetNumberOfServices() << endl;

    // Print available services for DAB
    for (itDab = stationListDab->begin(); itDab != stationListDab->end(); itDab++)
    {
        cout << "STEP 1 - Service (DAB) : " <<  itDab->GetServiceName() << " - Ensemble : " << itDab->GetEnsembleName() << endl;

        if (itDab->IsTuned())
        {
            cout << "         (TUNED)" << endl;
        }
    }

    ///
    // Step 2: Save the data and clear memory content, 0 services shall be reported
    ///
    // Save data from the DAB list
    ofstream outFileDab("dab.bin", ios::out | ios::binary);

    outFileDab << *stationListDab;

    outFileDab.close();

    // Save data from the FM list
    ofstream outFileFm("fm.bin", ios::out | ios::binary);

    outFileFm << *stationListFm;

    outFileFm.close();

    // Clear the list
    stationListFm->ClearServiceList();
    stationListDab->ClearServiceList();

    // Check number of services
    cout << "STEP 2 - Number of FM services after clear: " << stationListFm->GetNumberOfServices() << endl;
    cout << "STEP 2 - Number of DAB services after clear: " << stationListDab->GetNumberOfServices() << endl;

    ///
    // Step 3: Load data, 2 FM and 3 DAB services shall be reported
    ///
    // Load data
    ifstream inFileFm("fm.bin", ios::in | ios::binary);

    inFileFm >> *stationListFm;

    // Check number of FM services
    cout << "STEP 3 - Number of FM services after load: " << stationListFm->GetNumberOfServices() << endl;

    for (itFm = stationListFm->begin(); itFm != stationListFm->end(); itFm++)
    {
        cout << "STEP 3 - Service (AM/FM): " <<  itFm->GetServiceName() << endl;
    }

    ifstream inFileDab("dab.bin", ios::in | ios::binary);

    inFileDab >> *stationListDab;

    inFileDab.close();

    // Check number of DAB services
    cout << "STEP 3 - Number of DAB services after load: " << stationListDab->GetNumberOfServices() << endl;

    for (itDab = stationListDab->begin(); itDab != stationListDab->end(); itDab++)
    {
        cout << "STEP 3 - Service (DAB) : " <<  itDab->GetServiceName() << " - Ensemble : " << itDab->GetEnsembleName() << endl;
    }

    ///
    // Step 4: Use insert functions
    ///
    if (false == CheckIfServiceIsPresentInDab(0x5203, 0xE05000))
    {
        // Insert new service
        stationListDab->InsertDabService("Wrong insertion", 0x5203, 225648, BAND_DAB3, 0xE05000, "Rai Mux", -1);
    }

    if (false == CheckIfServiceIsPresentInDab(0x600D, 0xEE600D))
    {
        // Insert new service
        stationListDab->InsertDabService("OK insertion", 0x600D, 197648, BAND_DAB3, 0xEE600D, "A^_^L", -1);
    }

    ///
    // Step 5: Clear RAM data, 0 services shall be reported
    ///
    // Clear the list
    stationListFm->ClearServiceList();
    stationListDab->ClearServiceList();

    // Check number of services
    cout << "STEP 4 - Number of FM services after clear: " << stationListFm->GetNumberOfServices() << endl;
    cout << "STEP 4 - Number of DAB services after clear: " << stationListDab->GetNumberOfServices() << endl;
}

int StationListGlobal::LoadStationsList(int radioBand)
{
    std::list<RadioServiceFm>::iterator itFm;
    std::list<RadioServiceDab>::iterator itDab;
    int numberOfServices = 0;

    // Load stored data, Service Lists DAB and FM
    ifstream inFileFm("fm.bin", ios::in | ios::binary);

    if (!inFileFm)
    {
        inFileFm >> *stationListFm;

        // Check number of FM services
        cout << "STEP 3 - Number of FM services after load: " << stationListFm->GetNumberOfServices() << endl;

        for (itFm = stationListFm->begin(); itFm != stationListFm->end(); itFm++)
        {
            cout << "STEP 3 - Service (AM/FM): " <<  itFm->GetServiceName() << endl;
        }

        if (BAND_FM == radioBand)
        {
            numberOfServices = stationListFm->GetNumberOfServices();
        }
    }

    ifstream inFileDab("dab.bin", ios::in | ios::binary);

    if (!inFileDab)
    {
        inFileDab >> *stationListDab;

        inFileDab.close();

        // Check number of DAB services
        cout << "STEP 3 - Number of DAB services after load: " << stationListDab->GetNumberOfServices() << endl;

        for (itDab = stationListDab->begin(); itDab != stationListDab->end(); itDab++)
        {
            cout << "STEP 3 - Service (DAB) : " <<  itDab->GetServiceName() << " - Ensemble : " << itDab->GetEnsembleName() << endl;
        }

        if (BAND_DAB3 == radioBand)
        {
            numberOfServices = stationListDab->GetNumberOfServices();
        }
    }

    return numberOfServices;
}

RadioStatusGlobal StationListGlobal::GetTunedService(RadioStatusGlobal radiocontent)
{
    if (BAND_FM == radiocontent.radioStatus.persistentData.band)
    {
        std::list<RadioServiceFm>::iterator itFm;

        for (itFm = stationListFm->begin(); itFm != stationListFm->end(); itFm++)
        {
            if (itFm->IsTuned())
            {
                std::string tmpStr = itFm->GetServiceName();
                radiocontent.radioStatus.persistentData.serviceId = itFm->GetId();
                radiocontent.radioStatus.persistentData.frequency = itFm->GetFrequency();

                memcpy(&radiocontent.radioStatus.persistentData.serviceName[0], tmpStr.c_str(), STATION_NAME_LEN);
                radiocontent.radioStatus.persistentData.ensembleId = 0x00;
                tmpStr = "               ";
                memcpy(&radiocontent.radioStatus.persistentData.ensembleName[0], tmpStr.c_str(), ENSEMBLE_NAME_LEN);

                return radiocontent;
            }
        }
    }
    else if (BAND_DAB3 == radiocontent.radioStatus.persistentData.band)
    {
        std::list<RadioServiceDab>::iterator itDab;

        for (itDab = stationListDab->begin(); itDab != stationListDab->end(); itDab++)
        {
            if (itDab->IsTuned())
            {
                std::string tmpStr = itDab->GetServiceName();
                radiocontent.radioStatus.persistentData.serviceId = itDab->GetId();
                radiocontent.radioStatus.persistentData.frequency = itDab->GetFrequency();
                memcpy(&radiocontent.radioStatus.persistentData.serviceName[0], tmpStr.c_str(), STATION_NAME_LEN);
                radiocontent.radioStatus.persistentData.ensembleId = itDab->GetEnsembleId();
                tmpStr = itDab->GetEnsembleName();
                memcpy(&radiocontent.radioStatus.persistentData.ensembleName[0], tmpStr.c_str(), ENSEMBLE_NAME_LEN);

                return radiocontent;
            }
        }
    }

    return radiocontent;
}

// TODO: TEST code called at beginning just to start with some stored stations FM and DAB
void StationListGlobal::SetDebugInitList()
{
    std::list<RadioServiceFm>::iterator itFm;
    std::list<RadioServiceDab>::iterator itDab;

    // Step 1: For initial Test Create a list of 10 FM and 5 DAB services

    // #1 Insert a new entry in the service list (67)
    stationListFm->InsertFmService(" *Radio1", 0x5201, 94200, -1);

    // #2 Insert a new entry in the service list (96)
    stationListFm->InsertFmService("GAMMARAD", 0x521D, 97100, -1);

    // #3 Insert a new entry in the service list (150)
    stationListFm->InsertFmService("RTL102.5", 0x5218, 102500, -1);

    // #4 Insert a new entry in the service list (192)
    stationListFm->InsertFmService("R ITALIA", 0x5220, 106700, -1);

    // #5 Insert a new entry in the service list (162)
    stationListFm->InsertFmService("REPORTER", 0x5339, 103700, -1);

    // #6 Insert a new entry in the service list (56)
    stationListFm->InsertFmService(" M DUE O", 0x5233, 93100, -1);

    // #7 Insert a new entry in the service list (137)
    stationListFm->InsertFmService(" R 101 ", 0x5215, 101200, -1);

    // #8 Insert a new entry in the service list (124)
    stationListFm->InsertFmService("*Radio3", 0x5203, 99900, -1);

    // #9 Insert a new entry in the service list (90)
    stationListFm->InsertFmService("*DISCO*", 0x5348, 96500, -1);

    // #10 Insert a new entry in the service list (99)
    stationListFm->InsertFmService(" *Radio2", 0x5202, 97400, -1);

    // Remove a service
    //    stationListFm->RemoveService (0x5A00);

    // Add a duplicated entry on different system (DAB frequency added to existing DAB station)
    stationListDab->InsertDabService("Rai Radio 2", 0x5202, 225648, BAND_DAB3, 0xE05001, "DAB+ RAI", -1);

    // Add a duplicated entry on different system (DAB frequency added to existing DAB station)
    stationListDab->InsertDabService("Rai Radio 3", 0x5203, 225648, BAND_DAB3, 0xE05001, "DAB+ RAI", -1);

    // Add a duplicated entry on different system (DAB frequency added to existing DAB station)
    stationListDab->InsertDabService("R101", 0x5215, 227360, BAND_DAB3, 0xE05009, "*DAB ITALIA", -1);

    // Add a duplicated entry on different system (DAB frequency added to existing DAB station)
    stationListDab->InsertDabService(" M DUE O", 0x5233, 227360, BAND_DAB3, 0xE05009, "*DAB ITALIA", -1);

    // Insert a new entry in the service list
    stationListDab->InsertDabService("Rai Radio 1", 0x5201, 225648, BAND_DAB3, 0xE05001, "DAB+ RAI", -1);

    // Set service to tuned
    stationListDab->SetTunedStatus(0x5201, true);

    // Check number of FM services
    cout << "STEP 1 - Number of FM services: " << stationListFm->GetNumberOfServices() << endl;

    // Print available services for FM
    for (itFm = stationListFm->begin(); itFm != stationListFm->end(); itFm++)
    {
        cout << "STEP 1 - Service (AM/FM) : " <<  itFm->GetServiceName() << endl;

        if (itFm->IsTuned())
        {
            cout << "         (TUNED)" << endl;
        }
    }

    // Check number of DAB services
    cout << "STEP 1 - Number of DAB services: " << stationListDab->GetNumberOfServices() << endl;

    // Print available services for DAB
    for (itDab = stationListDab->begin(); itDab != stationListDab->end(); itDab++)
    {
        cout << "STEP 1 - Service (DAB) : " <<  itDab->GetServiceName() << " - Ensemble : " << itDab->GetEnsembleName() << endl;

        if (itDab->IsTuned())
        {
            cout << "         (TUNED)" << endl;
        }
    }

    // Save the loaded data and clear memory content, 0 services shall be reported

    // Save data from the DAB list
    ofstream outFileDab("dab.bin", ios::out | ios::binary);

    outFileDab << *stationListDab;

    outFileDab.close();

    // Save data from the FM list
    ofstream outFileFm("fm.bin", ios::out | ios::binary);

    outFileFm << *stationListFm;

    outFileFm.close();

    // Clear the list
    stationListFm->ClearServiceList();
    stationListDab->ClearServiceList();

    // Check number of services
    cout << "STEP 2 - Number of FM services after clear: " << stationListFm->GetNumberOfServices() << endl;
    cout << "STEP 2 - Number of DAB services after clear: " << stationListDab->GetNumberOfServices() << endl;
}
