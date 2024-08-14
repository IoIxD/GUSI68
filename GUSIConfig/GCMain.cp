/*
	File:		GCMain.cp

	Contains:	Main application code for GUSI Configurator.

	$Log: GCMain.cp,v $
	Revision 1.1  1999/04/10 03:52:48  neeri
	Initial Version
	
*/

#include <Types.h>
#include <Files.h>
#include <Gestalt.h>
#include <Processes.h>
#include <Menus.h>
#include <Fonts.h>
#include <DiskInit.h>
#include <Devices.h>
#include <TextUtils.h>
#include <Resources.h>
#include <Appearance.h>
#include <Navigation.h>

#define GUSI_SOURCE

#include <GUSIFileSpec.h>

#include "GCData.h"
#include "GCDialog.h"

#define	MAXLONG		0X7FFFFFFF

/* ==================================================================================*/
/* ======================= R E S O U R C E   N U M B E R S ==========================*/
/* ==================================================================================*/

enum
{
	kAlertStartupError		= 129
};

enum
{
	kErrorStrings			= 128,
	kWeirdSystemString		= 1,
	kNoAppearanceString		= 2,
	kResourceMissingString	= 3,
	kNoNavigationString		= 4
};

enum
{
	kAboutBoxDialogID		= 5000
};

#define kObjectWindowKind		2000

/* 	The following constants are used to identify menus and their items. The menu IDs
	have an "m" prefix and the item numbers within each menu have an "i" prefix. */

enum
{
	rMenuBar				= 128				/* application's menu bar */
};

enum
{
	mApple					= 128,				/* Apple menu */
	iAbout					= 1
};

enum
{
	mFile					= 129,				/* File menu */
	iNew					= 1,
	iOpen					= 2,
	iClose					= 3,
	iQuit					= 5
};

enum
{
	kHorizZoomKind			= 128,
	kVertZoomKind			= 129
};

enum
{
	kAboutCmd				= 'abou',
	kNewCmd					= 'new ',
	kOpenCmd				= 'open',
	kCloseCmd				= 'clos',
	kQuitCmd				= 'quit'
};

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Prototypes
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ

static void		InitToolbox(void);
static void		MainEventLoop(void);

/* Event handling routines */

static void		HandleEvent(EventRecord *event);
static void		HandleActivate(EventRecord *event);
static void		HandleDiskInsert(EventRecord *event);
static void		HandleKeyPress(EventRecord *event);
static void 	HandleMouseDown(EventRecord *event);
static void		HandleOSEvent(EventRecord *event);
static void		HandleUpdate(EventRecord *event);

static void		AdjustMenus(void);
static void		HandleMenuCommand(long menuResult);

/* AppleEvents handlers */

pascal OSErr DoOpen(const AppleEvent *message, AppleEvent *reply, long refcon);
pascal OSErr DoQuit(const AppleEvent *message, AppleEvent * reply, long refcon);

/* Utility routines */

static Boolean	CloseAnyWindow(WindowPtr window);
static void		DeathAlert(short errorNumber);
static Boolean	IsAppWindow(WindowPtr window);
static Boolean 	IsDAWindow(WindowPtr window);
static Boolean	IsDialogWindow(WindowPtr window);

static Boolean	GetObjectFromWindow( WindowPtr window, BaseWindow** wind );
static void		SetUpFontMenu();
static void		SetUpModifiersMenu();
static void		DoAboutBox();
static void		DoNew();
static void		DoOld();

static void		AutoSizeDialogTest();
static void		SyncVertZoomRects( WindowPtr window );
static void		SyncHorizZoomRects( WindowPtr window );
static OSErr	GetReportFileSpec( FSSpecPtr file );

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Globals
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ


Boolean			gQuit;			/* 	We set this to TRUE when the user selects
									Quit from the File menu. Our main event loop
									exits gQuit is TRUE. */

