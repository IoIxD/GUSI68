#######################################################################
# 	$Id: Makefile.mk,v 1.10 2002/01/07 05:58:39 neeri Exp $
#
#	Project	:	GUSI 2				-	Grand Unified Socket Interface
#	File	:	Makefile.mk			-	Build everything
#	Author	:	Matthias Neeracher		Language	:	dmake
#	
#	$Log: Makefile.mk,v $
#	Revision 1.10  2002/01/07 05:58:39  neeri
#	Releasing 2.2.1
#	
#	Revision 1.9  2001/01/17 07:13:45  neeri
#	Make LP tools configurable
#	
#	Revision 1.8  1999/09/03 06:31:07  neeri
#	Added manual target
#	
#	Revision 1.7  1999/08/26 05:48:44  neeri
#	Getting ready for 2.0b8
#	
#	Revision 1.6  1999/04/29 05:35:35  neeri
#	Fix fake Dcon support
#	
#	Revision 1.5  1999/02/25 03:31:54  neeri
#	Generate simple DCon header
#	
#	Revision 1.4  1998/08/01 21:31:58  neeri
#	About ready for 2.0a1
#
#	Revision 1.3  1998/02/11 12:57:08  neeri
#	PowerPC Build
#
#	Revision 1.2  1996/12/16 02:10:00  neeri
#	Simplified
#
# Revision 1.1.1.1  1996/11/03  02:43:32  neeri
# Imported into CVS
#
#######################################################################

.INCLUDE : GUSIConfig.mk

#######################################################################
# Build rules
#
all: lib

source lib lib-68k lib-ppc lib-sc lib-mrc liber cwpro5 cwpro7 .PHONY	: ":src:Makefile.mk"
	Directory :src
		Set OldEcho {Echo}
		Set Echo 0
		BuildProgram $@ 
		Set Echo {OldEcho}
	Directory ::

lib lib-68k lib-ppc lib-sc lib-mrc cwpro5 cwpro7 : source DCon

manual .PHONY : 
	Directory :doc:pod
		pod2ps -c GUSI.cmd
	Directory :::
	
tests .PHONY : lib ":test:Makefile.mk"
	Directory :test
		Set OldEcho {Echo}
		Set Echo 0
		BuildProgram source
		BuildProgram tests
		Set Echo {OldEcho}
	Directory ::

":src:Makefile.mk" : ":src:Makefile.nw"   
	$(UNTANGLE) -make -t -RMakefile.mk :src:Makefile.nw > :src:Makefile.mk

":test:Makefile.mk" : ":test:Makefile.nw"   
	$(UNTANGLE) -make -t -RMakefile.mk :test:Makefile.nw > :test:Makefile.mk

DCon:
	Echo "You don't seem to have the DCon SDK installed as an alias named"
	Echo "“DCon” in the top level GUSI2 directory. I will fake a minimal header"
	Echo "for you instead."
	NewFolder DCon
	NewFolder :DCon:Headers
	Begin
		Echo "/* Fake header for DCon SDK for the purposes of GUSI 2"
		Echo -n " * Written: "; Date
		Echo " */"
		Echo
		Echo "#include <stdarg.h>"
		Echo
		Echo "#ifdef __cplusplus"
		Echo "extern ∂"C∂" ∂{"
		Echo "#endif"
		Echo "void dprintf(const char *format,...);"
		Echo "void dfprintf(const char *log,const char *format,...);"
		Echo "void vdfprintf(const char *log,const char *format,va_list args);"
		Echo "#ifdef __cplusplus"
		Echo "∂}"
		Echo "#endif"
	End > :DCon:Headers:DCon.h
