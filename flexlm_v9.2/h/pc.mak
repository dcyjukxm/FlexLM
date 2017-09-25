#########################################################################
#            COPYRIGHT (c) 2002 by Macrovision Corporation.
#        This software has been provided pursuant to a License Agreement
#        containing restrictions on its use.  This software contains
#        valuable trade secrets and proprietary information of 
#        Macrovision Corporation and is protected by law.  It may 
#        not be copied or distributed in any form or medium, disclosed 
#        to third parties, reverse engineered or used in any manner not 
#        provided for in said License Agreement except with the prior 
#        written authorization from Macrovision Corporation.
##########################################################################
# $Id: pc.mak,v 1.42 2002/12/12 16:48:49 kmaclean Exp $
#
# Makefile to that defined the MACROS for various PC builds.
# This file is included by other pc.mak files
#
# To build a kit hosted on the PC but targeted for VXWORKS 
# invoke nmake with /DTARGET_VXWORKS in the CFLAGS


#!if "$(FLEXDIR)" == ""
#!MESSAGE NO FLEXDIR
#FLEXDIR=$(MAKEDIR)\..\
#!else
#!MESSAGE FLEXDIR is "$(FLEXDIR)"
#!endif
!if "$(FLEXLM_MAKE_DEBUGFLAG)" != "NONDEBUG"
!MESSAGE [DEBUG]
DEBUG=1
CLDEBUG_32 = /Zi
CLDEBUG_16 = /D _DEBUG /Zi /Od /Gx-
LINKDEBUG = /DEBUG
LINKDEBUG16 = /CO
CLMT_MD_DEBUG=d
LCMOBJDIR = debug
!else
!MESSAGE [NONDEBUG]
CLMT_MD_DEBUG=
CLDEBUG_32=
CLDEBUG_16 =   /D "NDEBUG"  /O1 /D 
LINKDEBUG=
LINKDEBUG16=
LCMOBJDIR = release
!endif
CERTDIR = \certicom

!if "$(FLEXLM_MAKE_RELEASEFLAG)" == "RELEASE"
!MESSAGE [RELEASE VERSION]
CLRELEASE=/DRELEASE_VERSION
LINKRELEASE = /INCREMENTAL:NO
!else
!MESSAGE [TEST VERSION]TRADLL
CLRELEASE=
LINKRELEASE=
!endif

!if "$(FLEXLM_MAKE_DLLFLAG)" == "DLL"
!MESSAGE [DLL]
DLL=1
DLL32NAME=..\src\lmgr8b.lib
DLL32PREFIX=lmgr8b
STATIC_INCLUDE=lmgr8b.lib 
#DIALOG_INCLUDE=lmgr.res
CLSTATIC=/DFLEXLM_DLL
XTRACLIBS= $(XTRACLIBS)
!if "$(FLEXLM_MAKE_PLATFORM)" == "i86_n3"
XTRADLL= ..\src\libs\intel\spromeps.lib
!else
XTRADLL=
#
# DEFAULT behavior
#
!ifndef FLEXLM_MAKE_PLATFORM
XTRACLIBS= ..\src\libs\intel\spromeps.lib $(XTRACLIBS)
!else
XTRACLIBS= $(XTRACLIBS)
!endif

!endif
!else
!MESSAGE [STATIC]
DLL32NAME=..\src\lmgr.lib  
DLL32PREFIX=lmgr
CLSTATIC=/DFLEX_STATIC
STATIC=1
STATIC_INCLUDE=lmgr.lib 
#DIALOG_INCLUDE=lmgr.res
!if "$(FLEXLM_MAKE_PLATFORM)" == "i86_n3"
XTRACLIBS= ..\src\libs\intel\spromeps.lib $(XTRACLIBS)
!else
XTRACLIBS= $(XTRACLIBS)
!endif
#
# DEFAULT behavior
#
!ifndef FLEXLM_MAKE_PLATFORM
XTRACLIBS= ..\src\libs\intel\spromeps.lib $(XTRACLIBS)
!else
XTRACLIBS= $(XTRACLIBS)
!endif
!endif



