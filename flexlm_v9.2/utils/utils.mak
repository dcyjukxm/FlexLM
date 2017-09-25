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
#       Module: $Id: utils.mak,v 1.12 1999/02/05 03:03:44 daniel Exp $
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
!ifndef PROCESSOR_ARCHITECTURE
PROCESSOR_ARCHITECTURE=INTEL
!endif

PLAT = $(PROCESSOR_ARCHITECTURE)

!if "$(PLAT)"=="x86"
PLAT = INTEL
!endif

!ifdef 16BIT
!MESSAGE [Windows 3.1]
L_TARGET = unused
!else if "$(PLAT)" == "INTEL"
!MESSAGE [Intel NT]
L_TARGET = i386
!else if "$(PLAT)" == "ALPHA"
!MESSAGE [Alpha NT]
L_TARGET = alpha
!else if "$(PLAT)" == "MIPS"
!MESSAGE [Mips NT]
L_TARGET = mips
!else
!MESSAGE WARNING:  Unknown platform specified: $(PLAT)
!endif

#****************************
# Control values
#****************************
!ifdef STATIC
DLL32NAME = lmgr.lib
!else
DLL32NAME = lmgr327a.lib
!endif

!ifdef 16BIT
PLAT = win16
!endif

!ifdef DEBUG
INTOBJDIR = $(PLAT)_DEBUG
BASEOBJDIR= $(PLAT)_DEBUG
!else
INTOBJDIR = $(PLAT)_REL
BASEOBJDIR=  $(PLAT)_REL
!endif

!ifdef STATIC
LIBOBJDIR=S$(INTOBJDIR)
!else
LIBOBJDIR=$(INTOBJDIR)
!endif
OBJDIR=$(LIBOBJDIR)


#****************************
# Directories
#****************************

BASE16 = f:\apps\msvc


BASEDIR = ..

APPDIR          = $(BASEDIR)\app
APPDIROBJS      = $(BASEDIR)\app\$(BASEOBJDIR)
MASTERDIR       = $(BASEDIR)\master
MASTERDIROBJS   = $(BASEDIR)\master\$(BASEOBJDIR)
SERVERDIR       = $(BASEDIR)\server
SERVERDIROBJS   = $(BASEDIR)\server\$(BASEOBJDIR)
SRCDIR          = $(BASEDIR)\src
SRCDIROBJS      = $(BASEDIR)\src\$(LIBOBJDIR)
UTILSDIR        = $(BASEDIR)\utils
UTILSDIROBJS    = $(BASEDIR)\utils\$(OBJDIR)
INCDIR          = ..\h
#****************************
# Compilation information
#****************************

!ifdef STATIC
STATIC_RES=$(SRCDIROBJS)\lmgr326a.res
!else
STATIC_RES=
!endif

#
# Include files
#

INCS =  /I . \
        /I ..\h \
         /I ..\machind  \
        /I ..\server \
        /I "$(MSDEVDIR)"\INCLUDE \
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
!ifdef DEBUG
DBG_CFLAGS32 = /MTd /Zi  \
                /Fd$*.pdb
!else
DBG_CFLAGS32 = /MT /O1 /D "RELEASE_VERSION" 
!endif

CFLAGS32 = /nologo /W2 /D"_CONSOLE" /D"PC" /D"WINNT" \
                /YX /Fp$*.pch $(INCS)  $(PHASE) \
                /D"LM_INTERNAL" /Fo$(OBJDIR)\ $(DBG_CFLAGS32)

#
# 16 bit compiler flags
#

!ifdef DEBUG
DBG_CFLAGS16 =  /D "_DEBUG" /Zi /Od
!else
DBG_CFLAGS16 =  /Ox /D "NDEBUG" /Gs /O1/D "RELEASE_VERSION" 
!endif

CFLAGS16 = /nologo /G2 /Mq /W3 /ALw /Gt3 /D "PC" \
                /D "_WINDOWS" /D "LM_INTERNAL" /Fo$(OBJDIR)\ \
                /FR$*.sbr /Fd$(OBJDIR)\utils.pdb \
                /YX /Fp$(OBJDIR)\utils.pch \
                $(DBG_CFLAGS16)

#
# Set the general macro depending if we're a sixteen or
# 32 bit comilation.
#

!ifdef 16BIT
CFLAGS = $(CFLAGS16)
!else
CFLAGS = $(CFLAGS32)
!endif

#
# Compilation rules
#

CC = cl

.c{$(OBJDIR)\}.obj:
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

CLIBS = oldnames.lib wsock32.lib \
        libcmt.lib kernel32.lib user32.lib netapi32.lib advapi32.lib gdi32.lib comdlg32.lib \
        comctl32.lib

LIBS =          ..\src\$(LIBOBJDIR)\$(DLL32NAME)

DAEMON_LIBS =   $(SERVERDIROBJS)\lmgrs.lib \
                $(APPDIROBJS)\lmgras.lib \
                $(LIBS)

#
# 16 bit libraries
#

