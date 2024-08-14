/*
	File:			GCDialog.cp

	Contains:	Code to drive our GCDialog.

	Derived from Apple's Appearance SDK

	$Log: GCDialog.cp,v $
	Revision 1.1  1999/04/10 03:52:47  neeri
	Initial Version
	
*/

#include "GCDialog.h"
#include "GCData.h"
#include "GCSocketsPane.h"
#include "GCDevicesPane.h"
#include "GCConfigPane.h"

#include <Appearance.h>
#include <Navigation.h>
#include <Dialogs.h>

#include <fstream>

enum
{
	kNoPane			= 0,
	kGCSocketsPane	= 1,
	kGCDevicesPane	= 2,
	kGCConfigPane	= 3
};

static AlertStdAlertParamRec sNoConfigAlertParam = {
	false, false, nil, "\pAdd Configuration Section", "\pCancel", nil, 
	kAlertStdAlertCancelButton, kAlertStdAlertCancelButton, kWindowDefaultPosition
};

GCDialog::GCDialog(FSSpec * spec) : BaseDialog( 1000 ), fFile(*spec)
{
	short button;

	DefaultFlags(false);
	DefaultSockets(false);
	DefaultDevices(false);
	
	fPane = nil;
	SwitchPane( kGCSocketsPane );
	fHeader = NewHandle(0);
	fFooter = NewHandle(0);
}

void GCDialog::Read()
{	
	short button;
	
	delete fPane;
	fPane = nil;	
	std::ifstream	s(fFile.FullPath());
			
	if (!ReadData(s, fHeader, fFooter)) {
		if (StandardAlert(
			kAlertCautionAlert, 
			"\pThe file you selected has no configuration section. Add one?", 
			"\p", 
			&sNoConfigAlertParam, &button)
		)
			button = kAlertStdAlertCancelButton;
		switch (button) {
		case kAlertStdAlertOKButton:
			break;
		case kAlertStdAlertCancelButton:
			delete this;
			return;
		}
	}
	SwitchPane( kGCSocketsPane );
}

GCDialog::~GCDialog()
{
	DisposeHandle(fHeader);
	DisposeHandle(fFooter);
	
	delete fPane;
}

void
GCDialog::Idle()
{
	if ( fPane ) fPane->Idle();
}

void
GCDialog::SwitchPane( SInt16 paneIndex )
{
	ControlHandle		control;

	if ( paneIndex == 0 ) return;

	delete fPane;
	fPane = nil;	
		
	GetDialogItemAsControl( fWindow, 1, &control );
	SetControlValue( control, paneIndex );

	switch ( paneIndex ) {
	case kGCSocketsPane:
		fPane = new GCSocketsPane( fWindow, CountDITL( fWindow ) );
		break;
	case kGCDevicesPane:
		fPane = new GCDevicesPane( fWindow, CountDITL( fWindow ) );
		break;
	case kGCConfigPane:
		fPane = new GCConfigPane( fWindow, CountDITL( fWindow ) );
		break;
	}	
}

void
GCDialog::HandleItemHit( SInt16 item )
{
	if ( item == 1 )
	{
		ControlHandle		control;
		
		GetDialogItemAsControl( fWindow, 1, &control );
		SwitchPane( GetControlValue( control ) );
	}
	else if ( fPane )
	{
		fPane->ItemHit( item );
	}
}

static AlertStdAlertParamRec sCloseAlertParam = {
	false, false, nil, "\pSave", "\pCancel", "\pDon't Save", 
	kAlertStdAlertOKButton, kAlertStdAlertCancelButton, kWindowDefaultPosition
};

Boolean
GCDialog::Close()
{
	SInt16	button;
	
	if (StandardAlert(kAlertCautionAlert, "\pSave Configuration Before Closing?", "\p", 
			&sCloseAlertParam, &button)
	)
		button = kAlertStdAlertOtherButton;
	
	switch (button) {
	case kAlertStdAlertCancelButton:
		return false;
	case kAlertStdAlertOKButton:
		{
			delete fPane;
			fPane = nil;	
			std::ofstream s(fFile.FullPath());
			
			WriteData(s, fHeader, fFooter);
		}
	default:
		delete this;
	}	
	
	return true;
}
