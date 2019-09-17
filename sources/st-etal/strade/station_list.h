///
//! \file          station_list.h
//! \brief         Class used to manage stations list
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

#ifndef STATION_LIST_H_
#define STATION_LIST_H_

#include <string> // std::string
#include <list>   // std::list
#include <fstream>
#include <iostream>
#include "target_config.h"

#if (defined CONFIG_USE_STANDALONE)
    #include "stm_types.h"
#endif // #if (defined CONFIG_USE_STANDALONE)

#if (defined CONFIG_USE_ETAL)
    #include "etal_types.h"
#endif // #if (defined CONFIG_USE_ETAL)

using namespace std;

namespace
{
    //! \brief <i><b> Maximum length allowed for service names (aligned with RDS and DAB requirements) </b></i>
    const int NAME_STRINGS_CHAR_LEN = 16;
}

//! \brief        <i><b> Class to manage generic station </b></i>
//!
//! This class manages a generic radio service. Specific, i.e. analog and digital, service management is implemented
//! with derived classes.
class RadioService
{
    public:
        //! \brief        <i><b> Class constructor </b></i>
        //!
        //! This functions take service data as input (parameters stored for each service)
        //! and creates a new service entry.
        //!
        //! \param[in]    name   Service name
        //! \param[in]    id     Service identifier. This is the PI informatino contained in RDS or the service ID
        //!                      transmitted for DAB, and other digital standards, services
        //! \param[in]    freq   Frequency of the service
        //! \param[in]    band   Band of the service (i.e. FM, AM, DAB or other)
        //! \param[in]    preset Set to a positive number if the servcie has been stored s a preset, -1 otherwise
        //! \return       None
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        RadioService(std::string name, unsigned int id, unsigned int freq, unsigned int band, short preset)
        {
            serviceName = name;
            identifier = id;
            frequency = freq;
            currentBand  = band;
            presetNumber = preset;
        }

        //! \brief        <i><b> Class destructor </b></i>
        //!
        //! Standard destructor for class RadioService.
        //!
        //! \return       None
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        virtual ~RadioService() { }

        //! \brief        <i><b> Retrieve service name </b></i>
        //!
        //! This function returns the service name.
        //!
        //! \return       Service name
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        std::string GetServiceName() const
        {
            return serviceName;
        }

        //! \brief        <i><b> Get radio service identifier </b></i>
        //!
        //! Call this function to retrieve the service identifier; this identifier is unique and it can be used
        //! to uniquely identify the service.
        //!
        //! \return       Service identifier
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        unsigned int GetId() const
        {
            return identifier;
        }

        //! \brief        <i><b> Get radio service frequency </b></i>
        //!
        //! The service frequency in kHz is returned.
        //!
        //! \return       Service identifier
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        unsigned int GetFrequency() const
        {
            return frequency;
        }

        //! \brief        <i><b> Get radio service band </b></i>
        //!
        //! The service band is returned.
        //!
        //! \return       Service band
        //!
        //! \remark       The band is returned as unsigned int, it is on the application to save
        //!               bands with meaningful numbers.
        //! \callgraph
        //! \callergraph
        unsigned int GetBand() const
        {
            return currentBand;
        }

        //! \brief        <i><b> Get preset </b></i>
        //!
        //! If the service has been stored in a preset entry a positive number representing the preset is returned,
        //! otherwise -1 is returned.
        //!
        //! \return       Preset number or -1
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        short GetPresetNumber() const
        {
            return presetNumber;
        }

        //! \brief        <i><b> Set service name </b></i>
        //!
        //! Call this function in order to store the service name.
        //!
        //! \param[in]    sn Service name
        //! \return       None
        //!
        //! \remark       There's no need for the application to call directly this function if the service name
        //!               is available at creation date.
        //! \callgraph
        //! \callergraph
        void SetServiceName(std::string sn)
        {
            serviceName = sn;
        }

        //! \brief        <i><b> Set service identifier </b></i>
        //!
        //! Call this function in order to store the service identifier.
        //!
        //! \param[in]    id Service identifier
        //! \return       None
        //!
        //! \remark       There's no need for the application to call directly this function if the service identifier
        //!               is available at creation date.
        //! \callgraph
        //! \callergraph
        void SetId(unsigned int id)
        {
            identifier = id;
        }

