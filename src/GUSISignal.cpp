
#include "GUSIInternal.h"
#include "GUSISignal.h"
#include "GUSIDiag.h"
#include "GUSITimer.h"
#include "GUSIPThread.h"

#include <stdlib.h>
#include <unistd.h>
#include <memory>

GUSI_USING_STD_NAMESPACE

sigset_t GUSISigContext::GetBlocked() const
{
	return fBlocked;
}

void GUSISigContext::SetBlocked(sigset_t sigs)
{
	fBlocked = sigs & ~CantBlock();
}

sigset_t GUSISigContext::Pending() const
{
	return fPending;
}

void GUSISigContext::ClearPending(sigset_t clear)
{
	fPending &= ~clear;
}

void GUSISigContext::Post(int sig)
{
	sigaddset(&fPending, sig);
}

sigset_t GUSISigContext::Pending(GUSISigProcess *proc) const
{
	return proc->Pending() | Pending();
}

sigset_t GUSISigContext::Ready(GUSISigProcess *proc)
{
	return Pending(proc) & ~GetBlocked();
}

void GUSISigContext::Raise(GUSISigProcess *proc)
{
	for (;;)
	{
		sigset_t todo = Ready(proc);

		if (!todo)
			return;

		proc->ClearPending(todo);
		ClearPending(todo);

		for (int sig = 1; todo; ++sig)
			if (sigismember(&todo, sig))
			{
				sigdelset(&todo, sig);
				proc->Raise(sig, this);
			}
	}
}

bool GUSISigContext::Interrupt(GUSISigProcess *proc, bool allSigs)
{
	bool interrupt = false;
	sigset_t todo = Ready(proc);

	for (int sig = 1; todo; ++sig)
		if (sigismember(&todo, sig))
		{
			sigdelset(&todo, sig);
			interrupt = proc->Interrupt(sig, this) || interrupt || allSigs;
		}

	return interrupt;
}

struct sigaction &GUSISigProcess::GetAction(int sig)
{
	return fAction[sig - 1];
}

sigset_t GUSISigProcess::Pending() const
{
	return fPending;
}

void GUSISigProcess::ClearPending(sigset_t clear)
{
	fPending &= ~clear;
}

void GUSISigProcess::Post(int sig)
{
	sigaddset(&fPending, sig);

	sigset_t sigmask = SigMask(sig);
	bool found = false;
	for (GUSIContextQueue::iterator context = GUSIContext::begin();
		 context != GUSIContext::end();
		 ++context)
		if (!(context->SigContext()->GetBlocked() & sigmask))
		{
			context->Wakeup();
			found = true;
		}
	if (!found)
		for (GUSIContextQueue::iterator context = GUSIContext::begin();
			 context != GUSIContext::end();
			 ++context)
			context->Wakeup();
}

bool GUSISigProcess::Interrupt(int sig, GUSISigContext *context)
{
	bool interrupt = true;
	struct sigaction &act = GetAction(sig);

	if (act.sa_handler == SIG_IGN)
		interrupt = false; // Ignore signal
	else if (act.sa_handler == SIG_DFL)
		interrupt = !DefaultIsIgnore(sig);
	else if (act.sa_flags & SA_RESTART)
	{
		sigdelset(&fPending, sig);

		Raise(sig, context);

		return false;
	}

	if (!interrupt)
		sigdelset(&fPending, sig);

	return interrupt;
}

void GUSISigProcess::Raise(int sig, GUSISigContext *context)
{
	struct sigaction &act = GetAction(sig);

	if (act.sa_handler == SIG_IGN)
		return; // Ignore signal
	else if (act.sa_handler == SIG_DFL)
		if (DefaultAction(sig, act))
			return;

	__sig_handler handler = act.sa_handler;
	sigset_t blockset = act.sa_mask;

	if (act.sa_flags & SA_RESETHAND)
		act.sa_handler = SIG_DFL;
	else if (!(act.sa_flags & SA_NODEFER))
		sigaddset(&blockset, sig);

	sigset_t blocksave = context->GetBlocked();
	//
	// Correct but not yet implementable
	//
	// context->SetBlocked(blocksave | blockset);
	//
	(*handler)(sig);
	sigset_t blocknew = context->GetBlocked();
	context->SetBlocked(blocksave | (blocknew & ~blockset));
}

