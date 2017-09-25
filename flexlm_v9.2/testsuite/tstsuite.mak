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
#       Module: $Id: tstsuite.mak,v 1.11 1999/02/07 01:22:38 daniel Exp $
#
#       Description:  Test suite makefile for the Windows
#
#       C. Mayman
#       5/31/96
#
#       Last changed:  %G% 
#
#*****************************************************************************
#
# 
# Date: Wed, 6 Nov 96 13:20:46 PST
# From: Daniel Birns <daniel@globes.com>
# To: blane@ss, charliem@ss
# Subject: testsuite
# 
# Here's my recommendations for the testsuite binaries to port:
# 
# 
# DAEMONS = demo \
#               demof \
#               demof2 \             
#               demo_r \        #later
#               path2 \         #No
#               path2_add \     #no
#               path2_noadd     #no
# 
# LMGRDS = lmgrd_r              #later
# 
# CLIENTS = basic_tests \
#               bigfile \
#               cryptest \
#               jobtests \      #no
#               lmfeats \       #no
#               lmreread \
#               lock_tests \    #???
#               lsapi_tests \   #No
#               no_s_tests \    
#               one_s_tests \
#               servtest \
#               path_tests \    #no
#               printenv \      #no
#               red_s_tests \   #later
#               reptest \       
#               timers_test \   #???
#               tsclient \      #no
#               tscrypt \       #no
#               utiltest  
#
#********************************************************************************
#
# P.S. one_s_tests  is very UNIX like.  It uses lots of signals, so it is
#                   not included.
#
#      timers_test  is very UNIX like.  It uses lots of signals too.
#
#********************************************************************************

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
PLAT = WIN16
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
DLL32NAME = lmgr
!else
DLL32NAME = lmgr327a
!endif
DLL16NAME = lmgr166b

!ifdef DEBUG
INTOBJDIR = $(PLAT)_DEBUG
!else
INTOBJDIR = $(PLAT)_REL
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

BASEDIR = ..

APPDIR          = $(BASEDIR)\app
APPDIROBJS      = $(BASEDIR)\app\$(INTOBJDIR)
MASTERDIR       = $(BASEDIR)\master
MASTERDIROBJS   = $(BASEDIR)\master\S$(INTOBJDIR)
SERVERDIR       = $(BASEDIR)\server
SERVERDIROBJS   = $(BASEDIR)\server\$(INTOBJDIR)
SRCDIR          = $(BASEDIR)\src
SRCDIROBJS      = $(BASEDIR)\src\$(LIBOBJDIR)
UTILSDIR        = $(BASEDIR)\utils
UTILSDIROBJS    = $(BASEDIR)\utils\$(OBJDIR)
#****************************
# Compilation information
#****************************

#
# Include files
#

INCS =   /I .\
         /I ..\h  \
         /I ..\machind  \
        /I "$(MSDEVDIR)"\include \
         /I ..\patches \
         /I ..\server

#
# 32 bit compiler flags
#

!ifdef DEBUG
DBG_CFLAGS32 = /MTd /Zi \
                /Fd$(OBJDIR)\tstsuite.pdb
!else
DBG_CFLAGS32 = /MT /O1 /D "RELEASE_VERSION" 
!endif

CFLAGS32 = /nologo /W2 /D"_CONSOLE" /D"PC" /D"WINNT" \
                /YX /Fp$(OBJDIR)\tstsuite.pch \
                /D"LM_INTERNAL" /Fo$(OBJDIR)\ $(DBG_CFLAGS32) $(INCS)

#
# 16 bit compiler flags
#

!ifdef DEBUG
DBG_CFLAGS16 =  /D "_DEBUG" /Zi /Od
!else
DBG_CFLAGS16 =  /Ox /D "NDEBUG" /Gs /O1 /D "RELEASE_VERSION" 
!endif

CFLAGS16 = /nologo /G2 /Mq /W3 /ALw /Gt3 /D "PC" \
                /D "_WINDOWS" /D "LM_INTERNAL" /Fo$(OBJDIR)\ \
                /FR$*.sbr /Fd$(OBJDIR)\tstsuite.pdb \
                /YX /Fp$(OBJDIR)\tstsuite.pch \
                $(DBG_CFLAGS16)

#
# Set the general macro depending if we're a sixteen or
# 32 bit comilation.
#

