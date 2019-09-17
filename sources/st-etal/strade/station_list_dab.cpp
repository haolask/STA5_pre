
#include <iostream>

#include "station_list_dab.h"
#include "radio_storage_types.h"

void StationListDab::SetEnsembleName (unsigned int id, std::string& ensembleName)
{
    std::list<RadioServiceDab>::iterator it;

    for (it = radioServiceList.begin(); it != radioServiceList.end(); it++)
    {
        if (it->GetId () == id)
        {
            it->SetEnsembleName (ensembleName);

            break;
        }
    }
}

void StationListDab::Test ()
{
    // Insert a new entry in the service list
    InsertDabService ("Radio Rai 1", 0x5201, 225648, BAND_DAB3, 0xE05202, "Rai Mux", -1);

    // Insert a new entry in the service list
    InsertDabService ("Radio Rai 2", 0x5202, 225648, BAND_DAB3, 0xE05202, "Rai Mux", -1);

    // Insert a new entry in the service list
    InsertDabService ("Capital", 0xA001, 227000, BAND_DAB3, 0xE0A000, "Club DAB", -1);

    // Remove a service
    RemoveService (0x5202);

    // Check number of services
    cout << "Number of services: " <<  GetNumberOfServices () << endl;

    // Print available services
    std::list<RadioServiceDab>::iterator it;

    for (it = radioServiceList.begin(); it != radioServiceList.end(); it++)
    {
        cout << "Service : " <<  it->GetServiceName () << " - Ensemble: " << it->GetEnsembleName () << endl;
    }

    // Clear the list
    ClearServiceList ();

    // Check number of services
    cout << "Number of services: " <<  GetNumberOfServices () << endl;
}
