
#ifndef _GUSIBasics_
#define _GUSIBasics_

#ifdef GUSI_SOURCE

#include <errno.h>
#include <sys/cdefs.h>
#include <stdarg.h>

#include <ConditionalMacros.h>
#include <Files.h>

#ifdef __MWERKS__
#define GUSI_COMPILER_HAS_NAMESPACE
#endif

#ifdef GUSI_COMPILER_HAS_NAMESPACE
#define GUSI_USING_STD_NAMESPACE \
                                 \
	using namespace std::rel_ops;
#else
#define GUSI_USING_STD_NAMESPACE
#endif

#if GENERATINGCFM
#define GUSI_COMPLETION_PROC_A0(proc, type) \
	pascal void (*proc##Entry)(type * param) = proc;
#define GUSI_COMPLETION_PROC_A1(proc, type) \
	pascal void (*proc##Entry)(type * param) = proc;
#elif defined(__MWERKS__)
#define GUSI_COMPLETION_PROC_A0(proc, type) \
	static pascal void proc##Entry(type *param : __A0) { proc(param); }
#define GUSI_COMPLETION_PROC_A1(proc, type) \
	static pascal void proc##Entry(type *param : __A1) { proc(param); }
#else
void *GUSIGetA0() ONEWORDINLINE(0x2008);
void *GUSIGetA1() ONEWORDINLINE(0x2009);
#define GUSI_COMPLETION_PROC_A0(proc, type)          \
	static pascal void proc##Entry()                 \
	{                                                \
		proc(reinterpret_cast<type *>(GUSIGetA0())); \
	}
#define GUSI_COMPLETION_PROC_A1(proc, type)          \
	static pascal void proc##Entry()                 \
	{                                                \
		proc(reinterpret_cast<type *>(GUSIGetA1())); \
	}
#endif

#if defined(__SC__)
#define mutable
#define GUSI_MUTABLE(class, field) const_cast<class *>(this)->field
#else
#define GUSI_MUTABLE(class, field) field
#endif

#if defined(__SC__)
#define for \
	if (0)  \
		;   \
	else for
#endif

#if defined(__SC__) || defined(__MRC__)
#define GUSI_NEEDS_QD QDGlobals qd;
#else
#define GUSI_NEEDS_QD
#endif

typedef unsigned long OSType;
typedef void (*GUSIHook)(void);
void GUSISetHook(OSType code, GUSIHook hook);
GUSIHook GUSIGetHook(OSType code);

typedef bool (*GUSISpinFn)(bool wait);
#define GUSI_SpinHook 'spin'

typedef bool (*GUSIExecFn)(const FSSpec *file);
#define GUSI_ExecHook 'exec'

struct EventRecord;
typedef void (*GUSIEventFn)(EventRecord *ev);
#define GUSI_EventHook 'evnt'

#ifdef GUSI_INTERNAL
extern GUSISpinFn gGUSISpinHook;
extern GUSIExecFn gGUSIExecHook;
#endif /* GUSI_INTERNAL */

typedef short OSErr;

int GUSISetPosixError(int error);
int GUSISetMacError(OSErr error);
int GUSIMapMacError(OSErr error);
int GUSISetHostError(int error);
int GUSISetMacHostError(OSErr error);

class GUSIErrorSaver
{
public:
	GUSIErrorSaver()
	{
		fSavedErrno = errno;
		errno = 0;
	}
	~GUSIErrorSaver()
	{
		if (!errno)
			errno = fSavedErrno;
	}

private:
	int fSavedErrno;
};

void GUSIHandleNextEvent(long sleepTime);

int GUSI_vsprintf(char *s, const char *format, va_list args);
int GUSI_sprintf(char *s, const char *format, ...);

#endif /* GUSI_SOURCE */

#endif /* _GUSIBasics_ */
