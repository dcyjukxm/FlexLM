XTRACFLAG = `gplatargs -O -g` -DLM_INTERNAL

XTRALINTFLAG = `gplatargs`

INCFLAGS = -I../h -I../machind -I.

LINTLIBFLAGS = -a -b -h -u $(XTRALINTFLAG)

LINTFLAGS = -a -b -h $(XTRALINTFLAG)

DEBUGFLAG = -g

CFLAGS = $(INCFLAGS) $(DEBUGFLAG) $(XTRACFLAG)

V4REPLOG_OBJS = \
	ls_badserv.o \
	ls_c_comm.o \
	ls_c_init.o \
	ls_client_db.o \
	ls_data.o \
	ls_down.o \
	ls_findbad.o \
	ls_flexdir.o \
	ls_gettime.o \
	ls_handle.o \
	ls_hookup.o \
	ls_i_master.o \
	ls_log.o \
	ls_logtime.o \
	ls_log_open.o \
	ls_lost.o \
	ls_malloc.o \
	ls_mast_rdy.o \
	ls_new_log.o \
	ls_on_host.o \
	ls_pause.o \
	ls_pick_mast.o \
	ls_quorum.o \
	ls_readready.o \
	ls_s_funcs.o \
	ls_sconnect.o \
	ls_serv_conn.o \
	ls_serv_time.o \
	ls_signals.o \
	ls_socket.o \
	ls_symtab.o \
	ls_udp_read.o \
	ls_wakeup.o 

OBJS = $(V4REPLOG_OBJS) \
	ls_dict.o \
	lsfilter.o \
	lsreplog.o 

SRCS = 			\
	ls_badserv.c \
	ls_c_comm.c \
	ls_c_init.c \
	ls_client_db.c \
	ls_data.c \
	ls_down.c \
	ls_findbad.c \
	ls_flexdir.c \
	ls_gettime.c \
	ls_handle.c \
	ls_hookup.c \
	ls_i_master.c \
	ls_lost.c \
	ls_malloc.c \
	ls_mast_rdy.c \
	ls_new_log.c \
	ls_on_host.c \
	ls_pause.c \
	ls_pick_mast.c \
	ls_quorum.c \
	ls_sconnect.c \
	ls_readready.c \
	ls_s_funcs.c \
	ls_serv_conn.c \
	ls_serv_time.c \
	ls_signals.c \
	ls_socket.c \
	ls_symtab.c \
	ls_udp_read.c \
	ls_wakeup.c 

LIBRARY	= liblmgr_s.a
LINTLIB = lmgr_s

all:
	$(MAKE) $(LIBRARY) CFLAGS="$(CFLAGS)"

v4replog:
	$(MAKE) $(LIBRARY) CFLAGS="$(CFLAGS)" OBJS="$(V4REPLOG_OBJS)"


$(LIBRARY):	$(OBJS)
	@echo -n "Creating library ..."
	@$(AR) cr $(LIBRARY) $(OBJS)
	-test -f /bin/ranlib && ranlib $(LIBRARY) || \
	test -f /usr/bin/ranlib && ranlib $(LIBRARY) || true

lint:	$(SRCS)
	lint $(INCFLAGS) $(LINTFLAGS) $(SRCS)

lintlib:	$(SRCS)
	lint $(INCFLAGS) $(LINTFLAGS) -C$(LINTLIB) $(SRCS)

clean:	
	@echo cleaning `pwd` ...
	@rm -f $(OBJS) $(LIBRARY) $(PROGRAM) 

#$(SRCS):;	sccs get $@

$(OBJS):	../h/lmachdep.h ../machind/lmclient.h ../machind/lsserver.h ../h/l_prot.h \
		../h/l_privat.h ls_sprot.h

ls_c_comm.o:	../h/lm_comm.h
ls_comm_debug.o:	ls_comm.h ../h/lm_comm.h
ls_findbad.o:	ls_glob.h ../h/lmselect.h
ls_hookup.o:	ls_glob.h
ls_hookup.o:	../h/lmselect.h
ls_hookup.o:	../h/lm_comm.h
ls_i_master.o:	../h/lm_comm.h
ls_lost.o:	lssignal.h ls_glob.h
ls_mast_rdy.o:	../h/lm_comm.h
ls_new_log.o:	../h/lm_comm.h
ls_quorum.o:	ls_glob.h
ls_readready.o:	ls_glob.h
ls_readready.o:	../h/lmselect.h
ls_s_funcs.o:	ls_glob.h
ls_sconnect.o:	ls_glob.h
ls_sconnect.o:	../h/lm_comm.h
ls_serv_conn.o: ../h/lm_comm.h ls_comm.h ls_glob.h
ls_signals.o:	lssignal.h ls_glob.h
ls_serv_time.o:	../h/lmselect.h
ls_symtab.o:	lssymtab.h
ls_udp_read.o:	../machind/lsserver.h
