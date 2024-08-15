#ifndef _GUSI_BASICS_
#define _GUSI_BASICS_

#define GUSI_MESSAGE_LEVEL 5

#include "GUSIInternal.h"
#include "GUSIBasics.h"
#include "GUSIDiag.h"

#include <errno.h>
#include <netdb.h>
#include <stdio.h>

#include <MacTypes.h>
#include <Events.h>
#include <Files.h>
#include <OSUtils.h>
#include <EPPC.h>
#include <LowMem.h>
#include <AppleEvents.h>
#include <Quickdraw.h>
#include <MacTCP.h>
#include <OpenTransport.h>

__BEGIN_DECLS

const short kGUSIDefaultEventMask = mDownMask + highLevelEventMask;
short gGUSIEventMask = kGUSIDefaultEventMask;
GUSIEventFn gGUSIEventHook[16];

__END_DECLS
void GUSISetHook(OSType code, GUSIHook hook)
{
	switch (code)
	{

	case GUSI_SpinHook:
		gGUSISpinHook = (GUSISpinFn)hook;
		break;

	default:
		GUSI_ASSERT_CLIENT(0,
						   ("Illegal code %lx ('%c%c%c%c') passed to GUSISetHook\n",
							code,
							(code >> 24) & 0xff,
							(code >> 16) & 0xff,
							(code >> 8) & 0xff,
							(code) & 0xff));
		break;
	}
}
GUSIHook GUSIGetHook(OSType code)
{
	switch (code)
	{

	case GUSI_SpinHook:
		return (GUSIHook)gGUSISpinHook;

	case GUSI_ExecHook:
		return (GUSIHook)gGUSIExecHook;

	case GUSI_EventHook + nullEvent:
	case GUSI_EventHook + mouseDown:
	case GUSI_EventHook + mouseUp:
	case GUSI_EventHook + keyDown:
	case GUSI_EventHook + keyUp:
	case GUSI_EventHook + autoKey:
	case GUSI_EventHook + updateEvt:
	case GUSI_EventHook + diskEvt:
	case GUSI_EventHook + activateEvt:
	case GUSI_EventHook + osEvt:
		return (GUSIHook)gGUSIEventHook[code - GUSI_EventHook];
	case GUSI_EventHook + kHighLevelEvent:
		return (GUSIHook)((gGUSIEventMask & highLevelEventMask) ? 0 : -1);

	default:
		GUSI_ASSERT_CLIENT(0,
						   ("Illegal code %lx ('%c%c%c%c') passed to GUSIGetHook\n",
							code,
							(code >> 24) & 0xff,
							(code >> 16) & 0xff,
							(code >> 8) & 0xff,
							(code) & 0xff));
		return (GUSIHook)nil;
	}
}

int GUSISetPosixError(int error)
{
	if (error > 0)
	{
		errno = error;

		return -1;
	}
	else if (error < 0)
	{
		errno = EINVAL;

		return -1;
	}
	else
		return 0;
}

inline int GUSIErrorMapping(OSErr error, int posixError)
{
	GUSI_MESSAGE(("Error MacOS %d -> POSIX %d\n", error, posixError));
	return posixError;
}

