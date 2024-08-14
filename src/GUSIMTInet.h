
#ifndef _GUSIMTInet_
#define _GUSIMTInet_

#ifdef GUSI_SOURCE

#include <sys/cdefs.h>

__BEGIN_DECLS

void GUSIwithMTInetSockets();

__END_DECLS

#ifdef GUSI_INTERNAL

#include "GUSISocket.h"
#include "GUSISocketMixins.h"

#include <netinet/in.h>
#include <MacTCP.h>


class GUSIMTInetSocket : 
	public 		GUSISocket, 
	protected 	GUSISMBlocking,
	protected	GUSISMState,
	protected	GUSISMInputBuffer,
	protected 	GUSISMOutputBuffer,
	protected	GUSISMAsyncError
{
public:
	GUSIMTInetSocket();
	
virtual int bind(void * addr, socklen_t namelen);

virtual int getsockname(void * addr, socklen_t * namelen);
virtual int getpeername(void * addr, socklen_t * namelen);

virtual int shutdown(int how);

virtual int fcntl(int cmd, va_list arg);

virtual int ioctl(unsigned int request, va_list arg);

virtual int getsockopt(int level, int optname, void *optval, socklen_t * optlen);

virtual int setsockopt(int level, int optname, void *optval, socklen_t optlen);

virtual bool Supports(ConfigOption config);

	
#if PRAGMA_STRUCT_ALIGN
	#pragma options align=mac68k
#endif
class MiniWDS {
public:
	u_short	fLength;
	char * 	fDataPtr;
	u_short fZero;

	MiniWDS() : fZero(0)		{}
	Ptr operator &()			{	return (Ptr)this;	}
};
class MidiWDS {
public:
	u_short	fLength;
	char * 	fDataPtr;
	u_short	fLength2;
	char * 	fDataPtr2;
	u_short fZero;

	MidiWDS() : fZero(0)		{}
	Ptr operator &()			{	return (Ptr)this;	}
};
#if PRAGMA_STRUCT_ALIGN
	#pragma options align=reset
#endif

	
static short	Driver();
static u_long	HostAddr();

protected:
	
StreamPtr	fStream;
sockaddr_in	fSockAddr;
sockaddr_in	fPeerAddr;

static short	sDrvrRefNum;
static OSErr	sDrvrState;
static u_long	sHostAddress;

};


#endif /* GUSI_INTERNAL */

#endif /* GUSI_SOURCE */

#endif /* _GUSIMTInet_ */
