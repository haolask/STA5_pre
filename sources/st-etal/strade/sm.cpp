///
//! \file          sm.cpp
//! \brief         State machine implementatino file
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

#include "sm.h"

SmState::SmState()
{
    // Initialize transition list
    transitions.clear();

    // Initialize the new state
    stateName.clear();
    enabled = true;

    // Set fallabck stat eto empty
    fallbackState.clear();
}

SmState::SmState(QString newStateName)
{
    // Initialize transition list
    transitions.clear();

    // Initialize the new state
    stateName = newStateName;
    enabled = true;
}

SmTransition::SmTransition()
{
    // Initialize members
    eventName.clear();
    toState.clear();
    priority = 0;
}

SmMachine::SmMachine(QString newName)
{
    // Set the new machine name
    machineName = newName;

    // Initialize state list member
    stateList.clear();

    // Initialize valid events list
    validEventList.clear();

    // Initialize current status
    currentState.clear();

    // Initialize last status
    lastState.clear();

    // Initialize default state
    defaultState.clear();
}

bool SmMachine::CheckIfStateAlreadyExists(QString stateName)
{
    bool found = false;

    // Search from state in the state list
    QList<SmState>::const_iterator iter;

    for (iter = stateList.constBegin(); iter != stateList.constEnd(); ++iter)
    {
        const SmState& tmpState = *iter;
        QString tmpName = tmpState.GetName();

        if (0 == tmpName.compare(stateName))
        {
            // Set found to true
            found = true;

            // A single state is target of this call
            break;
        }
    }

    return found;
}

bool SmMachine::InsertNewState(QString newStateName)
{
    // Check if the state already exist
    if (true == CheckIfStateAlreadyExists(newStateName))
    {
        // We cannot insert the state: it already exists
        return false;
    }

    SmState* newSmState = new SmState(newStateName);

    stateList.append(*newSmState);

    return true;
}

bool SmMachine::CheckIfEventAlreadyExists(QString eventName)
{
    bool found = false;

    // Search from state in the state list
    QList<QString>::const_iterator iter;

    for (iter = validEventList.constBegin(); iter != validEventList.constEnd(); ++iter)
    {
        const QString& tmpTransition = *iter;

        if (0 == tmpTransition.compare(eventName))
        {
            // This transition already exists, set found to true
            found = true;

            // Already existing transition
            break;
        }
    }

    return found;
}

bool SmMachine::InsertNewEvent(QString newEventName)
{
    bool opResult = false;

    // If the transition is not a duplicate one we insert it
    if (false == CheckIfEventAlreadyExists(newEventName))
    {
        validEventList.append(newEventName);

        opResult = true;
    }

    return opResult;
}

bool SmMachine::CheckEnabledStatus(QString stateName)
{
    bool opResult = false;

    // Search from state in the state list
    QList<SmState>::iterator iter;

    for (iter = stateList.begin(); iter != stateList.end(); ++iter)
    {
        SmState& state = *iter;
        QString tmpName = state.GetName();

        if (0 == tmpName.compare(stateName))
        {
            // Save status
            opResult = state.GetEnabledStatus();

            // A single state is target of this call
            break;
        }
    }

    return opResult;
}

bool SmMachine::ChangeEnabledStatus(QString stateName, bool newStatus)
{
    bool opResult = false;

    // Search from state in the state list
    QList<SmState>::iterator iter;

    for (iter = stateList.begin(); iter != stateList.end(); ++iter)
    {
        SmState& state = *iter;
        QString tmpName = state.GetName();

        if (0 == tmpName.compare(stateName))
        {
            // Set status
            state.SetEnabledStatus(newStatus);

            opResult = true;

            // A single state is target of this call
            break;
        }
    }

    return opResult;
}

bool SmMachine::SetFallbackState(QString stateName, QString fallbackStateName)
{
    bool opResult = false;

    // Search from state in the state list
    QList<SmState>::iterator iter;

    for (iter = stateList.begin(); iter != stateList.end(); ++iter)
    {
        SmState& state = *iter;
        QString tmpName = state.GetName();

        if (0 == tmpName.compare(stateName))
        {
            // Set status
            state.SetFallbackState(fallbackStateName);

            opResult = true;

            // A single state is target of this call
            break;
        }
    }

    return opResult;
}

bool SmMachine::EnableState(QString stateName)
{
    return ChangeEnabledStatus(stateName, true);
}

bool SmMachine::DisableState(QString stateName)
{
    return ChangeEnabledStatus(stateName, false);
}

bool SmMachine::InsertStateChange(QString fromState, QString event, QString toState, int priority)
{
    bool found = false;

    // Sanity check: the transition shall exists
    if (false == CheckIfEventAlreadyExists(event))
    {
        // The event isn't registered
        return false;
    }

    // Sanity check: both from and to states shall exist
    if (false == CheckIfStateAlreadyExists(fromState))
    {
        // The state isn't registered
        return false;
    }

    // To state can be nullptr indicating to return to previous status
    if (nullptr != toState)
    {
        if (false == CheckIfStateAlreadyExists(toState))
        {
            // The state isn't registered
            return false;
        }
    }

    // Search from state in the state list
    QList<SmState>::iterator iter;

    for (iter = stateList.begin(); iter != stateList.end(); ++iter)
    {
        SmState& state = *iter;
        QString tmpName = state.GetName();

        if (0 == tmpName.compare(fromState))
        {
            // Create the new transition
            SmTransition* transition = new SmTransition(event, toState, priority);

            // Insert transition
            state.AppendTransition(const_cast<const SmTransition *> (transition));

            // Set found to true
            found = true;

            // A single state is target of this call
            break;
        }
    }

    return found;
}

