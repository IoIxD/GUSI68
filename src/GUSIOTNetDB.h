
#ifndef _GUSIOTNetDB_
#define _GUSIOTNetDB_

#ifdef GUSI_INTERNAL
#include "GUSIOpenTransport.h"
#include "GUSINetDB.h"
#include "GUSIContext.h"


class	TInternetServices;
typedef TInternetServices*	InetSvcRef;



class GUSIOTNetDB : public GUSINetDB {
public:
	static void	Instantiate();
	bool Resolver();
	
	
virtual hostent * gethostbyname(const char * name);

virtual hostent * gethostbyaddr(const void * addr, size_t len, int type);

virtual char * inet_ntoa(in_addr inaddr);

virtual long gethostid();

private:
	GUSISpecificData<GUSIhostent, GUSIKillHostEnt>	fHost;
	
GUSIOTNetDB();

uint16_t		fEvent;
uint32_t		fCompletion;
OSStatus		fAsyncError;
InetSvcRef		fSvc;
GUSIContext *	fCreationContext;
friend pascal void GUSIOTNetDBNotify(GUSIOTNetDB *, OTEventCode, OTResult, void *);

};


#endif /* GUSI_INTERNAL */

#endif /* _GUSIOTNetDB_ */
