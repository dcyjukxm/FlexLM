#*****************************************************************************
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
#       %Id%
#
#*****************************************************************************
#****************************
# Environment setup
# Invoke ">MAKESETUP {DEBUG|NONDEBUG} \
#                      {i86_n3|i86_w3|alpha_n3|i86_z3} \
#                      {STATIC|DLL}
#                      {RELEASE|INTERNAL}
#                      {MT|MD}
#       The defaults are debug, static, 32bit, intel, debug, internal, mt
#****************************


!include "..\h\pc.mak"




#****************************
# Platform setup
#****************************
#
#
#
!ifdef NOTIMERS
CL_TIMER=/D "NO_TIMER_THREADS"
!else
CL_TIMER=
!endif

#
#
#


RCFLAGS32 = /l 0x409 
D_RCDEFINES32 = /d_DEBUG 
R_RCDEFINES32 = /dNDEBUG
D_RCDEFINES = -d_DEBUG
R_RCDEFINES = -dNDEBUG
RCFLAGS = 
RESFLAGS = 
CERTDIR=\certicom


RCDEFINES32 = $(D_RCDEFINES32)
RCDEFINES = $(D_RCDEFINES)


!ifdef STATIC
STATIC_FLAGS= /D "FLEX_STATIC"
!else
STATIC_FLAGS=
!endif


!ifdef WINNT_SMALL
CL_SMALL=/DWINNT_SMALL
!else
CL_SMALL=
!endif


!ifdef DEBUG
METER_LIB="$(FLEXMETERDEBUG)\fmeter.lib"
LCMOBJDIR=debug
!else
METER_LIB="$(FLEXMETERREL)\fmeter.lib"
LCMOBJDIR=release
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

#INCS =  /I . \
#        /I ..\h \
#        /I ..\machind  \
#        /I ..\patches 
#        /I $(FLEXLM_LCM) 
#        /I "$(FLEXMETERINC)" 


#
# 32 bit compiler flags
# /D "NO_TIMER_THREADS"


CFLAGS = $(CLFLAGS) /Fp$(DLL32PREFIX).pch /Fd$(DLL32PREFIX).pdb 


#
# Compilation rules
#
RC=rc
CC=$(CC)
#CC=cc /w5

all : ..\src\lmgr.lib lmgr.res
#dll 

#dll: 
#	$(MAKE) /f pc.mak ..\src\lmgr8b.lib FLEXLM_MAKE_DLLFLAG=DLL 


.c.obj: 
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

!if 0
LCMLIB = $(FLEXLM_LCM)\$(LCMOBJDIR)\lcminstwiz.lib
!else
LCMLIB =
!endif 

#CLIBS = oldnames.lib  \
#        libcmt.lib kernel32.lib user32.lib \
#        advapi32.lib netapi32.lib libs\$(LIBPLAT)\spromeps.lib \
#        comctl32.lib  comdlg32.lib gdi32.lib   $(LCMLIB) 
#        $(METER_LIB) 

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


#
# Link commands
#


#****************************
# Library creation
#****************************

LB = lib

 

#****************************
# File lists
#****************************

