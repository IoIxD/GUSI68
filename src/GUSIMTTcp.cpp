
#include "GUSIInternal.h"
#include "GUSIMTTcp.h"
#include "GUSIMTInet.h"
#include "GUSIMTNetDB.h"
#include "GUSIInet.h"
#include "GUSIDiag.h"
#include "GUSISocketMixins.h"

#include <errno.h>

#include <Devices.h>

#include <algorithm>

GUSI_USING_STD_NAMESPACE


class GUSIMTTcpSocket : public GUSIMTInetSocket, public GUSISMProcess {
public:
	GUSIMTTcpSocket();
	~GUSIMTTcpSocket();
	
virtual int connect(void * address, socklen_t addrlen);

virtual int listen(int queueLength);

virtual GUSISocket * accept(void *from, socklen_t *fromlen);

virtual ssize_t recvfrom(const GUSIScatterer & buffer, int, void * from, socklen_t * fromlen);

virtual ssize_t sendto(const GUSIGatherer & buffer, int flags, const void * to, socklen_t);

virtual bool	select(bool * canRead, bool * canWrite, bool * exception);

virtual int shutdown(int how);

private:
	GUSIMTTcpSocket ** fSelf;
	
TCPiopb						fSendPB;
MiniWDS						fSendWDS;
TCPiopb						fRecvPB;
friend void 				GUSIMTTSend(GUSIMTTcpSocket * sock);
friend void 				GUSIMTTRecv(GUSIMTTcpSocket * sock);
friend void 				GUSIMTTSendDone(TCPiopb * pb);
friend void 				GUSIMTTRecvDone(TCPiopb * pb);
static TCPIOCompletionUPP	sSendProc;
static TCPIOCompletionUPP	sRecvProc;

friend pascal void GUSIMTTNotify(
					StreamPtr, u_short, GUSIMTTcpSocket **, 
					u_short, struct ICMPReport *);
static TCPNotifyUPP	sNotifyProc;

friend void GUSIMTTConnectDone(TCPiopb * pb);
static TCPIOCompletionUPP sConnectProc;

struct Listener {
	StreamPtr			fTcp;
	GUSIMTTcpSocket **	fRef;
	sockaddr_in			fSockAddr;
	sockaddr_in			fPeerAddr;
	bool				fBusy;
};
Listener *	fListeners;
bool		fRestartListen;
char		fNumListeners;
char		fCurListener;
char		fNextListener;
friend void GUSIMTTListenDone(TCPiopb * pb);
friend void GUSIMTTListen(GUSIMTTcpSocket * sock);
static TCPIOCompletionUPP sListenProc;

StreamPtr CreateStream(GUSIMTTcpSocket ** socketRef);

void SetupListener(Listener & listener);

GUSIMTTcpSocket(Listener & listener);

};


void GUSIMTTSendDone(TCPiopb * pb)
{
	GUSIMTTcpSocket * sock = 
		reinterpret_cast<GUSIMTTcpSocket *>((char *)pb-offsetof(GUSIMTTcpSocket, fSendPB));
	GUSIProcess::A5Saver saveA5(sock->Process());	
	if (sock->fOutputBuffer.Locked())
		sock->fOutputBuffer.Defer(GUSIRingBuffer::Deferred(GUSIMTTSendDone), pb);
	else {
		sock->fOutputBuffer.ClearDefer();
		sock->fOutputBuffer.FreeBuffer(sock->fSendWDS.fDataPtr, sock->fSendWDS.fLength);
		if (sock->SetAsyncMacError(sock->fSendPB.ioResult)) {
			for (long valid; valid = sock->fOutputBuffer.Valid(); )
				sock->fOutputBuffer.FreeBuffer(nil, valid);
			sock->fWriteShutdown 	= true;
		}
		GUSIMTTSend(sock);
		sock->Wakeup();
	}
}

