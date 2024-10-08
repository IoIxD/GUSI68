
#ifndef _GUSINull_
#define _GUSINull_

#ifdef GUSI_INTERNAL

#include "GUSIDevice.h"

class GUSINullDevice : public GUSIDevice
{
public:
	static GUSINullDevice *Instance();
	virtual bool Want(GUSIFileToken &file);
	virtual GUSISocket *open(GUSIFileToken &file, int flags);
	virtual int stat(GUSIFileToken &file, struct stat *buf);
	GUSISocket *open();

protected:
	GUSINullDevice() {}
	static GUSINullDevice *sInstance;
};

inline GUSINullDevice *GUSINullDevice::Instance()
{
	if (!sInstance)
		sInstance = new GUSINullDevice;
	return sInstance;
}
#endif /* GUSI_INTERNAL */

#endif /* _GUSINull_ */
