
#include "GUSIInternal.h"
#include "GUSIOTInet.h"
#include "GUSIOpenTransport.h"
#include "GUSIOTNetDB.h"
#include "GUSIInet.h"
#include "GUSIDiag.h"

#include <stdlib.h>
#include <errno.h>
#include <netinet/in.h>
#include <net/if.h>

class GUSIOTTcpFactory : public GUSIOTStreamFactory
{
public:
	static GUSISocketFactory *Instance();

protected:
	GUSIOTTcpFactory() {}

	GUSISocket *socket(int domain, int type, int protocol);
	virtual GUSIOTStrategy *Strategy(int domain, int type, int protocol);

private:
	static GUSISocketFactory *sInstance;
};

class GUSIOTUdpFactory : public GUSIOTDatagramFactory
{
public:
	static GUSISocketFactory *Instance();

protected:
	GUSIOTUdpFactory() {}

	GUSISocket *socket(int domain, int type, int protocol);
	virtual GUSIOTStrategy *Strategy(int domain, int type, int protocol);

private:
	static GUSISocketFactory *sInstance;
};

class GUSIOTInetStrategy : public GUSIOTStrategy
{
protected:
	virtual int PackAddress(
		const void *address, socklen_t len, TNetbuf &addr, bool non_null);
	virtual int UnpackAddress(const TNetbuf &addr, void *address, socklen_t *len);
};

class GUSIOTMInetOptions
{
protected:
	bool DoGetSockOpt(
		int *result, EndpointRef endpoint, int level, int optname,
		void *optval, socklen_t *optlen);
	bool DoSetSockOpt(
		int *result, EndpointRef endpoint, int level, int optname,
		void *optval, socklen_t optlen);
	bool DoIoctl(int *result, unsigned int request, va_list arg);
};

class GUSIOTTcpStrategy : public GUSIOTInetStrategy
{
protected:
	virtual const char *ConfigPath();
};

class GUSIOTTcpSocket : public GUSIOTStreamSocket, protected GUSIOTMInetOptions
{
public:
	virtual GUSIOTSocket *Clone();

	virtual int getsockopt(int level, int optname, void *optval, socklen_t *optlen);

	virtual int setsockopt(int level, int optname, void *optval, socklen_t optlen);

	virtual int ioctl(unsigned int request, va_list arg);

protected:
	GUSIOTTcpSocket(GUSIOTStrategy *strategy) : GUSIOTStreamSocket(strategy) {}

	friend class GUSIOTTcpFactory;
};

class GUSIOTUdpStrategy : public GUSIOTInetStrategy
{
protected:
	virtual const char *ConfigPath();
};

class GUSIOTUdpSocket : public GUSIOTDatagramSocket, protected GUSIOTMInetOptions
{
public:
	virtual GUSIOTSocket *Clone();

	virtual int getsockopt(int level, int optname, void *optval, socklen_t *optlen);

	virtual int setsockopt(int level, int optname, void *optval, socklen_t optlen);

	virtual int ioctl(unsigned int request, va_list arg);

protected:
	GUSIOTUdpSocket(GUSIOTStrategy *strategy) : GUSIOTDatagramSocket(strategy) {}

	friend class GUSIOTUdpFactory;
};

GUSIOTStrategy *GUSIOTTcpFactory::Strategy(int, int, int)
{
	static GUSIOTStrategy *tcpStrategy = new GUSIOTTcpStrategy;

	return tcpStrategy;
}

GUSISocket *GUSIOTTcpFactory::socket(int domain, int type, int protocol)
{
	GUSIOTStrategy *strategy = Strategy(domain, type, protocol);
	if (Initialize() && strategy)
		return new GUSIOTTcpSocket(strategy);
	else
		return static_cast<GUSISocket *>(0);
}

GUSIOTStrategy *GUSIOTUdpFactory::Strategy(int, int, int)
{
	static GUSIOTStrategy *udpStrategy = new GUSIOTUdpStrategy;

	return udpStrategy;
}

GUSISocket *GUSIOTUdpFactory::socket(int domain, int type, int protocol)
{
	GUSIOTStrategy *strategy = Strategy(domain, type, protocol);
	if (Initialize() && strategy)
		return new GUSIOTUdpSocket(strategy);
	else
		return static_cast<GUSISocket *>(0);
}

