/*
	File:		AppearanceHelpers.c

	Contains:	Helper routines which wrap around Set/GetControlData.

	Derived from Apple's Appearance SDK

	$Log: AppearanceHelpers.c,v $
	Revision 1.1  1999/06/24 06:43:41  neeri
	Added missing files
	
*/

#include <Appearance.h>
#include <Resources.h>
#include "AppearanceHelpers.h"
#include "Assertions.h"

// Noone likes a mess. Here's some macros to help out.

#define ASSERT_ALIGNMENT( align )	\
	ASSERT( ((align) == kControlBevelButtonAlignCenter) || ((align) == kControlBevelButtonAlignTop)	\
			|| ((align) == kControlBevelButtonAlignBottom) || ((align) == kControlBevelButtonAlignLeft)	\
			|| ((align) == kControlBevelButtonAlignRight) || ((align) == kControlBevelButtonAlignTopLeft) \
			|| ((align) == kControlBevelButtonAlignTopRight) || ((align) == kControlBevelButtonAlignBottomLeft) \
			|| ((align) == kControlBevelButtonAlignBottomRight) || ((align) == kControlBevelButtonAlignSysDirection) )

#define ASSERT_TEXT_ALIGNMENT( align )	\
	ASSERT( ((align) == kControlBevelButtonAlignTextSysDirection) || ((align) == kControlBevelButtonAlignTextCenter)	\
			|| ((align) == kControlBevelButtonAlignTextFlushRight) || ((align) == kControlBevelButtonAlignTextFlushLeft) )

#define ASSERT_TEXT_PLACEMENT( align )	\
	ASSERT( ((align) == kControlBevelButtonPlaceSysDirection) || ((align) == kControlBevelButtonPlaceToRightOfGraphic)	\
			|| ((align) == kControlBevelButtonPlaceToLeftOfGraphic) || ((align) == kControlBevelButtonPlaceBelowGraphic) \
			|| ((align) == kControlBevelButtonPlaceAboveGraphic) || ((align) == kControlBevelButtonPlaceNormally ) )


