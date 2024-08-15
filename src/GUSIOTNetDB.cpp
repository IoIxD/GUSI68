
#include "GUSIInternal.h"
#include "GUSIOTNetDB.h"
#include "GUSIOTInet.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <OpenTptInternet.h>

GUSINetDB *GUSINetDB::sInstance;

inline uint32_t CompleteMask(OTEventCode code)
{
	return 1 << (code & 0x1F);
}

pascal void GUSIOTNetDBNotify(
	GUSIOTNetDB *netdb, OTEventCode code, OTResult result, void *cookie)
{
	GUSI_MESSAGE(("GUSIOTNetDBNotify %08x %d\n", code, result));
	GUSIContext *context = netdb->fCreationContext;

	switch (code & 0x7F000000L)
	{
	case 0:
		netdb->fEvent |= code;
		result = 0;
		break;
	case kPRIVATEEVENT:
	case kCOMPLETEEVENT:
		if (!(code & 0x00FFFFE0))
			netdb->fCompletion |= CompleteMask(code);
		switch (code)
		{
		case T_OPENCOMPLETE:
			netdb->fSvc = static_cast<InetSvcRef>(cookie);
			break;
		case T_DNRSTRINGTOADDRCOMPLETE:
		case T_DNRADDRTONAMECOMPLETE:
			context = static_cast<GUSIContext **>(cookie)[-1];
			break;
		}
		break;
	default:
		switch (code)
		{
		case kOTProviderWillClose:
		case kOTProviderIsClosed:
			netdb->fCreationContext = nil; // Close & reopen
			break;
		default:
			result = 0;
			break;
		}
		break;
	}
	if (result)
		netdb->fAsyncError = result;
	context->Wakeup();
}

void GUSIOTNetDB::Instantiate()
{
	if (!sInstance)
		sInstance = new GUSIOTNetDB;
}

bool GUSIOTNetDB::Resolver()
{
	if (!fCreationContext)
	{
		if (fSvc)
		{
			OTCloseProvider(fSvc);
			fSvc = nil;
		}
		fCreationContext = GUSIContext::Current();
		if (!GUSIOTFactory::Initialize())
			return false;
		fAsyncError = 0;
		OSStatus syncError = OTAsyncOpenInternetServices(
			kDefaultInternetServicesPath,
			0,
			reinterpret_cast<OTNotifyProcPtr>(GUSIOTNetDBNotify),
			this);
		if (!syncError)
			while (!fAsyncError && !(fCompletion & CompleteMask(T_OPENCOMPLETE)))
				GUSIContext::Yield(kGUSIBlock);
		GUSIContext::Raise();
	}
	return fSvc != 0;
}

static void CopyHost(InetHostInfo &otHost, GUSIhostent &unixHost)
{
	size_t len = strlen(otHost.name);
	len = (len + 4) & ~3;

	unixHost.Alloc(len + kMaxHostAddrs * 4);
	strcpy(unixHost.h_name, otHost.name);
	unixHost.h_aliases[0] = NULL; // Aliases not supported
	unixHost.h_addrtype = AF_INET;
	unixHost.h_length = 4;

	int addrs = 0;
	for (int i = 0; i < kMaxHostAddrs && otHost.addrs[addrs] != 0; ++i, ++addrs)
	{
		unixHost.h_addr_list[addrs] = unixHost.fName + len;
		len += 4;
		memcpy(unixHost.h_addr_list[addrs], &otHost.addrs[addrs], 4);
	}
	unixHost.h_addr_list[addrs] = NULL;
}