        //! \brief        <i><b> Set service frequency </b></i>
        //!
        //! Call this function in order to store the service frequency in kHz.
        //!
        //! \param[in]    f Service frequency in kHz
        //! \return       None
        //!
        //! \remark       There's no need for the application to call directly this function if the service frequency
        //!               is available at creation date.
        //! \callgraph
        //! \callergraph
        void SetFrequency(unsigned int f)
        {
            frequency = f;
        }

        //! \brief        <i><b> Set service band </b></i>
        //!
        //! Call this function in order to store the service band.
        //!
        //! \param[in]    f Service frequency in kHz
        //! \return       None
        //!
        //! \remark       There's no need for the application to call directly this function if the service frequency
        //!               is available at creation date.
        //! \callgraph
        //! \callergraph
        void SetBand(unsigned int b)
        {
            currentBand = b;
        }

        //! \brief        <i><b> Set service preset number </b></i>
        //!
        //! Call this function in order to store the preset where the service has been saved.
        //!
        //! \param[in]    pn Preset number
        //! \return       None
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        void SetPresetNumber(short pn)
        {
            presetNumber = pn;
        }

        //! \brief        <i><b> Retrieve tuned information </b></i>
        //!
        //! Call this function in order to retrieve the tuned status for a specific service.
        //!
        //! \return       True if the service is currently tuned, false otherwise
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        bool IsTuned()
        {
            return tuned;
        }

        //! \brief        <i><b> Set tune information </b></i>
        //!
        //! Call this function in order to retrieve the tuned status for a specific service.
        //!
        //! \param[in]    tunedStatus Boolean setting the tuned status for a service
        //! \return       True if the service is currently tuned, false otherwise
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        void SetTunedStatus(bool tunedStatus)
        {
            tuned = tunedStatus;
        }

    protected:
        std::string serviceName;  //!< Service name
        unsigned int identifier;  //!< Service identifier
        unsigned int frequency;   //!< Service frequency in kHz
        unsigned int currentBand; //!< Service band
        short presetNumber;       //!< Preset number
        bool tuned;               //!< Tuned information, true if the service is currently tuned, false otherwise
};

