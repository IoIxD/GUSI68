
#include "GUSIInternal.h"
#include "GUSIContext.h"

#include <ConditionalMacros.h>
#include <CodeFragments.h>


extern "C" {
#if !TARGET_RT_MAC_CFM
#define DECL_thread(name, pname, ret, args) ret name args;
#ifdef __MWERKS__
#pragma pointers_in_D0
#endif

DECL_thread(GUSIStdNewThread, "\pNewThread", OSErr, 
	(ThreadStyle threadStyle, ThreadEntryProcPtr threadEntry, 
	 void *threadParam, Size stackSize, ThreadOptions options, 
	 void **threadResult, ThreadID *threadMade))
DECL_thread(GUSIStdSetThreadSwitcher, "\pSetThreadSwitcher", OSErr, 
	(ThreadID thread, ThreadSwitchProcPtr threadSwitcher, 
	 void *switchProcParam, Boolean inOrOut))
DECL_thread(GUSIStdSetThreadTerminator, "\pSetThreadTerminator", OSErr, 
	(ThreadID thread, ThreadTerminationProcPtr threadTerminator, 
	 void *terminationProcParam))

#ifdef __MWERKS__
#pragma pointers_in_A0
#endif
static bool ConnectToThreadLibrary()	{ return true; } 
#define sGUSIThreadFailure 0
#else
#define DECL_thread(name, pname, ret, args) static ret (*name) args;

DECL_thread(GUSIStdNewThread, "\pNewThread", OSErr, 
	(ThreadStyle threadStyle, ThreadEntryProcPtr threadEntry, 
	 void *threadParam, Size stackSize, ThreadOptions options, 
	 void **threadResult, ThreadID *threadMade))
DECL_thread(GUSIStdSetThreadSwitcher, "\pSetThreadSwitcher", OSErr, 
	(ThreadID thread, ThreadSwitchProcPtr threadSwitcher, 
	 void *switchProcParam, Boolean inOrOut))
DECL_thread(GUSIStdSetThreadTerminator, "\pSetThreadTerminator", OSErr, 
	(ThreadID thread, ThreadTerminationProcPtr threadTerminator, 
	 void *terminationProcParam))


static OSErr sGUSIThreadFailure = 1;

static void DoConnectToThreadLibrary()
{
	CFragConnectionID 	ThreadsLib;
	CFragSymbolClass	symClass;
	Ptr 				whoCares;
	Str255				error;

	if (sGUSIThreadFailure = GetSharedLibrary(
		StringPtr("\pThreadsLib"), kPowerPCCFragArch, kLoadCFrag, 
		&ThreadsLib, &whoCares, error)
	)
		return;

#undef DECL_thread
#define DECL_thread(name, pname, ret, args) \
	if (sGUSIThreadFailure = FindSymbol(ThreadsLib, pname, (Ptr *) &name, &symClass)) \
		goto failed;
	
DECL_thread(GUSIStdNewThread, "\pNewThread", OSErr, 
	(ThreadStyle threadStyle, ThreadEntryProcPtr threadEntry, 
	 void *threadParam, Size stackSize, ThreadOptions options, 
	 void **threadResult, ThreadID *threadMade))
DECL_thread(GUSIStdSetThreadSwitcher, "\pSetThreadSwitcher", OSErr, 
	(ThreadID thread, ThreadSwitchProcPtr threadSwitcher, 
	 void *switchProcParam, Boolean inOrOut))
DECL_thread(GUSIStdSetThreadTerminator, "\pSetThreadTerminator", OSErr, 
	(ThreadID thread, ThreadTerminationProcPtr threadTerminator, 
	 void *terminationProcParam))

	
	return;
	
failed:
#undef DECL_thread
#define DECL_thread(name, pname, ret, args) name = 0;
	
DECL_thread(GUSIStdNewThread, "\pNewThread", OSErr, 
	(ThreadStyle threadStyle, ThreadEntryProcPtr threadEntry, 
	 void *threadParam, Size stackSize, ThreadOptions options, 
	 void **threadResult, ThreadID *threadMade))
DECL_thread(GUSIStdSetThreadSwitcher, "\pSetThreadSwitcher", OSErr, 
	(ThreadID thread, ThreadSwitchProcPtr threadSwitcher, 
	 void *switchProcParam, Boolean inOrOut))
DECL_thread(GUSIStdSetThreadTerminator, "\pSetThreadTerminator", OSErr, 
	(ThreadID thread, ThreadTerminationProcPtr threadTerminator, 
	 void *terminationProcParam))

}

static bool ConnectToThreadLibrary()
{
	if (sGUSIThreadFailure == 1)
		DoConnectToThreadLibrary();
	return !sGUSIThreadFailure;
}

#endif
}


pascal OSErr NewThread(
	ThreadStyle style, ThreadEntryProcPtr entry, void *param, 
	Size stack, ThreadOptions options, void **result, ThreadID *made)
{
	return GUSINewThread(style, entry, param, stack, options, result, made);
}

pascal OSErr SetThreadSwitcher(
	ThreadID thread, ThreadSwitchProcPtr switcher, void *param, Boolean inOrOut)
{
	return GUSISetThreadSwitcher(thread, switcher, param, inOrOut);
}

pascal OSErr SetThreadTerminator(
	ThreadID thread, ThreadTerminationProcPtr terminator, void *terminatorParam)
{
	return GUSISetThreadTerminator(thread, terminator, terminatorParam);
}


class GUSIThreadManagerForeignProxy : public GUSIThreadManagerProxy {
public:
	virtual OSErr NewThread(
		ThreadStyle style, ThreadEntryProcPtr entry, void *param, 
		Size stack, ThreadOptions options, void **result, ThreadID *made);
	virtual OSErr SetThreadSwitcher(ThreadID thread,
		ThreadSwitchProcPtr switcher, void *switchProcParam, Boolean inOrOut);
	virtual OSErr SetThreadTerminator(ThreadID thread, 	
		ThreadTerminationProcPtr terminator, void *terminatorParam);
};


GUSIThreadManagerProxy * GUSIThreadManagerProxy::MakeInstance()
{
	return new GUSIThreadManagerForeignProxy;
}


OSErr GUSIThreadManagerForeignProxy::NewThread(
	ThreadStyle style, ThreadEntryProcPtr entry, void *param, 
	Size stack, ThreadOptions options, void **result, ThreadID *made)
{
	if (!ConnectToThreadLibrary())
		return sGUSIThreadFailure;
	return GUSIStdNewThread(style, entry, param, stack, options, result, made);
}

OSErr GUSIThreadManagerForeignProxy::SetThreadSwitcher(
	ThreadID thread, ThreadSwitchProcPtr switcher, void *param, Boolean inOrOut)
{
	if (!ConnectToThreadLibrary())
		return sGUSIThreadFailure;
	return GUSIStdSetThreadSwitcher(thread, switcher, param, inOrOut);
}

OSErr GUSIThreadManagerForeignProxy::SetThreadTerminator(
	ThreadID thread, ThreadTerminationProcPtr terminator, void *terminatorParam)
{
	if (!ConnectToThreadLibrary())
		return sGUSIThreadFailure;
	return GUSIStdSetThreadTerminator(thread, terminator, terminatorParam);
}

