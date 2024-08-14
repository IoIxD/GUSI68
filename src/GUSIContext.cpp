
#include "GUSIInternal.h"
#include "GUSIContext.h"
#include "GUSIDiag.h"
#include "GUSISignal.h"
#include "GUSIConfig.h"
#include "GUSIDescriptor.h"
#include "GUSITimer.h"

#include <errno.h>
#include <signal.h>

#include <EPPC.h>
#include <LowMem.h>
#include <AppleEvents.h>
#include <Quickdraw.h>
#include <Devices.h>

#include <utility>
#include <memory>

GUSI_USING_STD_NAMESPACE


GUSI_NEEDS_QD

GUSIProcess::GUSIProcess(bool threading)
{
	GetCurrentProcess(&fProcess);
	fA5 = (long) LMGetCurrentA5();
	if (threading)
		AcquireTaskRef();
	else
		fTaskRef = 0;
	GUSIConfiguration::Instance()->AutoInitGraf();
	fReadyThreads 	= 0;
	fExistingThreads= 0;
	fResumeTicks  	= 0;
	fWillSleep 		= false;
	fDontSleep 		= false;
	fSigProcess		= GUSISigFactory::Instance()->CreateSigProcess();
	fClosing		= 0;
}

void GUSIProcess::QueueForClose(GUSISocket * sock)
{
	sock->Enqueue(&fClosing);
}

GUSIProcess::~GUSIProcess()
{
	UInt32 	start = LMGetTicks();
	UInt32	now	  = start;
	
	while (fClosing)	{ 
		fClosing->CheckClose(now);
		if (now < start+300)	// Normal speed for 5 seconds
			now = LMGetTicks();
		else					
			now += 300;			// Accelerate
	}
}

void GUSIContext::Setup(bool threading)
{
	bool	wasThreading = sHasThreading;
	if (threading)
		sHasThreading = true;
	if (!sCurrentContext && !sCreatingCurrentContext) {
		MaxApplZone();		// It's about time, too!
		
		sCreatingCurrentContext = true;
		sCurrentContext = 
			GUSIContextFactory::Instance()->CreateContext(kApplicationThreadID);
		sCreatingCurrentContext = false;
	} else if (!wasThreading && threading) {
		<<Upgrade application context to threading>>
	}
}

GUSIContext::GUSIContext(ThreadID id)
{
	StartSetup();
	fThreadID = id;
	sError	  = 0;
	FinishSetup();
}

GUSIContext::GUSIContext(ThreadEntryProcPtr threadEntry, void *threadParam, 
        Size stackSize, ThreadOptions options, void ** result, ThreadID * thread)
{
    StartSetup();
	if (!result)
		result = &fResult;
    sError = GUSIThreadManagerProxy::Instance()->NewThread(
				kCooperativeThread, threadEntry, threadParam, 
        		stackSize, options, result, &fThreadID);
	if (thread)
    	*thread = fThreadID;
	if (sError)
        return;
    FinishSetup();
}

GUSIContext * GUSIContext::Lookup(ThreadID id)
{
	for (GUSIContextQueue::iterator context = begin(); context != end(); ++context)
		if (context->fThreadID == id)
			return *context;
	return nil;
}

void GUSIContext::Queue::LiquidateAll()
{
	GUSIContextFactory::DeleteInstance();
	GUSIDescriptorTable::CloseAllDescriptors();
	GUSIProcess::DeleteInstance();
	while (!empty())
		front()->Liquidate();
}

void GUSIContext::Liquidate()
{
	GUSI_MESSAGE4(("GUSIContext::Liquidate %08x\n", fThreadID));
	switch (fThreadID) {
	case kApplicationThreadID:	// Main thread, restore switchers
		if (sHasThreading) {
			SetThreadSwitcher(fThreadID, fSwitchInProc,  fSwitchInParam,  true);
			SetThreadSwitcher(fThreadID, fSwitchOutProc, fSwitchOutParam, false);
		}
		break;
	default:					// Other thread, terminate
		if (!(fFlags & done)) {
			fFlags &= ~detached;	// Will destroy context ourselves
			DisposeThread(fThreadID, nil, false);
		}
		break;
	}
	delete this;
}

void GUSIContext::Wakeup()
{
	if (fWakeup) {
		GUSI_MESSAGE(("Duplicate wakeup #%d\n", fThreadID));
	} else {
		GUSI_MESSAGE(("Wakeup #%d\n", fThreadID));
		fWakeup = true;
		if (fThreadID && fThreadID != kApplicationThreadID)
			SetThreadReadyGivenTaskRef(Process()->GetTaskRef(), fThreadID);
		Process()->Wakeup();
	}
}

void GUSIProcess::Wakeup()
{
	if (fDontSleep) {
		GUSI_SMESSAGE("Duplicate WakeUpProcess\n");
	} else {
		fDontSleep 			= true;
		
		if (!fWillSleep) {
			GUSI_SMESSAGE("Caught sleep in time\n");
		} else {		
			GUSI_SMESSAGE("WakeUpProcess\n");
			
			WakeUpProcess(&fProcess);
		}
	}
}

