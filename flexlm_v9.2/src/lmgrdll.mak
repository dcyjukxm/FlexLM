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
#      Module: lmgrdll.mak v1.7  
#
#       Description:  Builds the LMGR DLL for 16 and 32 bits,
#                       on all platforms.
#
#       To build:
#
#               nmake /f lmgrdll.mak [DEBUG=] [PROFILE=] [STATIC] [MTD=1] [WINNT_SMALL=1]
#
#       The "CPU" environment variable is used to detect the platform
#       type.  Override it by setting "PLAT=xxx" on the invokation.
#
#       C. Mayman
#        4-May-97
#
#       Last changed:  12/23/98
#
#*****************************************************************************

#****************************
# Platform setup
#****************************
#
#
#
!ifdef NOTIMERS
TIMER_INCLUDE=/D "NO_TIMER_THREADS"
!else
TIMER_INCLUDE=
!endif

!ifdef 16BIT
MSDEVDIR=d:\msvc
!else
MSDEVDIR=f:\msdev
!endif
#
#
#
!ifndef PROCESSOR_ARCHITECTURE
PROCESSOR_ARCHITECTURE=INTEL
PLAT=INTEL
!endif

PLAT = $(PROCESSOR_ARCHITECTURE)
RESPLAT = $(PROCESSOR_ARCHITECTURE)
!if "$(PLAT)"=="x86"
PLAT = INTEL
RESPLAT=IX86
!endif

!ifdef 16BIT
!MESSAGE [Windows 3.1]
L_TARGET = unused
!else if "$(PLAT)" == "INTEL"
!MESSAGE [Intel NT]
L_TARGET = i386
PLAT_INCLUDE="_x86_"
!else if "$(PLAT)" == "ALPHA"
!MESSAGE [Alpha NT]
L_TARGET = alpha
PLAT_INCLUDE="_ALPHA_"
!else if "$(PLAT)" == "MIPS"
!MESSAGE [Mips NT]
L_TARGET = mips
PLAT_INCLUDE="_MIPS_"
!else
!MESSAGE WARNING:  Unknown platform specified: $(PLAT)
!endif

#****************************
# Control values
#****************************

DLL16NAMEBASE = lmgr167a
DLL16NAME = $(DLL16NAMEBASE).dll
DLL32NAMEBASE = lmgr
DLL32NAME = $(DLL32NAMEBASE).dll
DLL32DEF = $(DLL32NAMEBASE).def
LIBNAMEBASE = lmgr
LIBNAME = $(LIBNAMEBASE).lib

!ifdef 16BIT
PLAT = win16
!endif

RCFLAGS32 = /l 0x409 
D_RCDEFINES32 = /d_DEBUG 
R_RCDEFINES32 = /dNDEBUG
D_RCDEFINES = -d_DEBUG
R_RCDEFINES = -dNDEBUG
RCFLAGS = 
RESFLAGS = 


!ifdef DEBUG
OBJDIR3 = $(PLAT)_DEBUG
RCDEFINES32 = $(D_RCDEFINES32)
RCDEFINES = $(D_RCDEFINES)
!else
OBJDIR3 = $(PLAT)_REL
RCDEFINES32 = $(R_RCDEFINES32)
RCDEFINES = $(R_RCDEFINES)

!endif

!ifdef STATIC
OBJDIR0=S$(OBJDIR3)
STATIC_FLAGS= /D "FLEX_STATIC"
!else
STATIC_FLAGS=
OBJDIR0=$(OBJDIR3)
!endif

!ifdef MTD
CRT=/MD
OBJDIR1=D$(OBJDIR0)
FLEX_STATIC_MT=
!else
CRT=/MT
OBJDIR1=$(OBJDIR0)
FLEX_STATIC_MT=/DFLEX_STATIC_MT
!endif

!ifdef WINNT_SMALL
OBJDIR2=T$(OBJDIR1)
SMALL_INCLUDE=/DWINNT_SMALL
!else
SMALL_INCLUDE=
OBJDIR2=$(OBJDIR1)
!endif

