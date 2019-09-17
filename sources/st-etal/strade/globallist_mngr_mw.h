#ifndef GLOBALIST_MNGR_MW_H
#define GLOBALIST_MNGR_MW_H
#include <QObject>
#include <QDebug>
#include "globallist_mngr_base.h"

class MW_globalList_mngr : public GlobalList_mngr_base
{
    public:
        ~MW_globalList_mngr();

        void on(bool fm = true, bool dab = true, quint32 fm_fe = FE_HANDLE_1, quint32 dab_fe = FE_HANDLE_1);
        void off();
        void term();
        ServiceListTy  handlerFM();
        ServiceListTy handlerDAB();
};

#endif // GLOBALIST_MNGR_MW_H
