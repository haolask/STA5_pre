///
//! \file          sm.h
//! \brief         State machine framework
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

#ifndef SM_H
#define SM_H

#include <QString>
#include <QList>

class SmMachine; // Forward declaration

//! \brief        <i><b> Class representing state transitions </b></i>
//!
//! This class allows to define a transition. The transition identifier is its name.
//!
//! \remark None.
class SmTransition
{
    //! \brief SmMachine friend declaration
    //! \relates SmMachine
    friend class SmMachine;

    public:
        //! \brief        <i><b> Class constructor </b></i>
        //!
        //! This function creates an instance of the transition class.
        //!
        //! \return       None
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        SmTransition();

        //! \brief        <i><b> Class alternate constructor </b></i>
        //!
        //! This function creates an instance of the transition class already initializing the class members.
        //! This call already initialize completely the instance.
        //!
        //! \param[in]    eventCausingTransition event causing the transition
        //! \param[in]    targetState State the state machine moves into
        //! \param[in]    transitionPriority Priority of thsi transition (0 means default priority, higher number means
        //!               higher priority)
        //! \return       None
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        SmTransition(QString eventCausingTransition, QString targetState, int transitionPriority)
        {
            eventName = eventCausingTransition;
            toState = targetState;
            priority = transitionPriority;
        }

        //! \brief        <i><b> Virtual class destructor </b></i>
        //!
        //! This function free the memory allocated for the object and for the members allocated.
        //!
        //! \return       None
        //!
        //! \remark       None.
        //! \callgraph
        virtual ~SmTransition() { }

        //! \brief        <i><b> Get event name </b></i>
        //!
        //! Get the event name owner of this entry. The event name is the state identifier and cannot have duplicates.
        //!
        //! \return       String containings the event name
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        QString GetName() const
        {
            return eventName;
        }

        //! \brief        <i><b> Get target state name </b></i>
        //!
        //! Get the state name target of this transition.
        //!
        //! \return       String containings the target state name
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        QString GetTargetState() const
        {
            return toState;
        }

        //! \brief        <i><b> Get transition priority </b></i>
        //!
        //! Get the transition priority (0 means default and lower transition priority, higher numbers mean higher
        //! transition priority).
        //!
        //! \return       Integer representing the priority (0 = default/lowest, higher number means higher priority)
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        int GetPriority() const
        {
            return priority;
        }

    private:
        //! \brief Event name
        QString eventName;

        //! \brief Target state name
        QString toState;

        //! \brief Transition priority
        int priority;
};

//! \brief        <i><b> Class representing a state </b></i>
//!
//! This class allows to define a state. The state identifier is its name.
//!
//! \remark None.
class SmState
{
    //! \brief SmMachine friend declaration
    //! \relates SmMachine
    friend class SmMachine;

    public:
        //! \brief        <i><b> Class constructor </b></i>
        //!
        //! This function creates an instance of the state class.
        //!
        //! \return       None
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        SmState();

        //! \brief        <i><b> Class constructor (alternative) </b></i>
        //!
        //! This function creates an instance of the state class with already provided name.
        //!
        //! \param[in]    newStateName New state name
        //! \return       None
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        SmState(QString newStateName);

        //! \brief        <i><b> Virtual class destructor </b></i>
        //!
        //! This function free the memory allocated for the object and for the members allocated.
        //!
        //! \return       None
        //!
        //! \remark       None.
        //! \callgraph
        virtual ~SmState() { }

        //! \brief        <i><b> Add a new transition for a given state </b></i>
        //!
        //! .
        //! \param[in]    newTransition Transition pointer: points to the transition entry to be appended (List copy
        //!               the element, it does not reference the original instance
        //! \return       None
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        void AppendTransition(const SmTransition* newTransition)
        {
            transitions.append(*newTransition);
        }

        //! \brief        <i><b> Set state status </b></i>
        //!
        //! Set the state status: true means that this state is enabled, false it is disabled. A state in a disable
        //! status cannot be target of a transition.
        //!
        //! \param[in]    active True if the state is in active state, false if it is disabled
        //! \return       None
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        void SetEnabledStatus(bool active)
        {
            enabled = active;
        }