OBJDIR=$(BD)\$(OBJDIR2)
OBJDIR11=$(OBJDIR)

!ifdef DEBUG
METER_LIB="$(FLEXMETERDEBUG)\fmeter.lib"
!else
METER_LIB="$(FLEXMETERREL)\fmeter.lib"
!endif

#****************************
# Directories
#****************************

BASE16 = f:\apps\msvc

#****************************
# Compilation information
#****************************

#
# Include files
#

INCS =  /I . \
        /I ..\h \
         /I ..\machind  \
        /I "$(MSDEVDIR)\include" \
        /I ..\patches \
        /I "$(FLEXMETERINC)"


#
# 32 bit compiler flags
# /D "NO_TIMER_THREADS"

!ifdef DEBUG
DBG_CFLAGS32 = $(CRT)d /Zi  \
                /Fd$(OBJDIR)\$(DLL32NAMEBASE).pdb
!else
DBG_CFLAGS32 = $(CRT) /O1 /D "RELEASE_VERSION" 
!endif

CFLAGS32 = /nologo /W2 /D"_CONSOLE" /D"PC" /D"WINNT" /D $(PLAT_INCLUDE) $(TIMER_INCLUDE) $(STATIC_FLAGS)\
                /YX /Fp$(OBJDIR)\$(DLL32NAMEBASE).pch $(FLEX_STATIC_MT) \
                $(SMALL_INCLUDE) \
                /D"LM_INTERNAL" /D "_LMGR_WINDLL" /Fo$(OBJDIR)\ $(DBG_CFLAGS32)

#
# 16 bit compiler flags
#

!ifdef DEBUG
DBG_CFLAGS16 =  /D "_DEBUG" /Zi /Od /Gx-
!else
DBG_CFLAGS16 =   /D "NDEBUG"  /O1 /D "RELEASE_VERSION" 
!endif
# /Gt3
CFLAGS16 = /nologo /G2 /GD /GEf  /W3 /ALw /D "PC" \
                 /D "LM_INTERNAL"  /D "_WINDOWS" /D "_LMGR_WINDLL"  \
                /FR$(OBJDIR)\ /Fd$(OBJDIR)\$(DLL16NAMEBASE).pdb /Fo$(OBJDIR)\  \
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
RC = rc
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

CLIBS = oldnames.lib  \
        libcmt.lib kernel32.lib user32.lib \
        advapi32.lib netapi32.lib libs\$(PLAT)\spromeps.lib \
        libs\$(LIBPLAT)\hasp32b.obj libs\$(LIBPLAT)\haspms32.obj \
        comctl32.lib  comdlg32.lib gdi32.lib   $(METER_LIB)

#
# 16 bit libraries
#

CLIBS16 =       oldnames \
                libw \
                ldllcew \
                lzexpand \
                ..\winsockx\winsockx \
                libs\win16\sscanf

#
# Link flags
#

!ifdef DEBUG
DBG_LFLAGS = /DEBUG /pdb:$(OBJDIR)\$(DLL32NAMEBASE).pdb
!else
DBG_LFLAGS = /INCREMENTAL:NO /RELEASE
!endif

!ifdef PROFILE
DBG_LFLAGS = $(DBG_LFLAGS) /profile /map
!endif

!ifdef MAP
DBG_LFLAGS = $(DBG_LFLAGS) /map
!endif

LFLAGS = /NOLOGO /NOD /subsystem:console /machine:$(RESPLAT) \
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

LFLAGS16 =  /NOD  /PACKC:61440 /SEG:512 /ALIGN:16 \
             /MAP:FULL /ONERROR:NOEXE $(DBG_LFLAGS16)

# "/STACK:40000" required for non DLL builds.


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

