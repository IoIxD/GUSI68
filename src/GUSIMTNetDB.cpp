
#include "GUSIInternal.h"
#include "GUSIMTNetDB.h"
#include "GUSIMTInet.h"

#include <MacTCP.h>

#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>

static pascal void
DNRDone(struct hostInfo *, GUSIContext *context)
{
	context->Wakeup();
}

#if GENERATINGCFM
RoutineDescriptor uDNRDone =
	BUILD_ROUTINE_DESCRIPTOR(uppResultProcInfo, DNRDone);
#else
#define uDNRDone DNRDone
#endif

void GUSIMTNetDB::Instantiate()
{
	if (!sInstance)
		sInstance = new GUSIMTNetDB;
}

OSErr GUSIMTNetDB::sResolverState = 1;

bool GUSIMTNetDB::Resolver()
{
	/*GUSIMTInetSocket::Driver();
	if (sResolverState == 1)
		sResolverState = OpenResolver(nil);

	return !sResolverState;*/
	return false;
}
