#include "globallist_mngr_mw.h"

MW_globalList_mngr::~MW_globalList_mngr()
{ }

void MW_globalList_mngr::on(bool fm, bool dab, quint32 fm_fe, quint32 dab_fe)
{
    Q_UNUSED(fm);
    Q_UNUSED(dab);
    Q_UNUSED(fm_fe);
    Q_UNUSED(dab_fe);
}

void MW_globalList_mngr::off()
{ }

void MW_globalList_mngr::term()
{ }

ServiceListTy MW_globalList_mngr::handlerFM()
{
    ServiceListTy list;

    list.serviceList.clear();

    return list;
}

ServiceListTy MW_globalList_mngr::handlerDAB()
{
    ServiceListTy list;

    list.serviceList.clear();

    return list;
}