COMMON_OBJS =   $(OBJDIR)\l_dallas.obj \
                $(OBJDIR)\l_allfeat.obj \
                $(OBJDIR)\l_any_set.obj \
                $(OBJDIR)\l_asc_hostid.obj \
                $(OBJDIR)\l_baddate.obj \
                $(OBJDIR)\l_basic_conn.obj \
                $(OBJDIR)\l_check.obj \
                $(OBJDIR)\l_cksum.obj \
                $(OBJDIR)\l_cksum_ok.obj \
                $(OBJDIR)\l_config.obj \
                $(OBJDIR)\l_conn_msg.obj \
                $(OBJDIR)\l_connect.obj \
                $(OBJDIR)\l_date.obj \
                $(OBJDIR)\l_encode.obj \
                $(OBJDIR)\l_error.obj \
                $(OBJDIR)\l_fdata.obj \
                $(OBJDIR)\l_file_stub.obj \
                $(OBJDIR)\l_finder.obj \
                $(OBJDIR)\l_get_dlist.obj \
                $(OBJDIR)\l_get_id.obj \
                $(OBJDIR)\l_get_lfile.obj \
                $(OBJDIR)\l_getattr.obj \
                $(OBJDIR)\l_handshake.obj \
                $(OBJDIR)\l_heart.obj \
                $(OBJDIR)\l_host.obj \
                $(OBJDIR)\l_inet.obj \
                $(OBJDIR)\l_init_file.obj \
                $(OBJDIR)\l_ipaddr.obj \
                $(OBJDIR)\l_key.obj \
                $(OBJDIR)\l_keyword.obj \
                $(OBJDIR)\l_lfgets.obj \
                $(OBJDIR)\l_lmgrdr.obj \
                $(OBJDIR)\l_lookup.obj \
                $(OBJDIR)\l_malloc.obj \
                $(OBJDIR)\l_master.obj \
                $(OBJDIR)\l_master_lis.obj \
                $(OBJDIR)\l_mem.obj \
                $(OBJDIR)\l_modify.obj \
                $(OBJDIR)\l_msg_cksum.obj \
                $(OBJDIR)\l_now.obj \
                $(OBJDIR)\l_open_file.obj \
                $(OBJDIR)\l_pack.obj \
                $(OBJDIR)\l_package.obj \
                $(OBJDIR)\l_parse_attr.obj \
                $(OBJDIR)\l_pdaem.obj \
                $(OBJDIR)\l_platfm.obj \
                $(OBJDIR)\l_pserv.obj \
                $(OBJDIR)\l_rcvmsg.obj \
                $(OBJDIR)\l_read_lfile.obj \
                $(OBJDIR)\l_replog.obj \
                $(OBJDIR)\l_rerd.obj \
                $(OBJDIR)\l_select.obj \
                $(OBJDIR)\l_select_one.obj \
                $(OBJDIR)\l_sernum.obj \
                $(OBJDIR)\l_serv_msg.obj \
                $(OBJDIR)\l_sndmsg.obj \
                $(OBJDIR)\l_str_crypt.obj \
                $(OBJDIR)\l_supersede.obj \
                $(OBJDIR)\l_text.obj \
                $(OBJDIR)\l_uppercase.obj \
                $(OBJDIR)\l_validdate.obj \
                $(OBJDIR)\l_rdec.obj \
                $(OBJDIR)\l_wdec.obj \
                $(OBJDIR)\lgclient.obj \
                $(OBJDIR)\lis_getunit.obj \
                $(OBJDIR)\lm_baddate.obj \
                $(OBJDIR)\lm_chk_conf.obj \
                $(OBJDIR)\lm_ck_feats.obj \
                $(OBJDIR)\lm_ckin.obj \
                $(OBJDIR)\lm_ckout.obj \
                $(OBJDIR)\lm_config.obj \
                $(OBJDIR)\lm_cp_hostid.obj \
                $(OBJDIR)\lm_crstr.obj \
                $(OBJDIR)\lm_crypt.obj \
                $(OBJDIR)\lm_daemon.obj \
                $(OBJDIR)\lm_disconn.obj \
                $(OBJDIR)\lm_display.obj \
                $(OBJDIR)\lm_dump.obj \
                $(OBJDIR)\lm_feat_list.obj \
                $(OBJDIR)\lm_free_job.obj \
                $(OBJDIR)\lm_fset.obj \
                $(OBJDIR)\lm_get_attr.obj \
                $(OBJDIR)\lm_get_feats.obj \
                $(OBJDIR)\lm_get_redir.obj \
                $(OBJDIR)\lm_gethostid.obj \
                $(OBJDIR)\lm_getid_typ.obj \
                $(OBJDIR)\lm_heart.obj \
                $(OBJDIR)\lm_hostname.obj \
                $(OBJDIR)\lm_hosttype.obj \
                $(OBJDIR)\lm_init.obj \
                $(OBJDIR)\lm_isadmin.obj \
                $(OBJDIR)\lm_lic_where.obj \
                $(OBJDIR)\lm_log.obj \
                $(OBJDIR)\lm_lsapi.obj \
                $(OBJDIR)\lm_pconf.obj \
                $(OBJDIR)\lm_perror.obj \
                $(OBJDIR)\lm_redir_ver.obj \
                $(OBJDIR)\lm_remove.obj \
                $(OBJDIR)\lm_set_attr.obj \
                $(OBJDIR)\lm_shutdown.obj \
                $(OBJDIR)\lm_userlist.obj \
                $(OBJDIR)\lm_username.obj \
                $(OBJDIR)\lm_vsend.obj \
                $(OBJDIR)\lm_xstrcmp.obj \
                $(OBJDIR)\lmhostid.obj \
                $(OBJDIR)\lmpolicy.obj \
                $(OBJDIR)\lu_getdtab.obj \
                $(OBJDIR)\luhostid.obj \
                $(OBJDIR)\l_rainbow.obj \
                $(OBJDIR)\winflex.obj \
                $(OBJDIR)\lm_clean.obj\
                $(OBJDIR)\dir_pc.obj  

