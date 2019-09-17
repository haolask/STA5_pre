//!
//!  \file 		oserror.c
//!  \brief 	<i><b>OSAL Error Handling Functions</b></i>
//!  \details	This is the implementation file for the OSAL
//!             (Operating System Abstraction Layer) Error-Functions.
//!  \author 	Raffaele Belardi
//!  \author 	(original version) Luca Pesenti
//!  \version 	1.0
//!  \date 		15.12.2009
//!  \bug 		Unknown
//!  \warning	None
//!
#ifdef __cplusplus
extern "C" {
#endif

/* osal Header */
#include "osal.h"

/************************************************************************
|defines and macros (scope: module-local)
|-----------------------------------------------------------------------*/

/************************************************************************
|typedefs (scope: module-local)
|-----------------------------------------------------------------------*/

/************************************************************************
| variable definition (scope: module-local)
|-----------------------------------------------------------------------*/

	/* const Error-Strings, returned by OSAL_coszErrorText */
#ifdef CONFIG_TRACE_CLASS_OSALCORE
tCString linux_szErrorString_NOPERMISSION     =
         "The thread has no permission to call the function";

tCString linux_szErrorString_INVALIDVALUE     =
         "One parameter of the function is invalid e.g.too large";

tCString linux_szErrorString_NOSPACE          =
         "No system resources are available e.g. no free space";

tCString linux_szErrorString_BUSY             =
         "The system can not release the resource, as this is still used";

tCString linux_szErrorString_INTERRUPT        =
         "An Interruption has occurred whereas the thread waits";

tCString linux_szErrorString_NOACCESS         =
         "The access rights tom the resource are invalid";

tCString linux_szErrorString_ALREADYEXISTS    =
         "The resource exists already";

tCString linux_szErrorString_DOESNOTEXIST     =
         "Access to the resource can not be made,as this does not exist";

tCString linux_szErrorString_MAXFILES         =
         "The process has already too many file descriptors";

tCString linux_szErrorString_NOFILEDESCRIPTOR =
         "In the system no resources are available";

tCString linux_szErrorString_NAMETOOLONG      =
         "The name of the resource e.g. data file name is too long";

tCString linux_szErrorString_BADFILEDESCRIPTOR=
         "The specified data file descriptor does not exist";

tCString linux_szErrorString_MSGTOOLONG       =
         "The individual message is too long";

tCString linux_szErrorString_QUEUEFULL        =
         "The message box can not accept further messages";

tCString linux_szErrorString_WRONGPROCESS     =
         "The specified process does not exist";

tCString linux_szErrorString_WRONGTHREAD      =
         "The specified thread does not exist";

tCString linux_szErrorString_EVENTINUSE       =
         "A thread already waits for the evevnt";

tCString linux_szErrorString_WRONGFUNC        =
         "The specified function does not exist";

tCString linux_szErrorString_NOTINTERRUPTCALLABLE=
         "The function has been called by an interrupt level";

tCString linux_szErrorString_INPROGRESS       =
         "The function is not yet completed";

tCString linux_szErrorString_TIMEOUT          =
         "The function is aborted on account of a time out";

tCString linux_szErrorString_NOTSUPPORTED     =
         "The device does not support the specified function";

tCString linux_szErrorString_CANCELED         =
         "The asynchronous order has been aborted successfully";

tCString linux_szErrorString_UNKNOWN          =
         "An unknown error has occurred";

tCString linux_szErrorString_NO_ERROR         =
         "No error has occurred";

tCString linux_szErrorString_LIBERROR          =
         "A Library error has occurred";

tCString linux_szErrorString_GENERIC =
		"Generic error";

tCString linux_szErrorString_UNEXPECTED = 
		"Unexpected";

tCString linux_szErrorString_INVALID_PARAM = 
		"Invalid parameter";

tCString linux_szErrorString_DEVICE_INI =
		"Device Initialization";

tCString linux_szErrorString_DEVICE_NOT_OPEN =
		"Device Open";

tCString linux_szErrorString_ACTION_FORBIDDEN =
		"Action forbidden";

tCString linux_szErrorString_CANNOT_OPEN =
		"Cannot open";

tCString linux_szErrorString_CHECKSUM =
		"Checksum error";

tCString linux_szErrorString_FROM_SLAVE =
		"Error from slave";

tCString linux_szErrorString_NOT_SUPPORTED =
		"Not supported";

tCString linux_szErrorString_TIMEOUT_EXPIRED =
		"Timeout expired";

tCString linux_szErrorString_CALLBACK =
		"Callback error";

tCString linux_szErrorString_NOT_IMPLEMENTED =
		"Not implemented";
#endif // CONFIG_TRACE_CLASS_OSALCORE

/************************************************************************
| variable definition (scope: global)
|-----------------------------------------------------------------------*/
/************************************************************************
|function prototype (scope: module-local)
|-----------------------------------------------------------------------*/

static int osal2errno(tU32 osalerror);
#if 0
static tVoid OSAL_vErrorHook( const OSAL_tpfErrorHook  NewHook,OSAL_tpfErrorHook * pOldHook );
#endif 


/************************************************************************
|function implementation (scope: module-local)
|-----------------------------------------------------------------------*/
/************************************************************************
|function implementation (scope: global)
|-----------------------------------------------------------------------*/
tU32 errno2osal(void)
{
	tU32 vl_res;
	
    switch (errno)
    {
        case 0:
            vl_res = OSAL_E_NOERROR;
			break;
        case EACCES:
            vl_res = OSAL_E_NOACCESS;
			break;
        case EAGAIN:
		//case ETIMEDOUT:
            vl_res = OSAL_E_QUEUEFULL;
			break;
        case EBADF:
        case ENOENT:
            vl_res = OSAL_E_DOESNOTEXIST;
			break;
        case EBUSY:
            vl_res = OSAL_E_BUSY;
			break;
        case EEXIST:
            vl_res = OSAL_E_ALREADYEXISTS;
			break;
        case EINVAL:
            vl_res = OSAL_E_INVALIDVALUE;
			break;
        //case EMSGSIZE:
            //vl_res = OSAL_E_MSGTOOLONG;
        case ENAMETOOLONG:
            vl_res = OSAL_E_NAMETOOLONG;
			break;
        case EMFILE:
        case ENFILE:
        case ENOMEM:
        case ENOSPC:
            vl_res = OSAL_E_NOSPACE;
			break;
        case ETIMEDOUT:
            vl_res = OSAL_E_TIMEOUT;
			break;
        case ENOSYS:
            vl_res = OSAL_E_NOTSUPPORTED;
			break;
        case EPERM:
            vl_res = OSAL_E_NOPERMISSION;
			break;
        case ECHILD:                /* improper use in osproc.c, see Release Notes */
            vl_res = OSAL_E_WRONGTHREAD;
			break;
#if 0
        case EAFNOSUPPORT:          /* improper use in ostimer.c, see Release Notes */
            vl_res = OSAL_E_UNKNOWN;
			break;
#endif
        default:
            ASSERT_ON_DEBUGGING(0);		//lint !e960	- MISRA 12.10 - use of assert code
	        vl_res = OSAL_E_UNKNOWN; /* lint */
			break;
    }
	
	return(vl_res);
}

static int osal2errno(tU32 osalerror)
{
	int vl_res;
	
    switch (osalerror)
    {
        case OSAL_E_NOERROR:
            vl_res = 0;
			break;
        case OSAL_E_NOACCESS:
            vl_res = EACCES;
			break;
        case OSAL_E_QUEUEFULL:
            vl_res = EAGAIN;
			break;
        case OSAL_E_DOESNOTEXIST:
            vl_res = ENOENT; /* could also vl_res = EBADF */
			break;
        case OSAL_E_BUSY:
            vl_res = EBUSY;
			break;
        case OSAL_E_ALREADYEXISTS:
            vl_res = EEXIST;
			break;
        case OSAL_E_INVALIDVALUE:
            vl_res = EINVAL;
			break;
        //case OSAL_E_MSGTOOLONG:
            //vl_res = EMSGSIZE;
        case OSAL_E_NAMETOOLONG:
            vl_res = ENAMETOOLONG;
			break;
        case OSAL_E_NOSPACE:
            vl_res = ENOMEM; /* could be also ENOSPC or ENFILE */
			break;
        case OSAL_E_TIMEOUT:
            vl_res = ETIMEDOUT;
			break;
        case OSAL_E_NOTSUPPORTED:
            vl_res = ENOSYS;
			break;
        case OSAL_E_NOPERMISSION:
            vl_res = EPERM;
			break;
        case OSAL_E_WRONGTHREAD:
            vl_res = ECHILD;       /* improper use in osproc.c, see Release Notes */
			break;
#if 0
        case OSAL_E_UNKNOWN:
            vl_res = EAFNOSUPPORT; /* improper use in ostimer.c, see Release Notes */
			break;
#endif
        default:
            ASSERT_ON_DEBUGGING(0);		//lint !e960	- MISRA 12.10 - use of assert code
			vl_res = ENOSYS; /* lint */
			break;
    }
	
	return vl_res;
}

#if 0
tVoid Linux_vSetErrorCode(OSAL_tThreadID tid,
                          tU32 osal_error)
{
    ASSERT_ON_DEBUGGING(tid == OSAL_C_THREAD_ID_SELF);
    OSAL_vSetErrorCode(osal_error);
    OSAL_s32CallErrorHook(osal_error);
}
#endif

/**
 *
 * @brief    OSAL_u32ErrorCode
 *
 * @details  This function returns the error code that as occurred last.
 *              The error is bound to the calling thread and filed in the
 *              rispective thread control block.
 *
 *
 * @return  return error code
 *
 */
tU32 OSAL_u32ErrorCode( void )
{
   return(errno2osal());
}

/**
 *
 * @brief    OSAL_vSetErrorCode
 *
 * @details  This function sets the error code of the calling thread.
 *              The error handling routine is not called.
 *
 * @param    u32ErrorCode error code (I)
 *
 * @return   NULL
 *
 */
tVoid OSAL_vSetErrorCode( tU32 u32ErrorCode )
{
    errno = osal2errno(u32ErrorCode);
}

/**
 *
 * @brief    OSAL_coszErrorText
 *
 * @details  This function gives for an error code the corresponding
 *              description (in English) as string.
 *
 * @param    u32Ecode error code (I)
 *
 * @return
 *        - Description of corresponding error code
 *        - OSAL_NULL otherwise
 *
 */
tCString OSAL_coszErrorText( tS32 s32Ecode )
{
   tCString szErrorString = (tCString)NULL;
#ifdef CONFIG_TRACE_CLASS_OSALCORE
   switch (s32Ecode)
   {
      case OSAL_E_NOERROR:
         szErrorString = linux_szErrorString_NO_ERROR;
         break;
      case OSAL_E_NOPERMISSION:
         /*function call not allowed*/
         szErrorString = linux_szErrorString_NOPERMISSION;
         break;
      case OSAL_E_INVALIDVALUE:
         /*wrong parameter */
         szErrorString = linux_szErrorString_INVALIDVALUE;
         break;
      case OSAL_E_NOSPACE:
         /* no more ressources(memory)*/
         szErrorString = linux_szErrorString_NOSPACE;
         break;
      case OSAL_E_BUSY:
         /* resource is in use */
         szErrorString = linux_szErrorString_BUSY;
         break;
      case OSAL_E_INTERRUPT:
         /* interrupt during wait */
         szErrorString = linux_szErrorString_INTERRUPT;
         break;
      case OSAL_E_NOACCESS:
         /* no acces rights  */
         szErrorString = linux_szErrorString_NOACCESS;
         break;
      case OSAL_E_ALREADYEXISTS:
         /* ressource already exists */
         szErrorString = linux_szErrorString_ALREADYEXISTS;
         break;
      case OSAL_E_DOESNOTEXIST:
         /* ressource doesn't exist */
         szErrorString = linux_szErrorString_DOESNOTEXIST;
         break;
      case OSAL_E_MAXFILES:
         /* to much descriptors in use */
         szErrorString = linux_szErrorString_MAXFILES;
         break;
      case OSAL_E_NOFILEDESCRIPTOR:
         /* no more resources(filedescr.) */
         szErrorString = linux_szErrorString_NOFILEDESCRIPTOR;
         break;
      case OSAL_E_NAMETOOLONG:
         /* name for file or descr. too long */
         szErrorString = linux_szErrorString_NAMETOOLONG;
         break;
      case OSAL_E_BADFILEDESCRIPTOR:
         /* not existing filedescriptor */
         szErrorString = linux_szErrorString_BADFILEDESCRIPTOR;
         break;
      //case OSAL_E_MSGTOOLONG:
         /* message is to long */
         //szErrorString = linux_szErrorString_MSGTOOLONG;
         //break;
      case OSAL_E_QUEUEFULL:
         /* message queue is full */
         szErrorString = linux_szErrorString_QUEUEFULL;
         break;
       case OSAL_E_WRONGPROCESS:
         /* process doesn't exist */
         szErrorString = linux_szErrorString_WRONGPROCESS;
         break;
      case OSAL_E_WRONGTHREAD:
         /* thread doesn't exist */
         szErrorString = linux_szErrorString_WRONGTHREAD;
         break;
      case OSAL_E_EVENTINUSE:
         /* a task is already waiting for event */
         szErrorString = linux_szErrorString_EVENTINUSE;
         break;
      case OSAL_E_WRONGFUNC:
         /* function doesn't exist */
         szErrorString = linux_szErrorString_WRONGFUNC;
         break;
      case OSAL_E_NOTINTERRUPTCALLABLE:
         /* function call is not allowed */
         szErrorString = linux_szErrorString_NOTINTERRUPTCALLABLE;
         break;
      case OSAL_E_INPROGRESS:
         /* function is not ready */
         szErrorString = linux_szErrorString_INPROGRESS;
         break;
      case OSAL_E_TIMEOUT:
         /* function abort because of timeout */
         szErrorString = linux_szErrorString_TIMEOUT;
         break;
      case OSAL_E_NOTSUPPORTED:
         /* service not supported */
         szErrorString = linux_szErrorString_NOTSUPPORTED;
         break;
      case OSAL_E_CANCELED:
         /* asynchronous order successfully aborted */
         szErrorString = linux_szErrorString_CANCELED;
         break;
      case OSAL_E_UNKNOWN:
         /* unknown error */
         szErrorString = linux_szErrorString_UNKNOWN;
         break;
	  case OSAL_ERROR:
	  	szErrorString = linux_szErrorString_GENERIC;
		break;
	  case OSAL_ERROR_UNEXPECTED:
	  	szErrorString = linux_szErrorString_UNEXPECTED;
		break;
	  case OSAL_ERROR_INVALID_PARAM:
	  	szErrorString = linux_szErrorString_INVALID_PARAM;
		break;
	  case OSAL_ERROR_DEVICE_INIT:
	  	szErrorString = linux_szErrorString_DEVICE_INI;
		break;
	  case OSAL_ERROR_DEVICE_NOT_OPEN:
	  	szErrorString = linux_szErrorString_DEVICE_NOT_OPEN;
		break;
	  case OSAL_ERROR_ACTION_FORBIDDEN:
	  	szErrorString = linux_szErrorString_ACTION_FORBIDDEN;
		break;
	  case OSAL_ERROR_CANNOT_OPEN:
	  	szErrorString = linux_szErrorString_CANNOT_OPEN;
		break;
	  case OSAL_ERROR_CHECKSUM:
	  	szErrorString = linux_szErrorString_CHECKSUM;
		break;
	  case OSAL_ERROR_FROM_SLAVE:
	  	szErrorString = linux_szErrorString_FROM_SLAVE;
		break;
	  case OSAL_ERROR_NOT_SUPPORTED:
	  	szErrorString = linux_szErrorString_NOT_SUPPORTED;
		break;
	  case OSAL_ERROR_TIMEOUT_EXPIRED:
	  	szErrorString = linux_szErrorString_TIMEOUT_EXPIRED;
		break;
	  case OSAL_ERROR_CALLBACK:
	  	szErrorString = linux_szErrorString_CALLBACK;
		break;
	  case OSAL_ERROR_NOT_IMPLEMENTED:
	  	szErrorString = linux_szErrorString_NOT_IMPLEMENTED;
		break;
      default:
         szErrorString = linux_szErrorString_LIBERROR;
         break;
   }
#endif // CONFIG_TRACE_CLASS_OSALCORE
   return szErrorString;
}

#if 0
/**
 *
 * @brief    OSAL_vErrorHook
 *
 * @details  This function sets for the calling thread a new error handling
 *              routine. This function is called exactly if in an OSAL-Function
 *              on account of an error situation the error code is set again.
 *              Through this it is possible to react centrally to possible
 *              error situations, in stead of checking each return value of
 *              a function. During calling the error code is transferred
 *              to the error handling routine.
 *              The routine is bound to the calling thread.
 *
 * @param    NewHook  new handling routine (I)
 * @param    pOldHook pointer to old handling routine (->O)
 *
 * @return   NULL
 *
 */
static tVoid OSAL_vErrorHook( const OSAL_tpfErrorHook  NewHook,
                             OSAL_tpfErrorHook * pOldHook )
{
/* not implemented yet! */
#ifndef CONFIG_APP_OSALCORE_TESTS
    ASSERT_ON_DEBUGGING(0);		//lint !e960	- MISRA 12.10 - use of assert code
#endif
}
#endif

/**
 *
 * @brief    OSAL_s32CallErrorHook
 *
 * @details  This function executes the error handling routine of the
 *              calling thread, in case it has been set earlier. It is to be
 *			    noted that it is not possible to call another error hook
 *				function recursively within an error handling routine.
 *
 * @param    u32ErrorCode error code (I)
 *
 * @return
 *           - OSAL_OK if everythin goes right
 *           - OSAL_ERROR otherwise
 */
tS32 OSAL_s32CallErrorHook( tU32 u32ErrorCode )
{
    if (u32ErrorCode != OSAL_E_NOERROR)
    {
/* not implemented yet! */
#ifndef CONFIG_APP_OSALCORE_TESTS
        ASSERT_ON_DEBUGGING(0);		//lint !e960	- MISRA 12.10 - use of assert code
#endif
    }
    
    return OSAL_OK;
}

#ifdef __cplusplus
}
#endif

/** @} */

/* End of File */
