
#include "GUSIInternal.h"
#include "GUSISIOW.h"
#include "GUSIBasics.h"
#include "GUSIDescriptor.h"

#include <Events.h>
#include <Windows.h>

extern "C"
{

	void _DoActivate(WindowPtr win, int activate);

	void _DoIdle();

	void _DoUpdate(WindowPtr win);
}

static void GUSIDoActivate(WindowPtr win, bool activate)
{
	GrafPtr port;
	GetPort(&port);
	if (win)
		SetPort(win);
	_DoActivate(win, activate);
	SetPort(port);
}
static void GUSISIOWUpdate(EventRecord *ev)
{
	_DoUpdate((WindowPtr)ev->message);
}

static void GUSISIOWActivate(EventRecord *ev)
{
	GUSIDoActivate(reinterpret_cast<WindowPtr>(ev->message), ev->modifiers & activeFlag);
}

static void GUSISIOWSusRes(EventRecord *ev)
{
	switch ((ev->message >> 24) & 0xFF)
	{
	case suspendResumeMessage:
		GUSIDoActivate(FrontWindow(), ev->message & resumeFlag);
		break;
	case mouseMovedMessage:
		_DoIdle();
		break;
	}
}

static void GUSISetupSIOW()
{
	GUSISetHook(GUSI_EventHook + updateEvt, (GUSIHook)GUSISIOWUpdate);
	GUSISetHook(GUSI_EventHook + activateEvt, (GUSIHook)GUSISIOWActivate);
	GUSISetHook(GUSI_EventHook + osEvt, (GUSIHook)GUSISIOWSusRes);
}

void GUSIDefaultSetupConsole()
{
	GUSISetupConsoleDescriptors();
	GUSISetupSIOW();
	GUSISetupConsoleStdio();
}
