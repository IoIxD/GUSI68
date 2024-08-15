
#ifndef _GUSISIGNAL_
#define _GUSISIGNAL_

#include <signal.h>

#ifdef GUSI_SOURCE

#include <ConditionalMacros.h>

#if PRAGMA_STRUCT_ALIGN
#pragma options align = native
#endif

class GUSISigContext;

class GUSISigProcess
{
public:
	virtual struct sigaction &GetAction(int sig);
	virtual int SetAction(int sig, const struct sigaction &act);
	virtual sigset_t Pending() const;
	virtual void ClearPending(sigset_t clear);
	virtual void Post(int sig);
	virtual bool Interrupt(int sig, GUSISigContext *context);
	virtual void Raise(int sig, GUSISigContext *context);

	virtual ~GUSISigProcess();

protected:
	sigset_t fPending;
	struct sigaction fAction[NSIG - 1];

	virtual bool CantCatch(int sig);
	virtual bool CantIgnore(int sig);

	virtual bool DefaultIsIgnore(int sig);
	virtual bool DefaultAction(int sig, const struct sigaction &act);

	friend class GUSISigFactory;
	GUSISigProcess();
};

class GUSISigContext
{
public:
	virtual sigset_t GetBlocked() const;
	virtual void SetBlocked(sigset_t sigs);
	virtual sigset_t Pending() const;
	virtual sigset_t Pending(GUSISigProcess *proc) const;
	virtual void ClearPending(sigset_t clear);
	virtual void Post(int sig);
	virtual sigset_t Ready(GUSISigProcess *proc);
	virtual bool Interrupt(GUSISigProcess *proc, bool allSigs = false);
	virtual void Raise(GUSISigProcess *proc);

	virtual ~GUSISigContext();

protected:
	sigset_t fPending;
	sigset_t fBlocked;

	virtual sigset_t CantBlock();

	friend class GUSISigFactory;
	GUSISigContext(const GUSISigContext *parent);
};

class GUSISigFactory
{
public:
	virtual GUSISigProcess *CreateSigProcess();
	virtual GUSISigContext *CreateSigContext(const GUSISigContext *parent);

	virtual ~GUSISigFactory();

	static GUSISigFactory *Instance();
	static void SetInstance(GUSISigFactory *instance);

protected:
	GUSISigFactory() {}
};

#if PRAGMA_STRUCT_ALIGN
#pragma options align = reset
#endif

#endif

#endif /* _GUSISIGNAL_ */
