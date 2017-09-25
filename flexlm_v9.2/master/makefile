XTRACFLAG = `gplatargs -O -g` -DLM_INTERNAL

XTRALIB = `gplatargs -l`

XTRALINTFLAG = `gplatargs`

INCFLAGS = -I../h -I../machind -I../server

LINTFLAGS = -a -b -h $(XTRALINTFLAG)

MAIN = ls_lmgrd.o

DEBUGFLAG = -g 

CFLAGS = $(INCFLAGS) $(DEBUGFLAG) $(XTRACFLAG)

THREADLIB = `gplatargs -lthreads`
LDFLAGS = $(XTRALIB) $(THREADLIB)

LD = $(CC)

OBJS = \
	ls_chld_died.o \
	ls_daemons.o \
	ls_finder.o \
	ls_kill_chld.o \
	ls_lmgrd.o \
	ls_m_init.o \
	ls_m_main.o \
	ls_m_process.o \
	ls_startup.o \
	ls_statfile.o \
	ls_timestamp.o

SRCS = 	\
	ls_chld_died.c \
	ls_daemons.c \
	ls_finder.c \
	ls_kill_chld.c \
	ls_lmgrd.c \
	ls_m_init.c \
	ls_m_main.c \
	ls_m_process.c \
	ls_startup.c \
	ls_statfile.c \
	ls_timestamp.c

EXTLIB	= ../server/liblmgr_s.a ../src/liblmgr.a 
EXTLINTLIB = ../src/llib-llmgr.ln ../server/llib-llmgr_s.ln

LMGRDLIB = liblmgrd.a
LIBRARY = liblmgrd.a

all:	
	$(MAKE) _all CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" XTRALIB="$(XTRALIB)"

_all:	$(LMGRDLIB) lmgrd


$(LMGRDLIB):	$(OBJS)
	$(AR) cr liblmgrd.a $(OBJS)
	-test -f /bin/ranlib && ranlib $(LIBRARY) || \
	test -f /usr/bin/ranlib && ranlib $(LIBRARY) || true

lmgrd:	$(LMGRDLIB) $(EXTLIB) 
	$(CC) -o lmgrd $(LMGRDLIB) $(EXTLIB) $(XTRALIB) $(LDFLAGS)

clean:
	@echo cleaning `pwd` ...
	@rm -f $(OBJS) lmgrd $(LMGRDLIB)

lint:;	lint $(LINTFLAGS) $(INCFLAGS) $(SRCS) $(EXTLINTLIB)

#$(SRCS):;	sccs get $@

$(OBJS):	../machind/lmclient.h ../h/lmachdep.h ../h/l_prot.h \
		../server/ls_sprot.h l_m_prot.h ../h/l_privat.h
ls_chld_died.o: ../machind/lsserver.h ../server/ls_glob.h ../machind/lsmaster.h \
		ls_chld_died.c
ls_daemons.o:	../h/lm_comm.h
ls_finder.o:	../machind/lsserver.h
ls_kill_chld.o:	../machind/lsserver.h ../machind/lsmaster.h ls_kill_chld.c
ls_m_main.o:	../machind/lsserver.h ../server/ls_glob.h \
			../machind/lsmaster.h ../h/lmselect.h ls_m_main.c
ls_lmgrd.o:	../machind/lsserver.h  ../machind/lsmaster.h ls_lmgrd.c
ls_m_init.o:	../machind/lsserver.h ../server/ls_glob.h ../h/lmselect.h \
		../machind/lm_attr.h ../h/lgetattr.h ../server/lssignal.h \
		ls_m_init.c
ls_m_init.o:	../h/lm_comm.h
ls_m_process.o:	../machind/lsserver.h ../server/ls_glob.h ../machind/lsmaster.h \
			../server/lssignal.h ../server/ls_comm.h \
			../h/lmselect.h ../h/lm_comm.h ls_m_process.c
ls_daemons.o:	../machind/lsserver.h ../machind/lsmaster.h ls_daemons.c
ls_startup.o:	../machind/lsserver.h ../machind/lsmaster.h ls_startup.c
ls_timestamp.o:	../machind/lsserver.h ls_timestamp.c
ls_statfile.o:  ../machind/lsserver.h ../h/lmachdep.h ../machind/lmclient.h \
		ls_statfile.c
