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
#       Module: $Id: pc.mak,v 1.53 2003/03/08 07:19:47 jwong Exp $
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
#               reptest \       
#               tsclient \      #no
#               tscrypt \       #no
#               utiltest  
#
#********************************************************************************
#
# P.S. one_s_tests  is very UNIX like.  It uses lots of signals, so it is
#                   not included.
#
#
#********************************************************************************

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
#****************************
# Compilation information
#****************************

#
# Include files
#

INCS =   /I .\
         /I ..\h  \
         /I ..\machind  \
         /I ..\patches \
         /I ..\server


CFLAGS = $(CLFLAGS)

#
# 16 bit compiler flags
#


.c.obj:
        $(CC) $(CFLAGS) $(INCS) /c $<


.c.expand:
        $(CC) $(CFLAGS) /E $(INCS) /c $< > $*.expand


DLL32NAME=lmgr

!if "$(FLEXLM_MAKE_PLATFORM)" == "i64_n5"
FLEXLOCK_LIB = 
!else
FLEXLOCK_LIB = ..\$(PROCESSOR_DIRECTORY)\flock.lib
!endif


PUBKEY_LIBS =   ..\$(PROCESSOR_DIRECTORY)\libcrvs.lib \
                ..\$(PROCESSOR_DIRECTORY)\libsb.lib $(FLEXLOCK_LIB)
CLIENTLIBS = ..\src\$(DLL32NAME).lib  $(PUBKEY_LIBS)
#CLIENTLIBS = ..\src\$(DLL32NAME).lib $(FLEXMETERDEBUG)\fmeter.lib
TESTLIBS = ts.lib $(CLIENTLIBS) 
LIBS =  $(APPDIROBJS)\lmgras.lib \
                $(SERVERDIROBJS)\lmgrs.lib \
                $(CLIENTLIBS) 

LIBS_NOLMGRAS = $(SERVERDIROBJS)\lmgrs.lib \
                $(CLIENTLIBS)

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

XTRA_LFLAGS = /PROFILE

!ifdef DEBUG
DBG_LFLAGS = /DEBUG /pdb:$*.pdb
!else
DBG_LFLAGS =
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

#
# Link commands
#


OBJS =          basic_tests.obj \
                crotest.obj \
                cryptest.obj \
                demof.obj \
                demof2.obj \
                demo.obj \
                fake_appinit.obj \
                fakehostname.obj \
                jobtests.obj \
                lock_tests.obj \
                lsapi_tests.obj \
                pathd.obj \
                path_tests.obj \
                no_s_tests.obj \
                one_s_tests.obj \
                bigfile.obj \
                servtest.obj \
                st_bugs.obj \
                printenv.obj \
                red_s_tests.obj \
                reptest.obj \
                tsclient.obj \
                tscrypt.obj \
                ut_flex.obj \
                utiltest.obj \
                st_license.obj \
                fakehostname.obj 

LIBOBJS =       ts_config.obj \
		tsdsapp.obj\
		tsdsgen.obj\
                ts_dump.obj \
                ts_environ.obj \
                ts_hostid.obj \
                ts_lic_file.obj \
                ts_list.obj \
                ts_reread.obj \
                ts_serv_supp.obj \
                ts_utils.obj \
                ts_vmsg.obj 


DAEMONS =       demo.exe \
                demof.exe \
                demof2.exe \
                pathd.exe \
		demo_r.exe \
		lmgrd_r.exe

CLIENTS =        utiltest.exe  \
                servtest.exe \
                crotest.exe \
		cryptest.exe \
		basic_tests.exe \
		no_s_tests.exe  \
                tscrypt.exe \
                tsclient.exe \
		lock_tests.exe \
		red_s_tests.exe
        

#		borrtest.exe 

  

COPY_TARGETS =  lmgrd.exe 
#                lmgr326a.dll 
#               lmutil.exe

!ifdef DEBUG
!ifndef STATIC
COPY_TARGETS =  $(COPY_TARGETS) \
                lmgr.pdb 
 
!endif
!endif

#
# In debuggin mode, get the PDB file so we can
# debug the DLL.
#

!ifdef DEBUG
PDB_FILES = $(DLL32NAME).pdb
PDB_FILES16 = $(DLL16NAME).pdb
!else
PDB_FILES =
PDB_FILES16 =
!endif

LM_PARTS =       $(DLL32NAME).dll \
                $(PDB_FILES)

LM_PARTS16 =    $(DLL16NAME).dll \
                $(PDB_FILES16)

#****************************
# General Targets
#****************************

all : $(CLIENTS) $(DAEMONS)  $(COPY_TARGETS)

