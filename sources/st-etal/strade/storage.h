///
//! \file          storage.h
//! \brief         Template class used to store data on persistant memory
//! \author        Alberto Saviotti
//!
//! Project        STRaDe
//! Sw component   HMI
//!
//! Copyright (c)  STMicroelectronics
//!
//! History
//! Date           | Modification               | Author
//! 20180127       | Initial version            | Alberto Saviotti
///

#ifndef STORAGE_H_
#define STORAGE_H_

#include <fstream>
#include <iostream>
#include <string.h> // memset

//! \brief        <i><b> Connect and send commands to external program acting as a server </b></i>
//!
//! This template class allows to save and load data to/from persistant memory. The data passed shall not contain
//! memory pointers because the passed address will be saved as a contiguos bunch of memory.
//!
//! \tparam T The object to store. The object shall be a single memory area, i.e. it cannot contain pointers to
//!           memory areas not contigues. The data will be saved and loaded from memory
template <class T> class Storage
{
    //! \brief Using T as storageSpace_t data
    using storageSpace_t = T;

    public:
        //! \brief        <i><b> Create the instance </b></i>
        //!
        //! This function creates an instance of the storage class for an application defined object.
        //!
        //! \return       None
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        Storage ()
        {
            memset (static_cast<void *>(&storageSpace), 0, sizeof (storageSpace));
        }

        //! \brief        <i><b> Delete the instance </b></i>
        //!
        //! This function deletes an instance of the storage class for an application defined object and freed the
        //! allocated memory. The destructor is defined as virtual because this class can be derived in order to
        //! implement more sophisticated storage functions tailored to specific needs. The Storage class acts as a
        //! design pattern, the storage of complex memory types is left to be implemented in derived functions.
        //!
        //! \return       None
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        virtual ~Storage () { }

        //! \brief        <i><b> Create the instance and pass memory address to be saved </b></i>
        //!
        //! This function creates an instance of the storage class for an application defined object and copy
        //! passed object in private parameter.
        //!
        //! \param[in]    storage Object to store
        //! \return       None
        //!
        //! \remark       Calling this function makes the call to SetStatus() function not necessary.
        //! \callgraph
        //! \callergraph
        Storage (T storage)
        {
            storageSpace = storage;
        }

        //! \brief        <i><b> Store the object </b></i>
        //!
        //! This function copies the passed object in ordr to store it.
        //!
        //! \param[in]    status Object to store
        //! \return       None
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        void SetStatus (T status)
        {
            storageSpace = status;
        }

        //! \brief        <i><b> Return the stored object </b></i>
        //!
        //! This function returns the stored object.
        //!
        //! \return       The stored object
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        T GetStatus ()
        {
            return storageSpace;
        }

    protected:
        //! \brief        <i><b> Store the data </b></i>
        //!
        //! This function does the data write using an output stream.
        //!
        //! \param[in]    os Output stream
        //! \return       None
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        virtual std::ostream& Print (std::ostream &os) const
        {
            os.write(reinterpret_cast <char *> (const_cast <storageSpace_t *> (&storageSpace)), sizeof (storageSpace));

            os.flush ();

            return os;
        }

        //! \brief        <i><b> Load the data </b></i>
        //!
        //! This function does the data read using an input stream.
        //!
        //! \param[in]    os Input stream
        //! \return       Return the input stream
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        virtual std::istream& Load (std::istream& in)
        {
            in.read(reinterpret_cast <char *> (&storageSpace), sizeof (storageSpace));

            return in;
        }

        //! \brief Load operator
        //!
        //! \param[in]  in Input stream to load from
        //! \tparam[in] cm Template class to load
        template <typename F> friend std::istream&operator>>(std::istream&in, Storage <F>&cm);

        //! \brief Load operator
        //!
        //! \param[in]  in Output stream to store to
        //! \tparam[in] cm Template class to store
        template <typename F> friend std::ostream&operator<<(std::ostream&out, const Storage <F>&cm);

    private:
        //! \brief Storage area
        storageSpace_t storageSpace;
};

//! \name Operators used to perform data store and load for the Storage class.
//!
//! \param[in]  in Input stream to load data from
//! \tparam[in] cm Storage class
//!{
//! \brief Load operator
template <typename T> std::istream&operator>>(std::istream&in, Storage <T>&cm)
{
    return cm.Load (in);
}

//! \brief Save operator
template <typename T> std::ostream&operator<<(std::ostream&out, const Storage <T>&cm)
{
    return cm.Print (out);
}

//!}

#endif // STORAGE_H_

// End of file