OBJS16 =        $(COMMON_OBJS) \
                $(OBJDIR)\l_ether.obj \
                $(OBJDIR)\sauth400.obj \
                $(OBJDIR)\spromeps.obj \
                $(OBJDIR)\winflex.obj \
                $(OBJDIR)\stdcstub.obj                 

OBJS32 =        $(COMMON_OBJS) \
                $(OBJDIR)\sam32.obj \
                $(OBJDIR)\wsock32.obj \
                $(OBJDIR)\l__gethostname.obj \
                $(OBJDIR)\l_nt_connect.obj \
                $(OBJDIR)\lm_vb.obj \
                $(OBJDIR)\l_vol_id.obj \
                $(OBJDIR)\l_dialogs.obj \
                $(OBJDIR)\l_snmp.obj \
                $(OBJDIR)\l_timers.obj \
                $(OBJDIR)\l_crypt.obj \
                $(OBJDIR)\l_sap.obj  \
                $(OBJDIR)\l_sap2.obj  \
                $(OBJDIR)\l_ntadd.obj  \
                $(OBJDIR)\l_icmp.obj   \
                $(OBJDIR)\l_getenv.obj \
                $(OBJDIR)\l_instlc.obj \
                $(OBJDIR)\lmborrow.obj

OBJS32LIB =     $(COMMON_OBJS) \
                $(OBJDIR)\sam32.obj \
                $(OBJDIR)\wsock32.obj \
                $(OBJDIR)\l__gethostname.obj \
                $(OBJDIR)\l_nt_connect.obj \
                $(OBJDIR)\l_vol_id.obj \
                $(OBJDIR)\l_dialogs.obj \
                $(OBJDIR)\l_snmp.obj \
                $(OBJDIR)\l_timers.obj \
                $(OBJDIR)\l_icmp.obj  \
                $(OBJDIR)\l_sap.obj  \
                $(OBJDIR)\l_sap2.obj  \
                $(OBJDIR)\lm_vb.obj \
                $(OBJDIR)\lm_njob.obj\
                $(OBJDIR)\l_ntadd.obj \
                $(OBJDIR)\l_getenv.obj \
                $(OBJDIR)\l_instlc.obj \
                $(OBJDIR)\lmborrow.obj 
                     

#****************************
# General Targets
#****************************

!ifdef 16BIT
all : all16 LMGR167A.RES
!else
dll_only : $(OBJDIR)\$(DLL32NAME)
all : all32
!endif

