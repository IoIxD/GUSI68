
#include "GUSIInternal.h"
#include "GUSIMTUdp.h"
#include "GUSIMTInet.h"
#include "GUSIMTNetDB.h"
#include "GUSIInet.h"
#include "GUSIDiag.h"
#include "GUSISocketMixins.h"

#include <errno.h>

#include <algorithm>

#include <Devices.h>

GUSI_USING_STD_NAMESPACE


class GUSIMTUdpSocket : public GUSIMTInetSocket, public GUSISMProcess {
public:
	GUSIMTUdpSocket();
	~GUSIMTUdpSocket();
	
virtual int connect(void * address, socklen_t addrlen);

virtual ssize_t recvfrom(const GUSIScatterer & buffer, int, void * from, socklen_t * fromlen);

virtual ssize_t sendto(const GUSIGatherer & buffer, int flags, const void * to, socklen_t addrlen);

virtual bool	select(bool * canRead, bool * canWrite, bool * exception);

virtual int shutdown(int how);

private:
	
UDPiopb						fSendPB;
MidiWDS						fSendWDS;
UDPiopb						fRecvPB;
friend void 				GUSIMTUSend(GUSIMTUdpSocket * sock);
friend void 				GUSIMTURecv(GUSIMTUdpSocket * sock);
friend void 				GUSIMTUSendDone(UDPiopb * pb);
friend void 				GUSIMTURecvDone(UDPiopb * pb);
static UDPIOCompletionUPP	sSendProc;
static UDPIOCompletionUPP	sRecvProc;

int CreateStream();

};


void GUSIMTUSendDone(UDPiopb * pb)
{
	GUSIMTUdpSocket * sock = 
		reinterpret_cast<GUSIMTUdpSocket *>((char *)pb-offsetof(GUSIMTUdpSocket, fSendPB));
	GUSIProcess::A5Saver saveA5(sock->Process());	
	if (sock->fOutputBuffer.Locked())
		sock->fOutputBuffer.Defer(GUSIRingBuffer::Deferred(GUSIMTUSendDone), pb);
	else {
		sock->fOutputBuffer.ClearDefer();
		sock->fOutputBuffer.FreeBuffer(sock->fSendWDS.fDataPtr, sock->fSendWDS.fLength);
		switch (sock->fSendPB.ioResult) {
		case noErr:
			break; /* Everything ok */
		case insufficientResources:
			break; /* Queue overflow, discard packet but proceed */
		default:
			sock->SetAsyncMacError(sock->fSendPB.ioResult);
			for (long valid; valid = sock->fOutputBuffer.Valid(); )
				sock->fOutputBuffer.FreeBuffer(nil, valid);
			sock->fWriteShutdown 	= true;
		}
		GUSIMTUSend(sock);
		sock->Wakeup();
	}
}

struct GUSIUDPHeader {
	in_addr_t	fPeerAddr;
	in_port_t	fPeerPort;
	uint16_t	fLength;
};

void GUSIMTUSend(GUSIMTUdpSocket * sock)
{
	sock->fOutputBuffer.ClearDefer();
	if (!sock->fOutputBuffer.Valid() || sock->fOutputBuffer.Locked()) {
		if (!sock->fWriteShutdown)
			sock->fOutputBuffer.Defer(GUSIRingBuffer::Deferred(GUSIMTUSend), sock);
		else
			sock->fState = GUSISMState::Closing;
	} else {
		sock->fOutputBuffer.ClearDefer();
		
		GUSIUDPHeader	header;
		size_t			len = sizeof(GUSIUDPHeader);
		
		sock->fOutputBuffer.Consume(&header, len);
		
		
len						= header.fLength;
sock->fSendWDS.fDataPtr	= 
	static_cast<Ptr>(sock->fOutputBuffer.ConsumeBuffer(len));
sock->fSendWDS.fLength	= (u_short) len;
if (len < header.fLength) {
	len 						= header.fLength - len;
	sock->fSendWDS.fDataPtr2	= 
		static_cast<Ptr>(sock->fOutputBuffer.ConsumeBuffer(len));
	sock->fSendWDS.fLength2		= (u_short) len;
} else 
	sock->fSendWDS.fLength2		= 0;

		
		sock->fSendPB.ioCompletion				= sock->sSendProc;
		sock->fSendPB.csCode					= UDPWrite;
		sock->fSendPB.csParam.send.remoteHost	= header.fPeerAddr;
		sock->fSendPB.csParam.send.remotePort	= header.fPeerPort;
		sock->fSendPB.csParam.send.wdsPtr 		= &sock->fSendWDS;
		sock->fSendPB.csParam.send.checkSum 	= true;
		sock->fSendPB.csParam.send.sendLength 	= 0;
			
		PBControlAsync(ParmBlkPtr(&sock->fSendPB));		
	}
}

