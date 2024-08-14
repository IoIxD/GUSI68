
#ifndef _GUSISocketMixins_
#define _GUSISocketMixins_

#ifdef GUSI_INTERNAL

#include "GUSISocket.h"
#include "GUSIBuffer.h"

#include <fcntl.h>
#include <sys/ioctl.h>


class GUSISMBlocking {
public:
	GUSISMBlocking();
	bool	fBlocking;
	bool	DoFcntl(int * result, int cmd, va_list arg);
	bool 	DoIoctl(int * result, unsigned int request, va_list arg);
};


class GUSISMState {
public:
	enum State {
		Unbound,
		Unconnected, 	
		Listening,
		Connecting,
		Connected,
		Closing
	};
	GUSISMState();
	State	fState;
	bool	fReadShutdown;
	bool	fWriteShutdown;
	void	Shutdown(int how);
};


class GUSISMInputBuffer {
public:
	GUSIRingBuffer	fInputBuffer;
	GUSISMInputBuffer();
	bool 			DoGetSockOpt(
						int * result, int level, int optname, 
						void *optval, socklen_t * optlen);
	bool 			DoSetSockOpt(
						int * result, int level, int optname, 
						void *optval, socklen_t optlen);	
	bool 			DoIoctl(int * result, unsigned int request, va_list arg);
};


class GUSISMOutputBuffer {
public:
	GUSIRingBuffer	fOutputBuffer;
	GUSISMOutputBuffer();
	bool 		DoGetSockOpt(
						int * result, int level, int optname, 
						void *optval, socklen_t * optlen);
	bool 		DoSetSockOpt(
						int * result, int level, int optname, 
						void *optval, socklen_t optlen);	
};


class GUSISMAsyncError {
public:
	GUSISMAsyncError();
	int			fAsyncError;
	int			SetAsyncPosixError(int error);
	int			SetAsyncMacError(OSErr error);
	int			GetAsyncError();
	bool 		DoGetSockOpt(
						int * result, int level, int optname, 
						void *optval, socklen_t * optlen);
};


class GUSISMProcess {
public:
	GUSISMProcess();
	
	GUSIProcess * Process(); 
private:
	GUSIProcess *	fProcess;
};



inline GUSISMBlocking::GUSISMBlocking() : fBlocking(true)	{}

inline bool GUSISMBlocking::DoFcntl(int * result, int cmd, va_list arg)
{
	switch(cmd) {
	case F_GETFL : 
		return (*result = fBlocking ? 0: FNDELAY), true;
	case F_SETFL : 
		fBlocking = !(va_arg(arg, int) & FNDELAY);
		
		return (*result = 0), true;
	}
	return false;
}
inline bool GUSISMBlocking::DoIoctl(int * result, unsigned int request, va_list arg)
{
	if (request == FIONBIO) {
		fBlocking = !*va_arg(arg, int *);
		return (*result = 0), true;
	}
	return false;
}


inline GUSISMState::GUSISMState() : 
	fState(Unbound), fReadShutdown(false), fWriteShutdown(false)		{}

inline void GUSISMState::Shutdown(int how)
{
	if (!(how & 1))
		fReadShutdown = true;
	if (how > 0)
		fWriteShutdown = true;
}


inline GUSISMInputBuffer::GUSISMInputBuffer() : fInputBuffer(8192)	{}

inline bool GUSISMInputBuffer::DoGetSockOpt(
					int * result, int level, int optname, 
					void *optval, socklen_t *)
{
	if (level == SOL_SOCKET && optname == SO_RCVBUF) {
		*(int *)optval = (int)fInputBuffer.Size();
		
		return (*result = 0), true;
	}
	return false;
}

inline bool GUSISMInputBuffer::DoSetSockOpt(
					int * result, int level, int optname, 
					void *optval, socklen_t)
{
	if (level == SOL_SOCKET && optname == SO_RCVBUF) {
		fInputBuffer.SwitchBuffer(*(int *)optval);
		
		return (*result = 0), true;
	}
	return false;
}

inline bool GUSISMInputBuffer::DoIoctl(int * result, unsigned int request, va_list arg)
{
	if (request == FIONREAD) {
		*va_arg(arg, long *) = fInputBuffer.Valid();
		return (*result = 0), true;
	}
	return false;
}


inline GUSISMOutputBuffer::GUSISMOutputBuffer() : fOutputBuffer(8192)	{}

inline bool GUSISMOutputBuffer::DoGetSockOpt(
					int * result, int level, int optname, 
					void *optval, socklen_t *)
{
	if (level == SOL_SOCKET && optname == SO_SNDBUF) {
		*(int *)optval = (int)fOutputBuffer.Size();
		
		return (*result = 0), true;
	}
	return false;
}

inline bool GUSISMOutputBuffer::DoSetSockOpt(
					int * result, int level, int optname, 
					void *optval, socklen_t)
{
	if (level == SOL_SOCKET && optname == SO_SNDBUF) {
		fOutputBuffer.SwitchBuffer(*(int *)optval);
		
		return (*result = 0), true;
	}
	return false;
}


inline GUSISMAsyncError::GUSISMAsyncError()
 : fAsyncError(0)
{
}

inline int GUSISMAsyncError::SetAsyncPosixError(int error)
{
	if (error) {
		fAsyncError = error;
		GUSI_MESSAGE(("GUSISMAsyncError::SetAsyncPosixError %d\n", fAsyncError));
		
		return -1;
	}
	return 0;
}
inline int GUSISMAsyncError::GetAsyncError()
{
	int err = fAsyncError;
	fAsyncError = 0;
	return err;
}

inline int GUSISMAsyncError::SetAsyncMacError(OSErr error)
{
	if (error) {
		fAsyncError = GUSIMapMacError(error);
		GUSI_MESSAGE(("GUSISMAsyncError::SetAsyncMacError %d -> %d\n", error, fAsyncError));
		
		return -1;
	}
	return 0;
}

inline bool GUSISMAsyncError::DoGetSockOpt(
						int * result, int level, int optname, 
						void *optval, socklen_t * optlen)
{
	if (level == SOL_SOCKET && optname == SO_ERROR) {
		*(int *)optval 	= GetAsyncError();
		*optlen 		= sizeof(int);
		
		return (*result = 0), true;
	}
	return false;
}


inline GUSISMProcess::GUSISMProcess()
 : fProcess(GUSIProcess::Instance())
{
}

inline GUSIProcess * GUSISMProcess::Process()
{
	return fProcess;
}


#endif /* GUSI_INTERNAL */

#endif /* _GUSISocketMixins_ */