bool SmMachine::SetCurrentState(QString stateName)
{
    // Check if it is a valid state
    if (false == CheckIfStateAlreadyExists(stateName))
    {
        // The state isn't registered
        return false;
    }

    // Save current state in last state
    lastState = currentState;

    // Set new state
    currentState = stateName;

    return true;
}

SmState * SmMachine::GetSmStateEntry(QString stateName)
{
    // Search from state in the state list
    QList<SmState>::iterator iter;

    for (iter = stateList.begin(); iter != stateList.end(); ++iter)
    {
        SmState* statePtr = &(*iter);
        QString tmpName = statePtr->GetName();

        if (0 == tmpName.compare(stateName))
        {
            // Return the pointer
            return statePtr;
        }
    }

    return nullptr;
}

bool SmMachine::SetDefaultState(QString defaultStateName)
{
    // Check if it is a valid state
    if (false == CheckIfStateAlreadyExists(defaultStateName))
    {
        // The state isn't registered
        return false;
    }

    defaultState = defaultStateName;

    return true;
}

bool SmMachine::EnterDefaultState()
{
    bool opResult = false;

    // Enter default state: this shall be set otherwise we will not change state
    if (false == defaultState.isEmpty())
    {
        lastState = defaultState;

        currentState = defaultState;

        opResult = true;
    }

    return opResult;
}

QString SmMachine::CalculateNewState(QString eventOccurred)
{
    int lastPriority = -1;
    QString tmpTargetName;

    tmpTargetName.clear();

    // Check if we passed nullptr as NO EVENT indication (this can be used to calculate the new state
    // when a state is currently selected and it became disabled)
    if (nullptr != eventOccurred)
    {
        // Check if the event is valid
        if (false == CheckIfEventAlreadyExists(eventOccurred))
        {
            // Invalid event: there's no change, current state is returned
            return currentState;
        }

        // Calculate new state
        SmState* currentSmStateEntry = GetSmStateEntry(currentState);

        QList<SmTransition>::const_iterator transitionIter;

        for (transitionIter = currentSmStateEntry->transitions.constBegin(); transitionIter != currentSmStateEntry->transitions.constEnd(); ++transitionIter)
        {
            // Find legal target state and store priority
            const SmTransition& tmpTransition = *transitionIter;

            QString tmpEvent = tmpTransition.GetName();

            if (0 == tmpEvent.compare(eventOccurred))
            {
                int tmpTargetPriority = tmpTransition.GetPriority();

                // We consider only priorities greater than last one found
                if (tmpTargetPriority > lastPriority)
                {
                    // Now we need to evaluate if target state is enabled
                    QList<SmState>::const_iterator stateIter;

                    for (stateIter = stateList.begin(); stateIter != stateList.end(); ++stateIter)
                    {
                        const SmState& tmpState = *stateIter;

                        // We check if the name is the wanted one and if the status is enabled
                        if (0 == tmpState.GetName().compare(tmpTransition.GetTargetState()) && true == tmpState.GetEnabledStatus())
                        {
                            // Save the name as temporary target: it will be the real target only if no one else has higher priority
                            tmpTargetName = tmpState.GetName();

                            // We do not need to search further between state and we can return to iterate through state
                            break;
                        }
                        else if (true == tmpTransition.GetTargetState().isEmpty())
                        {
                            // We have an empty target meaning we need to return to old state if it is still enabled
                            // Now we need to evaluate if target state is enabled
                            QList<SmState>::const_iterator lastStateIter;

                            for (lastStateIter = stateList.begin(); lastStateIter != stateList.end(); ++lastStateIter)
                            {
                                const SmState& tmpLastState = *stateIter;

                                if (true == tmpLastState.GetEnabledStatus() && 0 == tmpLastState.GetName().compare(lastState))
                                {
                                    // We do not need to search further because we found a valid target
                                    tmpTargetName = tmpLastState.GetName();

                                    // We can break
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }

        // Check if the target is valid otherwise do not change state
        if (false == tmpTargetName.isEmpty())
        {
            lastState = currentState;

            currentState = tmpTargetName;
        }
    }
    else
    {
        // We have no action and we ned to calculate which is the new status: we use the fallback status if set
        // or the default state as last decision
        SmState* currentSmStateEntry = GetSmStateEntry(currentState);

        // If the state is enabled there's no transition, if it is disabled we need to calculate the new state
        if (false == currentSmStateEntry->GetEnabledStatus())
        {
            // Check fallback state
            QString fallbackName = currentSmStateEntry->GetFallbackState();

            if (false == fallbackName.isEmpty())
            {
                lastState = currentState;

                currentState = fallbackName;
            }
            else
            {
                // Enter default state: this shall be set otherwise we will not change state
                if (false == defaultState.isEmpty())
                {
                    lastState = currentState;

                    currentState = defaultState;
                }
            }
        }
    }

    return currentState;
}

// End of file