OSErr GUSINewThread(
	ThreadStyle, ThreadEntryProcPtr threadEntry, void *threadParam, 
	Size stackSize, ThreadOptions options, void **threadResult, ThreadID *threadMade)
{
	GUSIContext * context = GUSIContextFactory::Instance()->CreateContext(
				threadEntry, threadParam, stackSize, options, 
				threadResult, threadMade);
	OSErr err = context->Error();
	if (err)
		context->Liquidate();
	return err;
}

static GUSIContextFactory *	sGUSIContextFactory;
static bool					sGUSIContextFactorySetup;

GUSIContextFactory * GUSIContextFactory::Instance()
{
	if (!sGUSIContextFactorySetup) {
		sGUSIContextFactorySetup = true;
		GUSISetupContextFactory();
	}

	if (!sGUSIContextFactory) 
		SetInstance(new GUSIContextFactory());
	
	return sGUSIContextFactory;
}

void GUSIContextFactory::SetInstance(GUSIContextFactory * instance)
{
	sGUSIContextFactory = instance;
}

void GUSIContextFactory::DeleteInstance()
{
	delete sGUSIContextFactory;
	sGUSIContextFactory = 0;
}

GUSIContextFactory::GUSIContextFactory()
{
}

GUSIContextFactory::~GUSIContextFactory()
{
}

OSErr GUSISetThreadSwitcher(
	ThreadID thread, ThreadSwitchProcPtr threadSwitcher, void *switchProcParam, Boolean inOrOut)
{
	GUSIContext * context;
	if (!(context = GUSIContext::Lookup(thread)))
		return GUSIThreadManagerProxy::Instance()->SetThreadSwitcher(
			thread, threadSwitcher, switchProcParam, inOrOut);
	if (inOrOut) 
		context->SetSwitchIn(threadSwitcher, switchProcParam);
	else
		context->SetSwitchOut(threadSwitcher, switchProcParam);
	return noErr;
}

void GUSIContext::SetSwitchIn(ThreadSwitchProcPtr switcher, void *switchParam)
{
	fSwitchInProc = switcher;
	fSwitchInParam= switchParam;
}

void GUSIContext::SetSwitchOut(ThreadSwitchProcPtr switcher, void *switchParam)
{
	fSwitchOutProc = switcher;
	fSwitchOutParam= switchParam;
}

OSErr GUSISetThreadTerminator(
	ThreadID thread, ThreadTerminationProcPtr threadTerminator, void *terminationProcParam)
{
	GUSIContext * context;
	if (!(context = GUSIContext::Lookup(thread)))
		return GUSIThreadManagerProxy::Instance()->SetThreadTerminator(
			thread, threadTerminator, terminationProcParam);
	context->SetTerminator(threadTerminator, terminationProcParam);
	
	return noErr;
}

void GUSIContext::SetTerminator(ThreadTerminationProcPtr terminator, void *terminationParam)	
{
	fTerminateProc = terminator;
	fTerminateParam= terminationParam;
}

#if GENERATING68K && GENERATINGCFM
#define CallThreadSwitchProc(userRoutine, thread, context)	\
	CallUniversalProc((userRoutine), uppThreadSwitchProcInfo, (thread), (context))
#define CallThreadTerminationProc(userRoutine, thread, context)	\
	CallUniversalProc((userRoutine), uppThreadTerminationProcInfo, (thread), (context))
#else
#define CallThreadSwitchProc(userRoutine, thread, context)	\
	(*userRoutine)((thread), (context))
#define CallThreadTerminationProc(userRoutine, thread, context)	\
	(*userRoutine)((thread), (context))
#endif

pascal void GUSIThreadSwitchOut(ThreadID, GUSIContext * context)
{
	context->SwitchOut();
}

void GUSIContext::SwitchOut()
{
	if (fSwitchOutProc)
		CallThreadSwitchProc(fSwitchOutProc, fThreadID, fSwitchOutParam);
	fErrno		=	errno;
	fHostErrno	=	h_errno;
	
	ThreadTaskRef	taskRef = Process()->GetTaskRef();
	ThreadState		state;
	if (!GetThreadStateGivenTaskRef(taskRef, fThreadID, &state))
		if (state == kStoppedThreadState)
			if (fWakeup) {
				SetThreadReadyGivenTaskRef(taskRef, fThreadID);
			} else {
				GUSI_MESSAGE(("Sleep #%d\n", fThreadID));
				fFlags |= asleep;
				--fProcess->fReadyThreads;
			}
}

bool GUSIContext::Done(bool join)
{
	if ((fFlags & done) || !join || fJoin)
		return (fFlags & done);
	fJoin = GUSIContext::sCurrentContext;
	while (!(fFlags & done))
		Yield(kGUSIBlock);
	return (fFlags & done);
}

const int kThreadTimeSliceTicks		=	12;
const int kProcessTimeSliceTicks	= 	20;
const int kProcessSleepTicks		= 	60;

void GUSIContext::SigWait(sigset_t sigs)
{
	GUSIYieldMode 	wait		= kGUSIBlock;	
	<<Determine process and blocking parameters>>
	
	for (;;) {
		<<Suspend the current process if necessary>>
		if (Pending() & sigs)
			break;
		Interrupt();
		<<Suspend the current thread if possible>>
	}
	sCurrentContext->fWakeup = false;
}

