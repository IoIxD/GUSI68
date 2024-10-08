
#include "GUSIInternal.h"
#include "GUSITimer.h"
#include "GUSIPThread.h"

#include <pthread.h>
#include <sched.h>

struct GUSIPThreadAttr
{
	size_t fStackSize;
	enum
	{
		detached = 1 << 0
	};
	int fFlags;

	static GUSIPThreadAttr sDefault;
	static pthread_attr_t sDefaultAttr;
};

GUSIPThreadAttr GUSIPThreadAttr::sDefault = {20480, 0};
pthread_attr_t GUSIPThreadAttr::sDefaultAttr = &GUSIPThreadAttr::sDefault;

struct CreateArg
{
	GUSIPThreadProc fRealProc;
	void *fRealArg;

	CreateArg(GUSIPThreadProc realProc, void *realArg)
		: fRealProc(realProc), fRealArg(realArg) {}
};

static pascal void *GUSIThreadEntryProcWrapper(CreateArg *arg)
{
	CreateArg fixedArg = *arg;

	delete arg;
	return fixedArg.fRealProc(fixedArg.fRealArg);
}

int pthread_create(
	pthread_t *thread,
	const pthread_attr_t *attr, GUSIPThreadProc proc, void *arg)
{
	if (!attr)
		attr = &GUSIPThreadAttr::sDefaultAttr;

	GUSIContext::Setup(true);
	*thread = static_cast<GUSIPThread *>(
		GUSIContextFactory::Instance()->CreateContext(
			reinterpret_cast<ThreadEntryProcPtr>(GUSIThreadEntryProcWrapper),
			new CreateArg(proc, arg),
			attr[0]->fStackSize));
	switch (thread[0]->Error())
	{
	case noErr:
		if (attr[0]->fFlags & GUSIPThreadAttr::detached)
			thread[0]->Detach();
		return 0;
	default:
		thread[0]->Liquidate();
		*thread = nil;

		return ENOMEM; // Most likely candidate for error
	}
}

int pthread_detach(pthread_t thread)
{
	thread->Detach();

	return 0;
}

int pthread_join(pthread_t thread, void **value)
{
	if (thread->Done(true))
	{
		if (value)
			*value = thread->Result();
		thread->Liquidate();

		return 0;
	}
	else
		return ESRCH;
}

int pthread_exit(void *value)
{
	DisposeThread(GUSIContext::Current()->ID(), value, false);

	return 0; // Not reached
}

int pthread_key_create(pthread_key_t *key, GUSIPThreadKeyDestructor destructor)
{
	return (*key = new GUSIPThreadKey(destructor)) ? 0 : ENOMEM;
}
int pthread_key_delete(pthread_key_t key)
{
	delete key;

	return 0;
}
void *pthread_getspecific(pthread_key_t key)
{
	return GUSIContext::CreateCurrent(true)->GetSpecific(key);
}
int pthread_setspecific(pthread_key_t key, void *value)
{
	GUSIContext::CreateCurrent(true)->SetSpecific(key, value);

	return 0;
}
int pthread_mutexattr_init(pthread_mutexattr_t *)
{
	return 0;
}

int pthread_mutexattr_destroy(pthread_mutexattr_t *)
{
	return 0;
}
int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *)
{
	*mutex = new GUSIPThreadMutex;

	return 0;
}

