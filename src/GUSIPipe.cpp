
#include "GUSIInternal.h"
#include "GUSIPipe.h"
#include "GUSIBasics.h"
#include "GUSIBuffer.h"

#include <errno.h>

class GUSIPipeSocket : public GUSISocket
{
public:
	GUSIPipeSocket();
	virtual ~GUSIPipeSocket();

	virtual bool Supports(ConfigOption config);

	virtual ssize_t read(const GUSIScatterer &buffer);

	virtual bool select(bool *canRead, bool *canWrite, bool *exception);

	virtual int shutdown(int how);

	void SetPeer(GUSIPipeSocket *peer);

protected:
	GUSIRingBuffer fBuffer;
	bool fWriteShutdown;
	bool fBlocking;
	GUSIPipeSocket *fPeer;

	ssize_t write(const GUSIGatherer &buffer);
	bool Eof() { return !fPeer || fPeer->fWriteShutdown; }

	bool Shutdown() { return !fPeer || fWriteShutdown; }
	void WakeupPeer();
};

inline void GUSIPipeSocket::SetPeer(GUSIPipeSocket *peer) { fPeer = peer; }

extern "C" void GUSIwithLocalSockets()
{
	GUSISocketDomainRegistry::Instance()->AddFactory(AF_LOCAL, GUSIPipeFactory::Instance());
}

GUSISocketFactory *GUSIPipeFactory::sInstance = nil;

GUSISocket *GUSIPipeFactory::socket(int, int, int)
{
	return GUSISetPosixError(EOPNOTSUPP), static_cast<GUSISocket *>(nil);
}

int GUSIPipeFactory::socketpair(int, int, int, GUSISocket *s[2])
{
	GUSIErrorSaver saveError;
	GUSIPipeSocket *sock[2];

	if (s[0] = sock[0] = new GUSIPipeSocket)
		if (s[1] = sock[1] = new GUSIPipeSocket)
		{
			sock[0]->SetPeer(sock[1]);
			sock[1]->SetPeer(sock[0]);

			return 0;
		}
		else
			s[0]->close();

	if (!errno)
		return GUSISetPosixError(ENOMEM);
	else
		return -1;
}

void GUSIPipeSocket::WakeupPeer()
{
	if (fPeer)
		fPeer->Wakeup();
}

ssize_t GUSIPipeSocket::write(const GUSIGatherer &buffer)
{
	size_t buflen = buffer.Length();
	size_t len;
	size_t offset = 0;

restart:
	if (Shutdown())
		return GUSISetPosixError(EPIPE);
	if (fPeer->fBuffer.Free() < buflen)
	{

		if (!fBlocking && !fPeer->fBuffer.Free())
			return offset ? (int)offset : GUSISetPosixError(EWOULDBLOCK);
		len = buflen;
		fPeer->fBuffer.Produce(buffer, len, offset);
		buflen -= len;
		WakeupPeer();
		bool signal = false;
		AddContext();
		while (!Shutdown() && !fPeer->fBuffer.Free())
		{
			if (signal = GUSIContext::Yield(kGUSIBlock))
				break;
		}
		RemoveContext();
		if (signal)
		{
			GUSIContext::Raise();

			return offset ? (int)offset : GUSISetPosixError(EINTR);
		}
		goto restart;
	}
	fPeer->fBuffer.Produce(buffer, buflen, offset);
	WakeupPeer();

	return (size_t)buffer.Length();
}

GUSIPipeSocket::~GUSIPipeSocket()
{
	if (fPeer)
		fPeer->fPeer = nil;
	WakeupPeer();
}