COMMON_OBJS =   \
		flexevent.obj \
		flex_file.obj \
		flex_utils.obj \
		dir_pc.obj \
		l_allfeat.obj \
                l__gethostname.obj \
                l_any_set.obj \
                l_asc_hostid.obj \
                l_baddate.obj \
                l_basic_conn.obj \
		l_bios_id.obj \
                l_borrow.obj \
                l_check.obj \
                l_cksum.obj \
                l_cksum_ok.obj \
		l_comp_id.obj \
                l_config.obj \
                l_conn_msg.obj \
                l_connect.obj \
		l_cpu_id.obj \
                l_ctype.obj \
                l_date.obj \
		l_diskgeo_id.obj\
                l_encode.obj \
                l_error.obj \
                l_fdata.obj \
                l_file_stub.obj \
                l_finder.obj \
                l_get_dlist.obj \
                l_get_id.obj \
                l_get_lfile.obj \
                l_getattr.obj \
                l_getenv.obj \
                l_handshake.obj \
                l_heart.obj \
                l_host.obj \
                l_icmp.obj   \
                l_inet.obj \
                l_init_file.obj \
                l_instlc.obj \
                l_intelid.obj \
                l_ipaddr.obj \
                l_key.obj \
                l_keyword.obj \
                l_lfgets.obj \
                l_lmgrdr.obj \
                l_lookup.obj \
                l_malloc.obj \
                l_master.obj \
                l_master_lis.obj \
                l_mddl.obj \
                l_mem.obj \
                l_modify.obj \
                l_msg_cksum.obj \
                l_now.obj \
                l_nt_connect.obj \
                l_ntadd.obj  \
                l_open_file.obj \
                l_pack.obj \
                l_package.obj \
                l_parse_attr.obj \
                l_pcconn.obj \
                l_pdaem.obj \
                l_platfm.obj \
                l_pserv.obj \
                l_rcvmsg.obj \
                l_rdec.obj \
                l_read_lfile.obj \
                l_replog.obj \
                l_rerd.obj \
                l_sap.obj  \
                l_sap2.obj  \
                l_select.obj \
                l_select_one.obj \
                l_sernum.obj \
                l_serv_msg.obj \
		l_sha.obj \
                l_sndmsg.obj \
                l_str_crypt.obj \
                l_supersede.obj \
                l_text.obj \
		l_time.obj \
		l_timers.obj \
                l_ts.obj \
                l_unixmt.obj \
                l_uppercase.obj \
                l_validdate.obj \
                l_vol_id.obj \
                l_wdec.obj \
                l_winrsrc.obj \
                leintelid.obj \
                lgclient.obj \
                lis_getunit.obj \
                lm_baddate.obj \
                lm_chk_conf.obj \
                lm_ck_feats.obj \
                lm_ckin.obj \
                lm_ckout.obj \
                lm_clean.obj\
                lm_config.obj \
                lm_cp_hostid.obj \
                lm_crstr.obj \
                lm_crypt.obj \
                lm_daemon.obj \
                lm_disconn.obj \
                lm_display.obj \
                lm_dump.obj \
                lm_feat_list.obj \
                lm_free_job.obj \
                lm_fset.obj \
                lm_get_attr.obj \
                lm_get_feats.obj \
                lm_get_redir.obj \
                lm_gethostid.obj \
                lm_heart.obj \
                lm_hostname.obj \
                lm_hosttype.obj \
                lm_isadmin.obj \
                lm_lic_where.obj \
                lm_log.obj \
                lm_lsapi.obj \
                lm_pconf.obj \
                lm_perror.obj \
                lm_rand3.obj \
                lm_redir_ver.obj \
                lm_remove.obj \
                lm_set_attr.obj \
                lm_shutdown.obj \
		lm_scompid.obj \
                lm_userlist.obj \
                lm_username.obj \
                lm_vb.obj \
                lm_vsend.obj \
                lm_xstrcmp.obj \
                lmgrrsrc.obj \
                lmhostid.obj \
                lu_getdtab.obj \
                luhostid.obj \
                sam32.obj  \
!if "$(FLEXLM_MAKE_PLATFORM)" == "i64_n5"
                wsock32.obj \
                lGetNativeHostIdstub.obj
!else
                wsock32.obj 
!endif

!if "$(FLEXLM_MAKE_PLATFORM)" == "i64_n5"
ARCH_OBJS = 	l_dnglstub.obj \
		l_aladdin.obj
!else
ARCH_OBJS = 	l_dallas.obj \
		lgcspimx.obj \
		l_rainbow.obj \
		l_aladdin.obj 
!endif

PUBKEYOBJS	= l_prikey.obj
PUBKEYDIR 	= $(CERTDIR)\include

#	These files have d appended to the name
!if "$(FLEXLM_MAKE_DLLFLAG)" == "DLL"
DLL_OBJS =   	l_dialogsd.obj \
		lm_getid_typd.obj \
		lm_initd.obj \
		lm_njobd.obj \
		lmpolicyd.obj \
		l_lock_loadd.obj \
		winflexd.obj
!else 
#	unlike these
DLL_OBJS =   l_dialogs.obj \
		lm_getid_typ.obj \
		lm_init.obj \
		lm_njob.obj \
		lmpolicy.obj \
		l_lock_load.obj \
		winflex.obj
!endif

# JAVA_IF_OBJ gets exported to CVS for inclusion into lmgr.lib on i86_n3 only
JAVA_IF_CVS_TAG	= flexlm-java-tng-v9_0-branch


