
#include "GUSIInternal.h"
#include "GUSIPPC.h"
#include "GUSIBasics.h"
#include "GUSIBuffer.h"
#include "GUSISocketMixins.h"

#include <errno.h>

#include <algorithm>

GUSI_USING_STD_NAMESPACE

using std::min;

class GUSIPPCSocket : public GUSISocket,
					  protected GUSISMBlocking,
					  protected GUSISMState,
					  protected GUSISMInputBuffer,
					  protected GUSISMOutputBuffer
{
public:
	GUSIPPCSocket();
	virtual ~GUSIPPCSocket();

	virtual int bind(void *addr, socklen_t namelen);

	virtual int connect(void *address, socklen_t addrlen);

	virtual int listen(int queueLength);

	virtual GUSISocket *accept(void *from, socklen_t *fromlen);

	virtual bool Supports(ConfigOption config);

	virtual ssize_t recvfrom(const GUSIScatterer &buffer, int, void *from, socklen_t *fromlen);

	virtual ssize_t sendto(const GUSIGatherer &buffer, int flags, const void *to, socklen_t);

	virtual bool select(bool *canRead, bool *canWrite, bool *exception);

	virtual int fcntl(int cmd, va_list arg);

	virtual int ioctl(unsigned int request, va_list arg);

	virtual int shutdown(int how);

protected:
	OSErr fAsyncError;
	PPCParamBlockRec fSendPB;
	PPCParamBlockRec fRecvPB;
	friend void GUSIPPCSend(GUSIPPCSocket *sock);
	friend void GUSIPPCRecv(GUSIPPCSocket *sock);
	friend pascal void GUSIPPCSendDone(PPCParamBlockPtr pb);
	friend pascal void GUSIPPCRecvDone(PPCParamBlockPtr pb);
	static PPCCompUPP sSendProc;
	static PPCCompUPP sRecvProc;

	struct Listener
	{
		PPCSessRefNum fSession;
		LocationNameRec fLocation;
		PPCPortRec fPort;
		bool fBusy;
	};
	Listener *fListeners;
	bool fRestartListen;
	char fNumListeners;
	char fCurListener;
	char fNextListener;
	friend pascal void GUSIPPCListenDone(PPCParamBlockPtr pb);
	friend void GUSIPPCListen(GUSIPPCSocket *sock);
	static PPCCompUPP sListenProc;

	void SetupListener(Listener &listener);

	friend pascal void GUSIPPCDone(PPCParamBlockPtr pb);
	static PPCCompUPP sDoneProc;

	LocationNameRec fLocation;
	PPCPortRec fPort;
	LocationNameRec fPeerLoc;
	PPCPortRec fPeerPort;

	struct PortRef
	{
		short fRefCount;
		PPCPortRefNum fPort;

		PortRef(PPCPortRefNum port) : fRefCount(1), fPort(port) {}
	};
	PortRef *fPortRef;

	GUSIPPCSocket(GUSIPPCSocket *orig, Listener &listener);
};

pascal void GUSIPPCSendDone(PPCParamBlockPtr pb)
{
	GUSIPPCSocket *sock =
		reinterpret_cast<GUSIPPCSocket *>((char *)pb - offsetof(GUSIPPCSocket, fSendPB));
	if (sock->fOutputBuffer.Locked())
		sock->fOutputBuffer.Defer(GUSIRingBuffer::Deferred(GUSIPPCSendDone), pb);
	else
	{
		PPCWritePBRec &writeParam = pb->writeParam;

		sock->fOutputBuffer.ClearDefer();
		sock->fOutputBuffer.FreeBuffer(writeParam.bufferPtr, writeParam.actualLength);
		if (writeParam.ioResult)
		{
			for (long valid; valid = sock->fOutputBuffer.Valid();)
				sock->fOutputBuffer.FreeBuffer(nil, valid);
			sock->fWriteShutdown = true;
		}
		GUSIPPCSend(sock);
		sock->Wakeup();
	}
}

pascal void GUSIPPCRecvDone(PPCParamBlockPtr pb)
{
	GUSIPPCSocket *sock =
		reinterpret_cast<GUSIPPCSocket *>((char *)pb - offsetof(GUSIPPCSocket, fRecvPB));
	if (sock->fInputBuffer.Locked())
		sock->fInputBuffer.Defer(GUSIRingBuffer::Deferred(GUSIPPCRecvDone), pb);
	else
	{
		PPCReadPBRec &readParam = pb->readParam;
		sock->fInputBuffer.ClearDefer();
		switch (readParam.ioResult)
		{
		case noErr:
			sock->fInputBuffer.ValidBuffer(readParam.bufferPtr, readParam.actualLength);
			GUSIPPCRecv(sock);
			break;
		default:
			sock->fReadShutdown = true;
			break;
		}
		sock->Wakeup();
	}
}

extern "C" void GUSIwithPPCSockets()
{
	GUSISocketDomainRegistry::Instance()->AddFactory(AF_PPC, GUSIPPCFactory::Instance());
}

GUSISocketFactory *GUSIPPCFactory::sInstance = nil;

GUSISocket *GUSIPPCFactory::socket(int, int, int)
{
	return new GUSIPPCSocket;
}

PPCCompUPP GUSIPPCSocket::sSendProc = 0;
PPCCompUPP GUSIPPCSocket::sRecvProc = 0;

PPCCompUPP GUSIPPCSocket::sListenProc = 0;

void GUSIPPCSocket::SetupListener(Listener &listener)
{
	listener.fBusy = false;
}

PPCCompUPP GUSIPPCSocket::sDoneProc = 0;