void GUSIContext::SigSuspend()
{
	GUSIYieldMode 	wait		= kGUSIBlock;	
	<<Determine process and blocking parameters>>
	
	for (;;) {
		<<Suspend the current process if necessary>>
		if (Interrupt(true))
			break;
		<<Suspend the current thread if possible>>
	}
	sCurrentContext->fWakeup = false;
}

void GUSIProcess::Yield(GUSIYieldMode wait)
{
	<<Check for interrupts if we are the front process>>
	if (wait == kGUSIBlock) {
		fWillSleep = true;
		if (fReadyThreads > 1 || fDontSleep) {
			GUSI_SMESSAGE("Don't Sleep\n");
			wait = kGUSIYield;
		}
	}
	if (fExistingThreads < 2) // Single threaded process skips sleep only once
		fDontSleep = false;
	if (wait == kGUSIYield && LMGetTicks() - fResumeTicks < kProcessTimeSliceTicks) {
		fWillSleep 		= false;
		return;
	}
	if (gGUSISpinHook) {
		gGUSISpinHook(wait == kGUSIBlock);
	} else {
		GUSI_SMESSAGE("Suspend\n");
		GUSIHandleNextEvent(wait == kGUSIBlock ? kProcessSleepTicks : 0);
		GUSI_SMESSAGE("Resume\n");
	} 
	if (fExistingThreads < 2) 		// Single threaded process skips sleep only once
		fDontSleep = false;
	fWillSleep 		= false;
	fResumeTicks 	= LMGetTicks();
	if (fClosing)
		fClosing->CheckClose();
}

void GUSIContext::Raise()
{
	sCurrentContext->SigContext()->Raise(
		GUSIProcess::Instance()->SigProcess());
}

bool GUSIContext::Interrupt(bool allSigs)
{
	return sCurrentContext->SigContext()->Interrupt(
		GUSIProcess::Instance()->SigProcess(), allSigs);
}

sigset_t GUSIContext::Pending()
{
	return sCurrentContext->SigContext()->Pending(
		GUSIProcess::Instance()->SigProcess());
}

sigset_t GUSIContext::Blocked()
{
	return sCurrentContext->SigContext()->GetBlocked();
}

inline GUSIContext *& Context(ParamBlockRec * pb)
{
	return reinterpret_cast<GUSIContext **>(pb)[-1];
}
static pascal void GUSIIODone(ParamBlockRec * pb)
{
	if (Context(pb)) 
		Context(pb)->Wakeup();
}

GUSI_COMPLETION_PROC_A0(GUSIIODone, ParamBlockRec)

void GUSIStartIO(ParamBlockRec * pb)
{
	static IOCompletionUPP sIODone = 0;
	
	if (!sIODone)
		sIODone = NewIOCompletionProc(GUSIIODoneEntry);
	Context(pb) 		= nil;
	pb->ioParam.ioCompletion 	= sIODone;
}

OSErr GUSIFinishIO(ParamBlockRec * pb)
{
	Context(pb) 		= GUSIContext::CreateCurrent();
	while (pb->ioParam.ioResult > 0)
		GUSIContext::Yield(kGUSIBlock);
	return pb->ioParam.ioResult;
}

OSErr GUSIControl(ParamBlockRec * pb)
{
	GUSIStartIO(pb);
	PBControlAsync(reinterpret_cast<ParmBlkPtr>(pb));
	return GUSIFinishIO(pb);
}

static auto_ptr<GUSIThreadManagerProxy> sGUSIThreadManagerProxy;

OSErr GUSIThreadManagerProxy::NewThread(
		ThreadStyle threadStyle, ThreadEntryProcPtr threadEntry, void *threadParam, 
		Size stackSize, ThreadOptions options, 
		void **threadResult, ThreadID *threadMade)
{
	return ::NewThread(
				threadStyle, threadEntry, threadParam, stackSize, options, 
				threadResult, threadMade);
}

OSErr GUSIThreadManagerProxy::SetThreadSwitcher(ThreadID thread, 
		ThreadSwitchProcPtr threadSwitcher, void *switchProcParam, Boolean inOrOut)
{
	return ::SetThreadSwitcher(thread, threadSwitcher, switchProcParam, inOrOut);
}

OSErr GUSIThreadManagerProxy::SetThreadTerminator(ThreadID thread, 
		ThreadTerminationProcPtr threadTerminator, void *terminationProcParam)
{
	return ::SetThreadTerminator(thread, threadTerminator, terminationProcParam);
}

GUSIThreadManagerProxy * GUSIThreadManagerProxy::Instance()
{ 
	if (!sGUSIThreadManagerProxy.get()) 
		sGUSIThreadManagerProxy = auto_ptr<GUSIThreadManagerProxy>(MakeInstance()); 
	return sGUSIThreadManagerProxy.get(); 
}

GUSIThreadManagerProxy * GUSIThreadManagerProxy::MakeInstance()
{
	return new GUSIThreadManagerProxy;
}