static auto_ptr<GUSISigFactory> sGUSISigFactory;

GUSISigFactory *GUSISigFactory::Instance()
{
	if (!sGUSISigFactory.get())
		SetInstance(new GUSISigFactory);

	return sGUSISigFactory.get();
}

void GUSISigFactory::SetInstance(GUSISigFactory *instance)
{
	sGUSISigFactory = auto_ptr<GUSISigFactory>(instance);
}

GUSISigFactory::~GUSISigFactory()
{
}

GUSISigProcess *GUSISigFactory::CreateSigProcess()
{
	return new GUSISigProcess;
}

GUSISigContext *GUSISigFactory::CreateSigContext(const GUSISigContext *parent)
{
	return new GUSISigContext(parent);
}

int(sigaddset)(sigset_t *set, int signo)
{

	if (!GUSI_CASSERT_CLIENT(signo > 0 && signo < NSIG))
		return GUSISetPosixError(EINVAL);
	if (signo == 1) // SIGABRT in MPW and CW
		signo = SIGABRT;
	else if (signo == 4) // SIGINT in CW
		signo = SIGINT;

	return sigaddset(set, signo);
}

int(sigdelset)(sigset_t *set, int signo)
{

	if (!GUSI_CASSERT_CLIENT(signo > 0 && signo < NSIG))
		return GUSISetPosixError(EINVAL);
	if (signo == 1) // SIGABRT in MPW and CW
		signo = SIGABRT;
	else if (signo == 4) // SIGINT in CW
		signo = SIGINT;

	return sigdelset(set, signo);
}

int(sigemptyset)(sigset_t *set)
{
	return sigemptyset(set);
}

int(sigfillset)(sigset_t *set)
{
	return sigfillset(set);
}

int(sigismember)(const sigset_t *set, int signo)
{

	if (!GUSI_CASSERT_CLIENT(signo > 0 && signo < NSIG))
		return GUSISetPosixError(EINVAL);
	if (signo == 1) // SIGABRT in MPW and CW
		signo = SIGABRT;
	else if (signo == 4) // SIGINT in CW
		signo = SIGINT;

	return sigismember(set, signo);
}

int raise(int signo)
{

	if (!GUSI_CASSERT_CLIENT(signo > 0 && signo < NSIG))
		return GUSISetPosixError(EINVAL);
	if (signo == 1) // SIGABRT in MPW and CW
		signo = SIGABRT;
	else if (signo == 4) // SIGINT in CW
		signo = SIGINT;

	GUSIProcess::Instance()->SigProcess()->Post(signo);

	return 0;
}

int pthread_kill(pthread_t thread, int signo)
{
	if (!signo)
		return thread ? 0 : ESRCH;
	if (!GUSI_CASSERT_CLIENT(signo > 0 && signo < NSIG))
		return EINVAL;
	thread->SigContext()->Post(signo);
	thread->Wakeup();

	return 0;
}

int sigaction(int signo, const struct sigaction *act, struct sigaction *oact)
{

	if (!GUSI_CASSERT_CLIENT(signo > 0 && signo < NSIG))
		return GUSISetPosixError(EINVAL);
	if (signo == 1) // SIGABRT in MPW and CW
		signo = SIGABRT;
	else if (signo == 4) // SIGINT in CW
		signo = SIGINT;

	GUSISigProcess *proc = GUSIProcess::Instance()->SigProcess();
	if (oact)
		*oact = proc->GetAction(signo);
	if (act)
		if (proc->SetAction(signo, *act))
			return -1;
		else if (act->sa_handler == SIG_IGN)
			GUSIContext::CreateCurrent()->SigContext()->ClearPending(SigMask(signo));
	return 0;
}

__sig_handler signal(int signo, __sig_handler newhandler)
{
	struct sigaction oact;
	struct sigaction nact;

	nact.sa_handler = newhandler;
	nact.sa_mask = 0;
	nact.sa_flags = SA_RESETHAND;

	if (sigaction(signo, &nact, &oact))
		return reinterpret_cast<__sig_handler>(0);
	else
		return oact.sa_handler;
}