#!if "$(FLEXLM_MAKE_PLATFORM)" == "i86_n3"
JAVA_IF_OBJ = lGetNativeHostId.obj
#JAVA_IF_PATH = flexlm-java-tng\c\i86_n3\$(JAVA_IF_OBJ)
#JAVA_IF_PATH_CVS = flexlm-java-tng/c/i86_n3/$(JAVA_IF_OBJ)
#JAVA_IF_EXPORT = cvs export -r $(JAVA_IF_CVS_TAG) $(JAVA_IF_PATH_CVS)
#!else
#JAVA_IF_OBJ = 	
#JAVA_IF_PATH = 	
#JAVA_IF_EXPORT =
#!endif

java_if:	$(JAVA_IF_OBJ)
#!if "$(FLEXLM_MAKE_PLATFORM)" == "i86_n3"
#	if not exist $(JAVA_IF_PATH) $(JAVA_IF_EXPORT)
#	if not exist $(JAVA_IF_OBJ) copy $(JAVA_IF_PATH) $(JAVA_IF_OBJ)
#!endif



OBJS =        $(COMMON_OBJS) \
                $(PUBKEYOBJS) \
		$(DLL_OBJS) \
		$(ARCH_OBJS) \
		$(JAVA_IF_OBJ)



                     

#****************************
# General Targets
#****************************


lmgr.res:   lmgr.rc 
        $(RC)  $(RCFLAGS32) lmgr.rc 

lmgr16.res:   lmgr16.rc $(WINTEST_RCDEP)
       $(RC)  /l 0x409  lmgr16.rc 
       copy lmgr16.res lmgr167a.res

lgcspimx.c : baseid.exe
	baseid.exe > lgcspimx.c

baseid.obj : l_intelid.c
!if "$(FLEXLM_MAKE_PLATFORM)" == "i64_n5"
	$(CC) /DLM_INTERNAL /D"CPUID_GENERATOR" $(INCS) l_intelid.c /Fobaseid.obj 
!else
	$(CC) /DLM_INTERNAL /D"CPUID_GENERATOR" /D_x86_ l_intelid.c /I..\machind /I..\h /Fobaseid.obj 
!endif

$(PUBKEYOBJS):	
         $(CC) -c $(CFLAGS) -I $(PUBKEYDIR) l_prikey.c /Fl_prikey.obj


baseid.exe: baseid.obj
!if "$(FLEXLM_MAKE_PLATFORM)" == "i64_n5"
$(LD) $(LFLAGS) /out:$*.exe baseid.obj $(CLIBS)
!else
$(LD) $(LFLAGS) /out:$*.exe baseid.obj 
!endif

#lmgrrsrc.c : lmgrrsrc.res
#        $(RES2C) lmgrrsrc.res -o lmgrrsrc.c -p l_findrsrcdata


#..\src\lmgr327b.lib :  $(OBJS)  $(DLL32PREFIX).def ..\machind\lmclient.h ..\h\lmachdep.h 
#       $(RC)  $(RCFLAGS32)  lmgr.rc
#       $(RC)  $(RCFLAGS32)  lmgrver.rc
#        echo >NUL @<<$*.rsp
#/DLL $(LFLAGS) /NODEFAULTLIB /out:$(DLL32PREFIX).dll /machine:$(L_TARGET)
#/implib:$(DLL32PREFIX).lib  $(LINKDEBUG)
#!if "$(FLEXLM_MAKE_PLATFORM)" == "i64_n5"
#/def:$(DLL32PREFIX).def $(OBJS) $(CLIBS) $(XTRADLL)
#!else
#/def:$(DLL32PREFIX).def $(OBJS) libs\$(LIBPLAT)\hasp32b.obj libs\$(LIBPLAT)\haspms32.obj $(CLIBS) $(XTRADLL)
#!endif
#<<
#        $(LD) @$*.rsp

#..\src\lmgr8b.lib :  $(OBJS)  $(DLL32PREFIX).def ..\machind\lmclient.h ..\h\lmachdep.h 
#       $(RC)  $(RCFLAGS32)  lmgr.rc
#       $(RC)  $(RCFLAGS32)  lmgrver.rc
#        echo >NUL @<<$*.rsp
#/DLL $(LFLAGS) /NODEFAULTLIB /out:$(DLL32PREFIX).dll /machine:$(L_TARGET)
#/implib:$(DLL32PREFIX).lib  $(LINKDEBUG)
#!if "$(FLEXLM_MAKE_PLATFORM)" == "i64_n5"
#/def:$(DLL32PREFIX).def $(OBJS) libs\$(LIBPLAT)\haspw64.lib $(CLIBS) $(XTRADLL)
#!else
#/def:$(DLL32PREFIX).def $(OBJS) libs\$(LIBPLAT)\hasp32b.obj libs\$(LIBPLAT)\haspms32.obj $(CLIBS) $(XTRADLL)
#!endif
#<<
#        $(LD) @$*.rsp


