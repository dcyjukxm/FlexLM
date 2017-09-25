#########################################################################
#            COPYRIGHT (c) 2003 by Macrovision Corporation.
#        This software has been provided pursuant to a License Agreement
#        containing restrictions on its use.  This software contains
#        valuable trade secrets and proprietary information of 
#        Macrovision Corporation and is protected by law.  It may 
#        not be copied or distributed in any form or medium, disclosed 
#        to third parties, reverse engineered or used in any manner not 
#        provided for in said License Agreement except with the prior 
#        written authorization from Macrovision Corporation.
##########################################################################
#
# FLEXLM makefile for PCs
# If you have a real kit, change VENDORNAME=demo below to your real
# vendor daemon name
# Use nmake DLL=1 for dll clients
# Use nmake MD=1 for dynamic runtime library: msvcrt.lib
#
VENDORNAME = demo

DAEMON = $(VENDORNAME).exe

SRCDIR = ..\machind
!ifdef DEBUG
XTRACFLAG=/Zi /Od
LDFLAGS=/DEBUG 
CNAME=d
!else
XTRACFLAG =  /O1
LDFLAGS = 
CNAME=
!endif

LD = LINK /nologo /NODEFAULTLIB /OPT:NOREF $(LDFLAGS)
RC = RC

INCFLAGS = /I$(SRCDIR) /I.
!ifdef DLL
LMGRNAME=9a
MDNAME=
DLL_FLAG=/DFLEXLM_DLL
CRT_LIB=libcmt$(CNAME).lib
LMNEW_OBJ=lm_new.obj
CRT_FLAG=/MT$(CNAME)
!else
!ifdef MD
LMGRNAME=_md
LMNEW_OBJ=lm_new_md.obj 
CRT_FLAG=/MD$(CNAME)
CRT_LIB=msvcrt$(CNAME).lib 
!else
LMGRNAME=
CRT_FLAG=/MT$(CNAME)
LMNEW_OBJ=lm_new.obj
CRT_LIB=libcmt$(CNAME).lib
!endif

MDNAME=$(LMGRNAME)

!endif

LMGR_LIB=lmgr$(LMGRNAME).lib

XTRALIB1 =  oldnames.lib kernel32.lib user32.lib netapi32.lib \
        advapi32.lib  gdi32.lib comdlg32.lib  comctl32.lib wsock32.lib

XTRALIB =  $(XTRALIB1) $(CRT_LIB)

XTRACFLAG=$(XTRACFLAG) $(DLL_FLAG)

#
#	Use XTRAOBJS to define your object files for vendor daemon
#	initialization routines, checkout filter, checkin callback, etc.
#
XTRAOBJS = 

CFLAGS_NOOPT = /nologo /c  $(INCFLAGS) $(CRT_FLAG) $(XTRACFLAG)
CFLAGS = $(CFLAGS_NOOPT) 
LMNEW_CFLAGS = /nologo /c  $(INCFLAGS) $(CRT_FLAG) $(DLL_FLAG)

SRCS	= $(SRCDIR)\lsvendor.c

EXECS = lmsimple.exe lmflex.exe lmwin.exe makekey.exe makepkg.exe lmclient.exe lmcrypt.exe installs.exe

CLIENTLIB        = $(LMGR_LIB) libsb$(MDNAME).lib libcrvs$(MDNAME).lib flock$(MDNAME).lib
STATIC_CLIENTLIB = lmgr$(MDNAME).lib libsb$(MDNAME).lib libcrvs$(MDNAME).lib flock$(MDNAME).lib
LIBS             = lmgras$(MDNAME).lib lmgrs$(MDNAME).lib $(STATIC_CLIENTLIB)
DAEMONLIBS       = lmgras$(MDNAME).lib lmgrs$(MDNAME).lib lmgr$(MDNAME).lib libsb$(MDNAME).lib libcrvs$(MDNAME).lib
EVENTLOGMESSAGEFILE = VendorLicenseServerMsgs
EVENTLOGMESSAGEFILE2 = MvsnLicenseServerMsgs

all: lmwin.res \
!ifdef DLL
     lmgr9a.dll ltclient.exe \
!endif
     $(EXECS) $(DAEMON) utils

daemon:	$(DAEMON)

$(DAEMON): $(XTRAOBJS) $(DAEMONLIBS) lsvendor.obj $(SRCDIR)\lsserver.h $(LMNEW_OBJ)
           $(LD) /subsystem:console /out:$(DAEMON) lsvendor.obj $(LMNEW_OBJ) \
           $(XTRAOBJS) $(DAEMONLIBS) $(CRT_LIB) $(XTRALIB1)

lsvendor.obj: lm_new.obj
        $(CC) $(CFLAGS) -I../h lsrvend.c 
	if exist lsvendor.obj del lsvendor.obj
        ren lsrvend.obj lsvendor.obj
 
