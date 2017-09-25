XTRACFLAG = `gplatargs` -DLM_INTERNAL
METER_INCLUDE = `gplatargs -imeter`

SHAREDOPTS =  -assert pure-text 

XTRALINTFLAG = `gplatargs -lint` -DLM_INTERNAL

INCFLAGS = -I../h  $(METER_DIR) 

LINTLIBFLAGS = $(XTRALINTFLAG)

LINTFLAGS = `gplatargs -lint` $(XTRALINTFLAG)

LINTOUTFLAG = `gplatargs -lint`

DEBUGFLAG = -g

#PCFLAGS is used to set CFLAGS in the depend line (mkmf)
PCFLAGS = $(INCFLAGS) $(DEBUGFLAG) $(XTRACFLAG)
CFLAGS = $(INCFLAGS) $(DEBUGFLAG) $(XTRACFLAG)

SRCS	      = l_allfeat.c \
		l_any_set.c \
		l_apollo_id.c \
		l_asc_hostid.c \
		l_baddate.c \
		l_basic_conn.c \
		l_check.c \
		l_cksum.c \
		l_cksum_ok.c \
		l_config.c \
		l_conn_msg.c \
		l_connect.c \
		l_crypt.c \
		l_date.c \
		l_encode.c \
		l_error.c \
		l_ether_id.c \
		l_fdata.c \
		l_file_stub.c \
		l_finder.c \
		l_get_dlist.c \
		l_get_id.c \
		l_get_lfile.c \
		l_getattr.c \
		l_getenv.c \
		l_handshake.c \
		l_heart.c \
		l_host.c \
		l_inet.c \
		l_init_file.c \
		l_instlc.c \
		l_ipaddr.c \
		l_key.c \
		l_keyword.c \
		l_lfgets.c \
		l_lookup.c \
		l_malloc.c \
		l_master.c \
		l_master_lis.c \
		l_mem.c \
		l_modify.c \
		l_msg_cksum.c \
		l_now.c \
		l_open_file.c \
		l_pack.c \
		l_package.c \
		l_parse_attr.c \
		l_lmgrdr.c \
		l_platfm.c \
		l_pdaem.c \
		l_pserv.c \
		l_rcvmsg.c \
		l_rdec.c \
		l_read_lfile.c \
		l_redundant.c \
		l_replog.c \
		l_rerd.c \
		l_select.c \
		l_select_one.c \
		l_sernum.c \
		l_serv_msg.c \
		l_sndmsg.c \
		l_str_crypt.c \
		l_strkey.c \
		l_supersede.c \
		l_text.c \
		l_timers.c \
		l_tli.c \
		l_uppercase.c \
		l_validdate.c \
		l_wdec.c \
		lis_getunit.c \
		lm_baddate.c \
		lm_ckin.c \
		lm_ckout.c \
		lm_chk_conf.c \
		lm_ck_feats.c \
		lm_config.c \
		lm_cp_hostid.c \
		lm_crypt.c \
		lm_crstr.c \
		lm_daemon.c \
		lm_disconn.c \
		lm_display.c \
		lm_dump.c \
		lm_feat_list.c \
		lm_fset.c \
		lm_free_job.c \
		lm_get_attr.c \
		lm_get_feats.c \
		lm_get_redir.c \
		lm_gethostid.c \
		lm_getid_typ.c \
		lm_heart.c \
		lmhostid.c \
		lm_hostname.c \
		lm_hosttype.c \
		lm_init.c \
		lm_isadmin.c \
		lm_lic_where.c \
		lm_log.c \
		lm_lsapi.c \
		lm_njob.c \
		lm_pconf.c \
		lm_perror.c \
		lm_remove.c \
		lm_redir_ver.c \
		lm_set_attr.c \
		lm_shutdown.c \
		lm_slwg.c \
		lm_startup.c \
		lm_userlist.c \
		lm_username.c \
		lm_vb.c \
		lm_vsend.c \
		lm_xstrcmp.c \
		lmpolicy.c \
		lu_getdtab.c \
		lu_gethost.c \
		luhostid.c \
		lu_setitimer.c \
		lu_setline.c \
		lu_sleep.c \
		lu_wait3.c \
		lgclient.c 