!ifdef 16BIT
CFLAGS = $(CFLAGS16)
!else
CFLAGS = $(CFLAGS32)
INCS1 = $(INCS)
!endif

#
# Compilation rules
#

CC = cl  

.SUFFIXES :
.SUFFIXES : .c .expand .obj 

.c{$(OBJDIR)\}.obj:
        $(CC) $(CFLAGS) $(INCS) /c $<

#/I f:\v6\h $(CFLAGS)
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

CLIBS = oldnames.lib \
        libcmt.lib kernel32.lib user32.lib netapi32.lib advapi32.lib gdi32.lib comdlg32.lib \
        comctl32.lib


CLIENTLIBS = $(SRCDIROBJS)\$(DLL32NAME).lib
TESTLIBS = $(OBJDIR)\ts.lib $(CLIENTLIBS) 
LIBS =  $(APPDIROBJS)\lmgras.lib \
                $(SERVERDIROBJS)\lmgrs.lib \
                $(SRCDIROBJS)\$(DLL32NAME).lib \

LIBS_NOLMGRAS = $(SERVERDIROBJS)\lmgrs.lib \
                $(SRCDIROBJS)\$(DLL32NAME).lib

#
# 16 bit libraries
#

CLIBS16 =       oldnames \
                libw \
                llibcewq \
                lzexpand 
                

LIBS16 = $(SRCDIROBJS)\$(DLL16NAME).lib \
                ..\winsockx\winsockx.lib

#
# Link flags
#

XTRA_LFLAGS =

!ifdef DEBUG
DBG_LFLAGS = /DEBUG /pdb:$*.pdb
!else
DBG_LFLAGS =
!endif

!ifdef PROFILE
DBG_LFLAGS = $(DBG_LFLAGS) /profile /map
!endif

!ifdef MAP
DBG_LFLAGS = $(DBG_LFLAGS) /map
!endif

LFLAGS = /NOLOGO /NOD /subsystem:console /machine:$(L_TARGET) \
                $(DBG_LFLAGS) $(XTRA_LFLAGS)

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
# Library creation
#****************************

LB = lib

#****************************
# File lists
#****************************

OBJS =          $(OBJDIR)\basic_tests.obj \
                $(OBJDIR)\cryptest.obj \
                $(OBJDIR)\demof.obj \
                $(OBJDIR)\demof2.obj \
                $(OBJDIR)\demo.obj \
                $(OBJDIR)\fake_appinit.obj \
                $(OBJDIR)\fakehostname.obj \
                $(OBJDIR)\jobtests.obj \
                $(OBJDIR)\lock_tests.obj \
                $(OBJDIR)\lsapi_tests.obj \
                $(OBJDIR)\pathd.obj \
                $(OBJDIR)\path_tests.obj \
                $(OBJDIR)\no_s_tests.obj \
                $(OBJDIR)\one_s_tests.obj \
                $(OBJDIR)\bigfile.obj \
                $(OBJDIR)\servtest.obj \
                $(OBJDIR)\st_bugs.obj \
                $(OBJDIR)\printenv.obj \
                $(OBJDIR)\red_s_tests.obj \
                $(OBJDIR)\reptest.obj \
                $(OBJDIR)\tsclient.obj \
                $(OBJDIR)\tscrypt.obj \
                $(OBJDIR)\ut_flex.obj \
                $(OBJDIR)\utiltest.obj \
                $(OBJDIR)\fakehostname.obj 

LIBOBJS =       $(OBJDIR)\ts_config.obj \
                $(OBJDIR)\ts_dump.obj \
                $(OBJDIR)\ts_environ.obj \
                $(OBJDIR)\ts_hostid.obj \
                $(OBJDIR)\ts_lic_file.obj \
                $(OBJDIR)\ts_list.obj \
                $(OBJDIR)\ts_reread.obj \
                $(OBJDIR)\ts_serv_supp.obj \
                $(OBJDIR)\ts_utils.obj \
                $(OBJDIR)\st_vers.obj \
                $(OBJDIR)\ts_vmsg.obj 


DAEMONS =       $(OBJDIR)\demo.exe \
                $(OBJDIR)\demof.exe \
                $(OBJDIR)\demof2.exe \
                $(OBJDIR)\pathd.exe 

