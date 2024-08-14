/*
	File:		UDialogUtils.h

	Contains:	Dialog item utility routines.

	Derived from Apple's Appearance SDK

	$Log: UDialogUtils.h,v $
	Revision 1.1  1999/04/10 03:52:49  neeri
	Initial Version
	
*/

#ifndef _UDIALOGUTILS_
#define _UDIALOGUTILS_

#include <Appearance.h>
#include <Controls.h>
#include <Dialogs.h>

class UDialogUtils
{
	public:
		static	Handle		GetDialogItemHandle( DialogPtr theDialog, short item );
		static	void		SetDialogItemHandle( DialogPtr theDialog, short item, Handle handle );
		static	void		GetDialogItemRect( DialogPtr theDialog, short item, Rect& rect );
		static	void		SetDialogItemRect( DialogPtr theDialog, short item, const Rect& rect );
		static	void		SetDialogItemValue( DialogPtr dialog, short item, short value );
		static	SInt16		GetDialogItemValue( DialogPtr dialog, short item );
		static	void		SetDialogItemText( DialogPtr dialog, short item, StringPtr text );
		static	void		GetDialogItemText( DialogPtr dialog, short item, StringPtr text );
		static	void		FlashDialogItem( DialogPtr theDialog, short item );
		static	void		ToggleCheckBox( DialogPtr theDialog, short item );
		static	void		EnableDialogItem( DialogPtr dialog, short item, Boolean enableIt );
		static	void		SetFontStyle( DialogPtr dialog, short item, ControlFontStyleRec& style );
};

#endif // _UDIALOGUTILS_
