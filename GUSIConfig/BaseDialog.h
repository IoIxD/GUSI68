/*
	File:		BaseDialog.h

	Contains:	Base class declaration for a dialog.

	Derived from Apple's Appearance SDK

	$Log: BaseDialog.h,v $
	Revision 1.1  1999/04/10 03:52:45  neeri
	Initial Version
	
*/

#pragma once

#include "BaseWindow.h"

class BaseDialog : public BaseWindow
{
	public:
			BaseDialog();
			BaseDialog( SInt16 resID );
		virtual ~BaseDialog();
		
		virtual void		Update( EventRecord& event );
		virtual void		Deactivate( EventRecord& event );
		virtual void		Activate( EventRecord& event );
		virtual void		HandleClick( EventRecord& event );
		virtual void		HandleKeyDown( EventRecord& event );
		
	protected:
		virtual void		HandleItemHit( SInt16 item );		
};