CLIBS16 =       oldnames \
                libw \
                llibcewq \
                lzexpand

#
# Link flags
#

!ifdef DEBUG
DBG_LFLAGS = /DEBUG /pdb:$*.pdb
!else
DBG_LFLAGS = /INCREMENTAL:NO
!endif

!ifdef PROFILE
DBG_LFLAGS = $(DBG_LFLAGS) /profile /map
!endif

!ifdef MAP
DBG_LFLAGS = $(DBG_LFLAGS) /map
!endif

LFLAGS = /NOLOGO /NOD /subsystem:console /machine:$(L_TARGET) \
                $(DBG_LFLAGS)
LIBFLAGS = /nologo  /machine:$(L_TARGET)
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

UTILOBJS =          $(OBJDIR)\lm_cksum.obj \
                $(OBJDIR)\lm_diag.obj \
                $(OBJDIR)\lm_diag_ck.obj \
                $(OBJDIR)\lm_hostid.obj \
                $(OBJDIR)\lm_lic_info.obj \
                $(OBJDIR)\lm_spec.obj \
                $(OBJDIR)\lm_stat.obj \
                $(OBJDIR)\lm_ver.obj \
                $(OBJDIR)\lm_down.obj \
                $(OBJDIR)\lm_remov.obj \
                $(OBJDIR)\lm_rerd.obj \
                $(OBJDIR)\lm_swr.obj \
                $(OBJDIR)\lmutil.obj \
                $(OBJDIR)\lm_inst.obj \
                $(OBJDIR)\lm_dll_ver.obj \
                $(OBJDIR)\lm_boro.obj


#****************************
# General Targets

#
# Link commands
#

LD = link

#****************************
# File lists
#****************************

UTILS =         lmborrow.exe \
                lmcksum.exe \
                lmdiag.exe \
                lmdown.exe \
                lmhostid.exe \
                lminstall.exe \
                lmremove.exe \
                lmreread.exe \
                lmswitchr.exe \
                lmstat.exe \
                lmver.exe 

OUTPUTS =       $(OBJDIR)\lmrand1.exe  \
                $(OBJDIR)\makekey.exe \
                $(OBJDIR)\lmcrypt.exe \
                $(OBJDIR)\lmutil.exe \
                $(OBJDIR)\makepkg.exe \
                $(UTILS)
                



OUTPUTS16 =     $(OBJDIR)\lmcrypt.exe

#****************************
# General Targets
#****************************

!ifdef 16BIT
all : all16
!else
all : all32
!endif

all32 : timestamp $(OBJDIR) $(OUTPUTS)
        @echo !--------------------------------------
        @echo ! Done building utils
        @echo !--------------------------------------

all16 : timestamp $(OUTPUTS16)
        @echo !--------------------------------------
        @echo ! Done building utils
        @echo !--------------------------------------

timestamp :
        @echo !--------------------------------------
        @echo ! Building utils
        @echo !--------------------------------------

clean :
        @echo Cleaning up $(PLAT)
        -@del sintel_debug\*.o*
        -@del sintel_debug\*.s*
        -@del sintel_debug\*.p*
        -@del sintel_debug\*.i*
        -@del sintel_debug\*.e*
        -@del sintel_debug\*.r*
        -@del sintel_rel\*.o*
        -@del sintel_rel\*.s*
        -@del sintel_rel\*.p*
        -@del sintel_rel\*.i*
        -@del sintel_rel\*.e*
        -@del sintel_rel\*.r*
        -@del *.obj
        -@del *.l*
        -@del *.ex*
        -@del *.o*
        -@del *.i*
        -@del *.p*

$(OBJDIR) :
        -mkdir $(OBJDIR)
        

!ifndef 16bit

#
# Specific .EXE targets
#

$(OBJDIR)\demo.exe :  $(OBJDIR)\lmrand1.exe $(OBJDIR)\lsvendor.obj 
        $(LD) $(LFLAGS) /out:$*.exe $(OBJDIR)\lsvendor.obj $(OBJDIR)\lm_new.obj \
                 $(DAEMON_LIBS) $(CLIBS)


$(OBJDIR)\lmrand1.exe : $(OBJDIR)\lmrand1.obj $(OBJDIR)\lm_rand3.obj $(OBJDIR)\lmrand2.obj
        $(LD) $(LFLAGS) /out:$*.exe $(OBJDIR)\lmrand1.obj $(OBJDIR)\lm_rand3.obj \
                 $(LIBS) $(CLIBS)


$(OBJDIR)\lmcrypt.exe : $(OBJDIR)\lmcrypt.obj 
        $(LD) $(LFLAGS) /out:$*.exe $(OBJDIR)\lmcrypt.obj \
                 $(LIBS) $(CLIBS)

$(OBJDIR)\makekey.exe : $(OBJDIR)\makekey.obj 
        $(LD) $(LFLAGS) /out:$*.exe $(OBJDIR)\makekey.obj \
                 $(LIBS) $(CLIBS)

$(OBJDIR)\makekey.obj:  ..\machind\makekey.c
        $(CC) $(CFLAGS) $(INCS) /c ..\machind\makekey.c