        //! \brief        <i><b> Set fallback state </b></i>
        //!
        //! Set the fallback state, thi state will be used as fallback target state if explicit transition cannot
        //! be done because target states are disabled.
        //!
        //! \param[in]    stateName Fallback state name
        //! \return       None
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        void SetFallbackState(QString stateName)
        {
            fallbackState = stateName;
        }

        //! \brief        <i><b> Get fallback state </b></i>
        //!
        //! Set the fallback state, thi state will be used as fallback target state if explicit transition cannot
        //! be done because target states are disabled.
        //!
        //! \return       String representing the fallback state name
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        QString GetFallbackState() const
        {
            return fallbackState;
        }

        //! \brief        <i><b> Get state name </b></i>
        //!
        //! Get the state name owner of this entry. The state name is the state identifier and cannot have duplicates.
        //!
        //! \return       String containings the state name
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        QString GetName() const
        {
            return stateName;
        }

        //! \brief        <i><b> Get enabled state </b></i>
        //!
        //! Returns current status.
        //!
        //! \return       True if the state is enabled, false if it is disabled
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        bool GetEnabledStatus() const
        {
            return enabled;
        }

    protected:
        //! \brief Transition list
        QList<SmTransition> transitions;

    private:
        //! \brief State name
        QString stateName;

        //! \brief State status, true means enabeld, false disabled
        bool enabled;

        //! \brief Fallback state name. Only a single fallback stateis allowed
        QString fallbackState;
};

//! \brief        <i><b> Class for management of transitions between states </b></i>
//!
//! This class allows to setup a state machine and manage state transitions indipendently from the particular
//! domain of application. The class is developed tailored to have easy usage and understanding of the state
//! transitions; this is obtained using strings for state and event declaration.
//!
//! \remark For an embedded solution where performances are of great importance a class using name's hashes could
//!         be developed.
class SmMachine
{
    public:
        //! \brief        <i><b> Class constructor </b></i>
        //!
        //! This function creates an instance of the state machine class.
        //!
        //! \return       None
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        SmMachine(QString newName = nullptr);

        //! \brief        <i><b> Class destructor </b></i>
        //!
        //! This function free the memory allocated for the object and for the members allocated.
        //!
        //! \return       None
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        ~SmMachine()
        {
            stateList.clear();
            validEventList.clear();
        }

        //! \brief        <i><b> Insert a new state </b></i>
        //!
        //! This function creates a new state with the provided name. The name itself will be used as unique state
        //! identifier. The function call will fail if a duplicated name is used.
        //!
        //! \param[in]    newStateName The new state name
        //! \return       True if the function succeed, false otherwise
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        bool InsertNewState(QString newStateName);

        //! \brief        <i><b> Insert a new event </b></i>
        //!
        //! This function creates a new event with the provided name. The name itself will be used as unique event
        //! identifier. The function call will fail if a duplicated name is used.
        //!
        //! \param[in]    newEventName The new event name
        //! \return       True if the function succeed, false otherwise
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        bool InsertNewEvent(QString newEventName);

        //! \brief        <i><b> Set the fallback state </b></i>
        //!
        //! This function sets the fallback state for a particular state of the state machine. The fallback state will
        //! be used if allowed transitions are not available, e.g. because the states are in disable status.
        //! This status should be set for states that could be disabled: in case a state became disabled meanwhile it is
        //! enabled and no action is performed (i.e. the event causing the current status to became disabled is not one of
        //! the events causing transitions) the fallback state is used as target.
        //!
        //! \param[in]    newEventName The new event name
        //! \return       True if the function succeed, false otherwise
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        bool SetFallbackState(QString stateName, QString fallbackStateName);

        //! \brief        <i><b> Enable a state </b></i>
        //!
        //! This function enables a particular state.
        //!
        //! \param[in]    stateName The state name to enable
        //! \return       True if the function succeed, false otherwise
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        bool EnableState(QString stateName);

        //! \brief        <i><b> Disable a state </b></i>
        //!
        //! This function disables a particular state.
        //!
        //! \param[in]    stateName The state name to disable
        //! \return       True if the function succeed, false otherwise
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        bool DisableState(QString stateName);

        //! \brief        <i><b> Insert a transition between 2 states </b></i>
        //!
        //! This function insert a transition between 2 states.
        //!
        //! \param[in]    fromState Starting state name
        //! \param[in]    event Event causing the transition
        //! \param[in]    toState Ending state name
        //! \param[in]    priority Priority of this transition (default = 0). Higher number means higher priority
        //! \return       True if the function succeed, false otherwise
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        bool InsertStateChange(QString fromState, QString event, QString toState, int priority = 0);