void GUSIMTTSend(GUSIMTTcpSocket * sock)
{
	size_t	valid = sock->fOutputBuffer.Valid();
	
	sock->fOutputBuffer.ClearDefer();
	if (!valid) {
		if (!sock->fWriteShutdown)
			sock->fOutputBuffer.Defer(GUSIRingBuffer::Deferred(GUSIMTTSend), sock);
		else if (sock->fState == GUSIMTTcpSocket::Connected) {
			sock->fState								= GUSISMState::Closing;
			sock->fSendPB.ioCompletion					= nil;
			sock->fSendPB.csCode						= TCPClose;
			sock->fSendPB.csParam.close.validityFlags 	= timeoutValue | timeoutAction;
			sock->fSendPB.csParam.close.ulpTimeoutValue = 60 /* seconds */;
			sock->fSendPB.csParam.close.ulpTimeoutAction= 0 /* 0:abort 1:report */;

			PBControlAsync(ParmBlkPtr(&sock->fSendPB));
		} 
	} else {
		valid = min(valid, min((size_t)65535, sock->fOutputBuffer.Size() >> 1));

		sock->fSendWDS.fDataPtr	= 
			static_cast<Ptr>(sock->fOutputBuffer.ConsumeBuffer(valid));
		sock->fSendWDS.fLength	= (u_short) valid;
		
		sock->fSendPB.ioCompletion					= sock->sSendProc;
		sock->fSendPB.csCode						= TCPSend;
		sock->fSendPB.csParam.send.validityFlags 	= timeoutValue | timeoutAction;
		sock->fSendPB.csParam.send.ulpTimeoutValue 	= 60 /* seconds */;
		sock->fSendPB.csParam.send.ulpTimeoutAction = 0 /* 0:abort 1:report */;
		sock->fSendPB.csParam.send.wdsPtr 			= &sock->fSendWDS;
		sock->fSendPB.csParam.send.sendFree 		= 0;
		sock->fSendPB.csParam.send.sendLength 		= 0;
		sock->fSendPB.csParam.send.urgentFlag		= 0;
		sock->fSendPB.csParam.send.pushFlag			= 
			valid == sock->fOutputBuffer.Valid();
			
		PBControlAsync(ParmBlkPtr(&sock->fSendPB));		
	}
}

void GUSIMTTRecvDone(TCPiopb * pb)
{
	GUSIMTTcpSocket * sock = 
		reinterpret_cast<GUSIMTTcpSocket *>((char *)pb-offsetof(GUSIMTTcpSocket, fRecvPB));
	GUSIProcess::A5Saver saveA5(sock->Process());	
	if (sock->fInputBuffer.Locked())
		sock->fInputBuffer.Defer(GUSIRingBuffer::Deferred(GUSIMTTRecvDone), pb);
	else {
		sock->fInputBuffer.ClearDefer();
		switch (sock->fRecvPB.ioResult) {
		case noErr:
			sock->fInputBuffer.ValidBuffer(
				sock->fRecvPB.csParam.receive.rcvBuff,
				sock->fRecvPB.csParam.receive.rcvBuffLen);
			// Fall through
		case commandTimeout:
			GUSIMTTRecv(sock);
			break;
		default:
			sock->SetAsyncMacError(sock->fRecvPB.ioResult);
		case connectionClosing:
			sock->fReadShutdown = true;
			break;
		}
		sock->Wakeup();
	}
}

void GUSIMTTRecv(GUSIMTTcpSocket * sock)
{
	size_t	free = sock->fInputBuffer.Free();
	if (!free)
		sock->fInputBuffer.Defer(GUSIRingBuffer::Deferred(GUSIMTTRecv), sock);
	else {
		sock->fInputBuffer.ClearDefer();
		free = min(free, min((size_t)65535, sock->fInputBuffer.Size() >> 1));
		
		sock->fRecvPB.ioCompletion				= sock->sRecvProc;
		sock->fRecvPB.csCode 					= TCPRcv;
		sock->fRecvPB.csParam.receive.rcvBuff	= 
			static_cast<Ptr>(sock->fInputBuffer.ProduceBuffer(free));
		sock->fRecvPB.csParam.receive.rcvBuffLen= free;
		sock->fRecvPB.csParam.receive.commandTimeoutValue	= 120; 
		
		PBControlAsync(ParmBlkPtr(&sock->fRecvPB));		
	}
}

