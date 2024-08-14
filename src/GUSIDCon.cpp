
#include "GUSIInternal.h"
#include "GUSIDCon.h"
#include "GUSIBasics.h"
#include "GUSIDiag.h"

#include <fcntl.h>
#include <stddef.h>

#include <DCon.h>


class GUSIDConSocket : 
	public GUSISocket
{
	char *	fLog;
public:
	GUSIDConSocket(const char * log);
	~GUSIDConSocket();
	
virtual ssize_t	read(const GUSIScatterer & buffer);

virtual ssize_t write(const GUSIGatherer & buffer);

virtual bool Supports(ConfigOption config);

virtual int	isatty();

};


extern "C" void GUSIwithDConSockets()
{
	GUSIDeviceRegistry::Instance()->AddDevice(GUSIDConDevice::Instance());
}

GUSIDConDevice * GUSIDConDevice::sInstance = nil;

bool GUSIDConDevice::Want(GUSIFileToken & file)
{
	if (!file.IsDevice())
		return false;
	
	const char * path = file.Path();
	
	return file.WhichRequest() == GUSIFileToken::kWillOpen
	 &&	file.StrFragEqual(path+4, "dcon")
	 && (!path[8] || (path[8] == ':' && path[9]));
}

GUSISocket * GUSIDConDevice::open(GUSIFileToken & file, int)
{
	const char * path = file.Path();

	GUSISocket * sock = 
		path[8] ? new GUSIDConSocket(path+9) : new GUSIDConSocket(nil);
	if (!sock)
		GUSISetPosixError(ENOMEM);
	return sock;
}


GUSIDConSocket::GUSIDConSocket(const char * log)
	: fLog(nil)
{
	if (log)
		fLog = strcpy(new char[strlen(log)+1], log);
}
GUSIDConSocket::~GUSIDConSocket()
{
	delete fLog;
}

