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
#       
#       $Id: server.mak,v 1.5 1999/02/05 03:03:43 daniel Exp $
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

LIBNAMEBASE = lmgrs
LIBNAME = $(LIBNAMEBASE).lib

!ifdef DEBUG
OBJDIR1 = $(PLAT)_DEBUG
!else
OBJDIR1 = $(PLAT)_REL
!endif

!ifdef SD
OBJDIR = $(SD)\$(OBJDIR1)
!else
OBJDIR = $(OBJDIR1)
!endif

#****************************
# Directories
#****************************

#****************************
# Compilation information
#****************************

#
# Include files
#

INCS =  /I . \
        /I ..\h \
         /I ..\machind  \
        /I $(MSDEVDIR)\include \
        /I ..\patches \
        /I ..\src
#
# Compiler flags
#

!ifdef DEBUG
DBG_CFLAGS = /MTd /Zi /FR$*.sbr \
                /Fd$(OBJDIR)\$(LIBNAMEBASE).pdb
!else
DBG_CFLAGS = /MT /O1 /D "RELEASE_VERSION" 
!endif

CFLAGS = /nologo /W2 /D"_CONSOLE" /D"PC" /D"WINNT" \
                /D "FD_SETSIZE=4096" \
                /YX /Fp$(OBJDIR)\$(LIBNAMEBASE).pch \
                /D"LM_INTERNAL" /Fo$(OBJDIR)\ $(DBG_CFLAGS)


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
# Library creation
#****************************

LB = lib

LIBFLAGS = /nologo

#****************************
# File lists
#****************************

OBJS =          $(OBJDIR)\l_crypt.obj \
                $(OBJDIR)\l_key.obj \
                $(OBJDIR)\ls_badserv.obj \
                $(OBJDIR)\ls_c_comm.obj \
                $(OBJDIR)\ls_c_init.obj \
                $(OBJDIR)\ls_client_db.obj \
                $(OBJDIR)\ls_comm.obj \
                $(OBJDIR)\ls_data.obj \
                $(OBJDIR)\ls_dict.obj \
                $(OBJDIR)\ls_down.obj \
                $(OBJDIR)\ls_findbad.obj \
                $(OBJDIR)\ls_flexdir.obj \
                $(OBJDIR)\ls_gettime.obj \
                $(OBJDIR)\ls_handle.obj \
                $(OBJDIR)\ls_hookup.obj \
                $(OBJDIR)\ls_i_master.obj \
                $(OBJDIR)\ls_log.obj \
                $(OBJDIR)\ls_log_open.obj \
                $(OBJDIR)\ls_logtime.obj \
                $(OBJDIR)\ls_lost.obj \
                $(OBJDIR)\ls_mail.obj \
                $(OBJDIR)\ls_malloc.obj \
                $(OBJDIR)\ls_mast_rdy.obj \
                $(OBJDIR)\ls_new_log.obj \
                $(OBJDIR)\ls_on_host.obj \
                $(OBJDIR)\ls_pause.obj \
                $(OBJDIR)\ls_pick_mast.obj \
                $(OBJDIR)\ls_quorum.obj \
                $(OBJDIR)\ls_readready.obj \
                $(OBJDIR)\ls_s_funcs.obj \
                $(OBJDIR)\ls_sconnect.obj \
                $(OBJDIR)\ls_serv_conn.obj \
                $(OBJDIR)\ls_serv_time.obj \
                $(OBJDIR)\ls_signals.obj \
                $(OBJDIR)\ls_socket.obj \
                $(OBJDIR)\ls_symtab.obj \
                $(OBJDIR)\ls_udp_read.obj \
                $(OBJDIR)\ls_wakeup.obj \
                $(OBJDIR)\lsfilter.obj \
                $(OBJDIR)\lsreplog.obj

#****************************
# General Targets
#****************************

all : timestamp $(OBJDIR) $(OBJDIR)\$(LIBNAME)
        @echo !--------------------------------------
        @echo ! Done building $(LIBNAME)
        @echo !--------------------------------------

timestamp :
        @echo !--------------------------------------
        @echo ! Building $(LIBNAME)
        @echo !--------------------------------------

$(OBJDIR)\$(LIBNAME) : $(OBJS)
        $(LB) $(LIBFLAGS) /out:$(OBJDIR)\$(LIBNAME) $(OBJS)

clean :
        @echo Cleaning up $(PLAT)
        -@del sintel_debug\*.o*
        -@del sintel_debug\*.e*
        -@del sintel_debug\*.l*
        -@del sintel_debug\*.r*
        -@del sintel_debug\*.p*
        -@del sintel_debug\*.e*
        -@del intel_debug\*.o*
        -@del intel_debug\*.e*
        -@del intel_debug\*.l*
        -@del intel_debug\*.r*
        -@del intel_debug\*.p*
        -@del sintel_rel\*.e*
        -@del sintel_rel\*.o*
        -@del sintel_rel\*.e*
        -@del sintel_rel\*.l*
        -@del sintel_rel\*.r*
        -@del sintel_rel\*.p*
        -@del sintel_rel\*.e*

$(OBJDIR) :
        -mkdir $(OBJDIR)
        
#*******************************
#
#   Special Targets
#*******************************

l_crypt.c : ..\src\l_crypt.c
        -@del l_crypt.c
        -@del l_strkey.c
        copy ..\src\l_crypt.c
        copy ..\src\l_strkey.c

l_key.c : ..\src\l_key.c
        -@del l_key.c
        copy ..\src\l_key.c
#*******************************
# Dependencies
#*******************************

