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
# $Id: makefile,v 1.48 2003/04/21 17:29:38 sluu Exp $
#
# Makefile to build the client library
#
# Building a cross compiled version for VXWORKS is not automated and probably
# not working yet for the full kit for the host platform.
#	1. Build the whole thing with -DTARGETVXWORKS
#	2. Build the target client lib from the src directory
#	   for VxWorks
# The PC hosted and VXWORKS targeted version builds on the PC. 
#

THREADFLAG = `gplatargs -threads`
XTRACFLAG = `gplatargs` -DLM_INTERNAL 

SHAREDOPTS =  -assert pure-text 

XTRALINTFLAG = `gplatargs -lint` -DLM_INTERNAL

INCFLAGS = -I../h  -I../machind 

LINTLIBFLAGS = $(XTRALINTFLAG)

LINTFLAGS = `gplatargs -lint` $(XTRALINTFLAG)

LINTOUTFLAG = `gplatargs -lint`

DEBUGFLAG = -g
CERTDIR = /u/gsi/certicom/sb24ssr

#PCFLAGS is used to set CFLAGS in the depend line (mkmf)
PCFLAGS = $(INCFLAGS) $(DEBUGFLAG) $(XTRACFLAG)
CFLAGS = $(INCFLAGS) $(DEBUGFLAG) $(XTRACFLAG) 


OBJECTS 	= l_allfeat.o \
		l_any_set.o \
		l_apollo_id.o \
		l_asc_hostid.o \
		l_baddate.o \
		l_basic_conn.o \
		l_borrow.o \
		l_check.o \
		l_cksum.o \
		l_cksum_ok.o \
		l_comp_id.o \
		l_config.o \
		l_conn_msg.o \
		l_connect.o \
		l_cpu_id.o \
		l_ctype.o \
		l_date.o \
		l_diskgeo_id.o \
		l_encode.o \
		l_error.o \
		l_ether_id.o \
		l_fdata.o \
		l_file_stub.o \
		l_finder.o \
		l_get_dlist.o \
		l_get_id.o \
		l_get_lfile.o \
		l_getattr.o \
		l_getenv.o \
		l_gethn.o \
		l_handshake.o \
		l_heart.o \
		l_host.o \
		l_inet.o \
		l_init_file.o \
		l_instlc.o \
		l_ipaddr.o \
		l_key.o \
		l_keyword.o \
		l_lfgets.o \
		l_lookup.o \
		l_malloc.o \
		l_master.o \
		l_master_lis.o \
		l_mem.o \
		l_modify.o \
		l_msg_cksum.o \
		l_now.o \
		l_open_file.o \
		l_pack.o \
		l_package.o \
		l_parse_attr.o \
		l_lmgrdr.o \
		l_pdaem.o \
		l_pserv.o \
		l_rcvmsg.o \
		l_rdec.o \
		l_read_lfile.o \
		l_replog.o \
		l_rerd.o \
		l_select.o \
		l_select_one.o \
		l_sernum.o \
		l_serv_msg.o \
		l_sha.o \
		l_sndmsg.o \
		l_str_crypt.o \
		l_supersede.o \
		l_text.o \
		l_time.o \
		l_timers.o \
		l_uppercase.o \
		l_validdate.o \
		l_wdec.o \
		lgclient.o \
		lis_getunit.o \
		lm_baddate.o \
		lm_chk_conf.o \
		lm_ck_feats.o \
		lm_ckin.o \
		lm_ckout.o \
		lm_config.o \
		lm_cp_hostid.o \
		lm_crstr.o \
		lm_crypt.o \
		lm_daemon.o \
		lm_disconn.o \
		lm_display.o \
		lm_dump.o \
		lm_feat_list.o \
		lm_free_job.o \
		lm_fset.o \
		lm_get_attr.o \
		lm_get_feats.o \
		lm_get_redir.o \
		lm_gethostid.o \
		lm_getid_typ.o \
		lm_heart.o \
		lm_hostname.o \
		lm_hosttype.o \
		lm_init.o \
		lm_isadmin.o \
		lm_lic_where.o \
		lm_log.o \
		lm_lsapi.o \
		lm_njob.o \
		lm_pconf.o \
		lm_perror.o \
		lm_redir_ver.o \
		lm_rand3.o \
		lm_remove.o \
		lm_set_attr.o \
		lm_shutdown.o \
		lm_scompid.o \
		lm_slwg.o \
		lm_startup.o \
		lm_userlist.o \
		lm_username.o \
		lm_vb.o \
		lm_vsend.o \
		lm_xstrcmp.o \
		lmhostid.o \
		lmpolicy.o \
		lu_getdtab.o \
		lu_gethost.o \
		lu_setitimer.o \
		lu_setline.o \
		lu_sleep.o \
		lu_wait3.o \
		luhostid.o \
		flex_file.o \
		flex_utils.o 