int GUSIOTInetStrategy::PackAddress(
	const void *address, socklen_t len, TNetbuf &addr, bool non_null)
{
	const sockaddr_in *name = (const sockaddr_in *)address;
	if (!GUSI_ASSERT_CLIENT(
			len >= sizeof(struct sockaddr_in),
			("PackAddress: address len %d < %d\n", len, sizeof(struct sockaddr_in))))
		return GUSISetPosixError(EINVAL);
	if (!GUSI_ASSERT_CLIENT(
			name->sin_family == AF_INET,
			("PackAddress: family %d != %d\n", name->sin_family, AF_INET)))
		return GUSISetPosixError(EAFNOSUPPORT);
	if (non_null && (!name->sin_addr.s_addr || !name->sin_port))
		return GUSISetPosixError(EADDRNOTAVAIL);
	OTInitInetAddress(
		reinterpret_cast<InetAddress *>(addr.buf),
		name->sin_port, name->sin_addr.s_addr);
	addr.len = 16;

	return 0;
}

int GUSIOTInetStrategy::UnpackAddress(const TNetbuf &addr, void *address, socklen_t *len)
{
	sockaddr_in *name = (sockaddr_in *)address;
	if (!GUSI_ASSERT_CLIENT(
			*len >= sizeof(struct sockaddr_in),
			("UnpackAddress: address len %d < %d\n", len, sizeof(struct sockaddr_in))))
		return GUSISetPosixError(EINVAL);
	const InetAddress *otaddr = reinterpret_cast<InetAddress *>(addr.buf);
	name->sin_family = AF_INET;
	name->sin_port = otaddr->fPort;
	name->sin_addr.s_addr = otaddr->fHost;
	*len = sizeof(struct sockaddr_in);

	return 0;
}

bool GUSIOTMInetOptions::DoGetSockOpt(
	int *result, EndpointRef endpoint, int level, int optname,
	void *optval, socklen_t *optlen)
{
	TOptMgmt optReq;
	UInt8 optBuffer[kOTOptionHeaderSize + 50];
	TOption *opt = (TOption *)optBuffer;
	int len;

	optReq.flags = T_CURRENT;
	optReq.opt.buf = (UInt8 *)optBuffer;

	opt->level = INET_IP;
	opt->name = optname;

	switch (level)
	{
	case SOL_SOCKET:
		switch (optname)
		{
		case SO_REUSEPORT:
			opt->name = SO_REUSEADDR;
			// Fall through
		case SO_REUSEADDR:
		case SO_DONTROUTE:
			len = 4;
			break;
		default:
			goto notSupported;
		}
		break;
	case IPPROTO_IP:
		switch (optname)
		{
		case IP_OPTIONS:
			len = *optlen;
			break;
		case IP_TOS:
		case IP_TTL:
			len = 1;
			break;
		default:
			goto notSupported;
		}
		break;
	default:
		goto notSupported;
	}
	optReq.opt.len = opt->len = kOTOptionHeaderSize + len;
	optReq.opt.maxlen = sizeof(optBuffer);
	if (*result = GUSISetMacError(OTOptionManagement(endpoint, &optReq, &optReq)))
		return true;
	switch (optname)
	{
	case IP_TOS:
	case IP_TTL:
		*reinterpret_cast<int *>(optval) = *reinterpret_cast<char *>(opt->value);
		*optlen = 4;
		break;
	case IP_OPTIONS:
		len = optReq.opt.len;
		// Fall through
	default:
		memcpy(optval, opt->value, len);
		*optlen = len;
		break;
	}

	return true;
notSupported:
	return false;
}