$(OBJDIR)\lmgr.res:   lmgr.rc 
        $(RC)  $(RCFLAGS32) lmgr.rc 
        copy lmgr.res $(OBJDIR)\lmgr.res
$(OBJDIR)\lmgr16.res:   lmgr16.rc $(WINTEST_RCDEP)
       $(RC)  /l 0x409  lmgr16.rc 
       copy lmgr16.res $(OBJDIR)\lmgr16.res



all32 : timestamp $(OBJDIR)\$(DLL32NAME)  $(OBJDIR)\lmgr.res  $(OBJDIR)\$(LIBNAME)
        @echo !--------------------------------------
        @echo ! Done building $(DLLNAME)
        @echo !--------------------------------------

all16 : timestamp LMGR167A.RES $(DLL16NAME)
        @echo !--------------------------------------
        @echo ! Done building $(DLL16NAME)
        @echo !--------------------------------------

timestamp :
        @echo !--------------------------------------
        @echo ! Building LMGRD DLL lmgr.lib STATIC=1 MTD=1
        @echo !--------------------------------------

$(OBJDIR)\$(DLL32NAME) :  $(OBJS32)  $(DLL32DEF) $(OBJDIR)\lmgr.res ..\machind\lmclient.h ..\h\lmachdep.h
        echo >NUL @<<$*.rsp
/DLL $(LFLAGS) /out:$(OBJDIR)\$(DLL32NAME)
/implib:$(OBJDIR)\$(DLL32NAMEBASE).lib $(OBJDIR)\lmgr.res 
/def:$(DLL32NAMEBASE).def $(OBJS32) $(CLIBS)
<<
        $(LD) @$*.rsp

lmgr.lib : $(OBJDIR11) $(OBJS32LIB) $(OBJDIR)\lmgr.res ..\machind\lmclient.h ..\h\lmachdep.h
        echo blane this dependency is incorrect.  You should use $(OBJDIR)\lmgr.lib instead.  Otherwise it rebuilds every time.
        $(LB) @<<
        $(LIBFLAGS) /out:$(OBJDIR)\$(LIBNAME) $(OBJS32LIB) libs\$(PLAT)\Spromeps.lib \
        libs\$(LIBPLAT)\hasp32b.obj libs\$(LIBPLAT)\haspms32.obj $(METER_LIB)
<<

$(OBJDIR)\lmgr.lib : $(OBJDIR11) $(OBJS32LIB) $(OBJDIR)\lmgr.res ..\machind\lmclient.h ..\h\lmachdep.h
        $(LB) @<<
        $(LIBFLAGS) /out:$(OBJDIR)\$(LIBNAME) $(OBJS32LIB) libs\$(PLAT)\Spromeps.lib \
        libs\$(LIBPLAT)\hasp32b.obj libs\$(LIBPLAT)\haspms32.obj $(METER_LIB)
<<

lmgr_md.lib : $(OBJS32LIB) $(OBJDIR)\lmgr.res ..\machind\lmclient.h ..\h\lmachdep.h 
        $(LB) @<<
        $(LIBFLAGS) /out:$(OBJDIR)\lmgr_md.lib $(OBJS32LIB) libs\$(PLAT)\Spromeps.lib \
        libs\$(LIBPLAT)\hasp32b.obj libs\$(LIBPLAT)\haspms32.obj $(METER_LIB)
<<

$(OBJDIR)\lmgr.obj: $(OBJDIR)\lmgr.res
        cvtres /MACHINE:$(RESPLAT) $(OBJDIR)\lmgr.res /OUT:$(OBJDIR)\lmgr.obj

$(DLL16NAME) : $(OBJS16) $(OBJDIR)\$(DLL16NAME)

LMGR16.RES:   LMGR16.RC 
        $(RC)   $(RCFLAGS) $(RCDEFINES)  -r LMGR16.RC
        copy lmgr16.res $(OBJDIR)\lmgr16.res
