#ifndef MAIN_C
#define MAIN_C

#include <stdio.h>
#include <stdlib.h>

#include "osal.h"

#ifdef CONFIG_APP_OSALCORE_TESTS
#include "osaltest.h"

extern tS32 osalcore_test(tS32 cPar, tString aPar[]);

/*****************************************************************
| defines and macros (scope: module-local)
|----------------------------------------------------------------*/

/*****************************************************************
| variable defintion (scope: modul-local)
|----------------------------------------------------------------*/

/*****************************************************************
| function prototype (scope: module-local)
|----------------------------------------------------------------*/

/*****************************************************************
| function implementation (scope: module-local)
|----------------------------------------------------------------*/

/*****************************************************************
| function implementation (scope: global)
|----------------------------------------------------------------*/

int main(int argc, char **argv)
{
    TUNERDRIVER_system_init();
	return osalcore_test(argc, argv);
}
#endif

#endif