CLIENTS =        $(OBJDIR)\utiltest.exe  \
                $(OBJDIR)\servtest.exe \
		$(OBJDIR)\borrtest.exe \
		$(OBJDIR)\cryptest.exe


  

COPY_TARGETS =  $(OBJDIR)\lmgrd.exe 
#                $(OBJDIR)\lmgr326a.dll 
#               $(OBJDIR)\lmutil.exe

!ifdef DEBUG
!ifndef STATIC
COPY_TARGETS =  $(COPY_TARGETS) \
                $(OBJDIR)\lmgr.pdb 
 
!endif
!endif

#
# In debuggin mode, get the PDB file so we can
# debug the DLL.
#

!ifdef DEBUG
PDB_FILES = $(OBJDIR)\$(DLL32NAME).pdb
PDB_FILES16 = $(OBJDIR)\$(DLL16NAME).pdb
!else
PDB_FILES =
PDB_FILES16 =
!endif

LM_PARTS =      $(OBJDIR)\lmgrd.exe \
                $(OBJDIR)\$(DLL32NAME).dll \
                $(PDB_FILES)

LM_PARTS16 =    $(OBJDIR)\$(DLL16NAME).dll \
                $(PDB_FILES16)

#****************************
# General Targets
#****************************

!ifdef 16BIT
all : all16
!else
all : $(CLIENTS) timestamp $(DAEMONS) $(COPY_TARGETS)

!endif

all32 : timestamp $(DAEMONS) $(CLIENTS) $(LM_PARTS) $(COPY_TARGETS) install 
        @echo !--------------------------------------
        @echo ! Done building server test components
        @echo !--------------------------------------

timestamp :
        @echo !--------------------------------------
        @echo ! Building server test components
        @echo !--------------------------------------

all16 : timestamp $(OBJDIR)\srvtst16.exe $(LM_PARTS16) install
        @echo !--------------------------------------
        @echo ! Done building 16 bit test components
        @echo !--------------------------------------

install :
        @echo Installing binaries from $(OBJDIR)
        cd $(OBJDIR)
        -copy *.exe ..
        -copy *.dll ..
!ifdef DEBUG
        -copy *.pdb ..
!endif

daemons : $(DAEMONS)
        @echo !--------------------------------------
        @echo ! Done building daemons
        @echo !--------------------------------------

servtest_only : timestamp $(OBJDIR)\servtest.exe $(DAEMONS) $(COPY_TARGETS)
        @echo !--------------------------------------
        @echo ! Done building servtest components
        @echo !--------------------------------------
        
clean :
        @echo Cleaning up $(PLAT)
        -@del *.out
        -@del *.log
        -@del license.dat
        -@del pathtest.dat
        -@del rlicense*
        -@del opts
        -@del servtest.err
        -@del _*
        -@del bigfile.dat
        -@del *.ex*
        -@del *.met
        -@del *.o*
        -@del *.p*
        -@del *.ilk
        -@del *.l*
        -@del dant*
        -@del dant*.*
        -@del borrow.dat
        -@del tmp.dat
        -@del nosuch.dat
        -@del sintel_debug\*.e*
        -@del sintel_debug\*.l*
        -@del sintel_debug\*.o*
        -@del sintel_debug\*.i*
        -@del sintel_debug\*.p*
        -@del sintel_debug\*.r*
        -@del sintel_rel\*.e*
        -@del sintel_rel\*.l*
        -@del sintel_rel\*.o*
        -@del sintel_rel\*.i*
        -@del sintel_rel\*.p*
        -@del sintel_rel\*.r*
        -@del intel_rel\*.e*
        -@del intel_rel\*.l*
        -@del intel_rel\*.o*
        -@del intel_rel\*.i*
        -@del intel_rel\*.p*
        -@del intel_rel\*.r*

#****************************
# EXE targets
#****************************

$(OBJDIR)\ts.lib : $(LIBOBJS)
        $(LB) /out:$(OBJDIR)\ts.lib $(LIBOBJS)

$(OBJDIR)\basic_tests.exe : testsuite.h $(OBJDIR)\basic_tests.obj \
                        $(TESTLIBS)
        $(LD) $(LFLAGS) /out:$*.exe $(OBJDIR)\basic_tests.obj \
                $(TESTLIBS) $(CLIBS)


