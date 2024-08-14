
#ifndef _GUSIPThread_
#define _GUSIPThread_

#include "GUSISpecific.h"
#include "GUSIContext.h"
#include "GUSIContextQueue.h"

#include <pthread.h>

struct GUSIPThread : public GUSIContext
{
private:
    GUSIPThread() : GUSIContext(0) {} // Never called
};
struct GUSIPThreadKey : public GUSISpecific
{
    GUSIPThreadKey(GUSIPThreadKeyDestructor destructor) : GUSISpecific(destructor) {}
};
struct GUSIPThreadMutex : public GUSIContextQueue
{
    bool fPolling;

    GUSIPThreadMutex() : fPolling(false) {}
};
struct GUSIPThreadCond : public GUSIContextQueue
{
};

#endif /* _GUSIPThread_ */