bool GUSIOTMInetOptions::DoSetSockOpt(
	int *result, EndpointRef endpoint, int level, int optname,
	void *optval, socklen_t optlen)
{
	TOptMgmt optReq;
	UInt8 optBuffer[kOTOptionHeaderSize + sizeof(struct linger)];
	TOption *opt = (TOption *)optBuffer;
	t_kpalive kpal = {1, 120};
	int len;
	char val;

	optReq.flags = T_NEGOTIATE;
	optReq.opt.buf = (UInt8 *)optBuffer;

	opt->level = INET_IP;
	opt->name = optname;

	switch (level)
	{
	case SOL_SOCKET:
		switch (optname)
		{
		case SO_REUSEPORT:
			opt->name = SO_REUSEADDR;
			// Fall through
		case SO_REUSEADDR:
		case SO_DONTROUTE:
			len = 4;
			break;
		default:
			goto notSupported;
		}
		break;
	case IPPROTO_IP:
		switch (optname)
		{
		case IP_OPTIONS:
			len = optlen;
			break;
		case IP_TOS:
		case IP_TTL:
			val = *reinterpret_cast<int *>(optval);
			optval = &val;
			len = 1;
			break;
		default:
			goto notSupported;
		}
		break;
	default:
		goto notSupported;
	}
	optReq.opt.len = opt->len = kOTOptionHeaderSize + len;
	optReq.opt.maxlen = sizeof(optBuffer);
	memcpy(opt->value, optval, len);

	*result = GUSISetMacError(OTOptionManagement(endpoint, &optReq, &optReq));

	return true;
notSupported:
	return false;
}

static void SetAddress(sockaddr_in *sa, InetHost addr)
{
	sa->sin_family = AF_INET;
	sa->sin_port = 0;
	sa->sin_addr.s_addr = addr;
}

static void SetInterface(ifreq *ifr, int ifNum, int aliasNum, InetHost addr)
{
	GUSI_sprintf(ifr->ifr_name, (aliasNum ? "ot%d:%d" : "ot%d"), ifNum, aliasNum);
	SetAddress(reinterpret_cast<sockaddr_in *>(&ifr->ifr_addr), addr);
}

static int GetInterfaceList(ifconf *conf)
{
	InetInterfaceInfo info;
	int maxInterfaces = conf->ifc_len / sizeof(ifreq);
	int numInterfaces = 0;
	SInt32 interface = 0;
	ifreq *ifr = conf->ifc_req;

	if (!maxInterfaces)
		return GUSISetPosixError(EINVAL);

	while (!OTInetGetInterfaceInfo(&info, numInterfaces))
	{
		SetInterface(ifr++, numInterfaces, 0, info.fAddress);
		if (++interface == maxInterfaces)
			goto bufferFull;
		if (info.fIPSecondaryCount)
		{
			InetHost *secondaries = new InetHost[info.fIPSecondaryCount];
			OTInetGetSecondaryAddresses(secondaries, &info.fIPSecondaryCount, numInterfaces);
			for (int i = 0; i < info.fIPSecondaryCount; ++i)
			{
				SetInterface(ifr++, numInterfaces, i + 1, secondaries[i]);
				if (++interface == maxInterfaces)
					goto bufferFull;
			}
			delete[] secondaries;
		}
		++numInterfaces;
	}
bufferFull:
	conf->ifc_len = interface * sizeof(ifreq);

	return 0;
}

static int GetInterfaceParam(ifreq *ifr, unsigned int request)
{
	int ifnum = atoi(ifr->ifr_name + 2);
	int ifalias = 0;
	char *alias = strchr(ifr->ifr_name, ':');
	if (alias)
		ifalias = atoi(alias + 1);

	InetInterfaceInfo info;
	if (OTInetGetInterfaceInfo(&info, ifnum) || ifalias > info.fIPSecondaryCount)
		return GUSISetPosixError(ENOENT);
	if (ifalias && request == SIOCGIFADDR)
	{
		InetHost *secondaries = new InetHost[info.fIPSecondaryCount];
		OTInetGetSecondaryAddresses(secondaries, &info.fIPSecondaryCount, ifnum);
		info.fAddress = secondaries[ifalias - 1];
		delete[] secondaries;
	}
	switch (request)
	{
	case SIOCGIFADDR:
		SetAddress(reinterpret_cast<sockaddr_in *>(&ifr->ifr_addr), info.fAddress);
		break;
	case SIOCGIFFLAGS:
		ifr->ifr_flags = IFF_UP | IFF_BROADCAST | IFF_MULTICAST;
		break;
	case SIOCGIFBRDADDR:
		SetAddress(reinterpret_cast<sockaddr_in *>(&ifr->ifr_addr), info.fBroadcastAddr);
		break;
	case SIOCGIFNETMASK:
		SetAddress(reinterpret_cast<sockaddr_in *>(&ifr->ifr_addr), info.fNetmask);
		break;
	}

	return 0;
}

const char *GUSIOTTcpStrategy::ConfigPath()
{
	return kTCPName;
}

