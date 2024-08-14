
#ifndef _GUSISocket_
#define _GUSISocket_

#ifdef GUSI_SOURCE

#include "GUSIBasics.h"
#include "GUSIContext.h"
#include "GUSIContextQueue.h"
#include "GUSIBuffer.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <stdarg.h>

#include <ConditionalMacros.h>
#include <LowMem.h>

#if PRAGMA_STRUCT_ALIGN
#pragma options align=native
#endif


class GUSISocket {
	
public:
	void			AddReference();
	void 			RemoveReference();
	
	virtual void	close();
	void 			CheckClose(UInt32 now = LMGetTicks());
protected:
	GUSISocket();
	virtual			~GUSISocket();
	virtual	bool	Close(UInt32 now = LMGetTicks());
private:
	u_long	fRefCount;

	
public:
	void				Wakeup();
	void				AddContext(GUSIContext * context = nil);
	void				RemoveContext(GUSIContext * context = nil);
	
	class AddContextInScope {
	public:
		AddContextInScope(GUSISocket * sock, GUSIContext *	context = nil) 
			: fSocket(sock), fContext(context)	
								{ fSocket->AddContext(fContext);	}
		~AddContextInScope()	{ fSocket->RemoveContext(fContext);	}
	private:
		GUSISocket *	fSocket;
		GUSIContext *	fContext;
	};
private:
	GUSIContextQueue	fContexts;

	
public:
	void Enqueue(GUSISocket ** queue);
	void Dequeue();
private:
	GUSISocket **	fQueue;
	GUSISocket *	fNextSocket;
	GUSISocket *	fPrevSocket;

	
protected:
	enum ConfigOption {
		kSimpleCalls,		// [[read]], [[write]]
		kSocketCalls,		// [[recvfrom]], [[sendto]]
		kMsgCalls			// [[recvmsg]], [[sendmsg]]
	};
	virtual bool	Supports(ConfigOption config);

public:
	
virtual int	bind(void * name, socklen_t namelen);
virtual int getsockname(void * name, socklen_t * namelen);
virtual int getpeername(void * name, socklen_t * namelen);

	
virtual int listen(int qlen);
virtual GUSISocket * accept(void * address, socklen_t * addrlen);
virtual int connect(void * address, socklen_t addrlen);

	
virtual ssize_t	read(const GUSIScatterer & buffer);
virtual ssize_t write(const GUSIGatherer & buffer);
virtual ssize_t recvfrom(
			const GUSIScatterer & buffer, int flags, void * from, socklen_t * fromlen);
virtual ssize_t sendto(
			const GUSIGatherer & buffer, int flags, const void * to, socklen_t tolen);
virtual ssize_t	recvmsg(msghdr * msg, int flags);
virtual ssize_t sendmsg(const msghdr * msg, int flags);

ssize_t	read(void * buffer, size_t length);
ssize_t write(const void * buffer, size_t length);
ssize_t recvfrom(
			void * buffer, size_t length, int flags, void * from, socklen_t * fromlen);
ssize_t sendto(
			const void * buffer, size_t length, int flags, const void * to, socklen_t tolen);

	
virtual int getsockopt(int level, int optname, void *optval, socklen_t * optlen);
virtual int setsockopt(int level, int optname, void *optval, socklen_t optlen);
virtual int	fcntl(int cmd, va_list arg);
virtual int	ioctl(unsigned int request, va_list arg);
virtual int	isatty();

	
virtual int	fstat(struct stat * buf);
virtual off_t lseek(off_t offset, int whence);
virtual int ftruncate(off_t offset);

	
virtual bool pre_select(bool wantRead, bool wantWrite, bool wantExcept);
virtual bool select(bool * canRead, bool * canWrite, bool * exception);
virtual void post_select(bool wantRead, bool wantWrite, bool wantExcept);

	
virtual int shutdown(int how);

virtual int fsync();

};


#if PRAGMA_STRUCT_ALIGN
#pragma options align=reset
#endif


inline void GUSISocket::AddReference() 
{
	++fRefCount;
}

inline void GUSISocket::RemoveReference()
{
	if (!--fRefCount)
		close();
}


inline void GUSISocket::Wakeup()
{
	fContexts.Wakeup();
}


inline ssize_t	GUSISocket::read(void * buffer, size_t length)
{	
	return read(GUSIScatterer(buffer, length));	
}

inline ssize_t GUSISocket::write(const void * buffer, size_t length)
{	
	return write(GUSIGatherer(buffer, length));	
}

inline ssize_t GUSISocket::recvfrom(
	void * buffer, size_t length, int flags, void * from, socklen_t * fromlen)
{	
	return recvfrom(GUSIScatterer(buffer, length), flags, from, fromlen);	
}

inline ssize_t GUSISocket::sendto(
	const void * buffer, size_t length, int flags, const void * to, socklen_t tolen)
{	
	return sendto(GUSIGatherer(buffer, length), flags, to, tolen);	
}


#endif /* GUSI_SOURCE */

#endif /* _GUSISocket_ */
