
#include "GUSIInternal.h"
#include "GUSITimer.h"
#include "GUSIPThread.h"

#include <pthread.h>
#include <sched.h>


struct GUSIPThreadAttr {
	size_t	fStackSize;
	enum {
		detached = 1 << 0
	};
	int		fFlags;
	
	static 	GUSIPThreadAttr sDefault;
	static	pthread_attr_t  sDefaultAttr;
};

GUSIPThreadAttr	GUSIPThreadAttr::sDefault = { 20480, 0 };
pthread_attr_t	GUSIPThreadAttr::sDefaultAttr = &GUSIPThreadAttr::sDefault;

#warning: unhandled macro "definitions[mat]"
#warning: unhandled macro "definitions[mat]"
#warning: unhandled macro "definitions[mat]"
