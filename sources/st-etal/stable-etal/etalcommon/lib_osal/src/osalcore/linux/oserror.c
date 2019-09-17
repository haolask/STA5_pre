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

#include "target_config.h"

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
static tCString linux_szErrorString_NOPERMISSION     =
         "The thread has no permission to call the function";

static tCString linux_szErrorString_INVALIDVALUE     =
         "One parameter of the function is invalid e.g.too large";

static tCString linux_szErrorString_NOSPACE          =
         "No system resources are available e.g. no free space";

static tCString linux_szErrorString_BUSY             =
         "The system can not release the resource, as this is still used";

static tCString linux_szErrorString_INTERRUPT        =
         "An Interruption has occurred whereas the thread waits";

static tCString linux_szErrorString_NOACCESS         =
         "The access rights tom the resource are invalid";

static tCString linux_szErrorString_ALREADYEXISTS    =
         "The resource exists already";

static tCString linux_szErrorString_DOESNOTEXIST     =
         "Access to the resource can not be made,as this does not exist";

static tCString linux_szErrorString_MAXFILES         =
         "The process has already too many file descriptors";

static tCString linux_szErrorString_NOFILEDESCRIPTOR =
         "In the system no resources are available";

static tCString linux_szErrorString_NAMETOOLONG      =
         "The name of the resource e.g. data file name is too long";

static tCString linux_szErrorString_BADFILEDESCRIPTOR=
         "The specified data file descriptor does not exist";

static tCString linux_szErrorString_QUEUEFULL        =
         "The message box can not accept further messages";

static tCString linux_szErrorString_WRONGPROCESS     =
         "The specified process does not exist";

static tCString linux_szErrorString_WRONGTHREAD      =
         "The specified thread does not exist";

static tCString linux_szErrorString_EVENTINUSE       =
         "A thread already waits for the evevnt";

static tCString linux_szErrorString_WRONGFUNC        =
         "The specified function does not exist";

static tCString linux_szErrorString_NOTINTERRUPTCALLABLE=
         "The function has been called by an interrupt level";

static tCString linux_szErrorString_INPROGRESS       =
         "The function is not yet completed";

static tCString linux_szErrorString_TIMEOUT          =
         "The function is aborted on account of a time out";

static tCString linux_szErrorString_NOTSUPPORTED     =
         "The device does not support the specified function";

static tCString linux_szErrorString_CANCELED         =
         "The asynchronous order has been aborted successfully";

static tCString linux_szErrorString_UNKNOWN          =
         "An unknown error has occurred";

static tCString linux_szErrorString_NO_ERROR         =
         "No error has occurred";

static tCString linux_szErrorString_LIBERROR          =
         "A Library error has occurred";

static tCString linux_szErrorString_GENERIC =
		"Generic error";

static tCString linux_szErrorString_UNEXPECTED = 
		"Unexpected";

static tCString linux_szErrorString_INVALID_PARAM = 
		"Invalid parameter";

static tCString linux_szErrorString_DEVICE_INI =
		"Device Initialization";

static tCString linux_szErrorString_DEVICE_NOT_OPEN =
		"Device Open";

static tCString linux_szErrorString_ACTION_FORBIDDEN =
		"Action forbidden";

static tCString linux_szErrorString_CANNOT_OPEN =
		"Cannot open";

static tCString linux_szErrorString_CHECKSUM =
		"Checksum error";

static tCString linux_szErrorString_FROM_SLAVE =
		"Error from slave";

static tCString linux_szErrorString_NOT_SUPPORTED =
		"Not supported";

static tCString linux_szErrorString_TIMEOUT_EXPIRED =
		"Timeout expired";

static tCString linux_szErrorString_CALLBACK =
		"Callback error";

static tCString linux_szErrorString_NOT_IMPLEMENTED =
		"Not implemented";
#endif // CONFIG_TRACE_CLASS_OSALCORE

/************************************************************************
| variable definition (scope: global)
|-----------------------------------------------------------------------*/
/************************************************************************
|function prototype (scope: module-local)
|-----------------------------------------------------------------------*/
static int osal2errno(tU32 osalerror);
#ifdef CONFIG_HOST_OS_LINUX
extern tVoid Linux_vSetErrorCode(OSAL_tThreadID tid, tU32 osal_error);
#endif