$(OBJDIR)\demo.exe : $(LIBS) $(OBJDIR)\demo.obj $(OBJDIR)\lm_new.obj $(OBJDIR)\ts_hostid.obj $(OBJDIR)\ts_vmsg.obj $(SRCDIROBJS)\$(DLL32NAME).lib
        $(LD) $(LFLAGS) /out:$*.exe $(OBJDIR)\demo.obj $(OBJDIR)\ts_hostid.obj \
                $(OBJDIR)\ts_vmsg.obj $(OBJDIR)\lm_new.obj $(LIBS) \
                $(CLIBS)

$(OBJDIR)\demof2.exe : $(LIBS) $(OBJDIR)\demof2.obj $(SRCDIROBJS)\$(DLL32NAME).lib
        $(LD) $(LFLAGS) /out:$*.exe $(OBJDIR)\demof2.obj $(OBJDIR)\lm_newf2.obj $(LIBS) \
                $(CLIBS)

$(OBJDIR)\demof.exe : $(LIBS) $(OBJDIR)\demof.obj $(SRCDIROBJS)\$(DLL32NAME).lib
        $(LD) $(LFLAGS) /out:$*.exe $(OBJDIR)\demof.obj $(OBJDIR)\lm_newf.obj $(LIBS) \
                $(CLIBS)

$(OBJDIR)\pathd.exe : $(LIBS) $(OBJDIR)\pathd.obj $(SRCDIROBJS)\$(DLL32NAME).lib
        $(LD) $(LFLAGS) /out:$*.exe $(OBJDIR)\pathd.obj $(OBJDIR)\lm_npathd.obj $(LIBS) \
                $(CLIBS)

$(OBJDIR)\lock_tests.exe : $(OBJDIR)\lock_tests.obj $(LIBS) 
        $(LD) $(LFLAGS) /out:$*.exe $(OBJDIR)\lock_tests.obj \
                $(APPDIROBJS)\ls_lock.obj \
                $(APPDIROBJS)\ls_checkroot.obj $(LIBS_NOLMGRAS) \
                $(CLIBS)

$(OBJDIR)\no_s_tests.exe : testsuite.h $(OBJDIR)\no_s_tests.obj $(TESTLIBS)
        $(LD) $(LFLAGS) /out:$*.exe $(OBJDIR)\no_s_tests.obj \
                $(TESTLIBS) $(CLIBS)

$(OBJDIR)\bigfile.exe : $(OBJDIR)\bigfile.obj $(TESTLIBS)
        $(LD) $(LFLAGS) /out:$*.exe $(OBJDIR)\bigfile.obj $(TESTLIBS) \
                $(CLIBS)

$(OBJDIR)\reptest.exe : $(OBJDIR)\reptest.obj $(TESTLIBS)
        $(LD) $(LFLAGS) /out:$*.exe $(OBJDIR)\reptest.obj $(TESTLIBS) \
                $(CLIBS)

$(OBJDIR)\servtest.exe : ../utils/lmrand1.c testsuite.h $(OBJDIR)\servtest.obj $(OBJDIR)\lm_new.obj \
                        $(OBJDIR)\st_bugs.obj $(OBJDIR)\st_license.obj $(OBJDIR)\ts.lib $(TESTLIBS) $(SRCDIROBJS)\$(DLL32NAME).lib
        -del  lmappfil.c 
        -del lmkeyfil.c 
        -del $(OBJDIR)\tsappfil.c 
        -del $(OBJDIR)\tskeyfil.c
        ..\utils\$(OBJDIR)\lmrand1 -filter_gen  0xb2257d3d 0xaf4d347 0x0f8859ac9 -q
        copy lmappfil.c $(OBJDIR)\tsappfil.c
        copy lmkeyfil.c $(OBJDIR)\tskeyfil.c
        $(CC) $(CFLAGS) $(INCS) /c $(OBJDIR)\tsappfil.c
        $(CC) $(CFLAGS) $(INCS) /c $(OBJDIR)\tskeyfil.c
        $(LD) $(LFLAGS) /out:$*.exe $(OBJDIR)\servtest.obj \
                $(OBJDIR)\st_bugs.obj $(OBJDIR)\ts.lib  $(OBJDIR)\lm_new.obj \
                $(OBJDIR)\tsappfil.obj $(OBJDIR)\tskeyfil.obj   $(OBJDIR)\st_license.obj $(LIBS) $(CLIBS)