GUSIOTSocket *GUSIOTTcpSocket::Clone()
{
	return new GUSIOTTcpSocket(fStrategy);
}
int GUSIOTTcpSocket::getsockopt(int level, int optname, void *optval, socklen_t *optlen)
{
	int result = GUSIOTSocket::getsockopt(level, optname, optval, optlen);

	if (!result || errno != EOPNOTSUPP || GUSIOTMInetOptions::DoGetSockOpt(&result, fEndpoint, level, optname, optval, optlen))
		return result;

	TOptMgmt optReq;
	UInt8 optBuffer[kOTOptionHeaderSize + sizeof(long)];
	TOption *opt = (TOption *)optBuffer;
	int len;

	optReq.flags = T_CURRENT;
	optReq.opt.buf = (UInt8 *)optBuffer;

	opt->level = INET_TCP;
	opt->name = optname;

	switch (level)
	{
	case IPPROTO_TCP:
		switch (optname)
		{
		case TCP_MAXSEG:
		case TCP_NODELAY:
			len = 4;
			break;
		default:
			goto notSupported;
		}
		break;
	default:
		goto notSupported;
	}

	optReq.opt.len = opt->len = kOTOptionHeaderSize + len;
	optReq.opt.maxlen = sizeof(optBuffer);
	if (GUSISetMacError(OTOptionManagement(fEndpoint, &optReq, &optReq)))
		return -1;
	memcpy(optval, opt->value, len);
	*optlen = len;

	return 0;
notSupported:
	GUSI_ASSERT_CLIENT(false, ("getsockopt: illegal request %d\n", optname));

	return GUSISetPosixError(EOPNOTSUPP);
}
int GUSIOTTcpSocket::setsockopt(int level, int optname, void *optval, socklen_t optlen)
{
	int result = GUSIOTSocket::setsockopt(level, optname, optval, optlen);

	if (!result || errno != EOPNOTSUPP || GUSIOTMInetOptions::DoSetSockOpt(&result, fEndpoint, level, optname, optval, optlen))
		return result;

	TOptMgmt optReq;
	UInt8 optBuffer[kOTOptionHeaderSize + sizeof(TIPAddMulticast)];
	TOption *opt = (TOption *)optBuffer;
	int len;

	optReq.flags = T_NEGOTIATE;
	optReq.opt.buf = (UInt8 *)optBuffer;

	opt->level = INET_TCP;
	opt->name = optname;

	switch (level)
	{
	case IPPROTO_TCP:
		switch (optname)
		{
		case TCP_MAXSEG:
		case TCP_NODELAY:
			len = 4;
			break;
		default:
			goto notSupported;
		}
		break;
	default:
		goto notSupported;
	}
	optReq.opt.len = opt->len = kOTOptionHeaderSize + len;
	optReq.opt.maxlen = sizeof(optBuffer);
	memcpy(opt->value, optval, len);

	return GUSISetMacError(OTOptionManagement(fEndpoint, &optReq, &optReq));
notSupported:
	GUSI_ASSERT_CLIENT(false, ("setsockopt: illegal request %d\n", optname));

	return GUSISetPosixError(EOPNOTSUPP);
}
int GUSIOTTcpSocket::ioctl(unsigned int request, va_list arg)
{
	int result;

	if (GUSIOTMInetOptions::DoIoctl(&result, request, arg))
		return result;
	else
		return GUSIOTSocket::ioctl(request, arg);
}
const char *GUSIOTUdpStrategy::ConfigPath()
{
	return kUDPName;
}

