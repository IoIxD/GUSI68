
#ifndef _GUSIOpenTransport_
#define _GUSIOpenTransport_

#ifdef GUSI_INTERNAL

#include <OpenTransport.h>
#include <OpenTptInternet.h>

#include "GUSISocket.h"
#include "GUSISocketMixins.h"
#include "GUSIFactory.h"

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>

class GUSIOTStrategy
{

public:
	TEndpointInfo fEndpointInfo;
	OTConfiguration *CreateConfiguration();

	virtual int PackAddress(
		const void *address, socklen_t len, TNetbuf &addr, bool non_null = false) = 0;
	virtual int UnpackAddress(const TNetbuf &addr, void *address, socklen_t *len) = 0;
	virtual int CopyAddress(const TNetbuf &from, TNetbuf &to);

	TEndpointInfo *EndpointInfo() { return &fEndpointInfo; }

protected:
	OTConfiguration *fConfig;
	GUSIOTStrategy() : fConfig(nil) {}
	virtual ~GUSIOTStrategy();
};

class GUSIOTFactory : public GUSISocketFactory
{
public:
	static bool Initialize();

protected:
	virtual GUSIOTStrategy *Strategy(int domain, int type, int protocol) = 0;

private:
	static bool sOK;
};

class GUSIOTStreamFactory : public GUSIOTFactory
{
public:
	GUSISocket *socket(int domain, int type, int protocol);
};

class GUSIOTDatagramFactory : public GUSIOTFactory
{
public:
	GUSISocket *socket(int domain, int type, int protocol);
};

template <class T, int tag>
class GUSIOT : public T
{
public:
	void *operator new(size_t, EndpointRef ref)
	{
		OSStatus err;
		return OTAlloc(ref, tag, T_ALL, &err);
	}
	void *operator new(size_t, EndpointRef ref, int fields)
	{
		OSStatus err;
		return OTAlloc(ref, tag, fields, &err);
	}
	void operator delete(void *o)
	{
		if (o)
			OTFree(o, tag);
	}
};
template <class T, int tag>
class GUSIOTAddr : public GUSIOT<T, tag>
{
public:
	TNetbuf &addr;
	int Pack(GUSIOTStrategy *strategy, const void *address, socklen_t len, bool non_null = false)
	{
		return strategy->PackAddress(address, len, addr, non_null);
	}
	int Unpack(GUSIOTStrategy *strategy, void *address, socklen_t *len)
	{
		return strategy->UnpackAddress(addr, address, len);
	}
	int Copy(GUSIOTStrategy *strategy, GUSIOTAddr<T, tag> *to)
	{
		return strategy->CopyAddress(addr, to->addr);
	}
};

typedef GUSIOTAddr<TBind, T_BIND> GUSIOTTBind;
typedef GUSIOTAddr<TCall, T_CALL> GUSIOTTCall;
typedef GUSIOTAddr<TUnitData, T_UNITDATA> GUSIOTTUnitData;
typedef GUSIOTAddr<TUDErr, T_UDERROR> GUSIOTTUDErr;
typedef GUSIOT<TDiscon, T_DIS> GUSIOTTDiscon;
typedef GUSIOT<TOptMgmt, T_OPTMGMT> GUSIOTTOptMgmt;

class GUSIOTSocket : public GUSISocket,
					 protected GUSISMBlocking,
					 protected GUSISMState,
					 protected GUSISMAsyncError
{
public:
	virtual int bind(void *name, socklen_t namelen);

	virtual int getsockname(void *name, socklen_t *namelen);

	virtual int shutdown(int how);

	virtual int fcntl(int cmd, va_list arg);

	virtual int ioctl(unsigned int request, va_list arg);

	virtual bool pre_select(bool wantRead, bool wantWrite, bool wantExcept);

	virtual int getsockopt(int level, int optname, void *optval, socklen_t *optlen);

	virtual int setsockopt(int level, int optname, void *optval, socklen_t optlen);

	virtual bool Supports(ConfigOption config);

protected:
	GUSIOTSocket(GUSIOTStrategy *strategy);

	uint16_t fNewEvent;
	uint16_t fCurEvent;
	uint16_t fEvent;
	uint32_t fNewCompletion;
	uint32_t fCurCompletion;
	uint32_t fCompletion;
	friend void GUSIOTNotify(GUSIOTSocket *, OTEventCode, OTResult, void *);

	class Lock
	{
	public:
		Lock(EndpointRef end) : fEndpoint(end) { OTEnterNotifier(fEndpoint); }
		~Lock() { OTLeaveNotifier(fEndpoint); }

	private:
		EndpointRef fEndpoint;
	};

	virtual void MopupEvents();

	GUSIOTStrategy *fStrategy;
	EndpointRef fEndpoint;
	linger fLinger;
	UInt32 fDeadline;

	virtual void close();
	virtual ~GUSIOTSocket();

	virtual GUSIOTSocket *Clone() = 0;

	GUSIOTTBind *fSockName;
	int BindToAddress(GUSIOTTBind *addr);

	void Unbind();

	friend class GUSIOTStreamSocket;
	friend class GUSIOTDatagramSocket;
};

class GUSIOTStreamSocket : public GUSIOTSocket
{
public:
	int fNextListener;

	virtual GUSIOTSocket *Clone();

	virtual void close();
	virtual bool Close(UInt32 now);
	~GUSIOTStreamSocket();

	virtual void MopupEvents();

	virtual int listen(int qlen);

	virtual int getpeername(void *name, socklen_t *namelen);

	virtual GUSISocket *accept(void *address, socklen_t *addrlen);

	virtual int connect(void *address, socklen_t addrlen);

	virtual ssize_t recvfrom(const GUSIScatterer &buffer, int flags, void *from, socklen_t *fromlen);

	virtual ssize_t sendto(const GUSIGatherer &buffer, int flags, const void *to, socklen_t tolen);

	virtual bool select(bool *canRead, bool *canWrite, bool *except);

	virtual int shutdown(int how);

protected:
	GUSIOTStreamSocket(GUSIOTStrategy *strategy);

	friend class GUSIOTStreamFactory;

	GUSIOTTCall *fPeerName;
};

class GUSIOTDatagramSocket : public GUSIOTSocket
{
public:
	virtual GUSIOTSocket *Clone();

	~GUSIOTDatagramSocket();

	virtual int getpeername(void *name, socklen_t *namelen);

	virtual int connect(void *address, socklen_t addrlen);

	virtual ssize_t recvfrom(const GUSIScatterer &buffer, int flags, void *from, socklen_t *fromlen);

	virtual ssize_t sendto(const GUSIGatherer &buffer, int flags, const void *to, socklen_t tolen);

	virtual bool select(bool *canRead, bool *canWrite, bool *except);

protected:
	GUSIOTDatagramSocket(GUSIOTStrategy *strategy);

	friend class GUSIOTDatagramFactory;

	int BindIfUnbound();

	GUSIOTTBind *fPeerName;
};

#endif /* GUSI_INTERNAL */

#endif /* _GUSIOpenTransport_ */
