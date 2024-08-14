
#ifndef _GUSIMTUdp_
#define _GUSIMTUdp_

#ifdef GUSI_SOURCE

#include <sys/cdefs.h>

__BEGIN_DECLS

void GUSIwithMTUdpSockets();

__END_DECLS

#ifdef GUSI_INTERNAL

#include "GUSIFactory.h"

class GUSIMTUdpFactory : public GUSISocketFactory
{
public:
    static GUSISocketFactory *Instance();
    virtual GUSISocket *socket(int domain, int type, int protocol);

private:
    GUSIMTUdpFactory() {}
    static GUSISocketFactory *instance;
};
inline GUSISocketFactory *GUSIMTUdpFactory::Instance()
{
    if (!instance)
        instance = new GUSIMTUdpFactory;
    return instance;
}
#endif /* GUSI_INTERNAL */

#endif /* GUSI_SOURCE */

#endif /* _GUSIMTUdp_ */
