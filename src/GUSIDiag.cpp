
#include "GUSIInternal.h"
#include "GUSIDiag.h"
#include "GUSIBasics.h"

#include <DCon.h>
#include <LowMem.h>


char * GUSI_diag_log;

bool GUSI_pos(const char * file, int line)
{
	dfprintf(GUSI_diag_log, "File '%s'; Line %d # ", file, line);
	
	return false;
}

bool GUSI_log(const char * format, ...)
{
	va_list	ap;
		
	va_start(ap, format);
	
	char f[100];
	GUSI_sprintf(f, "%d: %s", LMGetTicks(), format);
	vdfprintf(GUSI_diag_log, f, ap);
		
	va_end(ap);

	return false;
}

bool GUSI_break(const char * format, ...)
{
	va_list	ap;
	char 	breakmsgbuf[250];
	
	va_start(ap, format);
	
	strcpy(breakmsgbuf+1, "GUSI: ");
	breakmsgbuf[0] = GUSI_vsprintf(breakmsgbuf+7, format, ap)+6;
		
	va_end(ap);
	
	DebugStr((StringPtr) breakmsgbuf);
	
	return false;
}

void dfprintf(const char *,const char *,...)
{
}

void vdfprintf(const char *,const char *,va_list)
{
}

