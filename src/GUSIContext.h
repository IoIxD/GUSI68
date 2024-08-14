
#ifndef _GUSIContext_
#define _GUSIContext_

#include <errno.h>
#include <sys/cdefs.h>
#include <sys/signal.h>

#include <MacTypes.h>
#include <Threads.h>

__BEGIN_DECLS
OSErr GUSINewThread(
	ThreadStyle threadStyle, ThreadEntryProcPtr threadEntry, void *threadParam,
	Size stackSize, ThreadOptions options,
	void **threadResult, ThreadID *threadMade);
OSErr GUSISetThreadSwitcher(ThreadID thread,
							ThreadSwitchProcPtr threadSwitcher, void *switchProcParam, Boolean inOrOut);
OSErr GUSISetThreadTerminator(ThreadID thread,
							  ThreadTerminationProcPtr threadTerminator, void *terminationProcParam);

__END_DECLS

#ifndef GUSI_SOURCE

typedef struct GUSIContext GUSIContext;

#else

#include "GUSISpecific.h"
#include "GUSIBasics.h"
#include "GUSIContextQueue.h"

#include <Files.h>
#include <Processes.h>
#include <OSUtils.h>

class GUSISocket;
class GUSIContext;
class GUSIProcess;
class GUSISigProcess;
class GUSISigContext;
class GUSITimer;

#include <ConditionalMacros.h>

#if PRAGMA_STRUCT_ALIGN
#pragma options align = native
#endif
#include "GUSISignal.h"

class GUSIThreadManagerProxy
{
public:
	virtual OSErr NewThread(
		ThreadStyle threadStyle, ThreadEntryProcPtr threadEntry, void *threadParam,
		Size stackSize, ThreadOptions options, void **threadResult, ThreadID *threadMade);
	virtual OSErr SetThreadSwitcher(
		ThreadID thread, ThreadSwitchProcPtr threadSwitcher, void *switchProcParam,
		Boolean inOrOut);
	virtual OSErr SetThreadTerminator(
		ThreadID thread, ThreadTerminationProcPtr threadTerminator, void *terminatorParam);

	virtual ~GUSIThreadManagerProxy() {}

	static GUSIThreadManagerProxy *Instance();

protected:
	GUSIThreadManagerProxy() {}

	static GUSIThreadManagerProxy *MakeInstance();
};

enum GUSIYieldMode
{
	kGUSIPoll,	// Busy wait for some unblockable condition
	kGUSIBlock, // Wait for some blockable condition
	kGUSIYield	// Yield to some other eligible thread
};

class GUSIProcess
{
public:
	static GUSIProcess *Instance();
	static void DeleteInstance();
	void GetPSN(ProcessSerialNumber *psn);
	void AcquireTaskRef();
	ThreadTaskRef GetTaskRef();
	long GetA5();
	bool Threading();
	void Yield(GUSIYieldMode wait);
	void Wakeup();
	GUSISigProcess *SigProcess();
	void QueueForClose(GUSISocket *sock);

	class A5Saver
	{
	public:
		A5Saver(long processA5);
		A5Saver(GUSIContext *context);
		A5Saver(GUSIProcess *process);
		~A5Saver();

	private:
		long fSavedA5;
	};

protected:
	friend class GUSIContext;

	GUSIProcess(bool threading);
	~GUSIProcess();

	int fReadyThreads;
	int fExistingThreads;
	GUSISigProcess *fSigProcess;

private:
	static GUSIProcess *sInstance;

	ProcessSerialNumber fProcess;
	ThreadTaskRef fTaskRef;
	long fA5;

	GUSISocket *fClosing;
	UInt32 fResumeTicks;
	bool fWillSleep;
	bool fDontSleep;
};

class GUSIContext : public GUSISpecificTable
{
public:
	friend class GUSIProcess;
	friend class GUSIContextFactory;

	ThreadID ID() { return fThreadID; }
	virtual void Wakeup();
	void ClearWakeups() { fWakeup = false; }
	GUSIProcess *Process() { return fProcess; }
	void Detach() { fFlags |= detached; }
	void Liquidate();
	OSErr Error() { return sError; }
	bool Done(bool join);
	void *Result() { return fResult; }
	GUSISigContext *SigContext() { return fSigContext; }

	static GUSIContext *Current() { return sCurrentContext; }
	static GUSIContext *CreateCurrent(bool threading = false)
	{
		if (!sCurrentContext)
			Setup(threading);
		return sCurrentContext;
	}
	static GUSIContext *Lookup(ThreadID id);
	static void Setup(bool threading);
	static bool Yield(GUSIYieldMode wait);
	static void SigWait(sigset_t sigs);
	static void SigSuspend();
	static void Raise();
	static bool Interrupt(bool allSigs = false);
	static sigset_t Pending();
	static sigset_t Blocked();

	void SetSwitchIn(ThreadSwitchProcPtr switcher, void *switchParam);
	void SetSwitchOut(ThreadSwitchProcPtr switcher, void *switchParam);
	void SetTerminator(ThreadTerminationProcPtr terminator, void *terminationParam);