..\src\lmgr.lib : $(OBJS) java_if
        $(LB) @<<
!if "$(FLEXLM_MAKE_PLATFORM)" == "i64_n5"
        $(LIBFLAGS) /out:$(DLL32NAME) $(OBJS) libs\$(LIBPLAT)\haspw64.lib $(LCMLIB)
!else
        $(LIBFLAGS) /out:$(DLL32NAME) $(OBJS) libs\$(LIBPLAT)\spromeps.lib libs\$(LIBPLAT)\hasp32b.obj libs\$(LIBPLAT)\haspms32.obj $(LCMLIB)
!endif

<<

lmgrrsrc.c : lmgrrsrc.res
        $(RES2C) lmgrrsrc.res -o lmgrrsrc.c -p l_findrsrcdata

#        $(METER_LIB) 



LMGR16.RES:   LMGR16.RC 
        $(RC)   $(RCFLAGS) $(RCDEFINES)  -r LMGR16.RC
        copy lmgr16.res $(OBJDIR)\lmgr16.res


clean :
        @echo Cleaning up $(PLAT)
        @if exist *.obj del *.obj
        @if exist *.p* del *.p*
        @if exist *.lib del *.lib
        @if exist baseid.exe del baseid.exe
        @if exist *.exp del *.exp
        @if exist *.ilk del *.ilk
	@if exist lmgr.res del lmgr.res

($OBJDIR) : 
        -mkdir $(OBJDIR)
        
($OBJDIR11) : 
        -mkdir $(OBJDIR11)
        
l_dialogsd.obj:	l_dialogs.c
	$(CC) $(CFLAGS) $(INCLS) /c l_dialogs.c /Fol_dialogsd.obj

lm_getid_typd.obj:	lm_getid_typ.c
	$(CC) $(CFLAGS) $(INCLS) /c lm_getid_typ.c /Folm_getid_typd.obj

lm_initd.obj:	lm_init.c
	$(CC) $(CFLAGS) $(INCLS) /c lm_init.c /Folm_initd.obj

lm_njobd.obj:	lm_njob.c
	$(CC) $(CFLAGS) $(INCLS) /c lm_njob.c /Folm_njobd.obj

lmpolicyd.obj:	lmpolicy.c
	$(CC) $(CFLAGS) $(INCLS) /c lmpolicy.c /Folmpolicyd.obj

l_lock_loadd.obj:	l_lock_load.c
	$(CC) $(CFLAGS) $(INCLS) /c l_lock_load.c /Fol_lock_loadd.obj

winflexd.obj:	winflex.c
	$(CC) $(CFLAGS) $(INCLS) /c winflex.c /Fowinflexd.obj


#*******************************
# Dependencies
#*******************************
$(OBJS): ..\machind\lmclient.h ..\h\l_prot.h ..\h\l_privat.h ..\h\lmachdep.h
flexevent.obj:	..\h\flexevent.h ..\machind\lmclient.h
flex_file.obj:	..\h\flex_file.h ..\h\flex_utils.h
flex_utils.obj:	..\h\flex_utils.h
lm_perror.obj:  ..\machind\lmerrors.h ..\machind\lm_lerr.h \
                ..\machind\lcontext.h

lm_ckout.obj:   l_crypt.c l_strkey.c ..\h\l_strkey.h
lm_crypt.obj:   l_crypt.c l_strkey.c ..\h\l_strkey.h
lm_fset.obj:    l_strkey.c ..\h\l_strkey.h
l_borrow.obj:	..\h\l_borrow.h
lm_ckout.obj:	..\h\l_borrow.h
lmpolicy.obj:	../machind/lmpolicy.h
l_allfeat.obj:	../h/lgetattr.h l_strkey.c l_crypt.c \
				../h/l_strkey.h