int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
	delete *mutex;
	*mutex = 0;

	return 0;
}
int pthread_mutex_lock(pthread_mutex_t *mutex)
{
	if (!*mutex)
		pthread_mutex_init(mutex, NULL);
	mutex[0]->push(GUSIContext::CreateCurrent(true));
	while (mutex[0]->front() != GUSIContext::Current())
	{
		GUSIContext::Yield(kGUSIBlock);
		GUSIContext::Raise();
	}

	return 0;
}
int pthread_mutex_trylock(pthread_mutex_t *mutex)
{
	if (!*mutex)
		pthread_mutex_init(mutex, NULL);
	if (mutex[0]->empty() || mutex[0]->front() == GUSIContext::CreateCurrent(true))
	{
		mutex[0]->push(GUSIContext::Current());

		return 0;
	}
	else
	{
		bool wasPolling = mutex[0]->fPolling;
		mutex[0]->fPolling = true;
		GUSIContext::Yield(wasPolling ? kGUSIYield : kGUSIPoll);
		GUSIContext::Raise();

		return EBUSY;
	}
}
int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
	if (!*mutex || mutex[0]->front() != GUSIContext::CreateCurrent(true))
		return EPERM; // We don't hold that lock
	mutex[0]->pop();
	if (GUSIContext *new_boss = mutex[0]->front())
		new_boss->Wakeup();
	if (mutex[0]->fPolling)
	{
		mutex[0]->fPolling = false;
		GUSIContext::Yield(kGUSIYield);
		GUSIContext::Raise();
	}
	return 0;
}
int pthread_condattr_init(pthread_condattr_t *)
{
	return 0;
}

int pthread_condattr_destroy(pthread_condattr_t *)
{
	return 0;
}
int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *)
{
	*cond = new GUSIPThreadCond;

	return 0;
}

int pthread_cond_destroy(pthread_cond_t *cond)
{
	pthread_cond_broadcast(cond);

	delete *cond;
	*cond = 0;

	return 0;
}
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
	if (!*mutex || mutex[0]->front() != GUSIContext::CreateCurrent(true))
		return EPERM; // We don't hold that lock
	mutex[0]->pop();
	if (GUSIContext *new_boss = mutex[0]->front())
		new_boss->Wakeup();
	if (!*cond)
		pthread_cond_init(cond, NULL);
	cond[0]->push(GUSIContext::Current());
	GUSIContext::Yield(kGUSIBlock);
	if (*cond)
		cond[0]->remove(GUSIContext::Current());
	GUSIContext::Raise();
	return pthread_mutex_lock(mutex);
}
static int do_pthread_cond_timedwait(
	pthread_cond_t *cond,
	pthread_mutex_t *mutex,
	const struct timespec *patience)
{
	GUSITimer timer;
	if (!*mutex || mutex[0]->front() != GUSIContext::CreateCurrent(true))
		return EPERM; // We don't hold that lock
	mutex[0]->pop();
	if (GUSIContext *new_boss = mutex[0]->front())
		new_boss->Wakeup();
	if (!*cond)
		pthread_cond_init(cond, NULL);
	cond[0]->push(GUSIContext::Current());
	GUSITime interval = GUSITime(*patience) - GUSITime::Now();
	GUSIContext::Current()->ClearWakeups();
	timer.MicroSleep(interval.Get(GUSITime::usecs));
	GUSIContext::Yield(kGUSIBlock);
	if (*cond)
		cond[0]->remove(GUSIContext::Current());
	int result = pthread_mutex_lock(mutex);
	return timer.Expired() ? ETIMEDOUT : result;
}

int pthread_cond_timedwait(
	pthread_cond_t *cond,
	pthread_mutex_t *mutex,
	const struct timespec *patience)
{
	int result = do_pthread_cond_timedwait(cond, mutex, patience);
	GUSIContext::Raise();
	return result;
}

int pthread_cond_signal(pthread_cond_t *cond)
{
	if (!*cond)
		return 0;

	if (GUSIContext *hey = cond[0]->front())
		hey->Wakeup();
	return 0;
}
int pthread_cond_broadcast(pthread_cond_t *cond)
{
	if (!*cond)
		return 0;

	cond[0]->Wakeup();

	return 0;
}
pthread_t pthread_self()
{
	return static_cast<GUSIPThread *>(GUSIContext::CreateCurrent(true));
}
int pthread_equal(pthread_t t1, pthread_t t2)
{
	return t1 == t2;
}
int pthread_once(pthread_once_t *once_block, GUSIPThreadOnceProc proc)
{
	if (!*once_block)
	{
		*once_block = 1;
		proc();
	}
	return 0;
}
int sched_yield()
{
	GUSIContext::Yield(kGUSIYield);
	GUSIContext::Raise();

	return 0;
}