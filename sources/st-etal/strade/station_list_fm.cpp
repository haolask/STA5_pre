
#include <iostream>

#include "station_list_fm.h"

#if 0
std::istream& operator>>(std::istream& in, StationListFm& cm)
{
    return in;
}

std::ostream& operator<<(std::ostream &out, const StationListFm& cm)
{
    cm.Print (out);

    return out;
}
#endif

void StationListFm::Test ()
{
    std::string tmp;

    // Insert a new entry in the service list
    InsertFmService ("Radio Rai 1", 0x5201, 88000, -1);

    // Insert a new entry in the service list
    InsertFmService ("102500", 0x5A00, 102500, -1);

    // Insert a new entry in the service list
    InsertFmService ("Capital", 0x5000, 81500, -1);

    // Remove a service
    RemoveService (0x5201);

    // Check number of services
    cout << "Number of services: " <<  GetNumberOfServices () << endl;

    // Print available services
    std::list<RadioServiceFm>::iterator it;

    for (it = radioServiceList.begin(); it != radioServiceList.end(); it++)
    {
        cout << "Service : " <<  it->GetServiceName () << endl;
    }

    // Clear the list
    ClearServiceList ();

    // Check number of services
    cout << "Number of services: " <<  GetNumberOfServices () << endl;
}
