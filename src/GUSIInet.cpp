
#include "GUSIInternal.h"
#include "GUSIOpenTransport.h"
#include "GUSIInet.h"
#include "GUSIMTInet.h"
#include "GUSIOTInet.h"

#include <sys/types.h>
#include <sys/socket.h>

GUSISocketTypeRegistry gGUSIInetFactories(AF_INET, 8);

#warning: unhandled macro "definitions[mat]"