OBJECTS 	= l_allfeat.o \
		l_any_set.o \
		l_apollo_id.o \
		l_asc_hostid.o \
		l_baddate.o \
		l_basic_conn.o \
		l_check.o \
		l_cksum.o \
		l_cksum_ok.o \
		l_config.o \
		l_conn_msg.o \
		l_connect.o \
		l_date.o \
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
		l_redundant.o \
		l_replog.o \
		l_rerd.o \
		l_select.o \
		l_select_one.o \
		l_sernum.o \
		l_serv_msg.o \
		l_sndmsg.o \
		l_str_crypt.o \
		l_supersede.o \
		l_text.o \
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
		lm_remove.o \
		lm_set_attr.o \
		lm_shutdown.o \
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
		luhostid.o 

PLATOBJS      = l_platfm.o

METEROBJS =	lmborrow.o

ASSOBJS	= l_sgi.o

MINIOBJS      = $(BASEOBJS) \
		$(SHAREDOBJS) \
		$(PLATOBJS) \
		lm_ministubs.o

OBJS	      = $(OBJECTS) \
		$(PLATOBJS)  \
		$(ASSOBJS)  \
		$(METEROBJS)

SPARCOBJS     = $(OBJS) 


LIBRARY	= liblmgr.a
MINILIBRARY = libmini_lmgr.a
LINTLIB = lmgr
VER = 1.0
SHAREDLIB = liblis.so
SHAREDLIBRARY = $(SHAREDLIB).$(VER)
UNSHAREDLIBRARY = liblis.a

all:	
	$(MAKE) _all CFLAGS="$(CFLAGS)"


_all:	$(LIBRARY) l_tli.o

test_release: 
	mv liblmgr.a liblmgr.asav
	$(MAKE) clean
	$(MAKE) _all DEBUGFLAG= XTRACFLAG="$(XTRACFLAG) -DRELEASE_VERSION"
	mv liblmgr.a liblmgr_rel.a
	$(MAKE) clean
	mv liblmgr.asav liblmgr.a

$(LIBRARY):	$(OBJS)
	@echo Creating $(LIBRARY) ...
	@ar cr $(LIBRARY) $(OBJS)
	ranlib $(LIBRARY)

$(MINILIBRARY):	$(MINIOBJS)
	@echo Creating $(MINILIBRARY) ...
	@ar cr $(MINILIBRARY) $(MINIOBJS)
	@ranlib $(MINILIBRARY)
	@echo " done"

$(UNSHAREDLIBRARY):	$(LIBRARY)
	@echo "Creating $(SHAREDLIBRARY)"
	@rm -f $(LIBRARY)
	@ar cr $(LIBRARY) $(SPARCOBJS)
	@ranlib $(LIBRARY)
	@ld -o $(SHAREDLIBRARY) $(SHAREDOPTS) $(SHAREDOBJS)
	@ar cr $(UNSHAREDLIBRARY) $(SHAREDOBJS)
	@ranlib $(UNSHAREDLIBRARY)

clean:;	rm -f $(OBJS) l_tli.o $(MINIOBJS) $(LIBRARY) $(SHAREDLIBRARY) \
	llib-$(LINTLIB).ln $(MINILIBRARY) $(UNSHAREDLIBRARY)

lintlib: $(SRCS)
	lint $(LINTFLAGS) $(INCFLAGS) $(LINTOUTFLAG)$(LINTLIB) $(SRCS)

lint: $(SRCS)
	lint $(LINTFLAGS) $(INCFLAGS) $(SRCS)

#$(SRCS):;	sccs get $@

$(PLATOBJS):	
		$(CC) -c $(CFLAGS) -DGPLATFORM=\"`gplatform`\" $*.c