daemons: $(DAEMONS)

lmgrd.exe:	 ..\master\lmgrd.exe
	copy ..\master\lmgrd.exe

        
clean :
        @echo Cleaning up testsuite
        if exist *.out del *.out
        if exist *.lob del *.log
        if exist license.dat del license.dat
        if exist pathtest.dat del pathtest.dat
        if exist rlicense* del rlicense*
        if exist opts del opts
        if exist *.opt del *.opt
        if exist servtest.err del servtest.err
        if exist _* del _*
        if exist bigfile.dat del bigfile.dat
        if exist *.expand del *.expand
        if exist *.exe del *.exe
        if exist *.met del *.met
        if exist *.obj del *.obj
        if exist *.p* del *.p*
        if exist *.ilk del *.ilk
        if exist *.l* del *.l*
        if exist dant* del dant*
        if exist dant*.* del dant*.*
        if exist borrow.dat del borrow.dat
        if exist tmp.dat del tmp.dat
        if exist nosuch.dat del nosuch.dat
        if exist flextest.* del flextest.*
        if exist create*.dat del create*.dat

#****************************
# EXE targets
#****************************

ts.lib : $(LIBOBJS)
        $(LB) /out:ts.lib $(LIBOBJS)

basic_tests.exe : testsuite.h basic_tests.obj lm_nold.obj \
                        $(TESTLIBS)
        $(LD) $(LFLAGS) /out:$*.exe basic_tests.obj lm_nold.obj \
                $(TESTLIBS) $(CLIBS)


demo.exe : $(LIBS) demo.obj lm_n.obj ts_hostid.obj ts_vmsg.obj \
        $(SRCDIROBJS)\$(DLL32NAME).lib  
        $(LD) $(LFLAGS) /out:$*.exe demo.obj \
                lm_n.obj $(TESTLIBS) $(LIBS) \
                $(CLIBS) 

demof2.exe : $(LIBS) demof2.obj $(SRCDIROBJS)\$(DLL32NAME).lib lm_n_f2.obj
        $(LD) $(LFLAGS) /out:$*.exe demof2.obj lm_n_f2.obj $(LIBS) \
                $(CLIBS)

demof.exe : $(LIBS) demof.obj $(SRCDIROBJS)\$(DLL32NAME).lib lm_nf.obj
        $(LD) $(LFLAGS) /out:$*.exe demof.obj lm_nf.obj $(LIBS) \
                $(CLIBS)

pathd.exe : $(LIBS) pathd.obj lm_npathd.obj $(SRCDIROBJS)\$(DLL32NAME).lib
        $(LD) $(LFLAGS) /out:$*.exe pathd.obj lm_npathd.obj $(LIBS) \
                $(CLIBS)

lock_tests.exe : lock_tests.obj $(LIBS) 
        $(LD) $(LFLAGS) /out:$*.exe lock_tests.obj \
                $(APPDIROBJS)\ls_lock.obj \
                $(APPDIROBJS)\ls_checkroot.obj $(LIBS_NOLMGRAS) \
                $(CLIBS)

red_s_tests.exe:	testsuite.h red_s_tests.obj $(TESTLIBS)
	$(LD) $(LFLAGS) /out:red_s_tests.exe lm_n.obj red_s_tests.obj $(TESTLIBS) $(LIBS) $(CLIBS)

no_s_tests.exe : testsuite.h no_s_tests.obj lm_nold.obj $(TESTLIBS)
        $(LD) $(LFLAGS) /out:$*.exe no_s_tests.obj lm_nold.obj \
                $(TESTLIBS) $(CLIBS)

bigfile.exe : bigfile.obj $(TESTLIBS)
        $(LD) $(LFLAGS) /out:$*.exe bigfile.obj $(TESTLIBS) \
                $(CLIBS)

reptest.exe : reptest.obj $(TESTLIBS)
        $(LD) $(LFLAGS) /out:$*.exe reptest.obj $(TESTLIBS) \
                $(CLIBS)

tskeyfil.obj:	tsappfil.obj

tsappfil.obj:   ..\utils\lmrand1.exe
        -del  lmappfil.c 
        -del lmkeyfil.c 
        -del tsappfil.c 
        -del tskeyfil.c
	..\utils\lmrand1.exe -filter_gen  0xb2257d3d 0xaf4d347 0x0f8859ac9 -q
        copy lmappfil.c tsappfil.c
        copy lmkeyfil.c tskeyfil.c
        $(CC) $(CFLAGS) $(INCS) /c tsappfil.c
        $(CC) $(CFLAGS) $(INCS) /c tskeyfil.c

