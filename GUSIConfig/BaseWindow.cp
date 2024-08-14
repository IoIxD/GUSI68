/*
	File:		BaseWindow.cp

	Contains:	Base class for a window.

	Derived from Apple's Appearance SDK

	$Log: BaseWindow.cp,v $
	Revision 1.1  1999/04/10 03:52:45  neeri
	Initial Version
	
*/

#include <MacWindows.h>
#include "BaseWindow.h"

BaseWindow::BaseWindow()
{
	fWindow = nil;
}

BaseWindow::BaseWindow( SInt16 resID )
{
	fWindow = GetNewCWindow( resID, NULL, (WindowRef)-1L );
	((WindowPeek)fWindow)->windowKind = 2000;
	SetWRefCon( fWindow, (long)this );
}

BaseWindow::~BaseWindow()
{
	MenuHandle theMenu;
	
	if ( fWindow ) DisposeWindow( fWindow );

	theMenu = GetMyMenu();
	
	if (theMenu)
	{
 		DeleteMenu( (**theMenu).menuID );
 		DrawMenuBar();
	}
}

Boolean BaseWindow::Close()
{
	delete this;
	
	return true;
}

void
BaseWindow::Idle()
{
}

void
BaseWindow::AdjustMenus()
{
}

void
BaseWindow::HandleMenuSelection( SInt16 menuID, SInt16 itemNo )
{
#pragma unused( menuID, itemNo )
}

void
BaseWindow::Activate( EventRecord& )
{
	MenuHandle theMenu = GetMyMenu();
	
	if ( theMenu )
	{
		InsertMenu( theMenu, 0 );
		DrawMenuBar();
	}
}

void
BaseWindow::Deactivate( EventRecord& )
{
	MenuHandle theMenu = GetMyMenu();

	if ( theMenu )
	{
		DeleteMenu( (**theMenu).menuID );
		InvalMenuBar();
	}
}

void
BaseWindow::Update( EventRecord& )
{
	BeginUpdate( fWindow );
	Draw();
	EndUpdate( fWindow );
}

void
BaseWindow::Draw()
{
}

void
BaseWindow::HandleKeyDown( EventRecord& )
{
}

void
BaseWindow::Resize( SInt16 width, SInt16 height )
{
	SizeWindow( fWindow, width, height, true );
}

void
BaseWindow::HandleClick( EventRecord& event )
{
#pragma unused( event )
}

MenuHandle BaseWindow::GetMyMenu(void)
{
	return(nil);  // if someone hasn't overridden this, they don't have a menu to get!
}

