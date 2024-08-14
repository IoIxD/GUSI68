/*
	File:		AppearanceHelpers.h

	Contains:	Helper routines which wrap around Get/SetControlData

	Derived from Apple's Appearance SDK

	$Log: AppearanceHelpers.h,v $
	Revision 1.1  1999/04/10 03:52:44  neeri
	Initial Version
	
*/


#ifndef _APPEARANCEHELPERS_
#define _APPEARANCEHELPERS_

#include <Appearance.h>
#include <Dialogs.h>
#include <Lists.h>

extern pascal OSErr AppendDialogItemList( DialogPtr dialog, SInt16 ditlID, DITLMethod method )
 THREEWORDINLINE(0x303C, 0x0412, 0xAA68);

extern pascal MenuHandle NewThemeMenu( SInt16 resID, ConstStr255Param title );

extern pascal OSStatus SetPushButtonDefaultState( ControlHandle control, Boolean isDefault );
extern pascal OSStatus GetPushButtonDefaultState( ControlHandle control, Boolean* isDefault );

extern pascal OSStatus GetEditTextText( ControlHandle control, StringPtr text );
extern pascal OSStatus SetEditTextText( ControlHandle control, ConstStr255Param text, Boolean draw );
extern pascal OSStatus SetEditTextKeyFilter( ControlHandle control, ControlKeyFilterUPP filter );
extern pascal OSStatus SetEditTextSelection( ControlHandle control, SInt16 selStart, SInt16 selEnd );
extern pascal OSStatus GetEditTextSelection( ControlHandle control, SInt16* selStart, SInt16* selEnd );
extern pascal OSStatus GetEditTextPasswordText( ControlHandle control, StringPtr password );
extern pascal OSStatus GetEditTextTextHandle( ControlHandle control, TEHandle* textHandle );

extern pascal OSStatus GetStaticTextText( ControlHandle control, StringPtr text );
extern pascal OSStatus SetStaticTextText( ControlHandle control, ConstStr255Param text, Boolean draw );
extern pascal OSStatus GetStaticTextTextHeight( ControlHandle control, SInt16* height );

extern pascal OSStatus	SetProgressIndicatorState( ControlHandle control, Boolean isDeterminate );
extern pascal OSStatus	GetProgressIndicatorState( ControlHandle control, Boolean* isDeterminate );

extern pascal OSStatus SetIconControlTransform( ControlHandle control, IconTransformType transform );
extern pascal OSStatus SetIconControlAlignment( ControlHandle control, IconAlignmentType align );

extern pascal OSStatus GetListBoxListHandle( ControlHandle control, ListHandle* list );
extern pascal OSStatus SetListBoxKeyFilter( ControlHandle control, ControlKeyFilterUPP filter );

extern pascal OSStatus	SetClockDateTime( ControlHandle control, const LongDateRec* time );
extern pascal OSStatus	GetClockDateTime( ControlHandle control, LongDateRec* time );

extern pascal OSErr GetBevelButtonMenuValue(ControlHandle button, SInt16 *value);
extern pascal OSErr SetBevelButtonMenuValue(ControlHandle button, SInt16 value);
extern pascal OSErr GetBevelButtonMenuHandle(ControlHandle button, MenuHandle *handle);
extern pascal OSErr GetBevelButtonContentInfo(ControlHandle button, ControlButtonContentInfoPtr content);
extern pascal OSErr SetBevelButtonContentInfo(ControlHandle button, ControlButtonContentInfoPtr content);
extern pascal OSErr SetBevelButtonTransform(ControlHandle button, IconTransformType transform);
extern pascal OSErr SetBevelButtonGraphicAlignment(ControlHandle button, ControlButtonGraphicAlignment align, SInt16 hOffset, SInt16 vOffset);
extern pascal OSErr SetBevelButtonTextAlignment(ControlHandle button, ControlButtonTextAlignment align, SInt16 hOffset);
extern pascal OSErr SetBevelButtonTextPlacement(ControlHandle button, ControlButtonTextPlacement where);

extern pascal OSErr GetImageWellContentInfo(ControlHandle button, ControlButtonContentInfoPtr content);
extern pascal OSErr SetImageWellContentInfo(ControlHandle button, ControlButtonContentInfoPtr content);
extern pascal OSErr SetImageWellTransform(ControlHandle button, IconTransformType transform);

extern pascal OSErr GetTabContentRect(ControlHandle tabControl, Rect *contentRect);
extern pascal OSErr SetTabEnabled(ControlHandle tabControl, SInt16 tabToHilite, Boolean enabled);

extern pascal OSErr SetDisclosureTriangleLastValue(ControlHandle tabControl, SInt16 value);

#endif // _APPEARANCEHELPERS_