clean :
        @echo Cleaning up $(PLAT)
        -@del intel_debug\*.o*
        -@del intel_debug\*.p*
        -@del intel_debug\*.l*
        -@del intel_debug\*.i*
        -@del intel_debug\*.r*
        -@del intel_debug\*.e*
        -@del sintel_debug\*.o*
        -@del sintel_debug\*.p*
        -@del sintel_debug\*.l*
        -@del sintel_debug\*.i*
        -@del sintel_debug\*.r*
        -@del sintel_debug\*.e*
        -@del sintel_rel\*.o*
        -@del sintel_rel\*.p*
        -@del sintel_rel\*.l*
        -@del sintel_rel\*.i*
        -@del sintel_rel\*.r*
        -@del sintel_rel\*.e*
        -@del intel_rel\*.o*
        -@del intel_rel\*.p*
        -@del intel_rel\*.l*
        -@del intel_rel\*.i*
        -@del intel_rel\*.r*
        -@del intel_rel\*.e*
        -@del *.obj
        -@del *.p*
        -@del *.lib
        -@del *.exp*
        -@del *.ilk

($OBJDIR) : 
        -mkdir $(OBJDIR)
        
($OBJDIR11) : 
        -mkdir $(OBJDIR11)
        
#link16 :
$(OBJDIR)\$(DLL16NAME) :
        echo >NUL @<<$*.rsp