//! \brief        <i><b> Template class to manage generic stations list </b></i>
//!
//! This class manages a generic radio services list.
//! Specific, i.e. analog and digital, service management is implemented with derived classes.
//!
//! \tparam T Service type parameter
template <class T> class StationList
{
    using radioServiceList_t = std::list <T>; //!< Name alias for std::list<T>

    public:
        using iterator = typename radioServiceList_t::iterator;             //!< Iterator over the service list
        using const_iterator = typename radioServiceList_t::const_iterator; //!< Constant iterator over the service list

        //! \brief        <i><b> Class standard constructor </b></i>
        //!
        //! The constructor initialize the class as empty: no services are stored in the list.
        //!
        //! \return       None
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        StationList() { availableNumOfService = 0; }

        //! \brief        <i><b> Retrieve service list first element </b></i>
        //!
        //! This function retruns an iterator pointing to the first element in the list.
        //!
        //! \return       Iterator pointing to the first element
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        iterator begin() { return radioServiceList.begin(); }

        //! \brief        <i><b> Retrieve service list last element </b></i>
        //!
        //! This function retruns an iterator pointing to the last element in the list.
        //!
        //! \return       Iterator pointing to the last element
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        iterator end() { return radioServiceList.end(); }

        //! \brief        <i><b> Retrieve service list first element </b></i>
        //!
        //! This function retruns a constant iterator pointing to the first element in the list. This iterator
        //! cannot be modified.
        //!
        //! \return       Constant iterator pointing to the first element
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        const_iterator begin() const { return radioServiceList.begin(); }

        //! \brief        <i><b> Retrieve service list last element </b></i>
        //!
        //! This function retruns a constant iterator pointing to the last element in the list. This iterator
        //! cannot be modified.
        //!
        //! \return       Constant iterator pointing to the last element
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        const_iterator end() const { return radioServiceList.end(); }

        //! \brief        <i><b> Retrieve service list first element </b></i>
        //!
        //! Returns a const_iterator pointing to the first element in the container.
        //! A const_iterator is an iterator that points to const content. This iterator can be increased and decreased
        //! but it cannot be used to modify the contents it points to, even if the list object is not itself const.
        //!
        //! \return       A const_iterator pointing to the first element
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        const_iterator cbegin() const { return radioServiceList.cbegin(); }

        //! \brief        <i><b> Retrieve service list last element </b></i>
        //!
        //! Returns a const_iterator pointing to the first element in the container.
        //! A const_iterator is an iterator that points to const content. This iterator can be increased and decreased
        //! but it cannot be used to modify the contents it points to, even if the list object is not itself const.
        //!
        //! \return       A const_iterator pointing to the last element
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        const_iterator cend() const { return radioServiceList.cend(); }

        //! \brief        <i><b> Standard destructor </b></i>
        //!
        //! Standard destructor function.
        //!
        //! \return       None
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        virtual ~StationList() { }

        //! \brief        <i><b> Get number of services stored in the list </b></i>
        //!
        //! This function can be called to know the number of services already in the list.
        //!
        //! \return       Number of services in the list
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        int GetNumberOfServices()
        {
            return availableNumOfService;
        }

        //! \brief        <i><b> Clear the service list </b></i>
        //!
        //! This function clear the list resetting it to an empty container.
        //!
        //! \return       None
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        void ClearServiceList()
        {
            radioServiceList.clear();

            availableNumOfService = 0;
        }

        //! \brief        <i><b> Check if a service exists </b></i>
        //!
        //! This function checks if a particular service, identified by the service identifier is present in the list.
        //!
        //! \param[in]    id Service identifier
        //! \return       None
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        bool CheckService(unsigned int id)
        {
            bool found = false;

            typename std::list <T>::iterator it;

            for (it = radioServiceList.begin(); it != radioServiceList.end(); )
            {
                if (id == it->GetId())
                {
                    // Signal that it is found
                    found = true;

                    // ID is unique we can exit the loop
                    break;
                }
                else
                {
                    it++;
                }
            }

            return found;
        }

        //! \brief        <i><b> Insert a service in the service list </b></i>
        //!
        //! Insert a service by service identifier in the service list.
        //!
        //! \tparam[in]   service Service
        //! \return       True if the service has been correctly inserted in the list, false otherwise
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        bool InsertService(T&service)
        {
            bool added = false;

            // Check if it exist, if not add it
            if (false == CheckService(service.GetId()))
            {
                availableNumOfService++;

                radioServiceList.push_back(service);

                added = true;
            }

            return added;
        }

        //! \brief        <i><b> Remove a service from the service list </b></i>
        //!
        //! Remove a service by service identifier from the service list.
        //!
        //! \param[in]    id Service identifier
        //! \return       True if the service has been correctly removed from the list, false otherwise
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        bool RemoveService(unsigned int id)
        {
            bool found = false;

            typename std::list <T>::iterator it;

            for (it = radioServiceList.begin(); it != radioServiceList.end(); )
            {
                if (id == it->GetId())
                {
                    // Erase element and increment
                    it = radioServiceList.erase(it);

                    // Reduce number of services
                    availableNumOfService--;

                    // Signal that it is found
                    found = true;

                    // ID is unique we can exit the loop
                    break;
                }
                else
                {
                    it++;
                }
            }

            return found;
        }

        //! \brief        <i><b> Set tuned status </b></i>
        //!
        //! Set tuned status for a service already present in the station list.
        //!
        //! \param[in]    id Service identifier
        //! \param[in]    status True if the service is tuned, false otherwise
        //! \return       True if the service has been correctly update, false otherwise
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        bool SetTunedStatus(unsigned int id, bool status)
        {
            typename std::list <T>::iterator it;

            for (it = radioServiceList.begin(); it != radioServiceList.end(); )
            {
                if (id == it->GetId())
                {
                    // Erase element and increment
                    it->SetTunedStatus(status);

                    // ID is unique we can exit the loop
                    break;
                }
                else
                {
                    it++;
                }
            }

            return status;
        }

        //! \brief        <i><b> Query service </b></i>
        //!
        //! Query for a specific service.
        //!
        //! \param[in]    id Service identifier
        //! \return       Pointer to the service data
        //!
        //! \remark       Not implemented.
        //! \callgraph
        //! \callergraph
        T * QueryService(unsigned int id)
        {
            return (T *)NULL;
        }

    protected:
        //! \brief        <i><b> Print service list data (save it) </b></i>
        //!
        //! This function save service information in order to retrieve it at next radio power up.
        //!
        //! \param[in]    os Output stream reference
        //! \return       Reference to the output stream
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        virtual std::ostream&Print(std::ostream&os) const
        {
            typename std::list <T>::const_iterator it;
            unsigned int tmp;

            // Save data: it shall be done same order as 'Load' (done in derived classes)
            for (it = radioServiceList.begin(); it != radioServiceList.end(); ++it)
            {
                // Service name
                os.write(it->GetServiceName().c_str(), NAME_STRINGS_CHAR_LEN);

                // Service ID
                tmp = it->GetId();
                os.write((const char *)&tmp, sizeof (unsigned int));

                // Frequency
                tmp = it->GetFrequency();
                os.write((const char *)&tmp, sizeof (unsigned int));

                // Band
                tmp = it->GetBand();
                os.write((const char *)&tmp, sizeof (unsigned int));

                // Preset number
                tmp = it->GetPresetNumber();
                os.write((const char *)&tmp, sizeof (short));
            }

            return os;
        }

        //! \brief        <i><b> Load service list data </b></i>
        //!
        //! This function load service information previously stored on non-volatile media.
        //!
        //! \param[in]    in Input stream reference
        //! \return       Reference to the input stream
        //!
        //! \remark       This is a pure vuirtual function: it needs to be implemented in derived classes.
        //! \callgraph
        //! \callergraph
        virtual std::istream&Load(std::istream&in) = 0;

        //! \brief        <i><b> Template class for overloaded operator '>>' </b></i>
        //!
        //! Overload operator '>>' template class.
        //!
        //! \param[in]    in Reference to input stream
        //! \tparam[in]   cm Station List reference
        //! \return       Standard input stream
        //! \callgraph
        //! \callergraph
        template <typename F> friend std::istream&operator>>(std::istream&in, StationList <F>&cm);

        //! \brief        <i><b> Template class for overloaded operator '<<' </b></i>
        //!
        //! Overload operator '<<' template class.
        //!
        //! \param[in]    out Reference to output stream
        //! \tparam[in]   cm Station List reference
        //! \return       Standard output stream
        //! \callgraph
        //! \callergraph
        template <typename F> friend std::ostream&operator<<(std::ostream&out, const StationList <F>&cm);

        radioServiceList_t radioServiceList; //!< Radio service list

    private:
        int availableNumOfService; //!< Number of services inserted in the list (available services)
};

//! \brief        <i><b> Operator '>>' </b></i>
//!
//! Overload implementation for the '>>' operator. It is used by \see StationList class (it is a friend class).
//!
//! \param[in]    in Input stream to load
//! \tparam[in]   cm StationList instance
//! \return       Standard input stream
//!
//! \remark       None.
//! \callgraph
//! \callergraph
template <typename T> std::istream&operator>>(std::istream&in, StationList <T>&cm)
{
    return cm.Load(in);
}

//! \brief        <i><b> Operator '<<' </b></i>
//!
//! Overload implementation for the '<<' operator. It is used by \see StationList class (it is a friend class).
//!
//! \param[in]    in Output stream to store
//! \tparam[in]   cm Reference to StationList instance (not modified)
//! \return       Standard output stream
//!
//! \remark       None.
//! \callgraph
//! \callergraph
template <typename T> std::ostream&operator<<(std::ostream&out, const StationList <T>&cm)
{
    return cm.Print(out);
}

#endif // STATION_LIST_H_

// End of file