!if "$(FLEXLM_MAKE_PLATFORM)" == "i86_w3"
!MESSAGE [16 bit]
16BIT=1
CL16_32= /G2 /GD /GEf  /W3 /ALw  /D "_WINDOWS" $(CLDEBUG_16)
LINK16_32 =  /NOD  /PACKC:61440 /SEG:512 /ALIGN:16 /MAP:FULL /ONERROR:NOEXE $(LINKDEBUG16)
!else
!MESSAGE [32 bit]
CL16_32 =  $(CLDEBUG_32) 
LINK16_32=$(LINKDEBUG)
!endif

!if "$(FLEXLM_MAKE_PLATFORM)" == "alpha_n3"
L_TARGET=ALPHA
LIBPLAT=ALPHA
PLAT_INCLUDE="_ALPHA_"
!endif

!if "$(FLEXLM_MAKE_PLATFORM)" == "i86_n3"
L_TARGET=i386
PLAT_INCLUDE="_x86_"
LIBPLAT=INTEL
PROCESSOR_DIRECTORY = i86_n3
!endif

!ifndef FLEXLM_MAKE_PLATFORM
L_TARGET=i386
PLAT_INCLUDE="_x86_"
LIBPLAT=INTEL
PROCESSOR_DIRECTORY = i86_n3
!endif

!if "$(FLEXLM_MAKE_PLATFORM)" == "i64_n5"
!MESSAGE [64 bit]
CC = cl.exe
CPU = IA64
L_TARGET = IA64
LIBPLAT = INTEL
PLAT_INCLUDE = "_ia64_"
PROCESSOR_DIRECTORY = it64_n
!endif

!if "$(FLEXLM_MAKE_MT)" == "MD"
CLCRT= /MD$(CLMT_MD_DEBUG)
CL_STATIC_MD=/DFLEX_STATIC_MD
!MESSAGE [/MD]
!else
!MESSAGE [/MT]
CLCRT= /MT$(CLMT_MD_DEBUG)
!if "$(FLEXLM_MAKE_DLLFLAG)" == "DLL"
CL_STATIC_MD=
!else
CL_STATIC_MD=$(CLSTATIC)_MT
!endif
!endif

LB = LIB
LD = $(LD) 
!if "$(LD)" == ""
LD = LINK
!endif
CC = $(CC)
RES2C = res2c.exe

CLFLAGS = /nologo /W2 /D_CONSOLE /D$(PLAT_INCLUDE) /DPC /DWINNT /DFLEXLM_KITBUILD $(CLRELEASE) $(CLSTATIC) /YX $(CL_STATIC_MD) /DLM_INTERNAL /D_LMGR_WINDLL  $(CLCRT) $(CL16_32)   /I .  /I ..\h  /I ..\machind   /I ..\patches  /c 


#

LINKFLAGS = /nologo /NOD /subsystem:console /machine:$(L_TARGET) \
                $(LINK16_32)  $(LINKRELEASE)  

CLIBS = oldnames.lib \
        ..\$(PROCESSOR_DIRECTORY)\libcrvs.lib \
        ..\$(PROCESSOR_DIRECTORY)\libsb.lib \
        wsock32.lib \
        libcmt$(CLMT_MD_DEBUG).lib \
        kernel32.lib \
        user32.lib \
        advapi32.lib \
        netapi32.lib \
        comctl32.lib \
        gdi32.lib \
        comdlg32.lib $(XTRACLIBS)
#        comdlg32.lib  \
#        ..\src\libs\$(LIBPLAT)\spromeps.lib 

LIBS =   ..\app\lmgras.lib \
        ..\server\lmgrs.lib \
        ..\src\$(STATIC_INCLUDE)  
