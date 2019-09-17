/************************************************************************
* PROJECT : OSAL CORE on OS21
* FILE : osaltest.h
*
* DESCRIPTION : This is the headerfile for the OSAL test suite  Component.
*
* COPYRIGHT :(c) 2008, STMicroelectronics
*
* VERSION : 0.1
* DATE (mm.dd.yyyy) : 11.06.2008
* AUTHOR : L. Pesenti - G. Di Martino - K. Singhi
* HISTORY :
* Revision 1.4  2010/02/01 09:00:52  shz2hi
* VENDOR_STM_2010_01_29_01 integrated
*
* Revision 1.1.1.1  2009/12/09 16:13:40  gol2hi
* Import from STM
*
* Revision 1.1  2009/09/11 09:51:32  shz2hi
* new OS21 version with ADR3 support
*
* Revision 1.5  2009/05/25 08:20:11  shz2hi
* new ST delivery OSAL_22_5_2009
*
************************************************************************/


#ifndef OSALTEST_H
#define OSALTEST_H

/*****************************************************************************
| includes of component-internal interfaces
| (scope: component-local)
|----------------------------------------------------------------------------*/

/*****************************************************************************
| defines and macros (scope: global)
|----------------------------------------------------------------------------*/
#define CONST_TARGET_OSALTEST_TASKSTACKSIZE_INIT 32768

/*****************************************************************************
|typedefs and struct defs (scope: global)
|----------------------------------------------------------------------------*/

/*****************************************************************************
| variable declaration (scope: global)
|----------------------------------------------------------------------------*/

/*****************************************************************************
|function prototypes (scope: global)
|----------------------------------------------------------------------------*/
tVoid CheckNumber(tS32 s32NumReadOrWritten, tS32 s32Expected);


#else
#error osaltest.h included several times
#endif
