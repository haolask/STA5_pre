#ifndef OBSERVER_H
#define OBSERVER_H

#include <list>     // std::list
#include <mutex>    // std::mutex
#include <QString>

#include "common.h"

template <class T>
class Observer
{
    public:
        virtual ~Observer () { }

        virtual void ObserverEntityReceiveEvent (T event, void *dataPtr) = 0;
};

template <class T>
struct ObserverList
{
    std::list<Observer<T> *> list;
};

template <class T>
class Observed
{
    public:
        virtual ~Observed () { }

        void ObservedEntityEvent (ObserverList<T>& ObserverList, T event, void *dataPtr = NULL)
        {
            // Get exclusive access to the event data list until function returns
            std::lock_guard<std::mutex> guard(exclusiveAccessArea);

            // Serve any registered observer
            typename std::list<Observer<T> *>::iterator it;

            for (it = ObserverList.list.begin(); it != ObserverList.list.end(); ++it)
            {
                (*it)->ObserverEntityReceiveEvent (event, dataPtr);
            }
        }

    protected:
        void Attach (Observer<T> *observer)
        {
            if (NULL != observer)
            {
                observerList.list.push_back (observer);
            }
        }

        void Detach (Observer<T> *observer)
        {
            if (NULL != observer)
            {
                typename std::list<Observer<T> *>::iterator it;

                for (it = observerList.begin(); it != observerList.end(); )
                {
                    if (*it == observer)
                    {
                        // Erase element
                        observerList.list.erase (it);

                        // We can exit the loop
                        break;
                    }
                    else
                    {
                        it++;
                    }
                }
            }
        }

        static ObserverList<T> observerList;

    private:
        std::mutex exclusiveAccessArea;
};

template <class T>
ObserverList<T> Observed<T>::observerList;

#endif // OBSERVER_H
