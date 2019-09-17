///
//! \file          postaloffice.h
//! \brief         Class used to dispatch messages between classes
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

#ifndef POSTALOFFICE_H
#define POSTALOFFICE_H

#include <QObject> // For 'Q_OBJECT' mandatory to use postal office as parameter
#include <cstring> // For memset

#include "postal_types.h"

//! \brief        <i><b> Base class for message dispatching </b></i>
//!
//! This base class implement object memory count in order to free the memory when a message is no more used.
//! When the object use cound reach 0 the object is freed and its contant is no more available.
//!
//! \remark This class shall not be used directly, the provided derived class shall be used instead.
class PostalOfficeBase : public QObject
{
    Q_OBJECT

    public:
        //! \brief        <i><b> Constructor </b></i>
        //!
        //! PostalOffcieBase constructor. Initialize the reference count to 0.
        //!
        //! \return       None
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        PostalOfficeBase() { referenceCount = 0; }

        //! \brief        <i><b> Get the type of data transported </b></i>
        //!
        //! This method can be used by the receiver to check what kind of data has been transmitted.
        //!
        //! \return       None
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        PostalType GetType()
        {
            return type;
        }

        //! \brief        <i><b> Lock function </b></i>
        //!
        //! This function increments the reference count allowing to keep it in memory until a function does not call
        //! the Unlock method.
        //!
        //! \return       None
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        void Lock()
        {
            ++referenceCount;
        }

        //! \brief        <i><b> Unlock function </b></i>
        //!
        //! This fucntino decrement the reference count and, if the counter reach 0, freed the memory deleting itself.
        //!
        //! \return       None
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        void Unlock()
        {
            --referenceCount;

            if (referenceCount == 0)
            {
                delete this;
            }
        }

    protected:
        //! \brief        <i><b> Destructor </b></i>
        //!
        //! Virtual destructor. The instance shall be deleted using the unlock function that decrements
        //! the reference count. Since this destructor is virtual the destructor of the derived class is called and
        //! then this one. If the base class is not declared virtual the one of teh derived class is not invoked.
        //!
        //! \return       None
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        //!
        virtual ~PostalOfficeBase() { }

        //! \brief        <i><b> Data type </b></i>
        //!
        //! Type of mail that is delivered. The alloed values are in a separate header file and are the
        //! agreement between the sender and the receiver: type is use dby teh receiver to check what kind of
        //! dat ahas been transmitted.
        PostalType type;

    private:
        //! \brief Reference count variable: it is used to maintain the object in memory until it counts to 0
        int referenceCount;
};

//! \brief        <i><b> Template class implementing message dispatching </b></i>
//!
//! This template class allows
//!
//! \tparam T
template <class T> class PostalOffice : public PostalOfficeBase
{
    //! \brief Using T as packet_t data
    using packet_t = T;

    public:
        //! \brief        <i><b> Default constructor </b></i>
        //!
        //! \return       None
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        PostalOffice() { }

        //! \brief        <i><b> Assignment operator overload </b></i>
        //!
        //! The assignment operator is overloaded in order to return the data packet transported with the
        //! postal instance.
        //!
        //! \param[in]    par Postal office reference
        //! \return       Posta office reference
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        PostalOffice&operator=(const PostalOffice&par)
        {
            if (this == &par)
            {
                return *this;
            }

            packet = par.packet;

            return *this;
        }

        //! \brief        <i><b> Put data function </b></i>
        //!
        //! This function shall be used by the sender to ship the data.
        //!
        //! \param[in]    what Type of transmitted data
        //! \tparam[in]   data Template parameter: data container
        //! \return       None
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        void PutPacket(PostalType what, T data)
        {
            packet = data;
            type = what;
        }

        //! \brief        <i><b> Get data function </b></i>
        //!
        //! This function shall be used by the receiver to retrieve the data.
        //!
        //! \return       Template return: the data sent is return as a copy
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        T GetPacket()
        {
            return packet;
        }

    private:
        //! \brief Data packet container
        packet_t packet;

        //! \brief        <i><b> Destructor </b></i>
        //!
        //! Virtual destructor. The instance shall be deleted using the unlock function that decrements
        //! the reference count. Since this destructor is virtual the destructor of the derived class is called and
        //! then this one. If the base class is not declared virtual the one of teh derived class is not invoked.
        //!
        //! \return       None
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        virtual ~PostalOffice() { }
};

#endif // POSTALOFFICE_H

// End of file
