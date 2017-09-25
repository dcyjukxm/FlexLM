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
#       Module: $Id: lmutil.mak,v 1.6 1999/02/05 03:03:44 daniel Exp $
#
#       Description:  Builds lmutil.exe
#
#       To build:
#
#               nmake /f lmutil.mak [DEBUG=] [PROFILE=] [16BIT=]
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
MSDEVDIR=d:\msvc
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

OUTNAMEBASE = lmutil
OUTNAME = $(OUTNAMEBASE).exe
LMGR32DLL = lmgr327a.lib
LMGR16DLL = lmgr167a.lib

!ifdef 16BIT
PLAT = win16
!endif

!ifdef DEBUG
OBJDIR = $(PLAT)_DEBUG
NONSDIR= $(PLAT)_DEBUG
!else
OBJDIR = $(PLAT)_REL
NONSDIR= $(PLAT)_REL
!endif



!ifdef STATIC
OBJDIR=S$(OBJDIR)
STATIC_INCLUDE=lmgr.lib
!else
STATIC_INCLUDE=lmgr327a.lib
!endif



#****************************
# Directories
#****************************

BASE16 = n:\msvc

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
        /I $(MSDEVDIR)\INCLUDE \
        /I ..\patches

#
# 32 bit compiler flags
#

!ifdef DEBUG
DBG_CFLAGS32 = /MTd /Zi  \
                /Fd$(OBJDIR)\$(OUTNAMEBASE).pdb
!else
DBG_CFLAGS32 = /MT /O1 /D "RELEASE_VERSION" 
!endif

CFLAGS32 = /nologo /W2 /D"_CONSOLE" /D"PC" /D"WINNT" \
                /YX /Fp$(OBJDIR)\$(OUTNAMEBASE).pch \
                /D"LM_INTERNAL" /Fo$(OBJDIR)\ $(DBG_CFLAGS32)

#
# 16 bit compiler flags
#

!ifdef DEBUG
DBG_CFLAGS16 =  /D "_DEBUG" /Zi /Od 
!else
DBG_CFLAGS16 =  /Ox /D "NDEBUG" /Gs /O1 /D "RELEASE_VERSION"
!endif

CFLAGS16 = /nologo /G2  /W3 /ALw /Gt3 /D "PC" \
                /D "_WINDOWS" /D "LM_INTERNAL" /Fo$(OBJDIR)\ \
                /FR$*.sbr /Fd$(OBJDIR)\$(OUTNAMEBASE).pdb \
                /YX /Fp$(OBJDIR)\$(OUTNAMEBASE).pch \
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
        libcmt.lib kernel32.lib user32.lib advapi32.lib netapi32.lib comctl32.lib \
        gdi32.lib comdlg32.lib

LIBS =  ..\src\$(OBJDIR)\$(STATIC_INCLUDE)

#
# 16 bit libraries
#

CLIBS16 =       oldnames \
                libw \
                llibcewq \
                lzexpand

LIBS16 = ..\src\$(OBJDIR)\$(LMGR16DLL) ..\src\winsockx.lib

#
# Link flags
#

!ifdef DEBUG
DBG_LFLAGS = /DEBUG /pdb:$(OBJDIR)\$(OUTNAMEBASE).pdb
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

#
# Link commands
#

LD = link

#****************************
# File lists
#****************************

OBJS =          $(OBJDIR)\lm_cksum.obj \
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
#****************************

all : timestamp $(OBJDIR)\$(OUTNAME)
        @echo !--------------------------------------
        @echo ! Done building utils
        @echo !--------------------------------------

timestamp :
        @echo !--------------------------------------
        @echo ! Building $(OUTNAME)
        @echo !--------------------------------------

!ifndef 16BIT

$(OBJDIR)\$(OUTNAME) : $(OBJS) 
        $(LD) $(LFLAGS) /out:$*.exe $(OBJS)  $(LIBS) $(CLIBS)

!else

$(OBJDIR)\$(OUTNAME) : $(OBJS) link_16lmutil

!endif

clean :
        @echo Cleaning up $(PLAT)
        -del *.expand
        cd $(OBJDIR)
        -del *.obj
        -del *.sbr
        -del *.pdb
        -del *.pch
        -del *.ilk
        -del *.exp
        -del *.exe

link_16lmutil :
        echo >NUL @<<$*.rsp
$(OBJDIR)\lm_cksum.obj +
$(OBJDIR)\lm_diag.obj +
$(OBJDIR)\lm_diag_ck.obj +
$(OBJDIR)\lm_hostid.obj +
$(OBJDIR)\lm_lic_info.obj +
$(OBJDIR)\lm_spec.obj +
$(OBJDIR)\lm_stat.obj +
$(OBJDIR)\lm_ver.obj +
$(OBJDIR)\lm_down.obj +
$(OBJDIR)\lm_remov.obj +
$(OBJDIR)\lm_rerd.obj +
$(OBJDIR)\lm_swr.obj +
$(OBJDIR)\lm_inst.obj +
$(OBJDIR)\lmutil.obj +
$(OBJDIR)\lm_dll_ver.obj
$(OBJDIR)\$(OUTNAME)
nul
$(BASE16)\lib\+
$(LIBS16)+
$(CLIBS16)
nul;
<<
        $(LD) $(LFLAGS16) @$*.rsp

#*******************************
# Dependencies
#*******************************

$(OBJDIR)\$(OUTNAME):   $(LIBS)