int GUSIMapMacError(OSErr error)
{
	switch (error)
	{
	case noErr:
		return 0;

	case opWrErr:
	case wrPermErr:
		return GUSIErrorMapping(error, EPERM);
	case bdNamErr:
		return GUSIErrorMapping(error, ENAMETOOLONG);
	case afpObjectTypeErr:
		return GUSIErrorMapping(error, ENOTDIR);
	case fnfErr:
	case nsvErr:
	case dirNFErr:
		return GUSIErrorMapping(error, ENOENT);
	case dupFNErr:
		return GUSIErrorMapping(error, EEXIST);
	case dirFulErr:
	case dskFulErr:
		return GUSIErrorMapping(error, ENOSPC);
	case fBsyErr:
		return GUSIErrorMapping(error, EBUSY);
	case tmfoErr:
		return GUSIErrorMapping(error, ENFILE);
	case fLckdErr:
	case permErr:
	case afpAccessDenied:
	case kOTAccessErr: /* -3152 Missing access permission					*/
		return GUSIErrorMapping(error, EACCES);
	case wPrErr:
	case vLckdErr:
		return GUSIErrorMapping(error, EROFS);
	case badMovErr:
		return GUSIErrorMapping(error, EINVAL);
	case diffVolErr:
		return GUSIErrorMapping(error, EXDEV);

	case openFailed:
		return GUSIErrorMapping(error, ECONNREFUSED);
	case duplicateSocket:
		return GUSIErrorMapping(error, EADDRINUSE);
	case connectionTerminated:
		return GUSIErrorMapping(error, ECONNREFUSED);
	case commandTimeout:
		return GUSIErrorMapping(error, ETIMEDOUT);
	case ipBadLapErr:
	case ipBadCnfgErr:
	case ipNoCnfgErr:
	case ipLoadErr:
		return GUSIErrorMapping(error, ENETDOWN);

	case kOTLookErr: /* -3158 An event occurred - call Look() 			*/
		return GUSIErrorMapping(error, ELOOK);
	case kOTAddressBusyErr: /* -3172 Address requested is already in use		*/
		return GUSIErrorMapping(error, EADDRINUSE);
	case kOTNoAddressErr: /* -3154 No address was specified					*/
	case kOTOutStateErr:  /* -3155 Call issued in wrong state					*/
		return GUSIErrorMapping(error, ENOTCONN);
	case kOTFlowErr: /* -3161 Provider is flow-controlled				*/
		return GUSIErrorMapping(error, EWOULDBLOCK);
	case kOTNotSupportedErr: /* -3167 Command is not supported					*/
		return GUSIErrorMapping(error, EOPNOTSUPP);
	case kOTCanceledErr: /* -3180 The command was cancelled					*/
		return GUSIErrorMapping(error, ECANCELED);
	case kEAGAINErr: /*  Seems to be returned for refused connections	*/
		return GUSIErrorMapping(error, ECONNREFUSED);

#define MAP_OT_POSIX_ERROR(err) \
	case k##err##Err:           \
		return err

		MAP_OT_POSIX_ERROR(EPERM);			 /*  Permission denied					*/
		MAP_OT_POSIX_ERROR(ENOENT);			 /*  No such file or directory			*/
		MAP_OT_POSIX_ERROR(EINTR);			 /*  Interrupted system service			*/
		MAP_OT_POSIX_ERROR(EIO);			 /*  I/O error							*/
		MAP_OT_POSIX_ERROR(ENXIO);			 /*  No such device or address			*/
		MAP_OT_POSIX_ERROR(EBADF);			 /*  Bad file number						*/
		MAP_OT_POSIX_ERROR(ENOMEM);			 /*  Not enough space					*/
		MAP_OT_POSIX_ERROR(EACCES);			 /*  Permission denied					*/
		MAP_OT_POSIX_ERROR(EFAULT);			 /*  Bad address							*/
		MAP_OT_POSIX_ERROR(EBUSY);			 /*  Device or resource busy				*/
		MAP_OT_POSIX_ERROR(EEXIST);			 /*  File exists							*/
		MAP_OT_POSIX_ERROR(ENODEV);			 /*  No such device						*/
		MAP_OT_POSIX_ERROR(EINVAL);			 /*  Invalid argument					*/
		MAP_OT_POSIX_ERROR(ENOTTY);			 /*  Not a character device				*/
		MAP_OT_POSIX_ERROR(EPIPE);			 /*  Broken pipe							*/
		MAP_OT_POSIX_ERROR(ERANGE);			 /*  Message size too large for STREAM	*/
		MAP_OT_POSIX_ERROR(EWOULDBLOCK);	 /* 										*/
		MAP_OT_POSIX_ERROR(EALREADY);		 /* 										*/
		MAP_OT_POSIX_ERROR(ENOTSOCK);		 /*  Socket operation on non-socket		*/
		MAP_OT_POSIX_ERROR(EDESTADDRREQ);	 /*  Destination address required		*/
		MAP_OT_POSIX_ERROR(EMSGSIZE);		 /*  Message too long					*/
		MAP_OT_POSIX_ERROR(EPROTOTYPE);		 /*  Protocol wrong type for socket		*/
		MAP_OT_POSIX_ERROR(ENOPROTOOPT);	 /*  Protocol not available				*/
		MAP_OT_POSIX_ERROR(EPROTONOSUPPORT); /*  Protocol not supported				*/
		MAP_OT_POSIX_ERROR(ESOCKTNOSUPPORT); /*  Socket type not supported			*/
		MAP_OT_POSIX_ERROR(EOPNOTSUPP);		 /*  Operation not supported on socket	*/
		MAP_OT_POSIX_ERROR(EADDRINUSE);		 /*  Address already in use				*/
		MAP_OT_POSIX_ERROR(EADDRNOTAVAIL);	 /*  Can't assign requested address		*/
		MAP_OT_POSIX_ERROR(ENETDOWN);		 /*  Network is down						*/
		MAP_OT_POSIX_ERROR(ENETUNREACH);	 /*  Network is unreachable				*/
		MAP_OT_POSIX_ERROR(ENETRESET);		 /*  Network dropped connection on reset	*/
		MAP_OT_POSIX_ERROR(ECONNABORTED);	 /*  Software caused connection abort	*/
		MAP_OT_POSIX_ERROR(ECONNRESET);		 /*  Connection reset by peer			*/
		MAP_OT_POSIX_ERROR(ENOBUFS);		 /*  No buffer space available			*/
		MAP_OT_POSIX_ERROR(EISCONN);		 /*  Socket is already connected			*/
		MAP_OT_POSIX_ERROR(ENOTCONN);		 /*  Socket is not connected				*/
		MAP_OT_POSIX_ERROR(ESHUTDOWN);		 /*  Can't send after socket shutdown	*/
		MAP_OT_POSIX_ERROR(ETOOMANYREFS);	 /*  Too many references: can't splice	*/
		MAP_OT_POSIX_ERROR(ETIMEDOUT);		 /*  Connection timed out				*/
		MAP_OT_POSIX_ERROR(ECONNREFUSED);	 /*  Connection refused					*/
		MAP_OT_POSIX_ERROR(EHOSTDOWN);		 /*  Host is down						*/
		MAP_OT_POSIX_ERROR(EHOSTUNREACH);	 /*  No route to host					*/
		MAP_OT_POSIX_ERROR(EINPROGRESS);	 /* 										*/
		MAP_OT_POSIX_ERROR(ESRCH);			 /* 										*/

	case destPortErr: /* -906 Port does not exist at destination*/
		return GUSIErrorMapping(error, EADDRNOTAVAIL);
	case portNameExistsErr: /* -910 port is already open (perhaps in another app)*/
		return GUSIErrorMapping(error, EISCONN);
	case userRejectErr: /* -912 Destination rejected the session request*/
		return GUSIErrorMapping(error, ECONNREFUSED);
	case noResponseErr: /* -916 unable to contact destination*/
		return GUSIErrorMapping(error, ENETUNREACH);

	case memFullErr:
		return GUSIErrorMapping(error, ENOMEM);
	default:
		return GUSIErrorMapping(error, EINVAL);
	}
}

