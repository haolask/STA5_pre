#ifndef POSTAL_TYPES_H
#define POSTAL_TYPES_H

enum PostalType
{
    POSTAL_TYPE_IS_NONE             = 0,
    POSTAL_TYPE_IS_DLS              = 1,
    POSTAL_TYPE_IS_SLS              = 2,
    POSTAL_TYPE_IS_ENSEMBLE_NAME    = 3,
    POSTAL_TYPE_IS_SERVICE_NAME     = 4,
    POSTAL_TYPE_IS_SERVICE_LIST     = 5,
    POSTAL_TYPE_IS_SERVICESEL       = 6,   // Not used
    POSTAL_TYPE_IS_SYNC_LEVEL       = 7,
    POSTAL_TYPE_IS_QUALITY_LEVEL    = 8,
    POSTAL_TYPE_IS_FREQ             = 9,
    POSTAL_TYPE_IS_RDS              = 10,
    POSTAL_TYPE_IS_EVENT_ANSWER     = 11,
    POSTAL_TYPE_IS_AUDIO_PLAYS      = 12
};

#endif // POSTAL_TYPES_H
