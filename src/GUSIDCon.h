
#ifndef _GUSIDCon_
#define _GUSIDCon_

#ifdef GUSI_INTERNAL

#include "GUSIDevice.h"

class GUSIDConDevice : public GUSIDevice
{
public:
	static GUSIDConDevice *Instance();
	virtual bool Want(GUSIFileToken &file);
	virtual GUSISocket *open(GUSIFileToken &file, int flags);

protected:
	GUSIDConDevice() {}
	static GUSIDConDevice *sInstance;
};

inline GUSIDConDevice *GUSIDConDevice::Instance()
{
	if (!sInstance)
		sInstance = new GUSIDConDevice;
	return sInstance;
}
#endif /* GUSI_INTERNAL */

#endif /* _GUSIDCon_ */
