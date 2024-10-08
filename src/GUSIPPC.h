
#ifndef _GUSIPPC_
#define _GUSIPPC_

#ifdef GUSI_INTERNAL

#include "GUSISocket.h"
#include "GUSIFactory.h"
#include <sys/ppc.h>

class GUSIPPCFactory : public GUSISocketFactory
{
public:
	static GUSISocketFactory *Instance();
	virtual GUSISocket *socket(int domain, int type, int protocol);

private:
	GUSIPPCFactory() {}
	static GUSISocketFactory *sInstance;
};

inline GUSISocketFactory *GUSIPPCFactory::Instance()
{
	if (!sInstance)
		sInstance = new GUSIPPCFactory;
	return sInstance;
}
#endif /* GUSI_INTERNAL */

#endif /* _GUSIPPC_ */
