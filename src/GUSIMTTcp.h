
#ifndef _GUSIMTTcp_
#define _GUSIMTTcp_

#ifdef GUSI_SOURCE

#include <sys/cdefs.h>

__BEGIN_DECLS

void GUSIwithMTTcpSockets();

__END_DECLS

#ifdef GUSI_INTERNAL

#include "GUSIFactory.h"

class GUSIMTTcpFactory : public GUSISocketFactory
{
public:
    static GUSISocketFactory *Instance();
    virtual GUSISocket *socket(int domain, int type, int protocol);

private:
    GUSIMTTcpFactory() {}
    static GUSISocketFactory *instance;
};
inline GUSISocketFactory *GUSIMTTcpFactory::Instance()
{
    if (!instance)
        instance = new GUSIMTTcpFactory;
    return instance;
}

#endif /* GUSI_INTERNAL */

#endif /* GUSI_SOURCE */

#endif /* _GUSIMTTcp_ */
