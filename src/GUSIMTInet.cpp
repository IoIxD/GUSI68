
#include "GUSIInternal.h"
#include "GUSIMTInet.h"
#include "GUSIMTTcp.h"
#include "GUSIMTUdp.h"
#include "GUSIDiag.h"
#include "GUSIFSWrappers.h"

#include <stdlib.h>
#include <errno.h>
#include <algorithm>

#include <Devices.h>

GUSI_USING_STD_NAMESPACE

short GUSIMTInetSocket::Driver()
{
	if (sDrvrState == 1)
		sDrvrState = GUSIFSOpenDriver((unsigned char *)"\p.IPP", &sDrvrRefNum);

	return sDrvrState ? 0 : sDrvrRefNum;
}

u_long GUSIMTInetSocket::HostAddr()
{
	if (!sHostAddress && Driver())
	{
		GUSIIOPBWrapper<GetAddrParamBlock> ga;

		ga->ioCRefNum = Driver();
		ga->csCode = ipctlGetAddr;

		if (!ga.Control())
			sHostAddress = ga->ourAddress;
	}
	return sHostAddress;
}

GUSIMTInetSocket::GUSIMTInetSocket()
	: fStream(nil)
{
	memset(&fSockAddr, 0, sizeof(sockaddr_in));
	fSockAddr.sin_family = AF_INET;

	fPeerAddr = fSockAddr;
}