lmborrow.o:	lmborrow.c
		$(CC) -c $(CFLAGS) $(METER_INCLUDE) lmborrow.c
		case `gplatform` in \
			sun4_u5|i86_l*|alpha_u*) \
			ld -r lmborrow.o `gplatargs -lmeter` -o xxx.o ; \
			mv xxx.o lmborrow.o ;;\
		esac 

$(ASSOBJS):	
		@echo "static int unused;" > l_sgi.c; $(CC) $(CFLAGS) -c l_sgi.c
		@if [ `gplatform` = sgi_u5 -o `gplatform` = sgir8_u6 -o `gplatform` = sgi32_u5 -o `gplatform` = sgi64_u6 -o `gplatform` = sgi32_u6 ] ;\
		then \
			echo "as -c -DSGI $(CFLAGS) $*.s -o $*.o" ;\
			rm $*.o ;\
			as -c -DSGI $(CFLAGS) $*.s -o $*.o ;\
		else \
			echo "making dummy l_sgi.o" ;\
		fi

$(OBJS):	../h/lmachdep.h ../h/lmclient.h ../h/l_prot.h ../h/l_privat.h
l_tli.o:	../h/lmachdep.h ../h/lmclient.h

depend:;	mkmf -f src.makefile CFLAGS="$(PCFLAGS)"

###
l_allfeat.o:	../h/lgetattr.h
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
l_pdaem.o:	../h/lsmaster.h 
l_rcvmsg.o:	../h/lm_comm.h ../h/lmselect.h 
l_redundant.o:  ../h/lgetattr.h  
l_replog.o:  	../h/lgetattr.h   ../h/lm_comm.h
l_sernum.o:	../h/lm_comm.h
l_serv_msg.o:	../h/lm_comm.h ../h/lgetattr.h
l_sndmsg.o:	../h/lm_comm.h 
l_timers.o:  ../h/lgetattr.h  
lm_ckin.o:	../h/lm_comm.h 
lm_ckout.o:	../h/lm_comm.h  ../h/lgetattr.h l_strkey.c l_crypt.c \
				../h/l_strkey.h
lm_ck_feats.o:  ../h/lgetattr.h 
lm_config.o:    ../h/l_openf.h  ../h/lm_comm.h
lm_daemon.o:	../h/l_openf.h 
lm_disconn.o:	
lm_display.o: ../h/lgetattr.h 
lm_feat_list.o:	../h/l_openf.h ../h/lgetattr.h 
lm_fset.o:	../h/l_openf.h 
lm_get_attr.o:	../h/lm_attr.h ../h/lgetattr.h 
lm_getid_typ.o:	../h/lm_comm.h  ../h/lgetattr.h
lm_hostname.o:	../h/lgetattr.h 
lm_get_feats.o:	../h/l_openf.h 
lm_get_redir.o:	../h/l_openf.h 
lm_hosttype.o:	../h/lmhtype.h ../h/lgetattr.h 
lm_init.o:	../h/lm_attr.h ../h/lmerrors.h  ../h/lm_comm.h ../h/lgetattr.h
lm_lsapi.o:	../h/lm_lsapi.h 
lm_log.o:	../h/lm_comm.h 
lm_pconf.o:	../h/l_pack.h
lm_perror.o:	../h/lmerrors.h  ../h/lm_lerr.h ../h/lcontext.h
lm_remove.o:	../h/lm_comm.h 
lm_set_attr.o:	../h/lm_attr.h  ../h/lgetattr.h
lm_shutdown.o:	../h/lm_comm.h ../h/l_openf.h 
lm_slwg.o:	../h/lm_attr.h ../h/lm_slwg.h 
lm_userlist.o:	../h/lm_comm.h  ../h/lgetattr.h
lm_username.o:	../h/lgetattr.h 
lm_vsend.o:	../h/lm_comm.h 
lm_get_dlist.c:	../h/lsmaster.h ../h/l_openf.h
lm_crypt.o:	l_crypt.c l_strkey.c ../h/l_strkey.h
lmpolicy.o:	../h/lmpolicy.h