$(LMNEW_OBJ): $(SRCDIR)\lsvendor.c $(SRCDIR)\lm_code.h 
	lmrand1 -i $(SRCDIR)\lsvendor.c
	$(CC) /c $(LMNEW_CFLAGS) -I../h lmcode.c
	$(LD) /subsystem:CONSOLE lmnewgen$(MDNAME).obj lmcode.obj \
        lmgr$(MDNAME).lib libcrvs$(MDNAME).lib libsb$(MDNAME).lib $(XTRALIB1) $(CRT_LIB) /out:lmnewgen.exe
	if exist lm_new.c del lm_new.c
	lmnewgen.exe $(VENDORNAME) -o lm_new.c
    $(CC) $(LMNEW_CFLAGS) /Fo$(LMNEW_OBJ) lm_new.c 

makekey.exe:	$(SRCDIR)\makekey.c $(SRCDIR)\lm_code.h lm_new.obj \
				$(SRCDIR)\lmclient.h $(STATIC_CLIENTLIB)
	$(CC) $(CFLAGS) $(SRCDIR)\makekey.c
	$(LD) /out:makekey.exe makekey.obj $(STATIC_CLIENTLIB) $(XTRALIB)
	if exist makekey.obj del makekey.obj

makepkg.exe: $(SRCDIR)\makepkg.c lm_new.obj \
		$(SRCDIR)\lmclient.h $(SRCDIR)\lm_code.h $(STATIC_CLIENTLIB)
	$(CC) $(CFLAGS) $(SRCDIR)\makepkg.c
	$(LD) /out:makepkg.exe makepkg.obj $(STATIC_CLIENTLIB) $(XTRALIB)
	if exist makepkg.obj del makepkg.obj

lmcrypt.exe: $(SRCDIR)\lmcrypt.c \
		$(SRCDIR)\lmclient.h $(SRCDIR)\lm_code.h $(STATIC_CLIENTLIB) 
	$(CC) $(CFLAGS) $(SRCDIR)\lmcrypt.c
	$(LD) /out:lmcrypt.exe lmcrypt.obj $(STATIC_CLIENTLIB) $(XTRALIB)

validdat.exe:	$(SRCDIR)\validdat.c  $(CLIENTLIB)
	$(CC) $(CFLAGS) $(SRCDIR)\validdat.c
	$(LD) /out:validdat.exe validdat.obj $(CLIENTLIB) $(XTRALIB)
	if exist validdat.obj del validdat.obj

lmclient.exe:	$(SRCDIR)\lmclient.c  $(LMNEW_OBJ) \
				$(CLIENTLIB) 
	$(CC) $(CFLAGS) $(SRCDIR)\lmclient.c
	$(LD) /out:lmclient.exe lmclient.obj $(LMNEW_OBJ) $(CLIENTLIB) $(XTRALIB)
	if exist lmclient.obj del lmclient.obj

lmsimple.exe:	$(SRCDIR)\lmsimple.c  $(LMNEW_OBJ) \
				$(CLIENTLIB) 
	$(CC) $(CFLAGS) $(SRCDIR)\lmsimple.c
	$(LD) /out:lmsimple.exe lmsimple.obj $(LMNEW_OBJ)\
			$(CLIENTLIB) $(XTRALIB)
	if exist lmsimple.obj del lmsimple.obj

lmwin.exe:	$(SRCDIR)\lmwin.c  $(LMNEW_OBJ)
        $(CC) $(CFLAGS) $(SRCDIR)\lmwin.c
	$(RC) $(SRCDIR)\lmwin.rc
        $(LD) /out:lmwin.exe lmwin.obj $(LMNEW_OBJ) \
                    $(SRCDIR)\lmwin.res $(CLIENTLIB) $(XTRALIB) 
	if exist lmwin.obj del lmwin.obj

lmflex.exe: $(SRCDIR)\lmflex.c  $(LMNEW_OBJ) \
            $(CLIENTLIB) 
	$(CC) $(CFLAGS) $(SRCDIR)\lmflex.c
	$(LD) /out:lmflex.exe lmflex.obj $(LMNEW_OBJ) $(CLIENTLIB) $(XTRALIB)
	if exist lmflex.obj del lmflex.obj

installs.exe: $(SRCDIR)\installs.c
        $(CC) $(CFLAGS) $(SRCDIR)\installs.c
        $(LD) /out:installs.exe installs.obj $(XTRALIB)
	if exist installs.obj del installs.obj

lmwin.res: $(SRCDIR)\lmwin.rc
        rc /l 0x409 /r /folmwin.res $(SRCDIR)\lmwin.rc