lmph2app.c:   ..\utils\lmrand1.exe
        -del  lmph2app.* 
        -del  lmph2gen.* 
	..\utils\lmrand1.exe -phase2  0x12345678 0x11122233 0x8765abcd -q
	
lmph2gen.obj: lmph2app.c
lmph2app.obj: lmph2app.c

servtest.exe : ..\utils\lmrand1.c testsuite.h servtest.obj lm_n.obj \
                st_bugs.obj st_license.obj ts.lib $(TESTLIBS) \
                $(SRCDIROBJS)\$(DLL32NAME).lib st_vers.obj st_8bit.obj \
                tsappfil.obj tskeyfil.obj lmph2app.obj lmph2gen.obj
        $(LD) $(LFLAGS) /out:$*.exe servtest.obj \
                st_bugs.obj st_8bit.obj st_vers.obj \
                st_license.obj  tsappfil.obj tskeyfil.obj   \
                        ts.lib lm_n.obj  lmph2gen.obj lmph2app.obj $(LIBS) $(CLIBS) 

cryptapp.obj:	$(LTESTLIBS)
cryptkey.obj:	$(LTESTLIBS)
cryptest.exe:	cryptest.obj $(TESTLIBS) ..\utils\lmrand1.exe \
                cryptapp.obj cryptkey.obj \
                                $(SRCDIROBJS)\$(DLL32NAME).lib
	-del lmappfil.c 
        -del lmkeyfil.c 
        -del cryplicf.c
	..\utils\lmrand1 -filter_gen 0x7135715a 0x9b26c91b 0xb0097e5f -q
	-del lmappfil.c
	copy lmkeyfil.c cryplicf.c
	$(CC) $(CFLAGS) $(INCS) /c cryplicf.c
	$(LD) $(LFLAGS) /out:cryptest.exe cryptest.obj cryplicf.obj ts.lib cryptapp.obj cryptkey.obj $(LIBS) $(CLIBS)

crotest.exe:	crotest.obj $(TESTLIBS) ..\utils\lmrand1.exe \
                                $(SRCDIROBJS)\$(DLL32NAME).lib lm_n.obj
	$(LD) $(LFLAGS) /out:crotest.exe crotest.obj lm_n.obj \
                                $(SRCDIROBJS)\$(DLL32NAME).lib $(CLIBS)
	        

                             
borrtest.exe : borrtest.obj $(TESTLIBS) $(SRCDIROBJS)\$(DLL32NAME).lib
        $(LD) $(LFLAGS) /out:$*.exe borrtest.obj $(TESTLIBS) \
                lm_n.obj $(CLIBS)





utiltest.exe : utiltest.obj ut_flex.obj $(CLIENTLIBS) lm_nold.obj
        $(LD) $(LFLAGS) /out:$*.exe utiltest.obj lm_nold.obj \
                ut_flex.obj $(CLIENTLIBS) $(CLIBS)

           

fake_appinit.obj:     $(APPDIR)\ls_app_init.c
        $(CC) $(CFLAGS) /U"RELEASE_VERSION" -I..\server -c $(APPDIR)\ls_app_init.c /Fo"fake_appinit.obj"





lmgrd_r.exe:  fakehostname.obj ..\master\lmgrd.exe \
                ../master/ls_chld_died.obj \
                ../master/ls_daemons.obj \
                ../master/ls_finder.obj \
                ../master/ls_kill_chld.obj \
                ../master/ls_lmgrd.obj \
                ../master/ls_m_init.obj \
                ../master/ls_m_main.obj \
                ../master/ls_m_process.obj \
                ../master/ls_startup.obj \
                ../master/ls_statfile.obj
                $(LD) $(LFLAGS)  /out:$*.exe \
                ../master/ls_chld_died.obj \
                ../master/ls_daemons.obj \
                ../master/ls_finder.obj \
                ../master/ls_kill_chld.obj \
                ../master/ls_lmgrd.obj \
                ../master/ls_m_init.obj \
                ../master/ls_m_main.obj \
                ../master/ls_m_process.obj \
                ../master/ls_startup.obj \
                ../master/ls_statfile.obj \
                ../master/ls_timestamp.obj \
                ../master/service.obj \
                fakehostname.obj \
                          $(LIBS) $(CLIBS)  

demo_r.exe:   $(LIBS) fakehostname.obj fake_appinit.obj ts.lib lm_n.obj
!if "$(FLEXLM_MAKE_PLATFORM)" == "i86_n3"
		if exist demo.obj del demo.obj
		$(CC) /c $(CFLAGS) /DRED_SERVER $(INCS) demo.c 
