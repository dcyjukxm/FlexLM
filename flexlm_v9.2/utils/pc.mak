#########################################################################
#            COPYRIGHT (c) 1996-2003 by Macrovision Corporation.
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
#       Module: $Id: pc.mak,v 1.32 2003/05/08 00:03:25 sluu Exp $
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

#
# Include files
#

INCS =  /I . \
        /I ..\h \
         /I ..\machind  \
        /I ..\server \
        /I ..\app \
        /I ..\patches \
        /I ..\vendor

#
# 32 bit compiler flags
#
!ifdef PHASE2
PHASE = /D"PHASE2"
!else
PHASE = 
!endif

#
# Set the general macro depending if we're a sixteen or
# 32 bit comilation.
#

CFLAGS = $(CLFLAGS)

#
# Compilation rules
#

CC = cl
RC = rc

.c.obj:
        $(CC) $(CLFLAGS) $(INCS) /c $<

#
# This will make a version of the file that
# has the macros expanded.
#

.c.expand:
        $(CC) $(CLFLAGS) /E $(INCS) /c $< > $*.expand

#****************************
# Link information
#****************************

#
# Libraries
#


#PUBKEY_LIBS =   ..\pubkey\$(PROCESSOR_DIRECTORY)\libcrvs.lib \
#                ..\pubkey\$(PROCESSOR_DIRECTORY)\libsb.lib

#LIBS =          ..\src\$(DLL32NAME) $(PUBKEY_LIBS)

DAEMON_LIBS =   ..\server\\lmgrs.lib \
                ..\app\lmgras.lib \
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

LFLAGS = $(LINKFLAGS)  $(DBG_LFLAGS)

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

UTILOBJS =          lm_cksum.obj \
                lm_diag.obj \
                lm_diag_ck.obj \
                lm_hostid.obj \
                lm_lic_info.obj \
                lm_spec.obj \
                lm_stat.obj \
                lm_ver.obj \
                lm_down.obj \
                lm_remov.obj \
                lm_rerd.obj \
                lm_swr.obj \
		lmborrow.obj \
		lm_path.obj \
                lmutil.obj \
                lm_inst.obj \
                lm_dll_ver.obj \
                ..\src\flex_utils.obj


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
		lmpath.exe \
                lmremove.exe \
                lmreread.exe \
                lmswitchr.exe \
                lmstat.exe \
                lmswitch.exe \
                lmver.exe \
		lmnewgen.obj

OUTPUTS =       makekey.exe \
                lmstrip.exe \
                lmcrypt.exe \
                lmutil.exe \
                makepkg.exe \
                lmkey.exe \
                $(UTILS)

LICENSE_OBJS = makekey.obj \
                lmcrypt.obj \
                makepkg.obj
                



OUTPUTS16 =     lmcrypt.exe

#****************************
# General Targets
#****************************

all : lmrand1.exe lmnewgen.obj $(OUTPUTS)
	cd lmstrip
	$(MAKE) -f pc.mak 
	cd ..
        @echo !--------------------------------------
        @echo ! Done building utils
        @echo !--------------------------------------

clean :
        @echo Cleaning up utils
        @if exist *.obj del *.obj
        @if exist *.l* del *.l*
        @if exist *.ex* del *.ex*
        @if exist *.o* del *.o*
        @if exist *.i* del *.i*
        @if exist *.p* del *.p*
	cd lmstrip
	$(MAKE) -f pc.mak clean
	cd ..


!ifndef 16bit

#
# Specific .EXE targets
#

demo.exe :  lmrand1.exe lsvendor.obj 
        $(LD) $(LFLAGS) /out:$*.exe lsvendor.obj lm_new.obj \
                 $(DAEMON_LIBS) $(CLIBS) $(DLL32NAME)


lmrand1.exe : lmrand1.obj ..\src\lmgr.lib
        $(LD) $(LFLAGS) /out:$*.exe lmrand1.obj \
                 $(CLIBS) ..\src\lmgr.lib


lmcrypt.exe : lmcrypt.obj  ..\vendor\lmprikey.h
        $(LD) $(LFLAGS) /out:$*.exe lmcrypt.obj  \
                 $(LIBS) $(CLIBS)

makekey.exe : makekey.obj  ..\vendor\lmprikey.h
        $(LD) $(LFLAGS) /out:$*.exe makekey.obj \
                 $(LIBS) $(CLIBS)

