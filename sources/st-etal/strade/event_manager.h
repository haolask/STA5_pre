///
//! \file          event_manager.h
//! \brief         Manage events storing and providing access to them
//! \author        Alberto Saviotti
//!
//! Project        MSR1
//! Sw component   lib_utils
//!
//! Copyright (c)  STMicroelectronics
//!
//! History
//! Date         | Modification               | Author
//! 20161206     | Initial version            | Alberto Saviotti
///

#ifndef EVENT_MANAGER_H_
#define EVENT_MANAGER_H_

#include <list>     // std::list
#include <vector>   // std::vector
#include <mutex>    // std::mutex

#include "defines.h"

////////////////////////////////////////////////////////////////////////////////
/// C++ <EventData> class                                                    ///
////////////////////////////////////////////////////////////////////////////////
template <class E>
struct EventData
{
    E eventType;
    int len;

    std::vector<unsigned char> blob;

    EventData (E type, int dataLen, unsigned char *dataPtr)
    {
        eventType = type;
        len = dataLen;
        blob.assign (dataPtr, (dataPtr + dataLen));
    }

    void Reset ()
    {
        eventType = 0;
        len = 0;
        blob.clear ();
    }
};


template <class E>
struct EventDataList
{
    std::list<EventData<E> *> list;
};

////////////////////////////////////////////////////////////////////////////////
/// C++ <EventManagerObserver> class                                         ///
////////////////////////////////////////////////////////////////////////////////
template <class E, class T>
class EventManagerObserver
{
    public:
        virtual ~EventManagerObserver() {}

        virtual void EventRaised (E eventType, int size, T *eventDataPtr) = 0;
};

template <class E, class T>
struct EventManagerObserverList
{
    std::list<EventManagerObserver<E, T> *> list;
};

////////////////////////////////////////////////////////////////////////////////
/// C++ <EventManager> class                                                 ///
////////////////////////////////////////////////////////////////////////////////
template <class E, class T>
class EventManager
{
    private:
        static EventDataList<E> eventDataList;
        static int eventNumber;

        static EventManagerObserverList<E, T> eventManagerObserverList;

        std::mutex exclusiveAccessArea;

    public:
        EventManager (EventManagerObserver<E, T> *observer = NULL);
        ~EventManager ();

        void Call ();

        int PushEvent (E eventType, int size, T *eventDataPtr);

        int PushEventOnlyExternal(E eventType, int size, T *eventDataPtr);

        int GetEventNumber () { return eventNumber; }

        void PullEvent (E &eventType, int &size, T *eventDataPtr);
};

template <class E, class T>
EventDataList<E> EventManager<E, T>::eventDataList;

template <class E, class T>
int EventManager<E, T>::eventNumber = 0;

template <class E, class T>
EventManagerObserverList<E, T> EventManager<E, T>::eventManagerObserverList;

template <class E, class T>
EventManager<E, T>::EventManager (EventManagerObserver<E, T> *observer)
{
    if (NULL != observer)
    {
        eventManagerObserverList.list.push_back (observer);
    }
}

template <class E, class T>
EventManager<E, T>::~EventManager ()
{

}

template <class E, class T>
void EventManager<E, T>::Call ()
{

}

template <class E, class T>
int EventManager<E, T>::PushEvent(E eventType, int size, T *eventDataPtr)
{
    // Get exclusive access to the event data list until function returns
    std::lock_guard<std::mutex> guard(exclusiveAccessArea);

    // Serve any registered observer
    typename std::list<EventManagerObserver<E, T> *>::iterator it;

    // Try to find the sub-channel and remove it
    for (it = eventManagerObserverList.list.begin(); it != eventManagerObserverList.list.end(); ++it)
    {
        (*it)->EventRaised (eventType, size, eventDataPtr);
    }

    // Push data
    eventDataList.list.push_back (EventData<E>(eventType, size, eventDataPtr));

    eventNumber++;

    return OSAL_OK;
}

template <class E, class T>
int EventManager<E, T>::PushEventOnlyExternal (E eventType, int size, T *eventDataPtr)
{
    // Get exclusive access to the event data list until function returns
    std::lock_guard<std::mutex> guard(exclusiveAccessArea);

    // Serve any registered observer
    //eventManagerObserver->EventRaised (eventDataPtr);
    typename std::list<EventManagerObserver<E, T> *>::iterator it;

    // Try to find the sub-channel and remove it
    for (it = eventManagerObserverList.list.begin(); it != eventManagerObserverList.list.end(); ++it)
    {
        (*it)->EventRaised (eventType, size, eventDataPtr);
    }

    return OSAL_OK;
}

template <class E, class T>
void EventManager<E, T>::PullEvent (E &eventType, int &size, T *eventDataPtr)
{
    // Get exclusive access to the event data list until function returns
    std::lock_guard<std::mutex> guard(exclusiveAccessArea);

    // Retrieve last element
    EventData<T> eventData = eventDataList.list.front ();

    // Get std::string from the vector container
    std::basic_string<unsigned char> tmpStr(eventData.blob.begin (), eventData.blob.end ());

    // Copy data inside the user buffer
    memcpy (eventDataPtr->dataPtr, tmpStr.c_str (), tmpStr.size ());
    size = eventData.len;
    eventType = eventData.eventType;

    // Remove last element
    eventDataList.list.pop_front ();

    // Signal event removal
    eventNumber--;
}

#endif // EVENT_MANAGER_H_

// End of file