        //! \brief        <i><b> Set current active state </b></i>
        //!
        //! This function can be used to set current state. The stae machine will consider current state as the one
        //! where the code is inside.
        //!
        //! \param[in]    stateName The state name to set to current
        //! \return       True if the function succeed, false otherwise
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        bool SetCurrentState(QString stateName);

        //! \brief        <i><b> Calculate a new state </b></i>
        //!
        //! This function returns the new state in which the state machine enters when an event occurrs.
        //!
        //! \param[in]    eventOccurred Event occurred
        //! \return       New state name. The name has alwasy to be a valid state.
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        QString CalculateNewState(QString eventOccurred);

        //! \brief        <i><b> Set default state </b></i>
        //!
        //! This function sets the default state for a state machine, this state will be used if no explicit transition
        //! is fitting current state machine status. Thsi can occur for states without a fallback state sets and
        //! with transitins towards state that can be ind isabled state.
        //!
        //! \param[in]    defaultStateName The default state name
        //! \return       True if the function succeed, false otherwise
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        bool SetDefaultState(QString defaultStateName);

        //! \brief        <i><b> Enter default state </b></i>
        //!
        //! Force the machine to enter default state.
        //!
        //! \return       True if the function succeed, false otherwise
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        bool EnterDefaultState();

        //! \brief        <i><b> Get current state </b></i>
        //!
        //! This function returns the current state in which the state machine is.
        //!
        //! \return       State name
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        QString GetCurrentState()
        {
            return currentState;
        }

        //! \brief        <i><b> Get previous state </b></i>
        //!
        //! This function returns the previous state in which the state machine was.
        //!
        //! \return       State name
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        QString GetLastState()
        {
            return lastState;
        }

        //! \brief        <i><b> Get default state </b></i>
        //!
        //! This function returns the default state in which the state machine is.
        //!
        //! \return       State name
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        QString GetDefaultState()
        {
            return defaultState;
        }

        //! \brief        <i><b> Get enable state </b></i>
        //!
        //! Return the enable status for a given state, true means enabled, false disabled
        //!
        //! \param[in]    stateName State name
        //! \return       True for enabled, false for disabled
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        bool CheckEnabledStatus(QString stateName);

    private:
        //! \brief        <i><b> Check duplicate state </b></i>
        //!
        //! This function checks for duplicate entry in the state table.
        //!
        //! \param[in]    stateName State name
        //! \return       True if the function succeed, false otherwise
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        bool CheckIfStateAlreadyExists(QString stateName);

        //! \brief        <i><b> Check duplicate event </b></i>
        //!
        //! This function checks for duplicate event in the state table.
        //!
        //! \param[in]    stateName State name
        //! \return       True if the function succeed, false otherwise
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        bool CheckIfEventAlreadyExists(QString eventName);

        //! \brief        <i><b> Check duplicate event </b></i>
        //!
        //! This function checks for duplicate event in the state table.
        //!
        //! \param[in]    stateName State name
        //! \param[in]    newStatus Modify the state status: true = enabled, false = disabled
        //! \return       True if the function succeed, false otherwise
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        bool ChangeEnabledStatus(QString stateName, bool newStatus);

        //! \brief        <i><b> Get a state machine state entry </b></i>
        //!
        //! This function return a state entry as it has been stored by the state machine.
        //! This means to get a 'SmState' object containing all state details. This fucntion can be used to
        //! modify the members of the state.
        //!
        //! \param[in]    stateName State name
        //! \return       Pointer to the state entry
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        SmState* GetSmStateEntry(QString stateName);

        //! \brief State machine name (it is used to uniquely identify the state machine but it is not really used)
        QString machineName;

        //! \brief List of valid states as inserted by the caller at state machine initialization
        QList<SmState> stateList;

        //! \brief List of valid events as inserted by the caller at state machine initialization
        QList<QString> validEventList;

        //! \brief Current state name
        QString currentState;

        //! \brief Previous state name
        QString lastState;

        //! \brief Default state name
        QString defaultState;
};

#endif // SM_H

// End of file