THREADOBJS 	= l_unixmt.o 


PLATOBJS      = l_platfm.o

PUBKEYOBJS	= l_prikey.o
PUBKEYDIR 	= $(CERTDIR)/include


ASSOBJS	= l_sgi.o

OBJS	      = $(OBJECTS) \
		$(PLATOBJS)  \
		$(PUBKEYOBJS)  \
		$(THREADOBJS) \
		$(ASSOBJS)  

SPARCOBJS     = $(OBJS) 


LIBRARY	= liblmgr.a
MINILIBRARY = libmini_lmgr.a
LINTLIB = lmgr
VER = 1.0

all:	
	$(MAKE) _all CFLAGS="$(CFLAGS)" CERTDIR="$(CERTDIR)"


_all:	$(LIBRARY)  lm_nomt.o

test_release: 
	mv liblmgr.a liblmgr.asav
	$(MAKE) clean
	$(MAKE) _all DEBUGFLAG= XTRACFLAG="$(XTRACFLAG) -DRELEASE_VERSION"
	mv liblmgr.a liblmgr_rel.a
	$(MAKE) clean
	mv liblmgr.asav liblmgr.a

JAVA_IF_DIR	= ../flexlm-java-tng
JAVA_IF_PATH	= $(JAVA_IF_DIR)/c
JAVA_IF_OBJ	= lGetNativeHostId.o
JAVA_IF_CVS_TAG	= flexlm-java-tng-9_0_0

$(LIBRARY):	$(OBJS)
	@echo Creating $(LIBRARY) ...
	if [ "`gplatform`" = "i86_r6" -o "`gplatform`" = "sun64_u5" -o \
	     "`gplatform`" = "sun4_u5" -o "`gplatform`" = "sun4_u7" ]; then \
#	cvs export -r $(JAVA_IF_CVS_TAG)\
#			 $(JAVA_IF_PATH)/`gplatform`/$(JAVA_IF_OBJ) ;\
		cp $(JAVA_IF_PATH)/`gplatform`/$(JAVA_IF_OBJ) . ;\
#		rm -rf $(JAVA_IF_DIR);\
		$(AR) cr $(LIBRARY) $(OBJS) $(JAVA_IF_OBJ); \
	else \
		$(AR) cr $(LIBRARY) $(OBJS); \
	fi
	-test -f /bin/ranlib && ranlib $(LIBRARY) || \
	test -f /usr/bin/ranlib && ranlib $(LIBRARY) || true

clean:	
	@echo "clean src"
	@rm -f $(OBJS) $(LIBRARY)  lm_nomt.o $(JAVA_IF_OBJ)
	
lm_nomt.o:	l_unixmt.c
		$(CC) -c $(CFLAGS) -DNO_MT l_unixmt.c -o lm_nomt.o
		if [ "`gplatargs -pic`" != "`gplatargs`" ] ; \
		then \
			$(CC) -c $(CFLAGS) -DNO_MT `gplatargs -pic` l_unixmt.c -o lm_nomt_pic.o; \
		fi

$(PLATOBJS):	
		$(CC) -c $(CFLAGS) -DGPLATFORM=\"`gplatform`\" $*.c

$(THREADOBJS):	
		$(CC) -c $(CFLAGS) $(THREADFLAG) $*.c

$(PUBKEYOBJS):	
		$(CC) -c $(CFLAGS) -I$(PUBKEYDIR) $*.c 


$(ASSOBJS):	
		@echo "static int unused;" > l_sgi.c; $(CC) $(CFLAGS) -c l_sgi.c
		@if [ `gplatform` = sgi_u5 -o `gplatform` = sgir8_u6 -o `gplatform` = sgi32_u5 -o `gplatform` = sgi64_u6 -o `gplatform` = sgi32_u6 -o `gplatform` = sgio32_u6 ] ;\
		then \
			echo "as -c -DSGI $(CFLAGS) $*.s -o $*.o" ;\
			rm $*.o ;\
			as -c -DSGI $(CFLAGS) $*.s -o $*.o ;\
		else \
			echo "making dummy l_sgi.o" ;\
		fi

$(OBJS) $(PUBKEYOBJS) lm_nomt.o:	../h/lmachdep.h ../machind/lmclient.h ../h/l_prot.h ../h/l_privat.h

depend:;	mkmf -f src.makefile CFLAGS="$(PCFLAGS)"