#define MIN( a, b )		( ( (a) < (b) ) ? (a) : (b) )

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ NewThemeMenu
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Creates a theme-savvy menu on the fly. Since NewMenu assumes the usage of
//	MDEF 0, we need to do a little fancy footwork to get around it. We get the
//	theme MDEF and stuff the menuProc field of the menu with it.
//
pascal MenuHandle
NewThemeMenu( SInt16 menuID, ConstStr255Param menuTitle )
{
	MenuHandle		menu;
	
	menu = NewMenu( menuID, menuTitle );
	if ( menu )
	{
		Handle		themeDefProc = GetResource( 'MDEF', kMenuStdMenuProc );
		
		if ( themeDefProc )
			(**menu).menuProc = themeDefProc;
	}
	return menu;
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ GetEditTextText
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Returns the text from an edit text control.
//
pascal OSStatus
GetEditTextText( ControlHandle control, StringPtr text )
{
	Size		actualSize;
	OSStatus	err;
	
	if ( control == nil )
		return paramErr;
		
	if ( text == nil )
		return paramErr;
		
	err = GetControlData( control, 0, kControlEditTextTextTag, 255, (Ptr)(text + 1), &actualSize );
	if ( err == noErr )
		text[0] = MIN( 255, actualSize );
		
	return err;
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ SetEditTextText
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Sets the text of an edit text control and optionally redraws it.
//
pascal OSStatus
SetEditTextText( ControlHandle control, ConstStr255Param text, Boolean draw )
{
	OSStatus	err;
	
	if ( control == nil )
		return paramErr;

	err = SetControlData( control, 0, kControlEditTextTextTag, text[0], (Ptr)(text+1) );
	if ( (err == noErr) && draw )
		DrawOneControl( control );
	
	return err;
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ SetEditTextKeyFilter
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Sets the text of an edit text control and optionally redraws it.
//
pascal OSStatus
SetEditTextKeyFilter( ControlHandle control, ControlKeyFilterUPP filter )
{
	OSStatus	err;
	
	if ( control == nil )
		return paramErr;
		
	if ( filter == nil )
		return paramErr;

	err = SetControlData( control, 0, kControlKeyFilterTag, sizeof( filter ),
			(Ptr)&filter );
	
	return err;
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ SetEditTextSelection
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Sets the selection of an edit text control and redraws it.
//
pascal OSStatus
SetEditTextSelection( ControlHandle control, SInt16 selStart, SInt16 selEnd )
{
	ControlEditTextSelectionRec	selection;
	OSStatus	err;

	if ( control == nil )
		return paramErr;
		
	selection.selStart = selStart;
	selection.selEnd = selEnd;
	
	err = SetControlData( control, 0, kControlEditTextSelectionTag,
			sizeof( selection ), (Ptr)&selection );
	
	if ( err == noErr )
		DrawOneControl( control );
	
	return err;
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ GetEditTextSelection
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Returns the selection for an edit text control.
//
pascal OSStatus
GetEditTextSelection( ControlHandle control, SInt16* selStart, SInt16* selEnd )
{
	ControlEditTextSelectionRec	selection;
	Size		actualSize;
	OSStatus	err;
	
	if ( control == nil )
		return paramErr;
		
	if ( selStart == nil )
		return paramErr;
		
	if ( selEnd == nil )
		return paramErr;
		
	err = GetControlData( control, 0, kControlEditTextSelectionTag,
			sizeof( selection ), (Ptr)&selection, &actualSize );
		 
	if ( err == noErr )
	{
		*selStart = selection.selStart;
		*selEnd = selection.selEnd;
	}
		
	return err;
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ GetEditTextPasswordText
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Returns the password text for an edit text password control.
//
pascal OSStatus
GetEditTextPasswordText( ControlHandle control, StringPtr text )
{
	Size		actualSize;
	OSStatus	err;
	
	if ( control == nil )
		return paramErr;
		
	if ( text == nil )
		return paramErr;

	err = GetControlData( control, 0, kControlEditTextPasswordTag,
			255, (Ptr)(text+1), &actualSize );
		 
	if ( err == noErr )
		text[0] = MIN( 255, actualSize );
		
	return err;
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ GetStaticTextText
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Returns the text from an edit text control.
//
pascal OSStatus
GetStaticTextText( ControlHandle control, StringPtr text )
{
	Size		actualSize;
	OSStatus	err;
	
	if ( control == nil )
		return paramErr;
		
	if ( text == nil )
		return paramErr;
		
	err = GetControlData( control, 0, kControlStaticTextTextTag, 255, (Ptr)(text + 1), &actualSize );
	if ( err == noErr )
		text[0] = MIN( 255, actualSize );
		
	return err;
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ SetStaticTextText
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Sets the text of an edit text control and optionally redraws it.
//
pascal OSStatus
SetStaticTextText( ControlHandle control, ConstStr255Param text, Boolean draw )
{
	OSStatus	err;
	
	if ( control == nil )
		return paramErr;

	err = SetControlData( control, 0, kControlStaticTextTextTag, text[0], (Ptr)(text+1) );
	if ( (err == noErr) && draw )
		DrawOneControl( control );
	
	return err;
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ GetStaticTextTextHeight
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Returns the actual height of the text, not the control height.
//
pascal OSStatus
GetStaticTextTextHeight( ControlHandle control, SInt16* height )
{
	Size		actualSize;
	OSStatus	err;
	
	if ( control == nil )
		return paramErr;
		
	if ( height == nil )
		return paramErr;
		
	err = GetControlData( control, 0, kControlStaticTextTextHeightTag, sizeof( SInt16 ),
		 (Ptr)height, &actualSize );
		
	return err;
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ SetProgressIndicatorState
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Sets a progress bar to the determinate or indeterminate state.
//
pascal OSStatus
SetProgressIndicatorState( ControlHandle control, Boolean isDeterminate )
{
	OSStatus	err;
	Boolean		state;
	
	if ( control == nil )
		return paramErr;

	state = !isDeterminate;	
	err = SetControlData( control, 0, kControlProgressBarIndeterminateTag, sizeof( state ),
			(Ptr)&state );
	
	return err;
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ GetProgressIndicatorState
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Returns the state of the button's default status.
//
pascal OSStatus
GetProgressIndicatorState( ControlHandle control, Boolean* isDeterminate )
{
	Size		actualSize;
	OSStatus	err;
	Boolean		temp;
	
	if ( control == nil )
		return paramErr;
		
	if ( isDeterminate == nil )
		return paramErr;
		
	err = GetControlData( control, 0, kControlListBoxListHandleTag, sizeof( temp ),
			 (Ptr)&temp, &actualSize );
	
	if ( err == noErr )
		*isDeterminate = !temp;
		
	return err;
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ SetPushButtonDefaultState
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Sets a push button's default flag. This lets the button know whether or not
//	to draw its default ring.
//
pascal OSStatus
SetPushButtonDefaultState( ControlHandle control, Boolean isDefault )
{
	OSStatus	err;
	
	if ( control == nil )
		return paramErr;

	err = SetControlData( control, 0, kControlPushButtonDefaultTag, sizeof( isDefault ),
			(Ptr)&isDefault );
	
	return err;
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ GetPushButtonDefaultState
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Returns the state of the button's default status.
//
pascal OSStatus
GetPushButtonDefaultState( ControlHandle control, Boolean* isDefault )
{
	Size		actualSize;
	OSStatus	err;
	
	if ( control == nil )
		return paramErr;
		
	if ( isDefault == nil )
		return paramErr;
		
	err = GetControlData( control, 0, kControlListBoxListHandleTag, sizeof( Boolean ),
			 (Ptr)isDefault, &actualSize );
		
	return err;
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ GetListBoxListHandle
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Returns the list handle from a list box control.
//
pascal OSStatus
GetListBoxListHandle( ControlHandle control, ListHandle* list )
{
	Size		actualSize;
	OSStatus	err;
	
	if ( control == nil )
		return paramErr;
		
	if ( list == nil )
		return paramErr;
		
	err = GetControlData( control, 0, kControlListBoxListHandleTag, sizeof( ListHandle ),
			 (Ptr)list, &actualSize );
		
	return err;
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ SetListBoxKeyFilter
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Sets the key filter for a list box control.
//
pascal OSStatus
SetListBoxKeyFilter( ControlHandle control, ControlKeyFilterUPP filter )
{
	OSStatus	err;
	
	if ( control == nil )
		return paramErr;
	
	if ( filter == nil )
		return paramErr;

	err = SetControlData( control, 0, kControlKeyFilterTag, sizeof( filter ),
			(Ptr)&filter );
	
	return err;
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ SetIconControlTransform
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Sets the transform for an icon control.
//
pascal OSStatus
SetIconControlTransform( ControlHandle control, IconTransformType transform )
{
	OSStatus	err;
	
	if ( control == nil )
		return paramErr;
	
	err = SetControlData( control, 0, kControlIconTransformTag, sizeof( transform ),
			(Ptr)&transform );
	
	return err;
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ SetIconControlAlignment
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Sets the alignment for an icon control.
//
pascal OSStatus
SetIconControlAlignment( ControlHandle control, IconAlignmentType align )
{
	OSStatus	err;
	
	if ( control == nil )
		return paramErr;
	
	err = SetControlData( control, 0, kControlIconAlignmentTag, sizeof( align ),
			(Ptr)&align );
	
	return err;
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ SetClockDateTime
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Sets the time for a clock control.
//
pascal OSStatus
SetClockDateTime( ControlHandle control, const LongDateRec* time )
{
	OSStatus	err;
	
	if ( control == nil )
		return paramErr;
	
	err = SetControlData( control, 0, kControlClockLongDateTag,
			sizeof( LongDateRec ), (Ptr)time );
	
	return err;
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ GetClockDateTime
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Returns the time from a clock control.
//
pascal OSStatus
GetClockDateTime( ControlHandle control, LongDateRec* time )
{
	Size		actualSize;
	OSStatus	err;
	
	if ( control == nil )
		return paramErr;
		
	if ( time == nil )
		return paramErr;
		
	err = GetControlData( control, 0, kControlClockLongDateTag,
			sizeof( LongDateRec ), (Ptr)time, &actualSize );
		
	return err;
}


// <3>
// The rest of these routines are already in the Appearance Library
// so we don't include these routines if we are compiling for CFM

#if !GENERATINGCFM
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//
// ¥ BEVEL BUTTON ROUTINES
//
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ



//=========================================================================================
//	¥ GetBevelButtonMenuValue														PUBLIC
//=========================================================================================
//	Returns the current menuValue of the bevel button, if any. If the bevel button supports
//	multivalued menus, it will return the last one chosen.
//
pascal OSErr
GetBevelButtonMenuValue( ControlRef button, SInt16* value )
{
	OSErr		err;
	
	ASSERT_GOTO( button != nil, noButton );

	err = GetControlData( button, 0, kControlBevelButtonMenuValueTag, sizeof( SInt16 ),
						(Ptr)value, nil );
	
	return err;

noButton:
	return paramErr;
}

//=========================================================================================
//	¥ SetBevelButtonMenuValue														PUBLIC
//=========================================================================================
//	Sets the current value of the menu.
//
pascal OSErr
SetBevelButtonMenuValue( ControlRef button, SInt16 value )
{
	OSErr		err;
	
	ASSERT_GOTO( button != nil, noButton );

	err = SetControlData( button, 0, kControlBevelButtonMenuValueTag, sizeof( SInt16 ),
						(Ptr)&value );
	
	return err;

noButton:
	return paramErr;
}


//=========================================================================================
//	¥ GetBevelButtonMenuHandle														PUBLIC
//=========================================================================================
//	Returns the current menuHandle of the bevel button, if any.
//
pascal OSErr
GetBevelButtonMenuHandle( ControlRef button, MenuHandle* handle )
{
	OSErr		err;
	
	ASSERT_GOTO( button != nil, noButton );

	err = GetControlData( button, 0, kControlBevelButtonMenuHandleTag, sizeof( MenuHandle ),
						(Ptr)handle, nil );
	
	return err;

noButton:
	return paramErr;
}

//=========================================================================================
//	¥ GetBevelButtonContentInfo													PUBLIC
//=========================================================================================
//	Returns the current type of data we are displaying and the handle to that data.
//
pascal OSErr
GetBevelButtonContentInfo( ControlRef button, const ControlButtonContentInfoPtr info )
{
	OSErr		err;
	
	ASSERT_GOTO( button != nil, noButton );

	err = GetControlData( button, 0, kControlBevelButtonContentTag, sizeof( ControlButtonContentInfo ),
						(Ptr)info, nil );
	
	return err;

noButton:
	return paramErr;
}

//=========================================================================================
//	¥ SetBevelButtonContentInfo													PUBLIC
//=========================================================================================
//	Sets the current content type and data for the button's content. If the current content
//	was created by the button (i.e. resource based) it is disposed.
//
pascal OSErr
SetBevelButtonContentInfo( ControlRef button, ControlButtonContentInfoPtr info )
{
	OSErr		err;
	
	ASSERT_GOTO( button != nil, noButton );

	err = SetControlData( button, 0, kControlBevelButtonContentTag, sizeof( ControlButtonContentInfo ),
				(Ptr)info );

	return err;
	
noButton:
	return paramErr;
}

//=========================================================================================
//	¥ SetBevelButtonTransform														PUBLIC
//=========================================================================================
//	Sets the transform that will be OR'ed into the calculated transform for the icon. This
//	can be used to add a label color or an offline attribute, etc.
//
pascal OSErr
SetBevelButtonTransform( ControlRef button, IconTransformType transform )
{
	OSErr		err;
	
	ASSERT_GOTO( button != nil, noButton );

	err = SetControlData( button, 0, kControlBevelButtonTransformTag, sizeof( transform ), (Ptr)&transform );

	return err;
	
noButton:
	return paramErr;
}

//=========================================================================================
//	¥ SetBevelButtonTextOptions														PUBLIC
//=========================================================================================
//	Sets the current text options.
//
pascal OSErr
SetBevelButtonTextAlignment( ControlRef button, ControlButtonTextAlignment align, SInt16 hOffset )
{
	OSErr		err;
	
	ASSERT_GOTO( button != nil, noButton );

	err = SetControlData( button, 0, kControlBevelButtonTextAlignTag, sizeof( ControlButtonTextAlignment ),
						(Ptr)&align );

	if ( err == noErr )
	{
		err = SetControlData( button, 0, kControlBevelButtonTextOffsetTag, sizeof( SInt16 ),
						(Ptr)&hOffset );
	}
	return err;
	
noButton:
	return paramErr;
}


//=========================================================================================
//	¥ SetBevelButtonGraphicAlignment												PUBLIC
//=========================================================================================
//	Sets the aligment options for the button graphic.
//
pascal OSErr
SetBevelButtonGraphicAlignment( ControlRef button, ControlButtonGraphicAlignment align, SInt16 hOffset, SInt16 vOffset )
{
	OSErr		err;
	Point		offset;
	
	ASSERT_GOTO( button != nil, noButton );

	ASSERT_ALIGNMENT( align );

	
	ASSERT( hOffset >= 0 );
	ASSERT( vOffset >= 0 );
	
	err = SetControlData( button, 0, kControlBevelButtonGraphicAlignTag, sizeof( ControlButtonGraphicAlignment ),
						(Ptr)&align );

	if ( err == noErr )
	{
		offset.h = hOffset;
		offset.v = vOffset;
		
		err = SetControlData( button, 0, kControlBevelButtonGraphicOffsetTag, sizeof( Point ),
						(Ptr)&offset );
	}

	return err;
	
noButton:
	return paramErr;
}

//=========================================================================================
//	¥ SetBevelButtonTextPlacement													PUBLIC
//=========================================================================================
//	Sets the text placement for buttons with a combination of text and a graphic. This
//	routine also sets the alignment to the proper alignment based on the current graphic
//	alignment.
//
pascal OSErr
SetBevelButtonTextPlacement( ControlRef button, ControlButtonTextPlacement where )
{
	OSErr		err;
	
	ASSERT_GOTO( button != nil, noButton );

	err = SetControlData( button, 0, kControlBevelButtonTextPlaceTag, sizeof( ControlButtonTextPlacement ),
				(Ptr)&where );

	return err;
	
noButton:
	return paramErr;
}

//=========================================================================================
//	¥ GetImageWellContentInfo														PUBLIC
//=========================================================================================
//	Returns the current type of data we are displaying and the handle to that data.
//
pascal OSErr
GetImageWellContentInfo( ControlRef button, const ControlButtonContentInfoPtr info )
{
	OSErr		err;
	
	ASSERT_GOTO( button != nil, noButton );

	err = GetControlData( button, 0, kControlImageWellContentTag, sizeof( ControlButtonContentInfo ),
						(Ptr)info, nil );
	
	return err;

noButton:
	return paramErr;
}


//=========================================================================================
//	¥ SetImageWellContentInfo														PUBLIC
//=========================================================================================
//	Sets the current content type and data for the button's content. If the current content
//	was created by the button (i.e. resource based) it is disposed.
//
pascal OSErr
SetImageWellContentInfo( ControlRef button, ControlButtonContentInfoPtr info )
{
	OSErr		err;
	
	ASSERT_GOTO( button != nil, noButton );

	err = SetControlData( button, 0, kControlImageWellContentTag, sizeof( ControlButtonContentInfo ), (Ptr)info );

	return err;
	
noButton:
	return paramErr;
}


//=========================================================================================
//	¥ SetImageWellTransform															PUBLIC
//=========================================================================================
//	Sets the transform that will be OR'ed into the calculated transform for the icon. This
//	can be used to add a label color or an offline attribute, etc.
//
pascal OSErr
SetImageWellTransform( ControlRef button, IconTransformType transform )
{
	OSErr		err;
	
	ASSERT_GOTO( button != nil, noButton );

	err = SetControlData( button, 0, kControlImageWellTransformTag, sizeof( transform ), (Ptr)&transform );

	return err;
	
noButton:
	return paramErr;
}

//=========================================================================================
//	¥ GetTabContentRect																PUBLIC
//=========================================================================================
//	Gets the content area of a tab control.
//
pascal OSErr
GetTabContentRect( ControlRef tabControl, Rect* contentRect )
{
	OSErr		err;
	
	ASSERT_GOTO( tabControl != nil, noButton );

	err = GetControlData( tabControl, 0, kControlTabContentRectTag, sizeof( Rect ),
						(Ptr)contentRect, nil );
	
	return err;

noButton:
	return paramErr;
}

//=========================================================================================
//	¥ SetTabEnabled																	PUBLIC
//=========================================================================================
//	Sets the hilite flag of a particular tab of a tab control.
//
pascal OSErr
SetTabEnabled( ControlRef tabControl, SInt16 tabToHilite, Boolean enabled )
{
	OSErr		err;
	
	ASSERT_GOTO( tabControl != nil, noButton );

	err = SetControlData( tabControl, tabToHilite, kControlTabEnabledFlagTag, sizeof( Boolean ),
						(Ptr)&enabled );
	
	return err;

noButton:
	return paramErr;
}



//=========================================================================================
//	¥ SetDisclosureTriangleLastValue												PUBLIC
//=========================================================================================
//	Sets the 'last value' field of a disclosure triangle. This is useful.
//
pascal OSErr
SetDisclosureTriangleLastValue( ControlRef triangle, SInt16 value )
{
	OSErr		err;
	
	ASSERT_GOTO( triangle != nil, noButton );

	err = SetControlData( triangle, 0, kControlTriangleLastValueTag, sizeof( SInt16 ), (Ptr)&value );
	
	return err;

noButton:
	return paramErr;
}

#endif //#if !GENERATINGCFM