l_basic_conn.obj:	../h/lm_comm.h ../h/lmselect.h 
l_check.obj:	../h/lgetattr.h ../h/lmselect.h ../h/lm_comm.h 
l_cksum_ok.obj:	../h/lm_comm.h 
l_conn_msg.obj:	../h/lm_comm.h 
l_connect.obj:	../h/lm_comm.h ../h/lmselect.h ../h/lgetattr.h 
l_encode.obj:	../h/lm_comm.h 
l_finder.obj:	../h/lmselect.h 
l_get_lfile.obj:	../h/lm_comm.h 
l_getattr.obj:	../h/lgetattr.h 
l_handshake.obj:	../h/lm_comm.h 
l_heart.obj:	../h/lm_comm.h
l_host.obj:	../h/lgetattr.h 
l_init_file.obj:	../h/l_openf.h ../h/lgetattr.h 
l_lfgets.obj:	../h/l_openf.h 
l_master_lis.obj:	../h/l_openf.h  
l_modify.obj:	../h/lm_comm.h 
l_msg_cksum.obj:	../h/lm_comm.h  ../h/lcommdbg.h
l_open_file.obj:	../h/l_openf.h 
l_pack.obj:	../h/l_pack.h
l_parse_attr.obj:	../h/lgetattr.h
l_rdec.obj:	../h/l_pack.h
l_wdec.obj:	../h/l_pack.h
l_parse_conf.obj:	../h/lgetattr.h
l_pdaem.obj:	../machind/lsmaster.h 
l_rcvmsg.obj:	../h/lm_comm.h ../h/lmselect.h 
l_replog.obj:  	../h/lgetattr.h   ../h/lm_comm.h
l_sernum.obj:	../h/lm_comm.h
l_serv_msg.obj:	../h/lm_comm.h ../h/lgetattr.h
l_sndmsg.obj:	../h/lm_comm.h 
l_timers.obj:  ../h/lgetattr.h  
lm_ckin.obj:	../h/lm_comm.h 
lm_ckout.obj:	../h/lm_comm.h  ../h/lgetattr.h l_strkey.c l_crypt.c \
				../h/l_strkey.h
lm_ck_feats.obj:  ../h/lgetattr.h 
lm_config.obj:    ../h/l_openf.h  ../h/lm_comm.h
lm_daemon.obj:	../h/l_openf.h 
lm_disconn.obj:	
lm_display.obj: ../h/lgetattr.h 
lm_feat_list.obj:	../h/l_openf.h ../h/lgetattr.h 
lm_fset.obj:	../h/l_openf.h  l_strkey.c
lm_get_attr.obj:	../machind/lm_attr.h ../h/lgetattr.h 
lm_getid_typ.obj:	../h/lm_comm.h  ../h/lgetattr.h
lm_hostname.obj:	../h/lgetattr.h 
lm_get_feats.obj:	../h/l_openf.h 
lm_get_redir.obj:	../h/l_openf.h 
lm_hosttype.obj:	../machind/lmhtype.h ../h/lgetattr.h 
lm_init.obj:	../machind/lm_attr.h ../machind/lmerrors.h  ../h/lm_comm.h ../h/lgetattr.h
lm_lsapi.obj:	../machind/lm_lsapi.h 
lm_log.obj:	../h/lm_comm.h 
lm_pconf.obj:	../h/l_pack.h
lm_perror.obj:	../machind/lmerrors.h  ../machind/lm_lerr.h ../machind/lcontext.h
lm_remove.obj:	../h/lm_comm.h 
lm_set_attr.obj:	../machind/lm_attr.h  ../h/lgetattr.h
lm_shutdown.obj:	../h/lm_comm.h ../h/l_openf.h 
lm_slwg.obj:	../machind/lm_attr.h ../h/lm_slwg.h 
lm_userlist.obj:	../h/lm_comm.h  ../h/lgetattr.h
lm_username.obj:	../h/lgetattr.h 
lm_vsend.obj:	../h/lm_comm.h 
lm_get_dlist.c:	../h/lsmaster.h ../h/l_openf.h
lm_crypt.obj:	l_crypt.c l_strkey.c ../h/l_strkey.h
lmpolicy.obj:	../machind/lmpolicy.h
l_borrow.obj:	../h/l_borrow.h ../h/l_rand.h
lm_ckout.obj:	../h/l_borrow.h

$(PUBKEYOBJS):	../h/l_pubkey.h