/************************************************************************
|function implementation (scope: module-local)
|-----------------------------------------------------------------------*/
/************************************************************************
|function implementation (scope: global)
|-----------------------------------------------------------------------*/
tU32 errno2osal(void)
{
    switch (errno)
    {
        case 0:
            return OSAL_E_NOERROR;
        case EACCES:
            return OSAL_E_NOACCESS;
        case EAGAIN:
		//case ETIMEDOUT:
            return OSAL_E_QUEUEFULL;
        case EBADF:
        case ENOENT:
            return OSAL_E_DOESNOTEXIST;
        case EBUSY:
            return OSAL_E_BUSY;
        case EEXIST:
            return OSAL_E_ALREADYEXISTS;
        case EINVAL:
            return OSAL_E_INVALIDVALUE;
        //case EMSGSIZE:
            //return OSAL_E_MSGTOOLONG;
        case ENAMETOOLONG:
            return OSAL_E_NAMETOOLONG;
        case EMFILE:
        case ENFILE:
        case ENOMEM:
        case ENOSPC:
            return OSAL_E_NOSPACE;
        case ETIMEDOUT:
            return OSAL_E_TIMEOUT;
        case ENOSYS:
            return OSAL_E_NOTSUPPORTED;
        case EPERM:
            return OSAL_E_NOPERMISSION;
        case ECHILD:                /* improper use in osproc.c, see Release Notes */
            return OSAL_E_WRONGTHREAD;
		case EINTR:
			return OSAL_E_INTERRUPT;
        default:
            ASSERT_ON_DEBUGGING(0);
	        return OSAL_E_UNKNOWN; /* lint */
    }
}

static int osal2errno(tU32 osalerror)
{
    switch (osalerror)
    {
        case OSAL_E_NOERROR:
            return 0;
        case OSAL_E_NOACCESS:
            return EACCES;
        case OSAL_E_QUEUEFULL:
            return EAGAIN;
        case OSAL_E_DOESNOTEXIST:
            return ENOENT; /* could also return EBADF */
        case OSAL_E_BUSY:
            return EBUSY;
        case OSAL_E_ALREADYEXISTS:
            return EEXIST;
        case OSAL_E_INVALIDVALUE:
            return EINVAL;
        //case OSAL_E_MSGTOOLONG:
            //return EMSGSIZE;
        case OSAL_E_NAMETOOLONG:
            return ENAMETOOLONG;
        case OSAL_E_NOSPACE:
            return ENOMEM; /* could be also ENOSPC or ENFILE */
        case OSAL_E_TIMEOUT:
            return ETIMEDOUT;
        case OSAL_E_NOTSUPPORTED:
            return ENOSYS;
        case OSAL_E_NOPERMISSION:
            return EPERM;
        case OSAL_E_WRONGTHREAD:
            return ECHILD;       /* improper use in osproc.c, see Release Notes */
		case OSAL_E_INTERRUPT:
			return EINTR;
        default:
            ASSERT_ON_DEBUGGING(0);
	    return ENOSYS; /* lint */
    }
}

#if CONFIG_HOST_OS_LINUX
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
/* USED ONLY BY the osalcore tests */
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
/* USED ONLY BY the osalcore tests */
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
#ifdef CONFIG_APP_OSALCORE_TESTS
		/* errors are normal during the test execution, don't panic */
#else
 #if defined(CONFIG_TRACE_CLASS_OSALCORE) && (CONFIG_TRACE_CLASS_OSALCORE >= TR_LEVEL_SYSTEM_MIN)
		/* not implemented yet! */
		//OSALUTIL_s32TracePrintf((OSAL_tIODescriptor)0, TR_LEVEL_SYSTEM_MIN, TR_CLASS_OSALCORE, "OSAL_s32CallErrorHook invoked");
 #endif
#endif
    }
    
    return OSAL_OK;
}


#ifdef __cplusplus
}
#endif

/** @} */

/* End of File */