Boolean			gInBackground;	/*	gInBackground is maintained by our osEvent
									handling routines. Any poart of the program
									can check it to find out if it is currently
									in the background. */

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Macros
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ

#define	HiWrd(aLong)	(((aLong) >> 16) & 0xFFFF)
#define	LoWrd(aLong)	((aLong) & 0xFFFF)

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ main
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Entry point for our program. We initialize the Toolbox, make sure we are
//	running on a sufficiently brawny machine, and put up the menu bar. Finally,
//	we start polling for events and handling then by entering our main event
//	loop.
//	
void main()
{
	/*	If you have stack requirements that differ from the default,
		then you could use SetApplLimit to increase StackSpace at
		this point, before calling MaxApplZone. */
		
	MaxApplZone();						/* 	Expand the heap so code segments load
											at the top */
	InitToolbox();						/*	Initialize the program */
	MainEventLoop();					/* 	Call the main event loop */
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ InitToolbox
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Set up the whole world, including global variables, Toolbox Managers, and
//	menus.
//	
static void
InitToolbox()
{
	Handle			menuBar;
	SInt32			result;
	OSErr			err;
	
	gInBackground = false;
	gQuit = false;
	
	InitGraf((Ptr) &qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(nil);
	InitCursor();
	
	err = Gestalt( gestaltAppearanceAttr, &result );
	if ( err )
		DeathAlert( kNoAppearanceString );
	if (!NavServicesAvailable)
		DeathAlert( kNoNavigationString );
		
	menuBar = GetNewMBar(rMenuBar);
	if ( menuBar == nil )
		DeathAlert( kResourceMissingString );
	
	RegisterAppearanceClient();

	SetMenuBar(menuBar);
	
	DisposeHandle(menuBar);
	AppendResMenu(GetMenuHandle(mApple),'DRVR');
	
	AdjustMenus();

	DrawMenuBar();
	
	AEInstallEventHandler( kCoreEventClass, kAEOpenDocuments,   (AEEventHandlerUPP)DoOpen, 0, false) ;
	AEInstallEventHandler( kCoreEventClass, kAEQuitApplication, (AEEventHandlerUPP)DoQuit, 0, false) ;
	
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ MainEventLoop
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Get events and handle them by calling HandleEvent. On every event, we call
//	idle on the frontmost window, if there is one.
//	
static void
MainEventLoop()
{
	RgnHandle		cursorRgn;
	Boolean			gotEvent;
	EventRecord		event;
	WindowPtr		theWindow;
	BaseWindow*		window;
	
	cursorRgn = nil;
	while(!gQuit) {
		if (WaitNextEvent(everyEvent, &event, MAXLONG, cursorRgn))
			HandleEvent(&event);
		
		theWindow = FrontWindow();
		if (theWindow && GetObjectFromWindow(theWindow, &window))
			window->Idle();			
	}
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ HandleEvent
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Do the right thing for an event. Determine what kind of event it is and
//	call the appropriate routines.
//
static void
HandleEvent(EventRecord *event)
{
	switch (event->what) {
	case mouseDown:
		HandleMouseDown(event);
		break;
		
	case keyDown:
	case autoKey:
		HandleKeyPress(event);
		break;
		
	case activateEvt:
		HandleActivate(event);
		break;
		
	case updateEvt:
		HandleUpdate(event);
		break;
		
	case diskEvt:
		HandleDiskInsert(event);
		break;
		
	case osEvt:
		HandleOSEvent(event);
		break;
		
	case kHighLevelEvent:
		AEProcessAppleEvent(event);
		break;
	}
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ HandleActivate
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	This is called when a window is activated or deactivated. In this sample,
//	the Window Manager's handling of activate and deactivate events is
//	sufficient. Others applications may have TextEdit records, controls, lists,
//	etc., to activate/deactivate.
//
static void
HandleActivate(EventRecord *event)
{
	WindowPtr		theWindow;
	Boolean			becomingActive;
	BaseWindow*		windObj;
	
	theWindow = (WindowPtr)event->message;
	becomingActive = (event->modifiers & activeFlag) != 0;

	if (IsDialogWindow(theWindow))
	{
		DialogRef		dialog;
		SInt16			itemHit;

		DialogSelect(event, &dialog, &itemHit);
	}
	else if (GetObjectFromWindow(theWindow, &windObj))
	{
		if ( becomingActive )
			windObj->Activate(*event);
		else
			windObj->Deactivate(*event);
	}
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ HandleDiskInsert
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Called when we get a disk-inserted event. Check the upper word of the
//	event message; if it's nonzero, then a bad disk was inserted, and it
//	needs to be formatted.
//	
static void
HandleDiskInsert( EventRecord *event )
{
	Point		aPoint = {100, 100};
	
	if ( HiWrd( event->message ) != noErr )
	{
		DIBadMount( aPoint, event->message );
	}
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ HandleKeyPress
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	The user pressed a key, what are you going to do about it?
//
static void
HandleKeyPress( EventRecord *event )
{
	char		key;
	
	key = event->message & charCodeMask;
	if ( event->modifiers & cmdKey )
	{
		AdjustMenus();

		//**************************************************************************//
		//	APPEARANCE ADOPTION ALERT!!												//
		//**************************************************************************//
		// Here we use the new MenuEvent routine instead of menu key. This allows
		// us to handle extended modifier keys for the menu items that use them.
		
		HandleMenuCommand( MenuEvent( event ) );
	}
	else
	{
		WindowPtr 	window = FrontWindow();
		BaseWindow*	object;
		
		if ( GetObjectFromWindow( window, &object ) )
		{
			object->HandleKeyDown( *event );
		}
	}
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ HandleMouseDown
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Called to handle mouse clicks. The user could have clicked anywhere,so
//	let's first find out where by calling FindWindow. That returns a number
//	indicating where in the screen the mouse was clicked. "switch" on that
//	number and call the appropriate routine.
//
static void
HandleMouseDown( EventRecord *event )
{
	long		newSize;
	Rect		growRect;
	WindowPtr	theWindow;
	short		part;
	
	part = FindWindow( event->where, &theWindow );
	
	switch ( part )
	{
	case inMenuBar:
		AdjustMenus();
		HandleMenuCommand( MenuSelect( event->where ) );
		break;
		
	case inSysWindow:
		SystemClick( event, theWindow );
		break;
		
	case inContent:
		if ( theWindow != FrontWindow() )
		{
			SelectWindow( theWindow );
		}
		else
		{
			BaseWindow* 	wind;
			
			if ( GetObjectFromWindow( theWindow, &wind ) )
				wind->HandleClick( *event );
		}
		break;
		
	case inDrag:
		DragWindow( theWindow, event->where, &qd.screenBits.bounds );
		if ( ((WindowPeek)theWindow)->windowKind == kHorizZoomKind )
			SyncHorizZoomRects( theWindow );
		else if ( ((WindowPeek)theWindow)->windowKind == kVertZoomKind )
			SyncVertZoomRects( theWindow );
		break;
		
	case inGrow:
		growRect = qd.screenBits.bounds;
		growRect.top = growRect.left = 80;
		newSize = GrowWindow(theWindow,event->where,&growRect);
		if (newSize != 0)
		{
			BaseWindow*		wind;
			
			if ( GetObjectFromWindow( theWindow, &wind ) )
				wind->Resize( LoWrd(newSize), HiWrd(newSize) );
			else
				SizeWindow( theWindow, LoWrd( newSize ), HiWrd( newSize ), true );
		}
		break;
		
	case inGoAway:
		if (TrackGoAway(theWindow,event->where))
			CloseAnyWindow(theWindow);
		break;
		
	case inZoomIn:
	case inZoomOut:
		if ( TrackBox( theWindow, event->where, part ) )
		{
			SetPort( theWindow );
			EraseRect( &theWindow->portRect );
			ZoomWindow( theWindow, part, true );
			InvalRect( &theWindow->portRect );
		}
		break;
	}
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ HandleOSEvent
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Deal with OSEvents. These are messages that the process manager sends to
//	us. Here, we deal with the suspend and resume message.
//	
static void
HandleOSEvent( EventRecord *event )
{
	switch( (event->message >> 24) & 0x00FF ) {
	case suspendResumeMessage:
			// In our SIZE resource, we say that we are MultiFinder aware.
			// This means that we take on the responsibility of activating
			// and deactivating our own windows on suspend/resume events. */

		gInBackground = (event->message & resumeFlag) == 0;

		WindowPtr		window = FrontWindow();
	
		if ( window )
		{
			BaseWindow*		wind;
			
			if ( GetObjectFromWindow( window, &wind ) )
			{
				if ( gInBackground )
					wind->Deactivate( *event );
				else
					wind->Activate( *event );
			}
		}
		break;
		
	case mouseMovedMessage:
		break;
	}
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ HandleUpdate
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	This is called when an update event is received for a window. It calls
//	DoUpdateWindow to draw the contents of an application window. As an
//	efficiency measure that does not have to be followed, it calls the drawing
//	routine only if the visRgn is nonempty. This will handle situations where
//	calculations for drawing or drawing itself is very time consuming.
//	
static void
HandleUpdate( EventRecord *event )
{
	 WindowPtr			theWindow = (WindowPtr)event->message;
	 BaseWindow*		wind;
	 
	if ( IsDialogWindow( theWindow ) )
	{
		DialogRef		dialog;
		SInt16			itemHit;

		DialogSelect( event, &dialog, &itemHit );
	}
	else if ( GetObjectFromWindow( theWindow, &wind ) )
	{
	 	if ( !EmptyRgn( theWindow->visRgn ) )
	 	{
	 		SetPort( theWindow );
	 		wind->Update( *event );
	 	}
	}
	else
	{
		BeginUpdate( theWindow );
		EndUpdate( theWindow );
	}
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ AdjustMenus
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Enable and disable menus based on the currene state. The user can only 
//	select enabled menu items. We set up all the menu items before calling
//	MenuSelect or MenuKey, since these are the only times that a menu item can
//	be selected. Note that MenuSelect is also the only time the user will see
//	menu items. This approach to deciding what enable/disable state a menu
//	item has has the advantage of concentrating all the decision making in one
//	routine, as opposed to being spread throughout the application. Other
//	application designs may take a different approach that is just as valid.
//	
static void
AdjustMenus()
{
	WindowPtr		theWindow;
	MenuHandle		menu;
	BaseWindow*		wind;
	
	theWindow = FrontWindow();
	
	menu = GetMenuHandle( mFile );
	
	if ( theWindow )
		EnableItem( menu, iClose );
	else
		DisableItem( menu, iClose );

	if ( GetObjectFromWindow( theWindow, &wind ) ) {
		DisableItem( menu, iNew );
		DisableItem( menu, iOpen );
		wind->AdjustMenus();
	} else {
		EnableItem( menu, iNew );
		EnableItem( menu, iOpen );
	}
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ HandleMenuCommand
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	This is called when an item is chosen from the menu bar (after calling
//	MenuSelect or MenuKey). It performs the right operation for each command.
//	It tries to get the command ID of the menu item selected. If it can't, or
//	the command is unknown, we pass it on to the front window, if any. We
//	also special case the Apple menu items.
//	
static void
HandleMenuCommand( long menuResult )
{
	short		menuID;
	short		menuItem;
	Str255		daName;
	UInt32		command;
	Boolean		handled;
	OSErr		err;
	
	menuID = HiWrd( menuResult );
	menuItem = LoWrd( menuResult );
	
	err = GetMenuItemCommandID( GetMenuHandle( menuID ), menuItem, &command );
	
	handled = false;
	
	if ( err || command == 0 )
	{
		if ( menuID == mApple )
		{
			GetMenuItemText( GetMenuHandle( mApple ), menuItem, daName );
			OpenDeskAcc( daName );
			handled = true;
		}
	}
	else
	{
		handled = true;
		
		switch( command )
		{
			case kAboutCmd:
				DoAboutBox();
				break;
			
			case kNewCmd:
				DoNew();
				break;
			
			case kOpenCmd:
				DoOld();
				break;
				
			case kCloseCmd:
				if ( FrontWindow() )
					CloseAnyWindow( FrontWindow() );
				break;
			
			case kQuitCmd:
				DoQuit(nil, nil, 0);
				break;
			
			default:
				handled = false;
				break;
					
		}
	}
	
	if ( !handled )
	{
		WindowPtr 	frontWindow = FrontWindow();
		BaseWindow* wind;
		
		if ( frontWindow )
		{
			if ( GetObjectFromWindow( frontWindow, &wind ) )
			{
				wind->HandleMenuSelection( menuID, menuItem );
			}
		}
	}

	HiliteMenu(0);
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ CloseAnyWindow
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Close the given window in a manner appropriate for that window. If the
//	window belongs to a DA, we call CloseDeakAcc. For dialogs, we simply hide
//	the window. If we had any document windows, we would probably call either
//	DisposeWindow or CloseWindow after disposing of any document data and/or
//	controls.
//	
static Boolean
CloseAnyWindow( WindowPtr window )
{
	BaseWindow*		wind;
	
	if ( IsDAWindow( window ) )
	{
		CloseDeskAcc( ((WindowPeek)window)->windowKind );
	}
	else if ( GetObjectFromWindow( window, &wind ) )
	{
		return wind->Close();
	}
	else
		DisposeWindow( window );
	
	return true;
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ DeathAlert
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Display an alert that tell the user an err occurred, then exit the
//	program. This routine is used as an ultimate bail-out for serious errors
//	that prohibit the continuation of the application. The error number is
//	used to index an 'STR#' resource so that a relevant message can be
//	displayed.
//
static void
DeathAlert( short errNumber )
{
	short			itemHit;
	Str255			theMessage;
	
	SetCursor( &qd.arrow );
	GetIndString( theMessage, kErrorStrings, errNumber );
	ParamText( theMessage, nil, nil, nil );
	itemHit = StopAlert( kAlertStartupError, nil );
	ExitToShell();
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ IsAppWindow
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Check to see if a window belongs to the application. If the window
//	pointer passed was NIL, then it could not be an application window.
//	WindowKinds that are negative belong to the system and windowKinds
//	less that userKind are reserved by Apple except for userKinds equal to
//	dialogKind, which means it's a dialog.
//	

static Boolean
IsAppWindow( WindowPtr window )
{
	short		windowKind;
	
	if ( window == nil )
		return false;
		
	windowKind = ((WindowPeek)window)->windowKind;
	return( (windowKind >= userKind) || (windowKind == dialogKind) );
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ IsDAWindow
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Check to see if a window belongs to a desk accessory. It belongs to a DA
//	if the windowKind field of the window record is negative.
//	
static Boolean
IsDAWindow( WindowPtr window )
{
	if ( window == nil )
		return false;

	return( ((WindowPeek)window)->windowKind < 0 );
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ IsDialogWindow
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Check to see if a window is a dialog window. We can determine this by
//	checking to see if the windowKind field is equal to dialogKind.
//
static Boolean
IsDialogWindow( WindowPtr window )
{
	if ( window == nil )
		return false;

	return( ((WindowPeek)window)->windowKind == dialogKind );
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ GetObjectFromWindow
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Gets the object pointer from the refCon of the given window if the kind
//	is right. If the kind is wrong, or the refCon is null, we return false.
//
static Boolean
GetObjectFromWindow( WindowPtr window, BaseWindow** wind )
{
	SInt32		test;
	
	if ( ((WindowPeek)window)->windowKind != kObjectWindowKind )
		return false;
		
	test = GetWRefCon( window );
	if ( test == nil ) return false;
	
	*wind = (BaseWindow*)test;

	return true;
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ DoAboutBox
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Puts up our about box dialog. Note that we use a dialog and NOT an alert.
//	This is a good practice to get into, since alerts are now colored differently
//	to distiguish them from normal dialogs.
//
static void
DoAboutBox()
{
	DialogPtr		dialog;
	SInt16			itemHit;
	
	dialog = GetNewDialog( kAboutBoxDialogID, nil, (WindowPtr)-1L );
	if ( dialog == nil ) return;

	AutoSizeDialog(dialog); 
	ModalDialog( nil, &itemHit );
	
	DisposeDialog( dialog );
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ SyncVertZoomRects
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Sets up the standard and user rectangles for our vertical zooming window.
//
static void
SyncVertZoomRects( WindowPtr window )
{
	Rect		bounds;
	
	SetPort( window );
	bounds = window->portRect;
	
	if ( (bounds.bottom - bounds.top) < 200 )
		bounds.bottom = bounds.top + 200;
		
	LocalToGlobal( (Point *)&bounds.top );
	LocalToGlobal( (Point *)&bounds.bottom );
	
	SetWindowStandardState( window, &bounds );
	
	bounds.bottom = bounds.top + 60;
	SetWindowUserState( window, &bounds );
}

//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	¥ SyncHorizZoomRects
//ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ
//	Sets up the standard and user rectangles for our horizontal zooming window.
//
static void
SyncHorizZoomRects( WindowPtr window )
{
	Rect		bounds;
	
	SetPort( window );
	bounds = window->portRect;

	if ( (bounds.right - bounds.left) < 240 )
		bounds.right = bounds.left + 240;
		
	LocalToGlobal( (Point *)&bounds.top );
	LocalToGlobal( (Point *)&bounds.bottom );
	
	SetWindowStandardState( window, &bounds );
	
	bounds.right = bounds.left + 60;
	SetWindowUserState( window, &bounds );
}

static void DoNew()
{
	NavReplyRecord	reply;
	FSSpec			file;
	DescType		type;
	Size			size;
			
	if (NavPutFile(nil, &reply, nil, nil, 'TEXT', 'GU·', 0)) 
		return;
	else if (reply.validRecord && 
		!AEGetNthPtr(&reply.selection, 1, typeFSS, 
			&type, &type, (Ptr)&file, sizeof(FSSpec), &size)
	)
		new GCDialog(&file);
	NavDisposeReply(&reply);
}

static void DoOld()
{
	NavReplyRecord	reply;
	FSSpec			file;
	DescType		type;
	Size			size;
			
	if (NavChooseFile(nil, &reply, nil, nil, nil, nil, (NavTypeList **)GetResource('open', 128), nil)) 
		return;
	else if (reply.validRecord && 
		!AEGetNthPtr(&reply.selection, 1, typeFSS, 
			&type, &type, (Ptr)&file, sizeof(FSSpec), &size)
	)
		(new GCDialog(&file))->Read();
	NavDisposeReply(&reply);
}

pascal OSErr DoOpen(const AppleEvent *message, AppleEvent *, long)
{
	FSSpec		file;
	DescType	type;
	Size		size;
	
	if (!AEGetParamPtr(message, keyDirectObject, typeFSS, 
			&type, (Ptr)&file, sizeof(FSSpec), &size)
	) 
		(new GCDialog(&file))->Read();
	
	return noErr;
}

pascal OSErr DoQuit(const AppleEvent *, AppleEvent *, long refcon)
{
	for (WindowPtr win; win = FrontWindow(); )
		if (!CloseAnyWindow(win))
			return userCanceledErr;
	
	gQuit = true;
	
	return noErr;
}