pascal void GUSIMTTNotify(
	StreamPtr, 
	u_short eventCode, GUSIMTTcpSocket ** sp, u_short, struct ICMPReport *)
{
	GUSIMTTcpSocket * sock = *sp;
	
	switch (eventCode) {
	case TCPClosing:
		sock->fReadShutdown		= true;
		break;
	case TCPTerminate:
		sock->fReadShutdown		= true;
		sock->fWriteShutdown	= true;
		sock->fState 			= GUSISMState::Unconnected;
		break;
	}
	sock->Wakeup();
}

void GUSIMTTConnectDone(TCPiopb * pb)
{
	GUSIMTTcpSocket * sock = 
		(GUSIMTTcpSocket *)((char *)pb-offsetof(GUSIMTTcpSocket, fSendPB));
	GUSIProcess::A5Saver saveA5(sock->Process());	
	if (!sock->SetAsyncMacError(pb->ioResult)) {
		sock->fSockAddr.sin_family		= AF_INET;
		sock->fSockAddr.sin_addr.s_addr = pb->csParam.open.localHost;
		sock->fSockAddr.sin_port 		= pb->csParam.open.localPort;
		sock->fPeerAddr.sin_family		= AF_INET;
		sock->fPeerAddr.sin_addr.s_addr	= pb->csParam.open.remoteHost;
		sock->fPeerAddr.sin_port 		= pb->csParam.open.remotePort;
		sock->fState 					= GUSISMState::Connected;

		GUSIMTTSend(sock);
		GUSIMTTRecv(sock);
	} else
		sock->fState 					= GUSISMState::Unconnected;
	GUSI_MESSAGE(("Connect %x\n", sock));
	sock->Wakeup();
}

void GUSIMTTListenDone(TCPiopb * pb)
{
	bool allowRestart = true;
	GUSIMTTcpSocket * sock = 
		(GUSIMTTcpSocket *)((char *)pb-offsetof(GUSIMTTcpSocket, fRecvPB));
	GUSIProcess::A5Saver saveA5(sock->Process());	
	switch (pb->ioResult) {
		case commandTimeout:
		case openFailed:
			break;
		default:
			if (!sock->SetAsyncMacError(pb->ioResult)) {
				GUSIMTTcpSocket::Listener & listener = sock->fListeners[sock->fCurListener];
				listener.fSockAddr.sin_family		= AF_INET;
				listener.fSockAddr.sin_addr.s_addr 	= pb->csParam.open.localHost;
				listener.fSockAddr.sin_port 		= pb->csParam.open.localPort;
				listener.fPeerAddr.sin_family		= AF_INET;
				listener.fPeerAddr.sin_addr.s_addr	= pb->csParam.open.remoteHost;
				listener.fPeerAddr.sin_port 		= pb->csParam.open.remotePort;
				listener.fBusy	= true;
				sock->fCurListener = (sock->fCurListener+1) % sock->fNumListeners;
				GUSI_MESSAGE(("Listen %x\n", &listener));
			} else 
				allowRestart = false;
		}
	sock->Wakeup();
	if (allowRestart)
		if (sock->fInputBuffer.Locked())
			sock->fInputBuffer.Defer(GUSIRingBuffer::Deferred(GUSIMTTListenDone), pb);
		else {
			sock->fInputBuffer.ClearDefer();
			GUSIMTTListen(sock);
		}
}

