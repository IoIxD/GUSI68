/*
	File:		BaseWindow.h

	Contains:	Base class for a window.

	Derived from Apple's Appearance SDK

	$Log: BaseWindow.h,v $
	Revision 1.1  1999/04/10 03:52:45  neeri
	Initial Version
	
*/

#pragma once

#include <Types.h>
#include <Events.h>
#include <Menus.h>

enum
{
	kBaseWindowKind		= 2000
};

class BaseWindow
{
	public:
			BaseWindow();
			BaseWindow( SInt16 resID );
		virtual ~BaseWindow();
		
		virtual void		Activate( EventRecord& event );
		virtual void		Deactivate( EventRecord& event );
		virtual void		Update( EventRecord& event );
		virtual void		Draw();
		virtual void		Resize( SInt16 width, SInt16 height );
		virtual void		HandleClick( EventRecord& event );
		virtual void		AdjustMenus();
		virtual void		HandleMenuSelection( SInt16 menuID, SInt16 itemNo );
		virtual void		Idle();
		virtual void		HandleKeyDown( EventRecord& event );
		virtual Boolean		Close();
		
	protected:
		virtual MenuHandle	GetMyMenu();
		
		WindowPtr			fWindow;
};