lmgr9a.dll: $(LMNEW_OBJ)
        $(LD) /def:"$(SRCDIR)\lmgr9a.def" \
              /out:lmgr9a.dll lmgr9a.obj /implib:"lmgr9a.lib" /dll \
              $(LMNEW_OBJ) \
	      lmgr$(MDNAME).lib libsb$(MDNAME).lib libcrvs$(MDNAME).lib flock$(MDNAME).lib \
              $(XTRALIB)

ltclient.exe:	$(SRCDIR)\ltclient.c  $(LMNEW_OBJ) 
	$(CC) /nologo /c $(INCFLAGS) $(CRT_FLAG) /O1 $(SRCDIR)\ltclient.c
	$(LD) /out:ltclient.exe ltclient.obj $(LMNEW_OBJ) lmgr9a.lib $(XTRALIB)
	if exist ltclient.obj del ltclient.obj

buildmsgfile:;
	if exist $(EVENTLOGMESSAGEFILE).dll copy $(EVENTLOGMESSAGEFILE).dll $(EVENTLOGMESSAGEFILE).dll_bak
	mc -u -U $(SRCDIR)\$(EVENTLOGMESSAGEFILE).mc
	$(RC) -r -fo $(EVENTLOGMESSAGEFILE).res $(EVENTLOGMESSAGEFILE).rc
	$(LD) /dll /noentry /machine:x86 /out:$(EVENTLOGMESSAGEFILE).dll $(EVENTLOGMESSAGEFILE).res
	if exist $(EVENTLOGMESSAGEFILE2).dll copy $(EVENTLOGMESSAGEFILE2).dll $(EVENTLOGMESSAGEFILE2).dll_bak
	mc -u -U $(SRCDIR)\$(EVENTLOGMESSAGEFILE2).mc
	$(RC) -r -fo $(EVENTLOGMESSAGEFILE2).res $(EVENTLOGMESSAGEFILE2).rc
	$(LD) /dll /noentry /machine:x86 /out:$(EVENTLOGMESSAGEFILE2).dll $(EVENTLOGMESSAGEFILE2).res

clean:;	
        if exist lmnewgen.obj ren lmnewgen.obj lmnewgen.sav
        if exist lmnewgen_md.obj ren lmnewgen_md.obj lmnewgen_md.sav
        if exist lmgr9a.obj ren lmgr9a.obj lmgr9a.sav
        if exist *.obj del *.obj
        if exist lmnewgen_md.sav ren lmnewgen_md.sav lmnewgen_md.obj
        if exist lmnewgen.sav ren lmnewgen.sav lmnewgen.obj
        if exist lmgr9a.sav ren lmgr9a.sav lmgr9a.obj
	if exist lmnewgen.exe del lmnewgen.exe
        if exist makekey.exe del makekey.exe 
        if exist makepkg.exe del makepkg.exe 
        if exist lmclient.exe del lmclient.exe 
        if exist lmcrypt.exe del lmcrypt.exe 
        if exist lmfeats.exe del lmfeats.exe 
        if exist $(DAEMON) del $(DAEMON)
        if exist lmcode.c del lmcode.c 
        if exist lmcode.obj del lmcode.obj 
        if exist lm_new.c del lm_new*.c 
        if exist lm_new.obj del lm_new*.obj 
        if exist lsrvend.c del lsrvend.c 
        if exist lmflex.exe del lmflex.exe
        if exist lmsimple.exe del lmsimple.exe
        if exist lmwin.exe del lmwin.exe
	if exist $(SRCDIR)\lmwin.res del $(SRCDIR)\lmwin.res
	if exist *.ilk del *.ilk
	if exist *.p* del *.p*
	if exist lmwin.res del lmwin.res
	if exist $(VENDORNAME)$(LMGRNAME).lib del $(VENDORNAME)$(LMGRNAME).lib
	if exist $(VENDORNAME).dll del $(VENDORNAME).dll
	if exist $(VENDORNAME).exp del $(VENDORNAME).exp
	if exist lmgr9a.dll del lmgr9a.dll
	if exist lmgr9a.exp del lmgr9a.exp
	if exist lmgr9a.lib del lmgr9a.lib
	if exist ltclient.* del ltclient.*
	if exist *.bin del *.bin
	if exist $(EVENTLOGMESSAGEFILE).h del $(EVENTLOGMESSAGEFILE).h
	if exist $(EVENTLOGMESSAGEFILE).res del $(EVENTLOGMESSAGEFILE).res
	if exist $(EVENTLOGMESSAGEFILE).rc del $(EVENTLOGMESSAGEFILE).rc
	if exist $(EVENTLOGMESSAGEFILE2).h del $(EVENTLOGMESSAGEFILE2).h
	if exist $(EVENTLOGMESSAGEFILE2).res del $(EVENTLOGMESSAGEFILE2).res
	if exist $(EVENTLOGMESSAGEFILE2).rc del $(EVENTLOGMESSAGEFILE2).rc