$(OBJDIR)\cryptest.exe:	$(OBJDIR)\cryptest.obj $(TESTLIBS) \
                                $(SRCDIROBJS)\$(DLL32NAME).lib
	del lmappfil.c 
        del lmkeyfil.c 
        del cryplicf.c
	..\utils\lmrand1 -filter_gen 0x7135715a 0x9b26c91b 0xb0097e5f -q
	del lmappfil.c
	copy lmkeyfil.c $(OBJDIR)\cryplicf.c
	$(CC) $(CFLAGS) $(INCS) /c $(OBJDIR)\cryplicf.c
	$(LD) $(LFLAGS) /out:$(OBJDIR)\cryptest.exe $(OBJDIR)\cryptest.obj $(OBJDIR)\cryplicf.obj $(OBJDIR)\ts.lib $(LIBS) $(CLIBS)

$(OBJDIR)\borrtest.exe : $(OBJDIR)\borrtest.obj $(TESTLIBS) $(SRCDIROBJS)\$(DLL32NAME).lib
        $(LD) $(LFLAGS) /out:$*.exe $(OBJDIR)\borrtest.obj $(TESTLIBS) \
                $(OBJDIR)\lm_new.obj $(CLIBS)





$(OBJDIR)\utiltest.exe : $(OBJDIR)\utiltest.obj \
                        $(OBJDIR)\ut_flex.obj $(CLIENTLIBS)
        $(LD) $(LFLAGS) /out:$*.exe $(OBJDIR)\utiltest.obj \
                $(OBJDIR)\ut_flex.obj $(CLIENTLIBS) $(CLIBS)

           

$(OBJDIR)\fake_appinit.obj:     $(APPDIR)\ls_app_init.c
        $(CC) $(CFLAGS) /U"RELEASE_VERSION" -c $(APPDIR)\ls_app_init.c /Fo"$(OBJDIR)\fake_appinit.obj"





$(OBJDIR)\lmgrd_r.exe:  $(OBJDIR)\fakehostname.obj $(OBJDIR)\lmgrd.exe
                $(LD) $(LFLAGS)  /out:$*.exe $(OBJDIR)\fakehostname.obj \
                         $(MASTERDIROBJS)\lmgrd.lib \
                          $(LIBS) $(CLIBS) /MAP:lmgrdr.map

$(OBJDIR)\demo_r.exe:   $(LIBS) $(OBJDIR)\demo.obj $(OBJDIR)\fakehostname.obj \
                        $(OBJDIR)\fake_appinit.obj $(OBJDIR)\ts.lib $(OBJDIR)\lm_new.obj
        $(LD)  $(LFLAGS)  /out:$*.exe  $(OBJDIR)\demo.obj $(OBJDIR)\lm_new.obj \
                        $(OBJDIR)\fakehostname.obj $(OBJDIR)\fake_appinit.obj \
                        $(LIBS) $(OBJDIR)\ts.lib  $(CLIBS)










#********************************
# Targets that are just copied
#********************************

$(OBJDIR)\lmgrd.exe : $(MASTERDIROBJS)\lmgrd.exe
        -del $(OBJDIR)\lmgrd.exe
        copy $(MASTERDIROBJS)\lmgrd.exe $(OBJDIR)

$(OBJDIR)\$(DLL32NAME).dll : $(SRCDIROBJS)\$(DLL32NAME).dll
        -del $(OBJDIR)\$(DLL32NAME).dll
        copy $(SRCDIROBJS)\$(DLL32NAME).dll $(OBJDIR)

$(OBJDIR)\$(DLL32NAME).pdb : $(SRCDIROBJS)\$(DLL32NAME).pdb
        -del $(OBJDIR)\$(DLL32NAME).pdb
        copy $(SRCDIROBJS)\$(DLL32NAME).pdb $(OBJDIR)

$(OBJDIR)\lmutil.exe : $(UTILSDIROBJS)\lmutil.exe
        -del $(OBJDIR)\lmutil.exe
        copy $(UTILSDIROBJS)\lmutil.exe $(OBJDIR)


#********************************
# Object targets
#********************************

