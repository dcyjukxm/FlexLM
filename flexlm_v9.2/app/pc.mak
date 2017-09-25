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
#       Module: app.mak v1.2.0.0
#       
#       Description:  Builds the LMGR server libraries.
#
#       To build:
#
#               nmake /f server.mak [DEBUG=] [PROFILE=]
#
#       The "CPU" environment variable is used to detect the platform
#       type.  Override it by setting "PLAT=xxx" on the invokation.
#
#       C. Mayman
#       04-May-97
#
#       Last changed:  11/10/98
#
#*****************************************************************************
#****************************
# Platform setup
#****************************
!include "..\h\pc.mak"

!if "$(DEBUG)" == "1"
FDFLAG=/Fdlmgras.pdb
TARGETLIB=Debug
OUTDIR=Debug
!else
FDFLAG=
TARGETLIB=Release
OUTDIR=Release
!endif



INCS =   /I ..\server  /I ..\src

CFLAGS = $(CLFLAGS) /Fplmgras.pch $(FDFLAG) /D"FD_SETSIZE=4096"

.c.obj:
        $(CC) $(CFLAGS) $(INCS) /I /c $<

#
# This will make a version of the file that
# has the macros expanded.
#

.c.expand:
        $(CC) $(CFLAGS) /E $(INCS) /c $< > $*.expand

#****************************
# Library creation
#****************************

LB = lib

LIBFLAGS = /nologo

#****************************
# File lists
#****************************

OBJS =          lg_stubs.obj \
                lg_vkeys.obj \
                ls_app_init.obj \
                ls_app_main.obj \
                ls_args.obj \
                ls_attr.obj \
		    ls_borrow.obj \
                ls_checkout.obj \
                ls_checkroot.obj \
                ls_ckin.obj \
                ls_ck_client.obj \
                ls_ck_feats.obj \
                ls_daemon.obj \
                ls_docmd.obj \
                ls_feature.obj \
                ls_get_info.obj \
                ls_get_opts.obj \
                ls_host.obj \
                ls_i_fhostid.obj \
                ls_init_feat.obj \
                ls_list.obj \
                ls_lock.obj \
                ls_main.obj \
                ls_max.obj \
                ls_process.obj \
                ls_reread.obj \
                ls_shakehand.obj \
                ls_since.obj \
                ls_ubase.obj \
                ls_user_opts.obj \
                ls_vd_info.obj \
                lslookup.obj \
                lsprfeat.obj 



#****************************
# General Targets
#****************************
lmgras.lib : $(OBJS)
        $(LB) $(LIBFLAGS) $(MVSN_CSL_EVENT_LOG_LIB) /out:lmgras.lib $(OBJS)

clean :
        @echo Cleaning up app
        @if exist *.obj del *.obj
        @if exist *.sbr del *.sbr
        @if exist *.pdb del *.pdb
        @if exist *.pch del *.pch
        @if exist *.lib del *.lib

BUILDMD : $(OBJS)
        $(LB) $(LIBFLAGS) $(MVSNEVENTLOG) /out:lmgras_md.lib $(OBJS)

$(OBJS):        ..\machind\lsfeatur.h ..\h\lmachdep.h ..\machind\lmclient.h \
                ..\machind\lsserver.h ls_aprot.h  ..\server\ls_sprot.h


ls_init_feat.obj:   ..\src\l_crypt.c ..\src\l_strkey.c ..\h\l_strkey.h
ls_lock.obj:	ls_lock.h
ls_borrow.obj:	..\h\l_rand.h ..\h\l_borrow.h
