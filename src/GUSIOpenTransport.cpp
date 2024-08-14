
#define GUSI_MESSAGE_LEVEL 1

#include "GUSIInternal.h"
#include "GUSIOpenTransport.h"
#include "GUSIDiag.h"
#include "GUSITimer.h"

#include <stdlib.h>
#include <errno.h>

#include <algorithm>
#include <memory>

GUSI_USING_STD_NAMESPACE

#warning: unhandled macro "definitions[mat]"
#warning: unhandled macro "definitions[mat]"
#warning: unhandled macro "definitions[mat]"
#warning: unhandled macro "definitions[mat]"

int GUSIOTStrategy::CopyAddress(const TNetbuf & from, TNetbuf & to)
{
	memcpy(to.buf, from.buf, to.len = from.len);
	
	return 0;
}


GUSIOTSocket::GUSIOTSocket(GUSIOTStrategy * strategy)
{
	
fNewEvent 		= 0;
fEvent 			= 0;
fNewCompletion	= 0;
fCompletion		= 0;

	SetAsyncMacError(
		OTAsyncOpenEndpoint(
			fStrategy->CreateConfiguration(), 
			0, fStrategy->EndpointInfo(), 
			reinterpret_cast<OTNotifyProcPtr>(GUSIOTNotify), 
			this)); 
	AddContext();
	MopupEvents();
	while (!fAsyncError && !(fCompletion & CompleteMask(T_OPENCOMPLETE))) {
		GUSIContext::Yield(kGUSIBlock);
		MopupEvents();
	}
	RemoveContext();
	GUSIContext::Raise();
	if (!fEndpoint)
		GUSISetPosixError(GetAsyncError());
}

int GUSIOTSocket::BindToAddress(GUSIOTTBind * addr)
{
	fSockName = new (fEndpoint) GUSIOTTBind;
	if (!fSockName)
		return GUSISetPosixError(ENOMEM);
	fCompletion	   &= ~CompleteMask(T_BINDCOMPLETE);
	fAsyncError		= 0;
	SetAsyncMacError(OTBind(fEndpoint, addr, fSockName));
	AddContext();
	MopupEvents();
	while (!fAsyncError && !(fCompletion & CompleteMask(T_BINDCOMPLETE))) {
		GUSIContext::Yield(kGUSIBlock);
		MopupEvents();
	}
	RemoveContext();
	GUSIContext::Raise();
	if (GUSISetPosixError(GetAsyncError())) {
		delete fSockName;
		fSockName = nil;
		
		return -1;
	} else
		return 0;
}


GUSIOTStreamSocket::GUSIOTStreamSocket(GUSIOTStrategy * strategy)
	: GUSIOTSocket(strategy)
{
	
fNextListener	= nil;

}

GUSIOTStreamSocket::~GUSIOTStreamSocket()
{
	delete fPeerName;
}

bool GUSIOTStreamSocket::Close(UInt32 now)
{
	
char 	discard[256];
OTFlags	otflags;
while (OTRcv(fEndpoint, discard, 256, &otflags) >= 0)
	;
MopupEvents();

	if (now < fDeadline && OTGetEndpointState(fEndpoint) > T_IDLE) {
		return false;
	} else {
		GUSIOTSocket::close();
		
		return true;
	}
}


GUSIOTDatagramSocket::GUSIOTDatagramSocket(GUSIOTStrategy * strategy)
	: GUSIOTSocket(strategy)
{
	#warning: unhandled macro "definitions[mat]"
}
