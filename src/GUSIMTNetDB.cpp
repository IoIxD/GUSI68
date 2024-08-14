
#include "GUSIInternal.h"
#include "GUSIMTNetDB.h"
#include "GUSIMTInet.h"
#include "GUSIContext.h"

#include <MacTCP.h>

#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>


#ifdef GUSI_COMPILER_HAS_NAMESPACE
namespace {
#include <AddressXlation.h>

#include "dnr.c"
}
#else
#include "dnr.c"
#endif

static pascal void DNRDone(struct hostInfo *, GUSIContext * context)
{
	context->Wakeup();
}

#if GENERATINGCFM
RoutineDescriptor	uDNRDone = 
		BUILD_ROUTINE_DESCRIPTOR(uppResultProcInfo, DNRDone);
#else
#define uDNRDone DNRDone
#endif


void GUSIMTNetDB::Instantiate()
{
	if (!sInstance)
		sInstance = new GUSIMTNetDB;
}

OSErr 	GUSIMTNetDB::sResolverState	= 1;

bool GUSIMTNetDB::Resolver()
{
	GUSIMTInetSocket::Driver();
	if (sResolverState == 1)
		sResolverState = OpenResolver(nil);
	
	return !sResolverState;
}

static void CopyHost(hostInfo & macHost, GUSIhostent & unixHost)
{
	/* for some reason there is a dot at the end of the name */
	size_t len = strlen(macHost.cname);
	if (macHost.cname[len-1] == '.')
		macHost.cname[--len] = 0;
	len = (len+4) & ~3;
	
	unixHost.Alloc(len+NUM_ALT_ADDRS*4);
	strcpy(unixHost.h_name, macHost.cname);
	unixHost.h_aliases[0] 	= NULL;			// Aliases not supported
	unixHost.h_addrtype		= AF_INET;	
	unixHost.h_length		= 4;
	
	int addrs = 0;
	for (int i=0; i<NUM_ALT_ADDRS && macHost.addr[addrs]!=0; ++i, ++addrs) {
		unixHost.h_addr_list[addrs] = unixHost.fName + len;
		len += 4;
		memcpy(unixHost.h_addr_list[addrs], &macHost.addr[addrs], 4);
	}
	unixHost.h_addr_list[addrs] = NULL;
}

