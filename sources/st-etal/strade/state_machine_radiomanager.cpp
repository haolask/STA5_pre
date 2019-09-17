#include "state_machine_radiomanager.h"

void StateMachineRadioManager::EventFromDeviceSlot(Events event)
{
    switch (event)
    {
        case EVENT_DATA_ON_SYNC:
        case EVENT_AUTO_CHECK_DATA:
            // If we would like on polling we need to set:
            // - dataCheck = false
            // Otherwise we need to emit the proper signal:
            // - emit CommandForRadioManagerSignal(EVENT_DATA_ON_SYNC)
            dataCheck = true;
            break;

        case EVENT_DATA_RDS_IN:
            rdsCheck = true;
            break;

        case EVENT_STOP_RDS:
            rdsCheck = false;
            break;

        case EVENT_QUALITY_SNR:
            qualityCheck = true;
            break;

        case EVENT_STOP_QUALITY_SNR:
            qualityCheck = false;
            break;

        case EVENT_STOP_ALL_EVENTS:
            globalEventsEnable = false;
            break;

        case EVENT_START_ALL_EVENTS:
            globalEventsEnable = true;
            break;

        default:
            // No code
            break;
    }
}

void StateMachineRadioManager::ExecutePollingActions()
{
    //qDebug() << "SMRM polling thread ID: " << QThread::currentThreadId() << "(" << QThread::currentThread() << ")";

    // Check if we have events globally disabled
    if (false == globalEventsEnable)
    {
        return;
    }

    // Get current time in ms
    QDateTime now = QDateTime::currentDateTime();
    qint64 tmpVal = now.toMSecsSinceEpoch();

    // Check if we have something to do (high priorities shall be on top)
    if (true == DoDabDataCheck(tmpVal))
    {
        emit CommandForRadioManagerSignal(EVENT_AUTO_CHECK_DATA, QVariant());
    }
    else if (true == DoGetRds(tmpVal))
    {
        emit CommandForRadioManagerSignal(EVENT_DATA_RDS_IN, QVariant());
    }
    else if (true == DoGetQuality(tmpVal))
    {
        emit CommandForRadioManagerSignal(EVENT_QUALITY_SNR, QVariant());
    }
    else if (true == DoCheckAlive(tmpVal))
    {
        emit CommandForRadioManagerSignal(EVENT_CHECK_ALIVE, QVariant());
    }
}

bool StateMachineRadioManager::DoGetRds(qint64 curTime)
{
    // Value in ms for get RDS
    #define RDS_GET_DELTA_TIME_MS           150

    bool res = false;
    qint64 delta;

    // If there's no request then return false
    if (false == rdsCheck)
    {
        return false;
    }

    // If we reach this point the RDS acquisition is active,
    // we need to check for delta from last check
    if (INVALID_LONGLONG_DATA != lastTimeGetRds)
    {
        delta = curTime - lastTimeGetRds;

        if (delta >= RDS_GET_DELTA_TIME_MS)
        {
            lastTimeGetRds = curTime;

            res = true;
        }
    }
    else
    {
        lastTimeGetRds = curTime;
    }

    return res;
}

bool StateMachineRadioManager::DoGetQuality(qint64 curTime)
{
    // Value in ms for get RDS
    #define FM_GET_DELTA_TIME_MS           400

    bool res = false;
    qint64 delta;

    // If there's no request then return false
    if (false == qualityCheck)
    {
        return false;
    }

    // If we reach this point the Quality SNR acquisition is active,
    // we need to check for delta from last check
    if (INVALID_LONGLONG_DATA != lastTimeGetQuality)
    {
        delta = curTime - lastTimeGetQuality;

        if (delta >= FM_GET_DELTA_TIME_MS)
        {
            lastTimeGetQuality = curTime;

            res = true;
        }
    }
    else
    {
        lastTimeGetQuality = curTime;
    }

    return res;
}

bool StateMachineRadioManager::DoDabDataCheck(qint64 curTime)
{
    // Value in ms for get DAB data
    #define DAB_DATA_CHECK_TIME_MS          400

    bool res = false;
    qint64 delta;

    // If there's no request then return false
    if (false == dataCheck)
    {
        return false;
    }

    // If we reach this point means that we had a request and
    // we have to check the delta time between 2 requests
    if (INVALID_LONGLONG_DATA != lastTimeDabDataCheck)
    {
        delta = curTime - lastTimeDabDataCheck;

        if (delta >= DAB_DATA_CHECK_TIME_MS)
        {
            lastTimeDabDataCheck = curTime;

            // Remove the request
            dataCheck = false;

            res = true;
        }
    }
    else
    {
        lastTimeDabDataCheck = curTime;
    }

    return res;
}

bool StateMachineRadioManager::DoCheckAlive(qint64 curTime)
{
    #define CHECK_ALIVE_DELTA_TIME_MS       30000

    bool res = false;
    qint64 delta;

    if (INVALID_LONGLONG_DATA != lastTimeCheckAlive)
    {
        delta = curTime - lastTimeCheckAlive;

        if (delta >= CHECK_ALIVE_DELTA_TIME_MS)
        {
            lastTimeCheckAlive = curTime;

            res = true;
        }
    }
    else
    {
        lastTimeCheckAlive = curTime;
    }

    return res;
}