void GUSIMTURecvDone(UDPiopb * pb)
{
	GUSIMTUdpSocket * sock = 
		reinterpret_cast<GUSIMTUdpSocket *>((char *)pb-offsetof(GUSIMTUdpSocket, fRecvPB));
	GUSIProcess::A5Saver saveA5(sock->Process());	
	if (sock->fInputBuffer.Locked())
		sock->fInputBuffer.Defer(GUSIRingBuffer::Deferred(GUSIMTURecvDone), pb);
	else {
		sock->fInputBuffer.ClearDefer();
		switch (sock->fRecvPB.ioResult) {
		case noErr: 
			
if (sock->fRecvPB.csCode == UDPRead) {
	long needed = sock->fRecvPB.csParam.receive.rcvBuffLen+8;
	if (sock->fInputBuffer.Size() < needed)
		; // Drop
	else if (sock->fInputBuffer.Free() < needed) {
		sock->fInputBuffer.Defer(GUSIRingBuffer::Deferred(GUSIMTURecvDone), pb);
		return;
	} else if (sock->fPeerAddr.sin_port &&
		(  sock->fRecvPB.csParam.receive.remoteHost != sock->fPeerAddr.sin_addr.s_addr
		|| sock->fRecvPB.csParam.receive.remotePort != sock->fPeerAddr.sin_port
		)
	) {
		; // Drop
	} else {
		GUSIUDPHeader	header;
		size_t			len    = sizeof(GUSIUDPHeader);
		
		header.fPeerAddr = sock->fRecvPB.csParam.receive.remoteHost;
		header.fPeerPort = sock->fRecvPB.csParam.receive.remotePort;
		header.fLength   = sock->fRecvPB.csParam.receive.rcvBuffLen;
		
		sock->fInputBuffer.Produce(&header, len);
		len = header.fLength;
		sock->fInputBuffer.Produce(
			sock->fRecvPB.csParam.receive.rcvBuff, len);
	}
	sock->fRecvPB.csCode 		= UDPBfrReturn;
	PBControlAsync(ParmBlkPtr(&sock->fRecvPB));
	
	return;
}

			// Fall through
		case commandTimeout:
			GUSIMTURecv(sock);
			break;
		default:
			sock->SetAsyncMacError(sock->fRecvPB.ioResult);
			sock->fReadShutdown = true;
			break;
		}
		sock->Wakeup();
	}
}

void GUSIMTURecv(GUSIMTUdpSocket * sock)
{
	sock->fRecvPB.ioCompletion						= sock->sRecvProc;
	sock->fRecvPB.csCode 							= UDPRead;
	sock->fRecvPB.csParam.receive.timeOut			= 0; 
	sock->fRecvPB.csParam.receive.secondTimeStamp	= 0; 
		
	PBControlAsync(ParmBlkPtr(&sock->fRecvPB));		
}


UDPIOCompletionUPP	GUSIMTUdpSocket::sSendProc	= 0;
UDPIOCompletionUPP	GUSIMTUdpSocket::sRecvProc	= 0;

int GUSIMTUdpSocket::CreateStream()
{
	fSendPB.ioCompletion				= nil;
	fSendPB.csCode 						= UDPCreate;
	fSendPB.csParam.create.rcvBuff 		= (char *)NewPtr(4096);
	fSendPB.csParam.create.rcvBuffLen 	= 4096;
	fSendPB.csParam.create.notifyProc 	= nil;
	fSendPB.csParam.create.userDataPtr	= nil;
	fSendPB.csParam.create.localPort	= fSockAddr.sin_port;
	
	PBControlSync(ParmBlkPtr(&fSendPB));
	
	if (fSendPB.ioResult)
		return GUSISetMacError(fSendPB.ioResult);
	
	fState				= Connected;
	fStream				= fRecvPB.udpStream	= fSendPB.udpStream;
	fSockAddr.sin_port	= fSendPB.csParam.create.localPort;
	
	GUSIMTUSend(this);
	GUSIMTURecv(this);
	
	return 0;
}

GUSIMTUdpSocket::GUSIMTUdpSocket()
{
	
fSendPB.ioCRefNum 							= GUSIMTInetSocket::Driver();
fRecvPB.ioCRefNum 							= GUSIMTInetSocket::Driver();

}

GUSIMTUdpSocket::~GUSIMTUdpSocket()
{
	if (fStream) {
		UDPiopb pb;
		
		pb.ioCRefNum	= GUSIMTInetSocket::Driver();
		pb.csCode 		= UDPRelease;
		pb.udpStream	= fStream;
	
		if (fState != Closing) {
			shutdown(2);
		
			AddContext();
			while (fState != Closing)
				GUSIContext::Yield(kGUSIBlock);
			RemoveContext();
			GUSIContext::Raise();
		}
		
		if (PBControlSync(ParmBlkPtr(&pb)))
			return;

		DisposePtr(pb.csParam.create.rcvBuff); /* there is no release pb */
	}
}


GUSISocketFactory * GUSIMTUdpFactory::instance = nil;

GUSISocket * GUSIMTUdpFactory::socket(int, int, int)
{
	return new GUSIMTUdpSocket();
}

void GUSIwithMTUdpSockets()
{
	gGUSIInetFactories.AddFactory(SOCK_DGRAM, 0, GUSIMTUdpFactory::Instance());
	GUSIMTNetDB::Instantiate();
}