GUSIPPCSocket::GUSIPPCSocket()
{
	if (!sSendProc)
		sSendProc = NewPPCCompProc(GUSIPPCSendDone);
	if (!sRecvProc)
		sRecvProc = NewPPCCompProc(GUSIPPCRecvDone);
}

GUSIPPCSocket::~GUSIPPCSocket()
{
	switch (fState)
	{
	case Listening:
		shutdown(2);

		fInputBuffer.Lock();
		for (int i = 0; i < fNumListeners; i++)
			if (fListeners[i].fBusy)
			{
				fSendPB.endParam.ioCompletion = sDoneProc;
				fSendPB.endParam.sessRefNum = fListeners[i].fSession;
				PPCEndAsync(&fSendPB.endParam);
				AddContext();
				while (fSendPB.endParam.ioResult == 1)
					GUSIContext::Yield(kGUSIBlock);
				RemoveContext();
				GUSIContext::Raise();
			}

		break;
	case Connected:
		shutdown(2);
		AddContext();
		while (fState == Connected)
			GUSIContext::Yield(kGUSIBlock);
		RemoveContext();
		// Fall through
	case Closing:
		fSendPB.endParam.ioCompletion = sDoneProc;
		PPCEndAsync(&fSendPB.endParam);
		AddContext();
		while (fSendPB.endParam.ioResult == 1)
			GUSIContext::Yield(kGUSIBlock);
		RemoveContext();
		break;
	}
	if (fPortRef && !--fPortRef->fRefCount)
	{
		fSendPB.closeParam.ioCompletion = sDoneProc;
		fSendPB.closeParam.portRefNum = fPortRef->fPort;
		PPCCloseAsync(&fSendPB.closeParam);
		AddContext();
		while (fSendPB.closeParam.ioResult == 1)
			GUSIContext::Yield(kGUSIBlock);
		RemoveContext();
		delete fPortRef;
	}
	GUSIContext::Raise();
}

void GUSIPPCSend(GUSIPPCSocket *sock)
{
	size_t valid = sock->fOutputBuffer.Valid();

	sock->fOutputBuffer.ClearDefer();
	if (!valid)
	{
		if (!sock->fWriteShutdown)
			sock->fOutputBuffer.Defer(GUSIRingBuffer::Deferred(GUSIPPCSend), sock);
		else if (sock->fState == GUSIPPCSocket::Connected)
			sock->fState = GUSIPPCSocket::Closing;
	}
	else
	{
		PPCWritePBRec &writeParam = sock->fSendPB.writeParam;
		valid = min(valid, min((size_t)65535, sock->fOutputBuffer.Size() >> 1));

		writeParam.bufferPtr =
			reinterpret_cast<char *>(sock->fOutputBuffer.ConsumeBuffer(valid));
		writeParam.bufferLength = (Size)valid;
		writeParam.more = false;
		writeParam.userData = 0;
		writeParam.blockCreator = 'GU∑I';
		writeParam.blockType = 'strm';

		PPCWriteAsync(&writeParam);
	}
}

void GUSIPPCRecv(GUSIPPCSocket *sock)
{
	size_t free = sock->fInputBuffer.Free();
	if (!free)
		sock->fInputBuffer.Defer(GUSIRingBuffer::Deferred(GUSIPPCRecv), sock);
	else
	{
		PPCReadPBRec &readParam = sock->fRecvPB.readParam;

		sock->fInputBuffer.ClearDefer();
		free = min(free, min((size_t)65535, sock->fInputBuffer.Size() >> 1));

		readParam.ioCompletion = sock->sRecvProc;
		readParam.bufferPtr =
			reinterpret_cast<char *>(sock->fInputBuffer.ProduceBuffer(free));
		readParam.bufferLength = free;

		PPCReadAsync(&readParam);
	}
}

pascal void GUSIPPCListenDone(PPCParamBlockPtr pb)
{
	GUSIPPCSocket *sock =
		(GUSIPPCSocket *)((char *)pb - offsetof(GUSIPPCSocket, fRecvPB));
	if (!(sock->fAsyncError = pb->informParam.ioResult))
	{
		GUSIPPCSocket::Listener &listener = sock->fListeners[sock->fCurListener];
		listener.fSession = pb->informParam.sessRefNum;
		listener.fBusy = true;
		sock->fCurListener = (sock->fCurListener + 1) % sock->fNumListeners;
	}
	sock->Wakeup();
	if (!sock->fAsyncError)
		if (sock->fInputBuffer.Locked())
			sock->fInputBuffer.Defer(GUSIRingBuffer::Deferred(GUSIPPCListenDone), pb);
		else
		{
			sock->fInputBuffer.ClearDefer();
			GUSIPPCListen(sock);
		}
}

void GUSIPPCListen(GUSIPPCSocket *sock)
{
	if (sock->fReadShutdown)
		return;
	if (sock->fRestartListen = sock->fListeners[sock->fCurListener].fBusy)
		return;
	PPCInformPBRec &informParam = sock->fRecvPB.informParam;
	GUSIPPCSocket::Listener &listener = sock->fListeners[sock->fCurListener];
	informParam.ioCompletion = GUSIPPCSocket::sListenProc;
	informParam.portRefNum = sock->fPortRef->fPort;
	informParam.autoAccept = true;
	informParam.portName = &listener.fPort;
	informParam.locationName = &listener.fLocation;
	informParam.userName = nil;

	PPCInformAsync(&informParam);
}

pascal void GUSIPPCDone(PPCParamBlockPtr pb)
{
	GUSIPPCSocket *sock =
		(GUSIPPCSocket *)((char *)pb - offsetof(GUSIPPCSocket, fSendPB));
	sock->fAsyncError = pb->endParam.ioResult;
	sock->Wakeup();
}
