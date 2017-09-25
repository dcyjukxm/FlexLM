#******************************************************************************
#
#           COPYRIGHT (c) 1996, 2000 by Globetrotter Software Inc.
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
#       
#       $Id: pc.mak,v 1.14 2002/10/18 23:13:11 jwong Exp $
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
#       Last changed:  9/28/98
#
#*****************************************************************************

#****************************
# Platform setup
#****************************

!include "..\h\pc.mak"

!ifdef MD
MDFLAG = _md
!else
MDFLAG = 
!endif

#****************************
# Control values
#****************************

LIBNAMEBASE = lmgrs$(MDFLAG)
LIBNAME = $(LIBNAMEBASE).lib

#
# Include files
#

INCS =   

#
# Compiler flags
#




#
# Compilation rules
#

!ifdef DEBUG
FDFLAG=/Fdlmgrs.pdb
!else
FDFLAG=
!endif

CFLAGS = $(CLFLAGS) /Fplmgrs.pch $(FDFLAG) /D"FD_SETSIZE=4096"

.c.obj:
        $(CC) $(CFLAGS) $(INCS) /c $<

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

OBJS  =         l_crypt.obj \
                l_key.obj \
                ls_badserv.obj \
                ls_c_comm.obj \
                ls_c_init.obj \
                ls_client_db.obj \
                ls_data.obj \
                ls_dict.obj \
                ls_down.obj \
                ls_findbad.obj \
                ls_flexdir.obj \
                ls_gettime.obj \
                ls_handle.obj \
                ls_hookup.obj \
                ls_i_master.obj \
                ls_log.obj \
                ls_log_open.obj \
                ls_logtime.obj \
                ls_lost.obj \
                ls_mail.obj \
                ls_malloc.obj \
                ls_mast_rdy.obj \
                ls_new_log.obj \
                ls_on_host.obj \
                ls_pause.obj \
                ls_pick_mast.obj \
                ls_quorum.obj \
                ls_readready.obj \
                ls_s_funcs.obj \
                ls_sconnect.obj \
                ls_serv_conn.obj \
                ls_serv_time.obj \
                ls_signals.obj \
                ls_socket.obj \
                ls_symtab.obj \
                ls_udp_read.obj \
                ls_wakeup.obj \
                lsfilter.obj \
                lsreplog.obj

#****************************
# General Targets
#****************************

all : $(LIBNAME)
        @echo !--------------------------------------
        @echo ! Done building $(LIBNAME)
        @echo !--------------------------------------


$(LIBNAME) : $(OBJS)
        $(LB) $(LIBFLAGS) /out:$(LIBNAME) $(OBJS)


clean :
        @echo Cleaning up server
        @if exist *.obj* del *.obj*
        @if exist *.exp* del *.exp*
        @if exist *.lib del *.lib
        @if exist *.res del *.res
        @if exist *.p* del *.p*

l_crypt.obj:    ..\src\l_crypt.c
        $(CC) $(CFLAGS) $(INCS) /c ..\src\l_crypt.c

l_key.obj:    ..\src\l_key.c
        $(CC) $(CFLAGS) $(INCS) /c ..\src\l_key.c
        
#*******************************
#
#   Special Targets
#*******************************

#*******************************
# Dependencies
#*******************************

$(OBJS):    ..\machind\lsserver.h ..\h\l_prot.h ..\machind\lmclient.h \
            ..\h\l_privat.h ls_sprot.h
