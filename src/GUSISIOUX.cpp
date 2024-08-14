
#include "GUSIInternal.h"
#include "GUSISIOUX.h"
#include "GUSIDevice.h"
#include "GUSIDescriptor.h"
#include "GUSIBasics.h"
#include "GUSIDiag.h"

#include <LowMem.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <console.h>


class GUSISIOUXSocket : public GUSISocket {
public:
	~GUSISIOUXSocket();
	
	
ssize_t	read(const GUSIScatterer & buffer);

ssize_t write(const GUSIGatherer & buffer);

virtual int	ioctl(unsigned int request, va_list arg);

virtual int	fstat(struct stat * buf);

virtual int	isatty();

bool select(bool * canRead, bool * canWrite, bool *);


	static GUSISIOUXSocket *	Instance();
private:
	static GUSISIOUXSocket *	sInstance;
	
	GUSISIOUXSocket();
};


class GUSISIOUXDevice : public GUSIDevice {
public:
	static GUSISIOUXDevice *	Instance();

	
virtual bool Want(GUSIFileToken & file);

virtual GUSISocket * open(GUSIFileToken &, int flags);

private:
	GUSISIOUXDevice() 								{}
	
	static GUSISIOUXDevice *	sInstance;
};


GUSISIOUXSocket * GUSISIOUXSocket::sInstance;

GUSISIOUXSocket * GUSISIOUXSocket::Instance()
{
	if (!sInstance)
		if (sInstance = new GUSISIOUXSocket)
			sInstance->AddReference();

	return sInstance;
}

// This declaration lies about the return type
extern "C" void SIOUXHandleOneEvent(EventRecord *userevent);

GUSISIOUXSocket::GUSISIOUXSocket() 
{
	InstallConsole(0);
	GUSISetHook(GUSI_EventHook+nullEvent, 	(GUSIHook)SIOUXHandleOneEvent);
	GUSISetHook(GUSI_EventHook+mouseDown, 	(GUSIHook)SIOUXHandleOneEvent);
	GUSISetHook(GUSI_EventHook+mouseUp, 	(GUSIHook)SIOUXHandleOneEvent);
	GUSISetHook(GUSI_EventHook+updateEvt, 	(GUSIHook)SIOUXHandleOneEvent);
	GUSISetHook(GUSI_EventHook+diskEvt, 	(GUSIHook)SIOUXHandleOneEvent);
	GUSISetHook(GUSI_EventHook+activateEvt, (GUSIHook)SIOUXHandleOneEvent);
	GUSISetHook(GUSI_EventHook+osEvt, 		(GUSIHook)SIOUXHandleOneEvent);
}
GUSISIOUXSocket::~GUSISIOUXSocket()
{
	RemoveConsole();
}


GUSISIOUXDevice * GUSISIOUXDevice::sInstance;


void GUSISetupConsoleDescriptors()
{
	GUSIDescriptorTable * table = GUSIDescriptorTable::Instance();
	GUSISIOUXSocket *     SIOUX = GUSISIOUXSocket::Instance();
	
	table->InstallSocket(SIOUX);
	table->InstallSocket(SIOUX);
	table->InstallSocket(SIOUX);
}

