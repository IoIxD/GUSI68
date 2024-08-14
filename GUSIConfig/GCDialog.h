/*
	File:		GCDialog.h

	Contains:	Run configuration dialog.

	Derived from Apple's Appearance SDK

	$Log: GCDialog.h,v $
	Revision 1.2  1999/06/24 06:39:34  neeri
	Added SIGINT flag
	
	Revision 1.1  1999/04/10 03:52:47  neeri
	Initial Version
	
*/

#pragma once
#define GUSI_SOURCE

#include <GUSIFileSpec.h>

#include "BaseDialog.h"
#include "GCPane.h"

class GCDialog : public BaseDialog
{
	public:
			GCDialog(FSSpec * file);
		virtual ~GCDialog();
	
		virtual void		Idle();
	
		void				Read();
	protected:
		virtual void		HandleItemHit( SInt16 item );
		virtual Boolean		Close();		

	private:
		void		SwitchPane( SInt16 paneIndex );
		
		GCPane*			fPane;
		Handle			fHeader;
		Handle			fFooter;
		GUSIFileSpec	fFile;
};