###
l_allfeat.o:	../h/lgetattr.h l_strkey.c l_crypt.c \
				../h/l_strkey.h ../h/l_rand.h
l_basic_conn.o:	../h/lm_comm.h ../h/lmselect.h 
l_check.o:	../h/lgetattr.h ../h/lmselect.h ../h/lm_comm.h 
l_cksum_ok.o:	../h/lm_comm.h 
l_conn_msg.o:	../h/lm_comm.h 
l_connect.o:	../h/lm_comm.h ../h/lmselect.h ../h/lgetattr.h 
l_encode.o:	../h/lm_comm.h 
l_finder.o:	../h/lmselect.h 
l_get_lfile.o:	../h/lm_comm.h 
l_getattr.o:	../h/lgetattr.h 
l_handshake.o:	../h/lm_comm.h 
l_heart.o:	../h/lm_comm.h
l_host.o:	../h/lgetattr.h 
l_init_file.o:	../h/l_openf.h ../h/lgetattr.h 
l_lfgets.o:	../h/l_openf.h 
l_master_lis.o:	../h/l_openf.h  
l_modify.o:	../h/lm_comm.h 
l_msg_cksum.o:	../h/lm_comm.h  ../h/lcommdbg.h
l_open_file.o:	../h/l_openf.h 
l_pack.o:	../h/l_pack.h
l_parse_attr.o:	../h/lgetattr.h
l_rdec.o:	../h/l_pack.h
l_wdec.o:	../h/l_pack.h
l_parse_conf.o:	../h/lgetattr.h
l_pdaem.o:	../machind/lsmaster.h 
l_rcvmsg.o:	../h/lm_comm.h ../h/lmselect.h 
l_replog.o:  	../h/lgetattr.h   ../h/lm_comm.h
l_sernum.o:	../h/lm_comm.h
l_serv_msg.o:	../h/lm_comm.h ../h/lgetattr.h
l_sndmsg.o:	../h/lm_comm.h 
l_timers.o:  ../h/lgetattr.h  
lm_ckin.o:	../h/lm_comm.h 
lm_ckout.o:	../h/lm_comm.h  ../h/lgetattr.h l_strkey.c l_crypt.c \
				../h/l_strkey.h ../h/l_rand.h
lm_ck_feats.o:  ../h/lgetattr.h 
lm_config.o:    ../h/l_openf.h  ../h/lm_comm.h
lm_daemon.o:	../h/l_openf.h 
lm_disconn.o:	
lm_display.o: ../h/lgetattr.h 
lm_feat_list.o:	../h/l_openf.h ../h/lgetattr.h 
lm_fset.o:	../h/l_openf.h  l_strkey.c
lm_get_attr.o:	../machind/lm_attr.h ../h/lgetattr.h 
lm_getid_typ.o:	../h/lm_comm.h  ../h/lgetattr.h
lm_hostname.o:	../h/lgetattr.h 
lm_get_feats.o:	../h/l_openf.h 
lm_get_redir.o:	../h/l_openf.h 
lm_hosttype.o:	../machind/lmhtype.h ../h/lgetattr.h 
lm_init.o:	../machind/lm_attr.h ../machind/lmerrors.h  ../h/lm_comm.h ../h/lgetattr.h
lm_lsapi.o:	../machind/lm_lsapi.h 
lm_log.o:	../h/lm_comm.h 
lm_pconf.o:	../h/l_pack.h
lm_perror.o:	../machind/lmerrors.h  ../machind/lm_lerr.h ../machind/lcontext.h
lm_remove.o:	../h/lm_comm.h 
lm_set_attr.o:	../machind/lm_attr.h  ../h/lgetattr.h
lm_shutdown.o:	../h/lm_comm.h ../h/l_openf.h 
lm_slwg.o:	../machind/lm_attr.h ../h/lm_slwg.h 
lm_userlist.o:	../h/lm_comm.h  ../h/lgetattr.h
lm_username.o:	../h/lgetattr.h 
lm_vsend.o:	../h/lm_comm.h 
lm_get_dlist.c:	../h/lsmaster.h ../h/l_openf.h
lm_crypt.o:	l_crypt.c l_strkey.c ../h/l_strkey.h ../h/l_rand.h
lmpolicy.o:	../machind/lmpolicy.h
l_borrow.o:	../h/l_borrow.h ../h/l_rand.h
lm_ckout.o:	../h/l_borrow.h
l_rerd.o:	../h/lm_comm.h
flex_file.o:	../h/flex_file.h
flex_utils.o:	../h/flex_utils.h

$(PUBKEYOBJS):	../h/l_pubkey.h

