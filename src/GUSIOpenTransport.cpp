
#define GUSI_MESSAGE_LEVEL 1

#include "GUSIInternal.h"
#include "GUSIOpenTransport.h"
#include "GUSIDiag.h"
#include "GUSITimer.h"

#include <stdlib.h>
#include <errno.h>

#include <algorithm>
#include <memory>
#include <OpenTransport.h>

GUSI_USING_STD_NAMESPACE

inline uint32_t CompleteMask(OTEventCode code)
{
	return 1 << (code & 0x1F);
}
bool GUSIOTFactory::sOK = false;

bool GUSIOTFactory::Initialize()
{
	if (!sOK)
		sOK = !InitOpenTransport();
	return sOK;
}
GUSISocket *GUSIOTStreamFactory::socket(int domain, int type, int protocol)
{
	GUSIOTStrategy *strategy = Strategy(domain, type, protocol);
	if (Initialize() && strategy)
		return new GUSIOTStreamSocket(strategy);
	else
		return static_cast<GUSISocket *>(0);
}

GUSISocket *GUSIOTDatagramFactory::socket(int domain, int type, int protocol)
{
	GUSIOTStrategy *strategy = Strategy(domain, type, protocol);
	if (Initialize() && strategy)
		return new GUSIOTDatagramSocket(strategy);
	else
		return static_cast<GUSISocket *>(0);
}

int GUSIOTStrategy::CopyAddress(const TNetbuf &from, TNetbuf &to)
{
	memcpy(to.buf, from.buf, to.len = from.len);

	return 0;
}

GUSIOTSocket::GUSIOTSocket(GUSIOTStrategy *strategy)
{

	fNewEvent = 0;
	fEvent = 0;
	fNewCompletion = 0;
	fCompletion = 0;

	/*SetAsyncMacError(
		OTAsyncOpenEndpoint(
			fStrategy->CreateConfiguration(),
			0, fStrategy->EndpointInfo(),
			reinterpret_cast<OTNotifyProcPtr>(GUSIOTNotify),
			this));*/
	AddContext();
	MopupEvents();
	while (!fAsyncError && !(fCompletion & CompleteMask(T_OPENCOMPLETE)))
	{
		GUSIContext::Yield(kGUSIBlock);
		MopupEvents();
	}
	RemoveContext();
	GUSIContext::Raise();
	if (!fEndpoint)
		GUSISetPosixError(GetAsyncError());
}

int GUSIOTSocket::BindToAddress(GUSIOTTBind *addr)
{
	// TODO: "Use of deleted constructor 'GUSIOTTBind'". Find out why!
	return -1;

	/*fSockName = new (fEndpoint) GUSIOTTBind;
	if (!fSockName)
		return GUSISetPosixError(ENOMEM);
	fCompletion &= ~CompleteMask(T_BINDCOMPLETE);
	fAsyncError = 0;
	SetAsyncMacError(OTBind(fEndpoint, addr, fSockName));
	AddContext();
	MopupEvents();
	while (!fAsyncError && !(fCompletion & CompleteMask(T_BINDCOMPLETE)))
	{
		GUSIContext::Yield(kGUSIBlock);
		MopupEvents();
	}
	RemoveContext();
	GUSIContext::Raise();
	if (GUSISetPosixError(GetAsyncError()))
	{
		// delete fSockName;
		fSockName = nil;

		return -1;
	}
	else
		return 0;*/
}

GUSIOTStreamSocket::GUSIOTStreamSocket(GUSIOTStrategy *strategy)
	: GUSIOTSocket(strategy)
{

	fNextListener = nil;
}

GUSIOTStreamSocket::~GUSIOTStreamSocket()
{
	delete fPeerName;
}

bool GUSIOTStreamSocket::Close(UInt32 now)
{

	char discard[256];
	OTFlags otflags;
	while (OTRcv(fEndpoint, discard, 256, &otflags) >= 0)
		;
	MopupEvents();

	if (now < fDeadline && OTGetEndpointState(fEndpoint) > T_IDLE)
	{
		return false;
	}
	else
	{
		GUSIOTSocket::close();

		return true;
	}
}

GUSIOTDatagramSocket::GUSIOTDatagramSocket(GUSIOTStrategy *strategy)
	: GUSIOTSocket(strategy)
{
	fPeerName = nil;
}