	static GUSIContextQueue::iterator begin() { return sContexts.begin(); }
	static GUSIContextQueue::iterator end() { return sContexts.end(); }
	static void LiquidateAll() { sContexts.LiquidateAll(); }

	friend class GUSIContextFactory;

	static void GUSIThreadSwitchIn(ThreadID thread, GUSIContext *context);
	static void GUSIThreadSwitchOut(ThreadID thread, GUSIContext *context);

	friend pascal void GUSIThreadTerminator(ThreadID thread, GUSIContext *context);

	GUSIContext(ThreadID id);
	GUSIContext(
		ThreadEntryProcPtr threadEntry, void *threadParam,
		Size stackSize, ThreadOptions options, void **threadResult, ThreadID *threadMade);

	virtual void SwitchIn();
	virtual void SwitchOut();
	virtual void Terminate();

	ThreadID fThreadID;
	GUSIProcess *fProcess;
	GUSIContext *fNext;
	GUSISigContext *fSigContext;
	ThreadSwitchProcPtr fSwitchInProc;
	ThreadSwitchProcPtr fSwitchOutProc;
	ThreadTerminationProcPtr fTerminateProc;
	void *fSwitchInParam;
	void *fSwitchOutParam;
	void *fTerminateParam;
	void *fResult;
	GUSIContext *fJoin;
	enum
	{
		done = 1 << 0,
		detached = 1 << 1,
		asleep = 1 << 2
	};
	char fFlags;
	bool fWakeup;
	UInt32 fEntryTicks;
	int fErrno;
	int fHostErrno;

	class Queue : public GUSIContextQueue
	{
	public:
		void LiquidateAll();

		~Queue() { LiquidateAll(); }
	};

	static Queue sContexts;
	static GUSIContext *sCurrentContext;
	static bool sCreatingCurrentContext;
	static bool sHasThreading;
	static OSErr sError;

	void StartSetup();
	void FinishSetup();

	virtual ~GUSIContext();
};

class GUSIContextFactory
{
public:
	static GUSIContextFactory *Instance();
	static void SetInstance(GUSIContextFactory *instance);
	static void DeleteInstance();

	virtual GUSIContext *CreateContext(ThreadID id);
	virtual GUSIContext *CreateContext(
		ThreadEntryProcPtr threadEntry, void *threadParam,
		Size stackSize, ThreadOptions options = kCreateIfNeeded,
		void **threadResult = nil, ThreadID *threadMade = nil);

	virtual ~GUSIContextFactory();

protected:
	GUSIContextFactory();
};

extern "C" void GUSISetupContextFactory();

void GUSIStartIO(ParamBlockRec *pb);
OSErr GUSIFinishIO(ParamBlockRec *pb);
OSErr GUSIControl(ParamBlockRec *pb);
template <class PB>
struct GUSIIOPBWrapper
{
	GUSIContext *fContext;
	PB fPB;

	GUSIIOPBWrapper() {}
	GUSIIOPBWrapper(const PB &pb) { memcpy(&fPB, &pb, sizeof(PB)); }

	PB *operator->() { return &fPB; }
	void StartIO() { GUSIStartIO(reinterpret_cast<ParamBlockRec *>(&fPB)); }
	OSErr FinishIO() { return GUSIFinishIO(reinterpret_cast<ParamBlockRec *>(&fPB)); }
	OSErr Control() { return GUSIControl(reinterpret_cast<ParamBlockRec *>(&fPB)); }
};

#if PRAGMA_STRUCT_ALIGN
#pragma options align = reset
#endif

inline GUSIProcess *GUSIProcess::Instance()
{
	if (!sInstance)
		sInstance = new GUSIProcess(GUSIContext::sHasThreading);
	return sInstance;
}
inline void GUSIProcess::DeleteInstance()
{
	delete sInstance;
	sInstance = 0;
}

inline void GUSIProcess::GetPSN(ProcessSerialNumber *psn)
{
	*psn = fProcess;
}
inline void GUSIProcess::AcquireTaskRef()
{
	GetThreadCurrentTaskRef(&fTaskRef);
}
inline ThreadTaskRef GUSIProcess::GetTaskRef()
{
	return fTaskRef;
}
inline long GUSIProcess::GetA5()
{
	return fA5;
}
inline bool GUSIProcess::Threading()
{
	return fTaskRef != 0;
}

inline GUSIProcess::A5Saver::A5Saver(long processA5)
{
	fSavedA5 = SetA5(processA5);
}
inline GUSIProcess::A5Saver::A5Saver(GUSIProcess *process)
{
	fSavedA5 = SetA5(process->GetA5());
}
inline GUSIProcess::A5Saver::A5Saver(GUSIContext *context)
{
	fSavedA5 = SetA5(context->Process()->GetA5());
}
inline GUSIProcess::A5Saver::~A5Saver()
{
	SetA5(fSavedA5);
}

#endif /* GUSI_SOURCE */

#endif /* _GUSIContext_ */
