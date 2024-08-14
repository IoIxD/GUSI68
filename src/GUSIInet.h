
#ifndef _GUSIInet_
#define _GUSIInet_

#ifdef GUSI_SOURCE

#include <sys/cdefs.h>

__BEGIN_DECLS

void GUSIwithInetSockets();

__END_DECLS

#ifdef GUSI_INTERNAL

#include "GUSIFactory.h"

extern GUSISocketTypeRegistry gGUSIInetFactories;

#endif /* GUSI_INTERNAL */

#endif /* GUSI_SOURCE */

#endif /* _GUSIInet_ */