!endif
        $(LD)  $(LFLAGS)  /out:$*.exe  demo.obj lm_n.obj \
                        fakehostname.obj fake_appinit.obj \
                        $(LIBS) ts.lib $(CLIBS)
!if "$(FLEXLM_MAKE_PLATFORM)" == "i86_n3"
		if exist demo.obj del demo.obj
!endif

lm_nold.obj:	demo.c code.h  ..\utils\lmrand1.exe ..\utils\lmnewgen.obj \
	$(SRCDIROBJS)\$(DLL32NAME).lib
	..\utils\lmrand1 -i demo.c
	$(CC) /c $(CFLAGS) /DLM_OLD_KEY_TYPE $(INCS) lmcode.c
	$(LD) $(LFLAGS) ..\utils\lmnewgen.obj lmcode.obj $(SRCDIROBJS)\$(DLL32NAME).lib /out:lmnewgen.exe $(CLIBS) $(PUBKEY_LIBS)
	lmnewgen demo  -o lm_nold.c
	$(CC) /c $(CFLAGS) $(INCS) lm_nold.c




#********************************
# Targets that are just copied
#********************************


#********************************
# Object targets
#********************************


lm_nf.obj:	demof.c code.h  ..\utils\lmrand1.exe ..\utils\lmnewgen.obj \
	$(SRCDIROBJS)\$(DLL32NAME).lib 
	..\utils\lmrand1 -i demof.c -o lmcodef.c lsrvendf.c
	$(CC) /c $(CFLAGS) -DLM_NEW_KEY_TYPE /I..\h lmcodef.c
	$(LD) $(LFLAGS) ..\utils\lmnewgen.obj lmcodef.obj \
		$(SRCDIROBJS)\$(DLL32NAME).lib /out:lmnewgenf.exe  $(CLIBS) 
	lmnewgenf demof -o lm_nf.c
	$(CC) /c $(CFLAGS) /I..\h lm_nf.c


lm_n_f2.obj:	demof2.c code.h  ..\utils\lmrand1.exe ..\utils\lmnewgen.obj \
	$(SRCDIROBJS)\$(DLL32NAME).lib 
	..\utils\lmrand1 -i demof2.c -o lmcodef2.c lsrvendf2.c
	$(CC) /c $(CFLAGS) -DLM_NEW_KEY_TYPE /I..\h lmcodef2.c
	$(LD) $(LFLAGS) ..\utils\lmnewgen.obj lmcodef2.obj \
		$(SRCDIROBJS)\$(DLL32NAME).lib /out:lmnewgenf2.exe  $(CLIBS)
	lmnewgenf2 demof2 -o lm_n_f2.c
	$(CC) /c $(CFLAGS) /I..\h lm_n_f2.c

demof.obj : demof.c 
        $(UTILSDIROBJS)\lmrand1.exe -i demof.c -o lmcodef.c lsrvendf.c
        $(CC) $(CFLAGS) $(INCS) /c lmcodef.c
        $(CC) $(CFLAGS) $(INCS) /c lsrvendf.c /Fodemof.obj
        $(LD) $(LFLAGS) /out:vendcodef.exe $(UTILSDIROBJS)\lmnewgen.obj lmcodef.obj \
                $(SRCDIROBJS)\$(DLL32NAME).lib $(CLIBS) $(PUBKEY_LIBS)
        -del lm_nf.c
        vendcodef.exe demof -o lm_nf.c
        $(CC) $(CFLAGS) $(INCS) /c lm_nf.c                         

demof2.obj : demof2.c 
       $(UTILSDIROBJS)\lmrand1.exe -i demof2.c -o lmcodef2.c lsrvendf2.c
        $(CC) $(CFLAGS) $(INCS) /c lmcodef2.c
        $(CC) $(CFLAGS) $(INCS) /c lsrvendf2.c /Fodemof2.obj
        $(LD) $(LFLAGS) /out:vendcodef2.exe  $(UTILSDIROBJS)\lmnewgen.obj lmcodef2.obj \
                $(SRCDIROBJS)\$(DLL32NAME).lib $(CLIBS)
        -del lm_n_f2.c
        vendcodef2.exe demof2 -o lm_nf2.c


demo.obj : lm_n.obj
        $(CC) $(CFLAGS) $(INCS) /c lsrvend.c /Fodemo.obj


