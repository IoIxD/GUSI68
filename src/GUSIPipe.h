
#ifndef _GUSIPipe_
#define _GUSIPipe_

#ifdef GUSI_INTERNAL

#include "GUSISocket.h"
#include "GUSIFactory.h"

class GUSIPipeFactory : public GUSISocketFactory
{
public:
	static GUSISocketFactory *Instance();
	virtual GUSISocket *socket(int domain, int type, int protocol);
	virtual int socketpair(int domain, int type, int protocol, GUSISocket *s[2]);

private:
	GUSIPipeFactory() {}
	static GUSISocketFactory *sInstance;
};

inline GUSISocketFactory *GUSIPipeFactory::Instance()
{
	if (!sInstance)
		sInstance = new GUSIPipeFactory;
	return sInstance;
}

#endif /* GUSI_INTERNAL */

#endif /* _GUSIPipe_ */
