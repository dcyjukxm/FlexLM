XTRACFLAG = `gplatargs -g` -DLM_INTERNAL

XTRALINTFLAG = `gplatargs`

INCFLAGS = -I../h -I../server -I../machind -I../src

LINTLIBFLAGS = -a -b -h -u $(XTRALINTFLAG)

LINTFLAGS = -a -b -h $(XTRALINTFLAG)

DEBUGFLAG = -g

CFLAGS = $(INCFLAGS) $(DEBUGFLAG) $(XTRACFLAG)

OBJS =	\
	lg_stubs.o \
	lg_vkeys.o \
	ls_app_init.o \
	ls_app_main.o \
	ls_args.o \
	ls_attr.o \
	ls_borrow.o \
	ls_checkout.o \
	ls_checkroot.o \
	ls_ck_client.o \
	ls_ck_feats.o \
	ls_ckin.o \
	ls_daemon.o \
	ls_docmd.o \
	ls_feature.o \
	ls_get_info.o \
	ls_get_opts.o \
	ls_host.o \
	ls_init_feat.o \
	ls_i_fhostid.o \
	ls_list.o \
	ls_lock.o \
	ls_main.o \
	ls_max.o \
	ls_process.o \
	ls_reread.o \
	ls_shakehand.o \
	ls_since.o \
	ls_ubase.o \
	ls_user_opts.o \
	ls_vd_info.o \
	lslookup.o \
	lsprfeat.o 

SRCS =	\
	lg_stubs.c \
	ls_app_init.c \
	ls_app_main.c \
	ls_args.c \
	ls_attr.c \
	ls_checkout.c \
	ls_checkroot.c \
	ls_ck_client.c \
	ls_ck_feats.c \
	ls_ckin.c \
	ls_daemon.c \
	ls_docmd.c \
	ls_feature.c \
	ls_get_info.c \
	ls_get_opts.c \
	ls_host.c \
	ls_init_feat.c \
	ls_i_fhostid.c \
	ls_lock.c \
	ls_list.c \
	ls_main.c \
	ls_max.c \
	ls_process.c \
	ls_reread.c \
	ls_shakehand.c \
	ls_since.c \
	ls_ubase.c \
	ls_user_opts.c \
	ls_vd_info.c \
	lslookup.c \
	lsprfeat.c 

FULLSRCS = $(SRCS) ls_stubs.c

FULLOBJS = $(OBJS) ls_stubs.o


LIBRARY	= liblmgr_as.a

LINTLIB = lmgr_as

all:
	$(MAKE) $(LIBRARY) CFLAGS="$(CFLAGS)"

$(LIBRARY):	$(OBJS)
	@echo -n "Creating library ..."
	@$(AR) cr $(LIBRARY) $(OBJS)
	@-test -f /bin/ranlib && ranlib $(LIBRARY) || \
	test -f /usr/bin/ranlib && ranlib $(LIBRARY) || true

lint:	$(SRCS)
	lint $(INCFLAGS) $(LINTFLAGS) $(SRCS)

lintlib:	$(SRCS)
	lint $(INCFLAGS) $(LINTLIBFLAGS) -C$(LINTLIB) $(SRCS)

clean:
	@echo cleaning `pwd`...
	@rm -f $(OBJS) $(LIBRARY)

#$(FULLSRCS):;	sccs get $@

$(FULLOBJS):	../machind/lmclient.h ../machind/lsserver.h ../h/lmachdep.h \
		../h/l_prot.h ls_aprot.h ../server/ls_sprot.h ../h/l_privat.h \
		../machind/lsfeatur.h

ls_adaem.h:	ls_aprot.h 
ls_app_init.o:	../server/ls_comm.h ../server/lssignal.h \
		../server/ls_glob.h ls_adaem.h  \
		../h/lm_comm.h ../server/ls_log.h 
ls_attr.o:	../machind/ls_attr.h 
ls_checkout.o:	ls_adaem.h ../server/ls_log.h 
ls_ck_feats.o:	../server/ls_log.h 
ls_daemon.o:	../server/ls_glob.h ls_adaem.h 
ls_docmd.o:	../h/lm_comm.h ../server/ls_comm.h ../h/lmselect.h \
		../server/ls_glob.h ../server/ls_log.h 
ls_ckin.o:	../server/ls_glob.h ../server/ls_log.h
ls_feature.o:	../server/ls_comm.h ../server/ls_glob.h \
		../h/lm_comm.h ../server/ls_log.h 
lslookup.o:	../server/ls_glob.h 
ls_get_info.o:	../server/ls_sprot.h ../h/lm_comm.h \
		ls_aprot.h
ls_get_opts.o:	../server/ls_glob.h ../server/ls_log.h 
ls_init_feat.o:	../server/ls_glob.h ls_adaem.h ../src/l_crypt.c \
		../h/l_strkey.h ../src/l_strkey.c
ls_list.o:	../h/lm_comm.h ../server/ls_comm.h \
		../server/ls_glob.h 
ls_lock.o:	../server/ls_glob.h  ls_lock.h
ls_main.o:	../h/lmselect.h ls_adaem.h ../server/ls_glob.h \
		ls_adaem.h 
ls_max.o:	../machind/lsserver.h ls_aprot.h
ls_process.o:	../h/lm_comm.h ../server/ls_comm.h ../h/lmselect.h \
		../server/ls_glob.h lg_genvd.h
ls_reread.o:	../h/lgetattr.h ../server/lssignal.h \
		../server/ls_glob.h ls_adaem.h 
ls_sendup.o:	../server/ls_comm.h 
ls_scmd.o:	../h/lm_comm.h ../server/ls_comm.h ../server/lssignal.h \
		ls_adaem.h ../server/ls_log.h 
ls_shakehand.o:	../h/lm_comm.h 
ls_since.o:	../server/ls_comm.h ../server/ls_glob.h 
ls_spawn.o:	../h/lm_comm.h ../server/lssignal.h ../server/ls_glob.h \
		ls_adaem.h 
ls_vd_info.o:	../server/ls_sprot.h ../h/lm_comm.h \
		ls_aprot.h
ls_borrow.o:	../h/l_borrow.h ../h/l_rand.h
