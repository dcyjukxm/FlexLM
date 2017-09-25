#******************************************************************************
#
#           COPYRIGHT (c) 1996 by Globetrotter Software Inc.
#       This software has been provided pursuant to a License Agreement
#       containing restrictions on its use.  This software contains
#       valuable trade secrets and proprietary information of 
#       Globetrotter Software Inc and is protected by law.  It may 
#       not be copied or distributed in any form or medium, disclosed 
#       to third parties, reverse engineered or used in any manner not 
#       provided for in said License Agreement except with the prior 
#       written authorization from Globetrotter Software Inc.
#
#*****************************************************************************
#       
#       Module: $Id: pc.mak,v 1.7 2001/04/18 17:11:45 daniel Exp $
#
#       Description:  Builds the LMGRD
#
#       To build:
#
#               nmake /f master.mak [DEBUG=] [PROFILE=] [STATIC]
#
#       The "CPU" environment variable is used to detect the platform
#       type.  Override it by setting "PLAT=xxx" on the invokation.
#
#       C. Mayman
#        4-May-97
#
#       Last changed:  %G%
#
#*****************************************************************************

!include "..\h\pc.mak"

OUTNAME = lmgrd.exe

!ifdef DEBUG
FDFLAG=/Fdlmgrd.pdb
!else
FDFLAG=
!endif


INCS =   /I ..\server \

RC = rc

#
# Compiler flags
#

CFLAGS = $(CLFLAGS) /Fplmgrd.pch $(FDFLAG) /D"FD_SETSIZE=4096"

.c.obj:
        $(CC) $(CFLAGS) $(INCS) /c $<

#
# This will make a version of the file that
# has the macros expanded.
#

.c.expand:
        $(CC) $(CFLAGS) /E $(INCS) /c $< > $*.expand

#****************************
# Link information
#****************************

#
# Libraries
#



#****************************
# File lists
#****************************

OBJS =          ls_chld_died.obj \
                ls_daemons.obj \
                ls_finder.obj \
                ls_kill_chld.obj \
                ls_lmgrd.obj \
                ls_m_init.obj \
                ls_m_main.obj \
                ls_m_process.obj \
                ls_startup.obj \
                ls_statfile.obj \
                ls_timestamp.obj \
                service.obj


#****************************
# General Targets
#****************************

all : lmgrd.exe 
        @echo !--------------------------------------
        @echo ! Done building $(OUTNAME)
        @echo !--------------------------------------

lmgrd.exe : $(OBJS) $(LIBS)
	$(RC) lmgrd.rc
        $(LD) $(LINKFLAGS) /out:$(OUTNAME) $(OBJS) lmgrd.res $(LIBS) $(CLIBS)

clean :
        @echo Cleaning up master
        @if exist *.ex* del *.ex*
        @if exist *.p* del *.p*
        @if exist *.i* del *.i*
        @if exist *.obj del *.obj
	@if exist *.res del *.res

$(OBJS):        ..\h\lmachdep.h ..\machind\lmclient.h \
                ..\machind\lsserver.h  l_m_prot.h

