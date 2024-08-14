/*
	File:		UDialogUtils.cp

	Contains:	Dialog item utilities

	Derived from Apple's Appearance SDK

	$Log: UDialogUtils.cp,v $
	Revision 1.1  1999/04/10 03:52:49  neeri
	Initial Version
	
*/


#include "UDialogUtils.h"

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ GetDialogItemHandle
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Returns the handle to the specified dialog item.
//
Handle
UDialogUtils::GetDialogItemHandle( DialogPtr theDialog, SInt16 item )
{
	SInt16		itemType;
	Handle		itemHand;
	Rect		itemRect;
	
	::GetDialogItem( theDialog, item, &itemType, &itemHand, &itemRect );
	return itemHand;
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ SetDialogItemHandle
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Sets the handle of the specified dialog item to the handle given.
//
void
UDialogUtils::SetDialogItemHandle( DialogPtr theDialog, SInt16 item, Handle handle )
{
	SInt16		itemType;
	Handle		itemHand;
	Rect		itemRect;
	
	::GetDialogItem( theDialog, item, &itemType, &itemHand, &itemRect );
	::SetDialogItem( theDialog, item, itemType, handle, &itemRect );
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ GetDialogItemRect
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Gets the bounding rectangle of the specified dialog item.
//
void
UDialogUtils::GetDialogItemRect( DialogPtr theDialog, SInt16 item, Rect& rect )
{
	SInt16		itemType;
	Handle		itemHand;
	
	::GetDialogItem( theDialog, item, &itemType, &itemHand, &rect );
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ SetDialogItemRect
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Sets the bounding rectangle of the specified dialog item to the given rectangle.
//
void
UDialogUtils::SetDialogItemRect( DialogPtr theDialog, SInt16 item, const Rect& rect )
{
	SInt16		itemType;
	Handle		itemHand;
	Rect		itemRect;
	
	::GetDialogItem( theDialog, item, &itemType, &itemHand, &itemRect );
	::SetDialogItem( theDialog, item, itemType, itemHand, &rect );
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ FlashDialogItem
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	This routine is used to flash a button for the cases when the user presses a key that
//	'clicks' a dialog button. If the item specified is not a button, the routine does nothing.
//
void
UDialogUtils::FlashDialogItem( DialogPtr theDialog, SInt16 item )
{
	ControlHandle	control;
	unsigned long	ticks;
	OSErr			err;
	
	err = ::GetDialogItemAsControl( theDialog, item, &control );
	if ( err == noErr )
	{
		HiliteControl( control, 1 );
		Delay( 8, &ticks );
		HiliteControl( control, 0 );
	}
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ ToggleCheckBox
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	This routine simply toggles a check box.
//
void
UDialogUtils::ToggleCheckBox( DialogPtr dialog, SInt16 item )
{
	SInt16	newState = 0;
	
	if ( GetDialogItemValue( dialog, item ) == 0 )
		newState = 1;
	SetDialogItemValue( dialog, item, newState );
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ SetDialogItemValue
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	This routine sets the value of the given item to the value passed in.
//
void
UDialogUtils::SetDialogItemValue( DialogPtr dialog, SInt16 item, SInt16 value )
{
	ControlHandle	control;
	OSErr			err;
		
	err = ::GetDialogItemAsControl( dialog, item, &control );
	if ( err ) return;
	
	::SetControlValue( control, value );
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ GetDialogItemValue
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	This routine gets the value of the given item and returns it.
//
SInt16
UDialogUtils::GetDialogItemValue( DialogPtr dialog, SInt16 item )
{
	ControlHandle	control;
	OSErr			err;
	
	err = ::GetDialogItemAsControl( dialog, item, &control );
	if ( err ) return 0;
	
	return ::GetControlValue( control );
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ SetDialogItemText
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	This routine sets the text of the given item to the value passed in.
//
void
UDialogUtils::SetDialogItemText( DialogPtr dialog, SInt16 item, StringPtr text )
{
	SInt16		itemType;
	Handle		itemHand;
	Rect		itemRect;
	ControlHandle	root;
	
	if (::GetRootControl(dialog, &root) == noErr)
		::GetDialogItemAsControl( dialog, item, (ControlHandle*)&itemHand );
	else
		::GetDialogItem( dialog, item, &itemType, &itemHand, &itemRect );

	short saveKind = ((WindowPeek)dialog)->windowKind;
	((WindowPeek)dialog)->windowKind = 2;
	::SetDialogItemText( itemHand, text );
	((WindowPeek)dialog)->windowKind = saveKind;
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ GetDialogItemText
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	This routine gets the text of the given item and returns it.
//
void
UDialogUtils::GetDialogItemText( DialogPtr dialog, SInt16 item, StringPtr text )
{
	SInt16		itemType;
	Handle		itemHand;
	Rect		itemRect;
	
	::GetDialogItem( dialog, item, &itemType, &itemHand, &itemRect );
	::GetDialogItemText( itemHand, text );
}


//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ EnableDialogItem
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	This routine enables or disables a specified item. enableIt is true for enabling, false
//	to disable.
//
void
UDialogUtils::EnableDialogItem( DialogPtr dialog, SInt16 item, Boolean enableIt )
{
	ControlHandle		control;
	OSErr				err;
	
	err = ::GetDialogItemAsControl( dialog, item, &control );
	if ( err ) return;
	
	if ( enableIt )
		::ActivateControl( control );
	else
	{
		::DeactivateControl( control );
	}
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ SetFontStyle
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	This routine sets the font style of a dialog item.
//
void
UDialogUtils::SetFontStyle( DialogPtr dialog, SInt16 item, ControlFontStyleRec& style )
{
	ControlHandle 	control;
	OSErr			err;
	
	err = ::GetDialogItemAsControl( dialog, item, &control );
	
	if ( err == noErr )
		::SetControlFontStyle( control, &style );
}