void GUSIMTTListen(GUSIMTTcpSocket * sock)
{
	if (sock->fRestartListen = sock->fListeners[sock->fCurListener].fBusy) 
		return; 
	sock->fRecvPB.tcpStream		= sock->fListeners[sock->fCurListener].fTcp;
	sock->fRecvPB.ioCompletion						= sock->sListenProc;
	sock->fRecvPB.csCode 							= TCPPassiveOpen;
	sock->fRecvPB.csParam.open.validityFlags 		= timeoutValue | timeoutAction;
	sock->fRecvPB.csParam.open.ulpTimeoutValue 		= 300 /* seconds */;
	sock->fRecvPB.csParam.open.ulpTimeoutAction 	= 1 /* 1:abort 0:report */;
	sock->fRecvPB.csParam.open.commandTimeoutValue	= 0 /* infinity */;
	sock->fRecvPB.csParam.open.remoteHost 	= 0;
	sock->fRecvPB.csParam.open.remotePort 	= 0;
	sock->fRecvPB.csParam.open.localHost 	= sock->fSockAddr.sin_addr.s_addr;
	sock->fRecvPB.csParam.open.localPort 	= sock->fSockAddr.sin_port;
	sock->fRecvPB.csParam.open.dontFrag 	= 0;
	sock->fRecvPB.csParam.open.timeToLive 	= 0;
	sock->fRecvPB.csParam.open.security 	= 0;
	sock->fRecvPB.csParam.open.optionCnt 	= 0;
	
	<<Do the [[TCPPassiveOpen]] and pick up port number if necessary>>
}


TCPIOCompletionUPP	GUSIMTTcpSocket::sSendProc	= 0;
TCPIOCompletionUPP	GUSIMTTcpSocket::sRecvProc	= 0;

TCPNotifyUPP	GUSIMTTcpSocket::sNotifyProc	= 0;

TCPIOCompletionUPP	GUSIMTTcpSocket::sConnectProc	= 0;

TCPIOCompletionUPP	GUSIMTTcpSocket::sListenProc	= 0;

StreamPtr GUSIMTTcpSocket::CreateStream(GUSIMTTcpSocket ** socketRef)
{
	fSendPB.ioCompletion				= nil;
	fSendPB.csCode 						= TCPCreate;
	fSendPB.csParam.create.rcvBuff 		= (char *)NewPtr(8192);
	fSendPB.csParam.create.rcvBuffLen 	= 8192;
	fSendPB.csParam.create.notifyProc 	= sNotifyProc;
	fSendPB.csParam.create.userDataPtr	= Ptr(socketRef);
	
	PBControlSync(ParmBlkPtr(&fSendPB));
	
	if (fSendPB.ioResult)
		return nil;
	else 
		return fSendPB.tcpStream;
}

void GUSIMTTcpSocket::SetupListener(Listener & listener)
{
	listener.fRef 	= new (GUSIMTTcpSocket *);
	*listener.fRef	= this;
	listener.fTcp	= CreateStream(listener.fRef);
	listener.fBusy	= false;
}

GUSIMTTcpSocket::GUSIMTTcpSocket()
{
	<<Initialize fields of [[GUSIMTTcpSocket]]>>
}

GUSIMTTcpSocket::~GUSIMTTcpSocket()
{
	TCPiopb pb;
	
	pb.ioCRefNum	= GUSIMTInetSocket::Driver();
	pb.csCode 		= TCPRelease;
	
	if (fState == Listening) {
		<<Shut down listening [[GUSIMTTcpSocket]]>>
	} else if (fStream) {
		pb.tcpStream	= fStream;
		switch (fState) {
		case Connecting:
		case Connected:
			shutdown(2);
		}
		AddContext();
		while (fState > Unconnected) {
			size_t consume = 0x7F000000;
			fInputBuffer.Consume(nil, consume);

			GUSIContext::Yield(kGUSIBlock);
		}
		RemoveContext();
		GUSIContext::Raise();
		
		if (PBControlSync(ParmBlkPtr(&pb)))
			return;

		DisposePtr(pb.csParam.create.rcvBuff); /* there is no release pb */
	}
}


GUSISocketFactory * GUSIMTTcpFactory::instance = nil;

GUSISocket * GUSIMTTcpFactory::socket(int, int, int)
{
	return new GUSIMTTcpSocket();
}

void GUSIwithMTTcpSockets()
{
	gGUSIInetFactories.AddFactory(SOCK_STREAM, 0, GUSIMTTcpFactory::Instance());
	GUSIMTNetDB::Instantiate();
}

