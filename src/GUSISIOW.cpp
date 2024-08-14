
#include "GUSIInternal.h"
#include "GUSISIOW.h"
#include "GUSIBasics.h"
#include "GUSIDescriptor.h"

#include <Events.h>
#include <Windows.h>

extern "C" {

void _DoActivate(WindowPtr win, int activate);

void _DoIdle();

void _DoUpdate(WindowPtr win);

}

#warning: unhandled macro "definitions[mat]"

static void GUSISetupSIOW()
{
	GUSISetHook(GUSI_EventHook+updateEvt,	(GUSIHook)GUSISIOWUpdate);
	GUSISetHook(GUSI_EventHook+activateEvt,	(GUSIHook)GUSISIOWActivate);
	GUSISetHook(GUSI_EventHook+osEvt,		(GUSIHook)GUSISIOWSusRes);
}

void GUSIDefaultSetupConsole()
{
	GUSISetupConsoleDescriptors();
	GUSISetupSIOW();
	GUSISetupConsoleStdio();
}