$(OBJDIR)\l_dallas.obj +
$(OBJDIR)\l_allfeat.obj +
$(OBJDIR)\l_any_set.obj +
$(OBJDIR)\l_asc_hostid.obj +
$(OBJDIR)\l_baddate.obj +
$(OBJDIR)\l_basic_conn.obj +
$(OBJDIR)\l_check.obj +
$(OBJDIR)\l_cksum.obj +
$(OBJDIR)\l_cksum_ok.obj +
$(OBJDIR)\l_config.obj +
$(OBJDIR)\l_conn_msg.obj +
$(OBJDIR)\l_connect.obj +
$(OBJDIR)\l_date.obj +
$(OBJDIR)\l_encode.obj +
$(OBJDIR)\l_error.obj +
$(OBJDIR)\l_fdata.obj +
$(OBJDIR)\l_file_stub.obj +
$(OBJDIR)\l_finder.obj +
$(OBJDIR)\l_get_dlist.obj +
$(OBJDIR)\l_get_id.obj +
$(OBJDIR)\l_get_lfile.obj +
$(OBJDIR)\l_getattr.obj +
$(OBJDIR)\l_handshake.obj +
$(OBJDIR)\l_heart.obj +
$(OBJDIR)\l_host.obj +
$(OBJDIR)\l_inet.obj +
$(OBJDIR)\l_init_file.obj +
$(OBJDIR)\l_ipaddr.obj +
$(OBJDIR)\l_key.obj +
$(OBJDIR)\l_keyword.obj +
$(OBJDIR)\l_lfgets.obj +
$(OBJDIR)\l_lmgrdr.obj +
$(OBJDIR)\l_lookup.obj +
$(OBJDIR)\l_malloc.obj +
$(OBJDIR)\l_master.obj +
$(OBJDIR)\l_master_lis.obj +
$(OBJDIR)\l_mem.obj +
$(OBJDIR)\l_modify.obj +
$(OBJDIR)\l_msg_cksum.obj +
$(OBJDIR)\l_now.obj +
$(OBJDIR)\l_open_file.obj +
$(OBJDIR)\l_pack.obj +
$(OBJDIR)\l_package.obj +
$(OBJDIR)\l_parse_attr.obj +
$(OBJDIR)\l_pdaem.obj +
$(OBJDIR)\l_platfm.obj +
$(OBJDIR)\l_pserv.obj +
$(OBJDIR)\l_rcvmsg.obj +
$(OBJDIR)\l_read_lfile.obj +
$(OBJDIR)\l_replog.obj +
$(OBJDIR)\l_rerd.obj +
$(OBJDIR)\l_select.obj +
$(OBJDIR)\l_select_one.obj +
$(OBJDIR)\l_sernum.obj +
$(OBJDIR)\l_serv_msg.obj +
$(OBJDIR)\l_sndmsg.obj +
$(OBJDIR)\l_str_crypt.obj +
$(OBJDIR)\l_supersede.obj +
$(OBJDIR)\l_text.obj +
$(OBJDIR)\l_uppercase.obj +
$(OBJDIR)\l_validdate.obj +
$(OBJDIR)\l_rdec.obj +
$(OBJDIR)\l_wdec.obj +
$(OBJDIR)\lgclient.obj +
$(OBJDIR)\lis_getunit.obj +
$(OBJDIR)\lm_baddate.obj +
$(OBJDIR)\lm_chk_conf.obj +
$(OBJDIR)\lm_ck_feats.obj +
$(OBJDIR)\lm_ckin.obj +
$(OBJDIR)\lm_ckout.obj +
$(OBJDIR)\lm_config.obj +
$(OBJDIR)\lm_cp_hostid.obj +
$(OBJDIR)\lm_crstr.obj +
$(OBJDIR)\lm_crypt.obj +
$(OBJDIR)\lm_daemon.obj +
$(OBJDIR)\lm_disconn.obj +
$(OBJDIR)\lm_display.obj +
$(OBJDIR)\lm_dump.obj +
$(OBJDIR)\lm_feat_list.obj +
$(OBJDIR)\lm_free_job.obj +
$(OBJDIR)\lm_fset.obj +
$(OBJDIR)\lm_get_attr.obj +
$(OBJDIR)\lm_get_feats.obj +
$(OBJDIR)\lm_get_redir.obj +
$(OBJDIR)\lm_gethostid.obj +
$(OBJDIR)\lm_getid_typ.obj +
$(OBJDIR)\lm_heart.obj +
$(OBJDIR)\lm_hostname.obj +
$(OBJDIR)\lm_hosttype.obj +
$(OBJDIR)\lm_init.obj +
$(OBJDIR)\lm_isadmin.obj +
$(OBJDIR)\lm_lic_where.obj +
$(OBJDIR)\lm_log.obj +
$(OBJDIR)\lm_lsapi.obj +
$(OBJDIR)\lm_pconf.obj +
$(OBJDIR)\lm_perror.obj +
$(OBJDIR)\lm_redir_ver.obj +
$(OBJDIR)\lm_remove.obj +
$(OBJDIR)\lm_set_attr.obj +
$(OBJDIR)\lm_shutdown.obj +
$(OBJDIR)\lm_userlist.obj +
$(OBJDIR)\lm_username.obj +
$(OBJDIR)\lm_vsend.obj +
$(OBJDIR)\lm_xstrcmp.obj +
$(OBJDIR)\lmhostid.obj +
$(OBJDIR)\lmpolicy.obj +
$(OBJDIR)\lu_getdtab.obj +
$(OBJDIR)\luhostid.obj +
$(OBJDIR)\l_rainbow.obj +
$(OBJDIR)\winflex.obj +
$(OBJDIR)\lm_clean.obj +
$(OBJDIR)\dir_pc.obj  +
$(OBJDIR)\l_ether.obj +
$(OBJDIR)\sauth400.obj +
$(OBJDIR)\spromeps.obj +
$(OBJDIR)\stdcstub.obj +
libs\win16\spro1.obj                
$(OBJDIR)\$(DLL16NAME)
$(OBJDIR)\lmgr16.map
$(BASE16)\lib\+
$(CLIBS16)
$(DLL16NAMEBASE).def;
<<
        $(LD) $(LFLAGS16) @$*.rsp
        copy $(OBJDIR)\$(DLL16NAME) $(DLL16NAME)
        $(RC) LMGR16.RES $(DLL16NAME)
        copy $(DLL16NAME) $(OBJDIR)\$(DLL16NAME)
        -@del $(DLL16NAME)
        implib  /nowep $(OBJDIR)\$(DLL16NAMEBASE).lib \
                $(OBJDIR)\$(DLL16NAME)

#*******************************
# Dependencies
#*******************************
$(COMMON_OBJS): ..\machind\lmclient.h ..\h\l_prot.h ..\h\l_privat.h
$(OBJS32): ..\machind\lmclient.h ..\h\l_prot.h ..\h\l_privat.h