pathd.obj : pathd.c
      $(UTILSDIROBJS)\lmrand1.exe -i pathd.c -o lmcodepathd.c lsrvendpathd.c
        $(CC) $(CFLAGS) $(INCS) /c lmcodepathd.c
        $(CC) $(CFLAGS) $(INCS) /c lsrvendpathd.c /Fopathd.obj
        $(LD) $(LFLAGS) /out:vendcodepathd.exe  $(UTILSDIROBJS)\lmnewgen.obj lmcodepathd.obj \
                $(SRCDIROBJS)\$(DLL32NAME).lib $(CLIBS)
        -del lm_npathd.c
        vendcodepathd.exe demo -o lm_npathd.c
        $(CC) $(CFLAGS) $(INCS) /c lm_npathd.c  
        
lm_n.obj : demo.c code.h $(UTILSDIROBJS)\lmrand1.exe \
		$(UTILSDIROBJS)\lmnewgen.obj $(SRCDIROBJS)\$(DLL32NAME).lib
        $(UTILSDIROBJS)\lmrand1.exe -i demo.c -o lmcode.c   lsrvend.c
        $(CC) $(CFLAGS) $(INCS) /c lmcode.c
        $(LD) $(LFLAGS) /out:lmnewgen.exe  $(UTILSDIROBJS)\lmnewgen.obj lmcode.obj \
                $(SRCDIROBJS)\$(DLL32NAME).lib $(CLIBS) $(PUBKEY_LIBS)
        -del lm_n.c                
        lmnewgen.exe demo -o lm_n.c
        $(CC) /W3 /c lm_n.c  

flextest.exe: flextest.obj
        $(LD) $(LFLAGS) /out:$*.exe flextest.obj \
                $(TESTLIBS) $(CLIBS)
        
dant.exe:	dant.obj $(LTESTLIBS) lm_n.obj
	$(LD) $(LFLAGS) /out:dant.exe dant.obj lm_n.obj $(TESTLIBS) \
    $(CLIBS) 

tsclient.exe:	tsclient.obj $(LTESTLIBS) lm_n.obj
	$(LD) $(LFLAGS) /out:tsclient.exe tsclient.obj lm_n.obj $(TESTLIBS) \
    $(CLIBS) 

lmflex.exe:	$(LTESTLIBS) ..\vendor\lm_new.obj
        $(CC) $(CFLAGS) $(INCS) /c ..\machind\lmflex.c
	$(LD) $(LFLAGS) /out:lmflex.exe lmflex.obj ..\vendor\lm_new.obj $(TESTLIBS) \
    $(CLIBS) 


tscrypt.exe:	tscrypt.obj $(LTESTLIBS)  
	$(LD) $(LFLAGS) /out:tscrypt.exe tscrypt.obj $(TESTLIBS) \
    $(CLIBS)
    


                
#*******************************
# 16 bit targets
#*******************************

#
# The environment must be set up for 16 bit compilation to build these
# targets.  Note that the 16 bit tools make extensive use of the
# in-line file creation method of passing parameters (from the "echo >"
# to the "<<" is an in-line file creation macro).
#

ts16.lib:: $(LIBOBJS)
        echo >NUL @<<$*.rsp
$@ /PAGESIZE:64
y
+ ts_config.obj &
+ ts_dump.obj &
+ ts_environ.obj &
+ ts_hostid.obj &
+ ts_lic_file.obj &
+ ts_list.obj &
+ ts_reread.obj &
+ ts_serv_supp.obj &
+ ts_utils.obj &
+ ts_vmsg.obj &
+ st_vers.obj &
;
<<
        if exist $@ del $@
        lib @$*.rsp

srvtst16.exe : testsuite.h servtest.obj \
                        st_bugs.obj \
                        ts16.lib $(LIBS16)
        echo >NUL @<<$*.rsp
servtest.obj +
st_bugs.obj
$*.exe
$*.map
$(BASE16)\lib\+
ts16.lib $(LIBS16) $(CLIBS16)
nul;
<<
        $(LD) $(LFLAGS16) @$*.rsp

$(DLL16NAME).dll : $(SRCDIROBJS)\$(DLL16NAME).dll
        -del $(DLL16NAME).dll
        copy $(SRCDIROBJS)\$(DLL16NAME).dll $(OBJDIR)

$(DLL16NAME).pdb : $(SRCDIROBJS)\$(DLL16NAME).pdb
        -del $(DLL16NAME).pdb
        copy $(SRCDIROBJS)\$(DLL16NAME).pdb $(OBJDIR)

#*******************************
# Dependencies
#*******************************

$(LIBOBJS) : testsuite.h ..\machind\lmclient.h

$(OBJS) : ..\machind\lmclient.h

$(DAEMONS):                       $(LIBS)
$(CLIENTS):                        $(TESTLIBS)
$(OBJS):        ..\machind\lmclient.h testsuite.h code.h
$(LIBOBJS):        ..\machind\lmclient.h testsuite.h code.h
