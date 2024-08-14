
#ifndef _GUSIMTNetDB_
#define _GUSIMTNetDB_

#ifdef GUSI_INTERNAL
#include "GUSINetDB.h"


class GUSIMTNetDB : public GUSINetDB {
public:
	static void	Instantiate();
	static bool Resolver();
	
	
virtual hostent * gethostbyname(const char * name);

virtual hostent * gethostbyaddr(const void * addr, size_t len, int type);

virtual char * inet_ntoa(in_addr inaddr);

virtual long gethostid();

private:
	GUSIMTNetDB()									{}
	GUSISpecificData<GUSIhostent,GUSIKillHostEnt>	fHost;
	static OSErr	sResolverState;
};


#endif /* GUSI_INTERNAL */

#endif /* _GUSIMTNetDB_ */