$(OBJDIR)\demof.obj : demof.c 
        $(UTILSDIROBJS)\lmrand1.exe -i demof.c -o $(OBJDIR)\lmcodef.c $(OBJDIR)\lsrvendf.c
        $(CC) $(CFLAGS) $(INCS) /c $(OBJDIR)\lmcodef.c
        $(CC) $(CFLAGS) $(INCS) /c $(OBJDIR)\lsrvendf.c /Fo$(OBJDIR)\demof.obj
        $(LD) $(LFLAGS) /out:$(OBJDIR)\vendcodef.exe $(UTILSDIROBJS)\lmrand2.obj $(OBJDIR)\lmcodef.obj \
                $(SRCDIROBJS)\$(DLL32NAME).lib $(CLIBS)
        -del $(OBJDIR)\lm_newf.c
        $(OBJDIR)\vendcodef.exe demof -o $(OBJDIR)\lm_newf.c
        $(CC) $(CFLAGS) $(INCS) /c $(OBJDIR)\lm_newf.c                         

$(OBJDIR)\demof2.obj : demof2.c 
       $(UTILSDIROBJS)\lmrand1.exe -i demof2.c -o $(OBJDIR)\lmcodef2.c $(OBJDIR)\lsrvendf2.c
        $(CC) $(CFLAGS) $(INCS) /c $(OBJDIR)\lmcodef2.c
        $(CC) $(CFLAGS) $(INCS) /c $(OBJDIR)\lsrvendf2.c /Fo$(OBJDIR)\demof2.obj
        $(LD) $(LFLAGS) /out:$(OBJDIR)\vendcodef2.exe  $(UTILSDIROBJS)\lmrand2.obj $(OBJDIR)\lmcodef2.obj \
                $(SRCDIROBJS)\$(DLL32NAME).lib $(CLIBS)
        -del $(OBJDIR)\lm_newf2.c
        $(OBJDIR)\vendcodef2.exe demof2 -o $(OBJDIR)\lm_newf2.c
        $(CC) $(CFLAGS) $(INCS) /c $(OBJDIR)\lm_newf2.c                         


$(OBJDIR)\demo.obj : $(OBJDIR)\lm_new.obj
        $(CC) $(CFLAGS) $(INCS) /c $(OBJDIR)\lsrvend.c /Fo$(OBJDIR)\demo.obj


$(OBJDIR)\pathd.obj : pathd.c
      $(UTILSDIROBJS)\lmrand1.exe -i pathd.c -o $(OBJDIR)\lmcodepathd.c $(OBJDIR)\lsrvendpathd.c
        $(CC) $(CFLAGS) $(INCS) /c $(OBJDIR)\lmcodepathd.c
        $(CC) $(CFLAGS) $(INCS) /c $(OBJDIR)\lsrvendpathd.c /Fo$(OBJDIR)\pathd.obj
        $(LD) $(LFLAGS) /out:$(OBJDIR)\vendcodepathd.exe  $(UTILSDIROBJS)\lmrand2.obj $(OBJDIR)\lmcodepathd.obj \
                $(SRCDIROBJS)\$(DLL32NAME).lib $(CLIBS)
        -del $(OBJDIR)\lm_npathd.c
        $(OBJDIR)\vendcodepathd.exe demo -o $(OBJDIR)\lm_npathd.c
        $(CC) $(CFLAGS) $(INCS) /c $(OBJDIR)\lm_npathd.c  
        
$(OBJDIR)\lm_new.obj : demo.c code.h $(UTILSDIROBJS)\lmrand1.exe $(UTILSDIROBJS)\lmrand2.obj
        $(UTILSDIROBJS)\lmrand1.exe -i demo.c -o $(OBJDIR)\lmcode.c   $(OBJDIR)\lsrvend.c
        $(CC) $(CFLAGS) $(INCS) /c $(OBJDIR)\lmcode.c
        $(LD) $(LFLAGS) /out:$(OBJDIR)\lmrand2.exe  $(UTILSDIROBJS)\lmrand2.obj $(OBJDIR)\lmcode.obj \
                $(SRCDIROBJS)\$(DLL32NAME).lib $(CLIBS)
        -del $(OBJDIR)\lm_new.c                
        $(OBJDIR)\lmrand2.exe demo -o $(OBJDIR)\lm_new.c
        $(CC) $(CFLAGS) $(INCS) /c $(OBJDIR)\lm_new.c  
     

                
