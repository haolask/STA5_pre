#ifndef GLOBALLIST_MNGR_BASE_H
#define GLOBALLIST_MNGR_BASE_H

#include "common.h"

typedef struct
{
    quint32 freq;
    quint32 pi;
    QString ps;
    quint16 preset;
} FMService;

typedef enum
{
    FE_HANDLE_1 = 0,
    FE_HANDLE_2 = 1,
    FE_HANDLE_3 = 2,
    FE_HANDLE_4 = 3,
} FE_ID;

class GlobalList_mngr_base
{
    public:
        virtual ~GlobalList_mngr_base() = default;

        virtual void on(bool fm = true, bool dab = true, quint32 fm_fe = FE_HANDLE_1, quint32 dab_fe = FE_HANDLE_1) = 0;
        virtual void off() = 0;
        virtual void term() = 0;
        virtual ServiceListTy handlerFM() = 0;
        virtual ServiceListTy handlerDAB() = 0;
};

#endif // GLOBALLIST_MNGR_BASE_H
