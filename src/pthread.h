
#ifndef _PTHREAD_H_
#define _PTHREAD_H_

#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/time.h>

__BEGIN_DECLS

// typedef struct GUSIPThread *	pthread_t;

// typedef struct GUSIPThreadAttr *pthread_attr_t;

// typedef struct GUSIPThreadKey *pthread_key_t;

// typedef char pthread_once_t;

enum
{
	PTHREAD_ONCE_INIT = 0
};

// typedef struct GUSIPThreadMutex *pthread_mutex_t;
// typedef void *pthread_mutexattr_t;

#define PTHREAD_MUTEX_INITIALIZER 0

// typedef struct GUSIPThreadCond *pthread_cond_t;
// typedef void *pthread_condattr_t;

#define PTHREAD_COND_INITIALIZER 0

int pthread_attr_init(pthread_attr_t *attr);

int pthread_attr_destroy(pthread_attr_t *attr);

int pthread_attr_getdetachstate(const pthread_attr_t *attr, int *state);
int pthread_attr_setdetachstate(pthread_attr_t *attr, int state);

int pthread_attr_getstacksize(const pthread_attr_t *attr, size_t *size);
int pthread_attr_setstacksize(pthread_attr_t *attr, size_t size);

__BEGIN_DECLS
typedef void *(*GUSIPThreadProc)(void *);
__END_DECLS

int pthread_create(
	pthread_t *thread,
	const pthread_attr_t *attr, GUSIPThreadProc proc, void *arg);

int pthread_detach(pthread_t thread);

int pthread_join(pthread_t thread, void **value);

int pthread_exit(void *value);

__BEGIN_DECLS
typedef void (*GUSIPThreadKeyDestructor)(void *);
__END_DECLS

int pthread_key_create(pthread_key_t *key, GUSIPThreadKeyDestructor destructor);

int pthread_key_delete(pthread_key_t key);

void *pthread_getspecific(pthread_key_t key);

int pthread_setspecific(pthread_key_t key, void *value);

int pthread_mutexattr_init(pthread_mutexattr_t *attr);
int pthread_mutexattr_destroy(pthread_mutexattr_t *attr);

int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *);
int pthread_mutex_destroy(pthread_mutex_t *mutex);

int pthread_mutex_lock(pthread_mutex_t *mutex);

int pthread_mutex_trylock(pthread_mutex_t *mutex);

int pthread_mutex_unlock(pthread_mutex_t *mutex);

int pthread_condattr_init(pthread_condattr_t *attr);
int pthread_condattr_destroy(pthread_condattr_t *attr);

int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *);
int pthread_cond_destroy(pthread_cond_t *cond);

int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);

int pthread_cond_timedwait(
	pthread_cond_t *cond, pthread_mutex_t *mutex,
	const struct timespec *patience);

int pthread_cond_signal(pthread_cond_t *cond);

int pthread_cond_broadcast(pthread_cond_t *cond);

pthread_t pthread_self(void);

int pthread_equal(pthread_t t1, pthread_t t2);

__BEGIN_DECLS
typedef void (*GUSIPThreadOnceProc)(void);
__END_DECLS

int pthread_once(pthread_once_t *once_block, GUSIPThreadOnceProc proc);

__END_DECLS

#endif /* _PTHREAD_H_ */