lmkey.exe : lmkey.obj 
        $(LD) $(LFLAGS) /out:$*.exe lmkey.obj \
                 $(LIBS) $(CLIBS)

makekey.obj:  ..\machind\makekey.c ../vendor/lmprikey.h
        $(CC) $(CFLAGS) $(INCS) /c ..\machind\makekey.c

lmcrypt.obj:  ..\machind\lmcrypt.c ..\vendor\lmprikey.h 
        $(CC) $(CFLAGS) $(INCS) /c ..\machind\lmcrypt.c

..\vendor\lmprikey.h: lmrand1.exe lmnewgen.obj ..\machind\lm_code.h
        cd ..\vendor 
        $(MAKE) /f pc.mak lm_new.obj
        cd ..\utils

makepkg.obj:  ..\machind\makepkg.c ../vendor/lmprikey.h
        $(CC) $(CFLAGS) $(INCS) /c ..\machind\makepkg.c

lmutil.exe : $(UTILOBJS) 
	$(RC) lmutil.rc
        $(LD) $(LFLAGS) /out:lmutil.exe $(UTILOBJS) lmutil.res $(LIBS) $(CLIBS)

lmborrow.exe:       lmutil.exe
        copy lmutil.exe lmborrow.exe
lmcksum.exe:       lmutil.exe
        copy lmutil.exe lmcksum.exe
lmdiag.exe:       lmutil.exe
        copy lmutil.exe lmdiag.exe
lmdown.exe:       lmutil.exe
        copy lmutil.exe lmdown.exe
lmhostid.exe:       lmutil.exe
        copy lmutil.exe lmhostid.exe
lmpath.exe:       lmutil.exe
        copy lmutil.exe lmpath.exe
lmremove.exe:       lmutil.exe
        copy lmutil.exe lmremove.exe
lmreread.exe:       lmutil.exe
        copy lmutil.exe lmreread.exe
lmswitchr.exe:       lmutil.exe
        copy lmutil.exe lmswitchr.exe
lminstall.exe:       lmutil.exe
        copy lmutil.exe lminstall.exe
lmstat.exe:       lmutil.exe
        copy lmutil.exe lmstat.exe
lmswitch.exe:       lmutil.exe
        copy lmutil.exe lmswitch.exe
lmver.exe:       lmutil.exe
        copy lmutil.exe lmver.exe

!ifdef PHASE2



!endif

!else

#
# 16 bit targets
#

#lmcrypt.exe : lmcrypt.obj link_16lmcrypt

!endif

#
# Intermediate file targets
#

#lsr_vend.c : lmrandom.exe lsvendor.c
#       lmrandom < lsvendor.c > lsr_vend.c

lm_new.obj : lmnewgen.obj lmrand1.exe ..\machind\lmclient.h utilscode.h
        lmrand1 -o lmcode.c lsrvend.c
        @if exist lmappfil.c del lmappfil.c
        @if exist lmkeyfil.c del lmkeyfil.c
        lmrand1 -filter_gen 0x23456789 0x3456789a 0x456789ab -q
        $(CC) $(CFLAGS) $(INCS) /c lmcode.c
        $(LD) $(LFLAGS) /out:lmnewgen.exe lmcode.obj lmnewgen.obj \
                $(LIBS) $(CLIBS)
        @if exist lm_new.c del  lm_new.c  
        lmnewgen demo -o lm_new.c
        $(CC) $(CFLAGS) $(INCS) /c lm_new.c /Folm_new.obj

dant.exe :	
	$(LD) $(LFLAGS) /out:dant.exe dant.obj \
                $(LIBS) $(CLIBS)

lmnewgen.obj :
        $(CC) $(CFLAGS) /Fdlmnewgen.pdb $(INCS) /c lmnewgen.c /Folmnewgen.obj

lmnewgen_md.obj :
        $(CC) $(CFLAGS) /Fdlmnewgen_md.pdb $(INCS) /c lmnewgen.c /Folmnewgen_md.obj


lmflex.obj :
        $(CC) $(CFLAGS) $(INCS) /c lmflex.c 

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

$(OUTPUTS) :  $(LIBS)
$(UTILOBJS) $(LICENSE_OBJS): ..\h\lm_code.h ..\machind\lmclient.h ..\h\lmachdep.h \
                ..\h\l_prot.h ..\h\l_privat.h  ..\machind\lm_code2.h

$(OUTPUTS16) : ..\h\lm_code.h
$(LICENSE_OBJS) : ..\h\lm_code.h ..\machind\lm_code2.h