#*******************************
# 16 bit targets
#*******************************

#
# The environment must be set up for 16 bit compilation to build these
# targets.  Note that the 16 bit tools make extensive use of the
# in-line file creation method of passing parameters (from the "echo >"
# to the "<<" is an in-line file creation macro).
#

$(OBJDIR)\ts16.lib:: $(LIBOBJS)
        echo >NUL @<<$*.rsp
$@ /PAGESIZE:64
y
+ $(OBJDIR)\ts_config.obj &
+ $(OBJDIR)\ts_dump.obj &
+ $(OBJDIR)\ts_environ.obj &
+ $(OBJDIR)\ts_hostid.obj &
+ $(OBJDIR)\ts_lic_file.obj &
+ $(OBJDIR)\ts_list.obj &
+ $(OBJDIR)\ts_reread.obj &
+ $(OBJDIR)\ts_serv_supp.obj &
+ $(OBJDIR)\ts_utils.obj &
+ $(OBJDIR)\ts_vmsg.obj &
+ $(OBJDIR)\st_vers.obj &
;
<<
        if exist $@ del $@
        lib @$*.rsp

$(OBJDIR)\srvtst16.exe : testsuite.h $(OBJDIR)\servtest.obj \
                        $(OBJDIR)\st_bugs.obj \
                        $(OBJDIR)\ts16.lib $(LIBS16)
        echo >NUL @<<$*.rsp
$(OBJDIR)\servtest.obj +
$(OBJDIR)\st_bugs.obj
$*.exe
$*.map
$(BASE16)\lib\+
$(OBJDIR)\ts16.lib $(LIBS16) $(CLIBS16)
nul;
<<
        $(LD) $(LFLAGS16) @$*.rsp

$(OBJDIR)\$(DLL16NAME).dll : $(SRCDIROBJS)\$(DLL16NAME).dll
        -del $(OBJDIR)\$(DLL16NAME).dll
        copy $(SRCDIROBJS)\$(DLL16NAME).dll $(OBJDIR)

$(OBJDIR)\$(DLL16NAME).pdb : $(SRCDIROBJS)\$(DLL16NAME).pdb
        -del $(OBJDIR)\$(DLL16NAME).pdb
        copy $(SRCDIROBJS)\$(DLL16NAME).pdb $(OBJDIR)

#*******************************
# Dependencies
#*******************************

$(LIBOBJS) : testsuite.h ..\machind\lmclient.h

$(OBJS) : ..\machind\lmclient.h

$(OBJDIR)\basic_tests.obj:        testsuite.h code.h basic_tests.c
$(OBJDIR)\cryptest.obj:           testsuite.h cryptest.c
$(OBJDIR)\demof.obj:              code.h
$(OBJDIR)\demo.obj:           code.h
$(OBJDIR)\inout.obj:              code.h inout.c
$(OBJDIR)\$(OBJDIR)\lock_tests.obj:         testsuite.h lock_tests.c
$(OBJDIR)\no_s_tests.obj:         testsuite.h code.h no_s_tests.c
$(OBJDIR)\one_s_tests.obj:        testsuite.h code.h one_s_tests.c
$(OBJDIR)\servtest.obj:           testsuite.h code.h servtest.c
$(OBJDIR)\st_bugs.obj:            testsuite.h code.h st_bugs.c
$(OBJDIR)\path_tests.obj:         testsuite.h code.h path_tests.c
$(OBJDIR)\printenv.obj:           printenv.c
$(OBJDIR)\ts_config.obj:          code.h ts_config.c
$(OBJDIR)\ts_serv_supp.obj:       code.h ..\machind\lm_code.h ts_serv_supp.c
$(OBJDIR)\ts_dump.obj:            ts_dump.c
$(OBJDIR)\ts_environ.obj:         ts_environ.c
$(OBJDIR)\ts_lic_file.obj:        ts_lic_file.c
$(OBJDIR)\ts_hostid.obj:          ts_hostid.c
$(OBJDIR)\ts_list.obj:            ts_list.c
$(OBJDIR)\ts_utils.obj:           ts_utils.c
$(OBJDIR)\reptest.obj:            ..\machind\lm_code.h
$(DAEMONS):                       $(LIBS)
$(CLIENTS):                        $(SRCDIROBJS)\$(DLL32NAME).lib 