GUSIOTSocket *GUSIOTUdpSocket::Clone()
{
	return new GUSIOTUdpSocket(fStrategy);
}
int GUSIOTUdpSocket::getsockopt(int level, int optname, void *optval, socklen_t *optlen)
{
	int result = GUSIOTSocket::getsockopt(level, optname, optval, optlen);

	if (!result || errno != EOPNOTSUPP || GUSIOTMInetOptions::DoGetSockOpt(&result, fEndpoint, level, optname, optval, optlen))
		return result;

	TOptMgmt optReq;
	UInt8 optBuffer[kOTOptionHeaderSize + sizeof(long)];
	TOption *opt = (TOption *)optBuffer;
	int len;

	optReq.flags = T_CURRENT;
	optReq.opt.buf = (UInt8 *)optBuffer;

	opt->name = optname;

	switch (level)
	{
	case SOL_SOCKET:
		opt->level = INET_IP;
		switch (optname)
		{
		case SO_BROADCAST:
			len = 4;
			break;
		default:
			goto notSupported;
		}
		break;
	case IPPROTO_IP:
		opt->level = INET_IP;
		switch (optname)
		{
		case IP_HDRINCL:
		case IP_RCVDSTADDR:
		case IP_MULTICAST_IF:
			len = 4;
			break;
		case IP_MULTICAST_TTL:
		case IP_MULTICAST_LOOP:
			len = 1;
			break;
		default:
			goto notSupported;
		}
		break;
	default:
		goto notSupported;
	}
	optReq.opt.len = opt->len = kOTOptionHeaderSize + len;
	if (GUSISetMacError(OTOptionManagement(fEndpoint, &optReq, &optReq)))
		return -1;
	switch (optname)
	{
	case IP_MULTICAST_TTL:
	case IP_MULTICAST_LOOP:
		*reinterpret_cast<int *>(optval) = *reinterpret_cast<char *>(opt->value);
		*optlen = 4;
		break;
	case IP_HDRINCL:
	case IP_RCVDSTADDR:
	case SO_BROADCAST:
	case IP_MULTICAST_IF:
		memcpy(optval, opt->value, len);
		*optlen = len;
		break;
	}

	return 0;
notSupported:
	GUSI_ASSERT_CLIENT(false, ("getsockopt: illegal request %d\n", optname));

	return GUSISetPosixError(EOPNOTSUPP);
}
int GUSIOTUdpSocket::setsockopt(int level, int optname, void *optval, socklen_t optlen)
{
	int result = GUSIOTSocket::setsockopt(level, optname, optval, optlen);

	if (!result || errno != EOPNOTSUPP || GUSIOTMInetOptions::DoSetSockOpt(&result, fEndpoint, level, optname, optval, optlen))
		return result;

	TOptMgmt optReq;
	UInt8 optBuffer[kOTOptionHeaderSize + sizeof(TIPAddMulticast)];
	TOption *opt = (TOption *)optBuffer;
	char val;

	optReq.flags = T_NEGOTIATE;
	optReq.opt.buf = (UInt8 *)optBuffer;

	opt->level = INET_IP;
	opt->name = optname;

	int len;
	switch (level)
	{
	case SOL_SOCKET:
		switch (optname)
		{
		case SO_BROADCAST:
			len = 4;
			break;
		default:
			goto notSupported;
		}
		break;
	case IPPROTO_IP:
		switch (optname)
		{
		case IP_HDRINCL:
		case IP_RCVDSTADDR:
		case IP_MULTICAST_IF:
			len = 4;
			break;
		case IP_MULTICAST_TTL:
		case IP_MULTICAST_LOOP:
			val = *reinterpret_cast<long *>(optval) != 0;
			optval = &val;
			len = 1;
			break;
		case IP_ADD_MEMBERSHIP:
		case IP_DROP_MEMBERSHIP:
			len = 8;
			break;
		default:
			goto notSupported;
		}
		break;
	}
	optReq.opt.len = opt->len = kOTOptionHeaderSize + len;
	memcpy(opt->value, optval, len);

	return GUSISetMacError(OTOptionManagement(fEndpoint, &optReq, &optReq));
notSupported:
	GUSI_ASSERT_CLIENT(false, ("setsockopt: illegal request %d\n", optname));

	return GUSISetPosixError(EOPNOTSUPP);
}
int GUSIOTUdpSocket::ioctl(unsigned int request, va_list arg)
{
	int result;

	if (GUSIOTMInetOptions::DoIoctl(&result, request, arg))
		return result;
	else
		return GUSIOTSocket::ioctl(request, arg);
}
void GUSIwithOTTcpSockets()
{
	gGUSIInetFactories.AddFactory(SOCK_STREAM, 0, GUSIOTTcpFactory::Instance());
	GUSIOTNetDB::Instantiate();
}

void GUSIwithOTUdpSockets()
{
	gGUSIInetFactories.AddFactory(SOCK_DGRAM, 0, GUSIOTUdpFactory::Instance());
	GUSIOTNetDB::Instantiate();
}

void GUSIwithOTInetSockets()
{
	GUSIwithOTTcpSockets();
	GUSIwithOTUdpSockets();
}
