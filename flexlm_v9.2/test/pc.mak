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
#       Module: $Id: pc.mak,v 1.22 2003/03/08 01:00:03 sluu Exp $
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
#       Last changed:  9/30/98
#
#*****************************************************************************

!include "..\h\pc.mak"


#
# Include files
#

INCS =  /I . \
         /I ..\machind  \
        /I "$(MSDEVDIR)"\INCLUDE \
        /I ..\patches \

CFLAGS = $(CLFLAGS) 

#
# 32 bit compiler flags
#
!ifdef PHASE2
PHASE = /D"PHASE2"
!else
PHASE = 
!endif
!ifdef DEBUG
DBG_CFLAGS32 = /MTd /Zi  \
                /Fd$*.pdb
!else
DBG_CFLAGS32 = /MT /O1 /D "RELEASE_VERSION" 
!endif


#
# Compilation rules
#

CC = cl


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

#CLIBS = oldnames.lib wsock32.lib \
 #       libcmt$(CLMT_MD_DEBUG).lib kernel32.lib user32.lib netapi32.lib advapi32.lib gdi32.lib comdlg32.lib \
 #       comctl32.lib


LFLAGS=$(LINKFLAGS)


OUTPUTS =        lmflex.exe \
                lmcrypt.exe \
                lmclient.exe  \
                lmsimple.exe  \
                vdinfo.exe                
#                borrutil.exe \
#                borrowa.exe \


OUTPUTS16 =     lmcrypt.exe


all : $(OUTPUTS)

clean :
        @echo Cleaning up test
        @if exist *.expand del *.expand
        @if exist *.obj del *.obj
        @if exist *.sbr del *.sbr
        @if exist *.pdb del *.pdb
        @if exist *.pch del *.pch
        @if exist *.ilk del *.ilk
        @if exist *.exp del *.exp
        @if exist *.exe del *.exe

.c.obj:
        $(CC) $(CFLAGS) $(INCS) /c $<


lmsimple.exe : lmsimple.obj lm_new.obj
        $(LD) $(LFLAGS) /out:$*.exe lmsimple.obj lm_new.obj  \
                 $(CLIBS) $(DLL32NAME) ..\$(PROCESSOR_DIRECTORY)\flock.lib

lmflex.exe : lmflex.obj lm_new.obj 
        $(LD) $(LFLAGS) /out:$*.exe lmflex.obj  lm_new.obj  \
                 $(CLIBS) $(DLL32NAME) ..\$(PROCESSOR_DIRECTORY)\flock.lib

lmclient.exe : lmclient.obj lm_new.obj 
        $(LD) $(LFLAGS) /out:$*.exe lmclient.obj lm_new.obj\
                 $(CLIBS) $(DLL32NAME) ..\$(PROCESSOR_DIRECTORY)\flock.lib

lmclient.obj: ..\machind\lmclient.c
        $(CC) $(CFLAGS) $(INCS) /c ..\machind\lmclient.c

borrutil.exe : borrutil.obj lm_new.obj
        $(LD) $(LFLAGS) /out:$*.exe borrutil.obj  lm_new.obj\
                 $(LIBS) $(CLIBS)
        lib /NOLOGO  /subsystem:console /machine:$(L_TARGET) /out:borrutil.lib borrutil.obj

vdinfo.exe:     vdinfo.obj
        $(LD) $(LFLAGS) /out:$*.exe vdinfo.obj lm_new.obj  $(LIBS) $(CLIBS)
        

borrowa.exe : borrowa.obj $(LIBS)
        $(LD) $(LFLAGS) /out:$*.exe borrowa.obj \
                 $(LIBS) $(CLIBS)


lmrand1.exe : lmrand1.obj lm_rand3.obj
        $(LD) $(LFLAGS) /out:$*.exe lmrand1.obj lm_rand3.obj \
                 $(LIBS) $(CLIBS)


#
# Intermediate file targets
#


lm_new.obj :   ..\utils\lmrand1.exe ..\utils\lmnewgen.obj ..\src\l_intelid.obj
        ..\utils\lmrand1 -i ..\machind\lsvendor.c -o ..\utils\lmcode.c ..\utils\lsrvend.c
        -if exist lmappfil.c del lmappfil.c
        -if exist lmkeyfil.c del lmkeyfil.c
        ..\utils\lmrand1 -filter_gen 0x23456789 0x3456789a 0x456789ab -q
        $(CC) $(CFLAGS) $(INCS) /c ..\utils\lmcode.c
        $(LD) $(LFLAGS) /out:lmnewgen.exe ..\test\lmcode.obj \
            ..\utils\lmnewgen.obj $(CLIBS) $(DLL32NAME) \
            ..\$(PROCESSOR_DIRECTORY)\libsb.lib \
            ..\$(PROCESSOR_DIRECTORY)\libcrvs.lib 
        if exist lm_new.c del  lm_new.c  
        lmnewgen demo -o lm_new.c
        $(CC) $(CFLAGS) $(INCS) /c lm_new.c /Folm_new.obj


vdinfo.obj : vdinfo.c
        $(CC) $(CFLAGS) $(INCS) /c vdinfo.c 

lmsimple.obj : ..\machind\lmsimple.c
        $(CC) $(CFLAGS) $(INCS) /c ..\machind\lmsimple.c 

lmflex.obj : ..\machind\lmflex.c
        $(CC) $(CFLAGS) $(INCS) /c ..\machind\lmflex.c 

borrow.obj :
        $(CC) $(CFLAGS) $(INCS) /c borrow.c 

borrowa.obj : borrutil.c
        $(CC) $(CFLAGS) /D"DEMOF" $(INCS) /c borrutil.c /Foborrowa.obj


!ifdef PHASE2
lmkeyfil.obj :
        $(CC) $(CFLAGS) $(INCS) /c lmkeyfil.c 

lmappfil.obj :
        $(CC) $(CFLAGS) $(INCS) /c lmappfil.c 
!endif

#
#
# In-line-file commands.  (They're messy so I always put
# them at the end.
#

link_16lmcrypt :
        echo >NUL @<<$*.rsp
lmcrypt.obj
lmcrypt.exe
nul
$(BASE16)\lib\+
$(CLIBS16)
$nul;
<<
        $(LD) $(LFLAGS16) @$*.rsp

#*******************************
# Dependencies
#*******************************

$(OUTPUTS) : ..\h\lm_code.h ..\machind\lmclient.h $(LIBS)
$(OUTPUTS16) : ..\h\lm_code.h ..\machind\lmclient.h 