int GUSISetMacError(OSErr error)
{
	return GUSISetPosixError(GUSIMapMacError(error));
}

int h_errno;

int GUSISetHostError(int error)
{
	if (error)
	{
		h_errno = error;

		return -1;
	}
	else
		return 0;
}

int GUSISetMacHostError(OSErr error)
{
	switch (error)
	{
	case noErr:
		return 0;
	case nameSyntaxErr:
	case noNameServer:
	case authNameErr:
		h_errno = HOST_NOT_FOUND;
		break;
	case noResultProc:
	case dnrErr:
	default:
		h_errno = NO_RECOVERY;
		break;
	case noAnsErr:
	case outOfMemory:
		h_errno = TRY_AGAIN;
		break;
	}
	return -1;
}

void GUSIHandleNextEvent(long sleepTime)
{
	EventRecord event;

	if (WaitNextEvent(gGUSIEventMask | 1, &event, sleepTime, nil))
		switch (event.what)
		{
		case mouseDown:
			if (!gGUSIEventHook[mouseDown])
			{
				WindowPtr win;
				if (FindWindow(event.where, &win) == inSysWindow)
					SystemClick(&event, win);
				return;
			}
			break;
		case kHighLevelEvent:
			AEProcessAppleEvent(&event); // Ignore errors

			return;
		}

	if (gGUSIEventHook[event.what])
		gGUSIEventHook[event.what](&event);
}

#endif