veryclean:
	if exist lmhostid.exe del lmhostid.exe
	if exist lmver.exe del lmver.exe
	if exist lmcksum.exe del lmcksum.exe
	if exist lmdown.exe del lmdown.exe
	if exist lmremove.exe del lmremove.exe
	if exist lmreread.exe del lmreread.exe
	if exist lmswitchr.exe del lmswitchr.exe
	if exist lmstat.exe del lmstat.exe
	if exist lmborrow.exe del lmborrow.exe
	if exist lmdiag.exe del lmdiag.exe
	if exist lminstall.exe del lminstall.exe
        $(MAKE) clean

kitclean:
	if exist lmcode.c del lmcode.c
	if exist lmcode.obj del lmcode.obj
	if exist lmcrypt.obj del lmcrypt.obj
	if exist lm_new.c del lm_new.c
	if exist lm_new.obj del lm_new.obj
	if exist lmnewgen.exe del lmnewgen.exe
	if exist lsvendor.obj del lsvendor.obj
	if exist lmhostid.exe del lmhostid.exe
	if exist lmver.exe del lmver.exe
	if exist lmcksum.exe del lmcksum.exe
	if exist lmdown.exe del lmdown.exe
	if exist lmhostid.exe del lmhostid.exe
	if exist lminstall.exe del lminstall.exe
	if exist lmremove.exe del lmremove.exe
	if exist lmreread.exe del lmreread.exe
	if exist  lmswitchr.exe del  lmswitchr.exe
	if exist lmstat.exe del lmstat.exe
	if exist lmdiag.exe del lmdiag.exe
	if exist lmver.exe  del lmver.exe 
	if exist lmflex.exe  del lmflex.exe 
	if exist lmsimple.exe  del lmsimple.exe 
	if exist lmgr9a.dll del lmgr9a.dll
	if exist lmgr9a.exp del lmgr9a.exp
	if exist lmgr9a.lib del lmgr9a.lib
	if exist *.bin del *.bin
	if exist $(EVENTLOGMESSAGEFILE).h del $(EVENTLOGMESSAGEFILE).h
	if exist $(EVENTLOGMESSAGEFILE).res del $(EVENTLOGMESSAGEFILE).res
	if exist $(EVENTLOGMESSAGEFILE).rc del $(EVENTLOGMESSAGEFILE).rc
	if exist $(EVENTLOGMESSAGEFILE2).h del $(EVENTLOGMESSAGEFILE2).h
	if exist $(EVENTLOGMESSAGEFILE2).res del $(EVENTLOGMESSAGEFILE2).res
	if exist $(EVENTLOGMESSAGEFILE2).rc del $(EVENTLOGMESSAGEFILE2).rc


lmcrypt.obj:	$(SRCDIR)\lmcrypt.c $(SRCDIR)\lm_code.h $(SRCDIR)\lmclient.h
	$(CC) $(CFLAGS) $(SRCDIR)\lmcrypt.c
	if exist lmcrypt.exe del lmcrypt.exe

utils:	lmhostid.exe lmver.exe lmdown.exe lmremove.exe lmreread.exe lmswitchr.exe lmstat.exe lmdiag.exe lminstall.exe lmborrow.exe lmpath.exe

lmhostid.exe:   lmutil.exe
    copy lmutil.exe lmhostid.exe

lmver.exe:   lmutil.exe
    copy lmutil.exe lmver.exe

lmdown.exe:   lmutil.exe
    copy lmutil.exe lmdown.exe

lmremove.exe:   lmutil.exe
    copy lmutil.exe lmremove.exe

lmswitchr.exe:   lmutil.exe
    copy lmutil.exe lmswitchr.exe

lmreread.exe:   lmutil.exe
    copy lmutil.exe lmreread.exe

lmstat.exe:   lmutil.exe
    copy lmutil.exe lmstat.exe

lmborrow.exe:   lmutil.exe
    copy lmutil.exe lmborrow.exe

lmpath.exe:   lmutil.exe
    copy lmutil.exe lmpath.exe

lmdiag.exe:   lmutil.exe
    copy lmutil.exe lmdiag.exe

lminstall.exe:   lmutil.exe
    copy lmutil.exe lminstall.exe

        copy lmutil.exe lmhostid.exe
        copy lmutil.exe lmver.exe
        copy lmutil.exe lmdown.exe
        copy lmutil.exe lmremove.exe
        copy lmutil.exe lmreread.exe
        copy lmutil.exe lmswitchr.exe
        copy lmutil.exe lmstat.exe
        copy lmutil.exe lmborrow.exe
        copy lmutil.exe lmdiag.exe
        copy lmutil.exe lminstall.exe
    	copy lmutil.exe lmpath.exe


