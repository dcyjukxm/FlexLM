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
#       Module: $Id: pc.mak,v 1.13 2001/12/19 15:37:21 daniel Exp $
#
#       Description:  Builds the utils programs
#
#       To build:
#
#               nmake /f utils.mak [DEBUG=] [PROFILE=] [16BIT=]
#
#       The "CPU" environment variable is used to detect the platform
#       type.  Override it by setting "PLAT=xxx" on the invokation.
#
#       C. Mayman
#        4-May-97
#
#       Last changed:  12/21/98
#
#*****************************************************************************

#****************************
# Platform setup
#****************************

!include "..\h\pc.mak"


BASEDIR = ..

APPDIR          = $(BASEDIR)\app
APPDIROBJS      = $(BASEDIR)\app
MASTERDIR       = $(BASEDIR)\master
MASTERDIROBJS   = $(BASEDIR)\master
SERVERDIR       = $(BASEDIR)\server
SERVERDIROBJS   = $(BASEDIR)\server
SRCDIR          = $(BASEDIR)\src
SRCDIROBJS      = $(BASEDIR)\src
UTILSDIR        = $(BASEDIR)\utils
UTILSDIROBJS    = $(BASEDIR)\utils
INCDIR          = ..\machind
#****************************
# Compilation information
#****************************

#
# Include files
#

INCS =  /I . \
        /I ..\h \
         /I ..\machind  \
        /I ..\server \
        /I ..\app \
        /I ..\patches

#
# 32 bit compiler flags
#
!ifdef PHASE2
PHASE = /D"PHASE2"
!else
PHASE = 
!endif

CFLAGS = $(CLFLAGS)

#
# Compilation rules
#

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


LIBS =          ..\src\$(DLL32NAME) 

DAEMON_LIBS =   $(SERVERDIROBJS)\lmgrs.lib \
                $(APPDIROBJS)\lmgras.lib \
                $(LIBS)

PUBKEY_LIBS =   ..\$(PROCESSOR_DIRECTORY)\libcrvs.lib \
                ..\$(PROCESSOR_DIRECTORY)\libsb.lib
#
# 16 bit libraries
#

CLIBS16 =       oldnames \
                libw \
                llibcewq \
                lzexpand


!ifdef DEBUG
XTRA_LFLAGS = /DEBUG /pdb:$*.pdb
!else
XTRA_LFLAGS = /INCREMENTAL:NO
!endif

!ifdef PROFILE
XTRA_LFLAGS = $(XTRA_LFLAGS) /profile /map
!endif

!ifdef MAP
XTRA_LFLAGS = $(XTRA_LFLAGS) /map
!endif

LFLAGS = $(LINKFLAGS)  $(XTRA_LFLAGS)
#
# 16 bit link flags
#

!ifdef DEBUG
DBG_LFLAGS16 = /CO
!else
DBG_LFLAGS16 =
!endif

LFLAGS16 = /NOLOGO /NOD /NOE /PACKC:57344 /SEG:500 /ALIGN:16 \
             /MAP /ONERROR:NOEXE /STACK:40000 $(DBG_LFLAGS16)


#****************************
# File lists
#****************************

OUTPUTS =        demo.exe  


#****************************
# General Targets
#****************************


all : $(OUTPUTS) lm_new2.obj

clean :
        @echo Cleaning up vendor
        @if exist *.expand del *.expand
        @if exist *.obj del *.obj
        @if exist *.sbr del *.sbr
        @if exist *.pdb del *.pdb
        @if exist *.pch del *.pch
        @if exist *.ilk del *.ilk
        @if exist *.exp del *.exp
        @if exist *.exe del *.exe



#
# Specific .EXE targets
#

demo.exe :  ..\utils\lmrand1.exe lsvendor.obj lm_new.obj 
        $(LD) $(LFLAGS) /out:$*.exe lsvendor.obj lm_new.obj \
                 $(DAEMON_LIBS) $(CLIBS) $(PUBKEY_LIBS)


lmrand1.exe : lmrand1.obj $(LIBS)
        $(LD) $(LFLAGS) /out:$*.exe lmrand1.obj \
                 $(LIBS) $(CLIBS)
l_gcspim.c : ..\utils\makecpuid.exe ..\src\lmgr.lib
	..\utils\makecpuid.exe > l_gcspim.c



#
# Intermediate file targets
#

#lsr_vend.c : lmrandom.exe lsvendor.c
#       lmrandom < lsvendor.c > lsr_vend.c

lm_new2.obj:	lm_new.obj

lm_new.obj : ..\machind\lsvendor.c ..\utils\lmnewgen.obj ..\utils\lmrand1.exe
        ..\utils\lmrand1 -i ..\machind\lsvendor.c
        @if exist lmappfil.c del lmappfil.c
        @if exist lmkeyfil.c del lmkeyfil.c
        ..\utils\lmrand1 -filter_gen 0x23456789 0x3456789a 0x456789ab -q
        $(CC) $(CFLAGS) $(INCS) /c lmcode.c
        $(LD) $(LFLAGS) /out:lmnewgen.exe lmcode.obj ..\utils\lmnewgen.obj \
                $(LIBS) $(CLIBS) 
        @if exist lm_new.c del  lm_new.c  
        lmnewgen demo -o lm_new.c
        $(CC) $(CFLAGS) $(INCS) /c lm_new.c /Folm_new.obj

lsvendor.obj : lm_new.obj 
        $(CC) $(CFLAGS) $(INCS) /c lsrvend.c /Folsvendor.obj



!ifdef PHASE2
lmkeyfil.obj :
        $(CC) $(CFLAGS) $(INCS) /c lmkeyfil.c 

lmappfil.obj :
        $(CC) $(CFLAGS) $(INCS) /c lmappfil.c 

!endif

#*******************************
# Dependencies
#*******************************

$(OUTPUTS) : ..\h\lm_code.h ..\machind\lm_code2.h