$(OBJDIR)\lmcrypt.obj:  ..\machind\lmcrypt.c
        $(CC) $(CFLAGS) $(INCS) /c ..\machind\lmcrypt.c

$(OBJDIR)\makepkg.obj:  ..\machind\makepkg.c
        $(CC) $(CFLAGS) $(INCS) /c ..\machind\makepkg.c

$(OBJDIR)\lmutil.exe : $(UTILOBJS) 
        $(LD) $(LFLAGS) /out:$(OBJDIR)\lmutil.exe $(UTILOBJS)  $(LIBS) $(CLIBS)

lmborrow.exe:       $(OBJDIR)\lmutil.exe
        copy $(OBJDIR)\lmutil.exe lmborrow.exe
lmcksum.exe:       $(OBJDIR)\lmutil.exe
        copy $(OBJDIR)\lmutil.exe lmcksum.exe
lmdiag.exe:       $(OBJDIR)\lmutil.exe
        copy $(OBJDIR)\lmutil.exe lmdiag.exe
lmdown.exe:       $(OBJDIR)\lmutil.exe
        copy $(OBJDIR)\lmutil.exe lmdown.exe
lmhostid.exe:       $(OBJDIR)\lmutil.exe
        copy $(OBJDIR)\lmutil.exe lmhostid.exe
lmremove.exe:       $(OBJDIR)\lmutil.exe
        copy $(OBJDIR)\lmutil.exe lmremove.exe
lmreread.exe:       $(OBJDIR)\lmutil.exe
        copy $(OBJDIR)\lmutil.exe lmreread.exe
lmswitchr.exe:       $(OBJDIR)\lmutil.exe
        copy $(OBJDIR)\lmutil.exe lmswitchr.exe
lmstat.exe:       $(OBJDIR)\lmutil.exe
        copy $(OBJDIR)\lmutil.exe lmstat.exe
lmver.exe:       $(OBJDIR)\lmutil.exe
        copy $(OBJDIR)\lmutil.exe lmver.exe

!ifdef PHASE2



!endif

!else

#
# 16 bit targets
#

#lmcrypt.exe : $(OBJDIR)\lmcrypt.obj link_16lmcrypt

!endif

#
# Intermediate file targets
#

#lsr_vend.c : $(OBJDIR)\lmrandom.exe lsvendor.c
#       $(OBJDIR)\lmrandom < lsvendor.c > lsr_vend.c

$(OBJDIR)\lm_new.obj : $(OBJDIR)\lmrand2.obj $(OBJDIR)\lmrand1.exe
        $(OBJDIR)\lmrand1 -o $(OBJDIR)\lmcode.c $(OBJDIR)\lsrvend.c
        -del lmappfil.c
        -del lmkeyfil.c
        $(OBJDIR)\lmrand1 -filter_gen 0x23456789 0x3456789a 0x456789ab -q
        $(CC) $(CFLAGS) $(INCS) /c $(OBJDIR)\lmcode.c
        $(LD) $(LFLAGS) /out:$(OBJDIR)\lmrand2.exe $(OBJDIR)\lmcode.obj $(OBJDIR)\lmrand2.obj \
                $(LIBS) $(CLIBS)
        del  $(OBJDIR)\lm_new.c  
        $(OBJDIR)\lmrand2 demo -o $(OBJDIR)\lm_new.c
        $(CC) $(CFLAGS) $(INCS) /c $(OBJDIR)\lm_new.c /Fo$(OBJDIR)\lm_new.obj



$(OBJDIR)\lmrand2.obj :
        $(CC) $(CFLAGS) $(INCS) /c lmrand2.c /Fo$(OBJDIR)\lmrand2.obj


$(OBJDIR)\lm_rand3.obj :
        $(CC) $(CFLAGS) $(INCS) /c lm_rand3.c 

$(OBJDIR)\lmflex.obj :
        $(CC) $(CFLAGS) $(INCS) /c lmflex.c 

!ifdef PHASE2
$(OBJDIR)\lmkeyfil.obj :
        $(CC) $(CFLAGS) $(INCS) /c lmkeyfil.c 

$(OBJDIR)\lmappfil.obj :
        $(CC) $(CFLAGS) $(INCS) /c lmappfil.c 
!endif

#
#
# In-line-file commands.  (They're messy so I always put
# them at the end.
#

link_16lmcrypt :
        echo >NUL @<<$*.rsp
$(OBJDIR)\lmcrypt.obj
$(OBJDIR)\lmcrypt.exe
nul
$(BASE16)\lib\+
$(CLIBS16)
$nul;
<<
        $(LD) $(LFLAGS16) @$*.rsp

#*******************************
# Dependencies
#*******************************

$(OUTPUTS) : ..\machind\lm_code.h ..\machind\lmclient.h ..\h\lmachdep.h \
                ..\h\l_prot.h ..\h\l_privat.h $(LIBS)
$(OUTPUTS16) : ..\machind\lm_code.h