int sigpending(sigset_t *pending)
{
	if (pending)
		*pending = GUSIProcess::Instance()->SigProcess()->Pending() | GUSIContext::CreateCurrent()->SigContext()->Pending();
	return 0;
}

int pthread_sigmask(int how, const sigset_t *mask, sigset_t *omask)
{
	GUSISigContext *context = GUSIContext::CreateCurrent(true)->SigContext();
	if (omask)
		*omask = context->GetBlocked();
	if (mask)
		switch (how)
		{
		case SIG_BLOCK:
			context->SetBlocked(context->GetBlocked() | *mask);
			break;
		case SIG_SETMASK:
			context->SetBlocked(*mask);
			break;
		case SIG_UNBLOCK:
			context->SetBlocked(context->GetBlocked() & ~*mask);
			break;
		default:
			return EINVAL;
		}
	GUSIContext::Raise();
	return 0;
}

int sigprocmask(int how, const sigset_t *mask, sigset_t *omask)
{
	return GUSISetPosixError(pthread_sigmask(how, mask, omask));
}

int sigsuspend(const sigset_t *mask)
{
	GUSISigContext *context = GUSIContext::CreateCurrent()->SigContext();
	sigset_t blocksave = context->GetBlocked();
	context->SetBlocked(*mask);
	GUSIContext::SigSuspend();
	sigset_t blocknew = context->GetBlocked();
	context->SetBlocked(blocksave | (blocknew & ~*mask));

	return GUSISetPosixError(EINTR);
}

int pause()
{
	GUSIContext::CreateCurrent()->SigSuspend();

	return GUSISetPosixError(EINTR);
}

int sigwait(const sigset_t *sigs, int *signo)
{
	if (!GUSI_CASSERT_CLIENT(sigs && signo && !(*sigs & ~GUSIContext::Blocked())))
		return GUSISetPosixError(EINVAL);
	GUSIContext::SigWait(*sigs);
	const sigset_t cursigs = *sigs & GUSIContext::Pending();
	for (*signo = 1; !sigismember(&cursigs, *signo); ++*signo)
		;
	GUSIContext::CreateCurrent()->SigContext()->ClearPending(SigMask(*signo));
	GUSIProcess::Instance()->SigProcess()->ClearPending(SigMask(*signo));

	return 0;
}

void abort(void)
{
	raise(SIGABRT);

	_exit(2);
}

#ifdef __MWERKS__
extern int __aborting;

extern "C" void _exit(int code)
{
	__aborting = 1;

	exit(code);
}
#endif

class GUSIAlarm : public GUSITimer
{
public:
	GUSIAlarm(long interval = 0) : GUSITimer(true), fInterval(interval) {}

	virtual void Wakeup();
	long Restart(long interval = 0);

private:
	long fInterval;
};

void GUSIAlarm::Wakeup()
{
	GUSIProcess::Instance()->SigProcess()->Post(SIGALRM);
	if (fInterval)
		Sleep(fInterval, true);
}

long GUSIAlarm::Restart(long interval)
{
	fInterval = interval;
	RmvTime(Elem());
	long rest = tmCount;
	if (rest < 0)
		rest = -rest;
	else
		rest *= 1000;
	tmCount = 0;
	tmWakeUp = 0;
	tmReserved = 0;
	InsXTime(Elem());

	return rest;
}

static auto_ptr<GUSIAlarm> sGUSIAlarm;

unsigned int alarm(unsigned int delay)
{
	unsigned int togo = 0;

	GUSIAlarm *a = sGUSIAlarm.get();
	if (a)
		togo = static_cast<unsigned int>(a->Restart() / 1000000);
	else
		sGUSIAlarm = auto_ptr<GUSIAlarm>(a = new GUSIAlarm);
	if (a && delay)
		a->Sleep(delay * 1000);

	return togo;
}

useconds_t ualarm(useconds_t delay, useconds_t interval)
{
	useconds_t togo = 0;

	GUSIAlarm *a = sGUSIAlarm.get();
	if (a)
		togo = static_cast<useconds_t>(a->Restart(-static_cast<long>(interval)));
	else
		sGUSIAlarm = auto_ptr<GUSIAlarm>(a = new GUSIAlarm(-static_cast<long>(interval)));
	if (a && delay)
		a->MicroSleep(delay);

	return togo;
}
