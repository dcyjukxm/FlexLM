UTILSDIR = ../utils

MASTERDIR = ../master

XTRACFLAG = `gplatargs` -DLM_INTERNAL

THREADLIB = `gplatargs -lthreads`
XTRALIB = `gplatargs -l` 

INCFLAGS = -I../h -I../machind -I../server 

LINTFLAGS = -a -b -h

DAEMON_MAIN = ../app/ls_app_main.o
LMGRD_MAIN = ../master/ls_lmgrd.o
SMALL=



DEBUGFLAG = -g

CFLAGS = $(INCFLAGS) $(DEBUGFLAG) $(XTRACFLAG)

SRCS =		basic_tests.c \
		bigfile.c \
		cryptest.c \
		demof.c \
		demof2.c \
		demo.c \
		fakehostname.c \
		lock_tests.c \
		lsapi_tests.c \
		pathd.c \
		red_s_tests.c \
		reptest.c \
		no_s_tests.c \
		servtest.c  \
		st_bugs.c \
		st_license.c \
		st_vers.c \
		st_8bit.c \
		tsclient.c \
		tscrypt.c

OBJS =		basic_tests.o \
		cryptest.o \
		demof.o \
		demof2.o \
		lmcode.o \
		lmcodef.o \
		lmcodef2.o \
		lmcodepathd.o \
		lm_npathd.o \
		demo.o \
		fake_appinit.o \
		fakehostname.o \
		lock_tests.o \
		lsapi_tests.o \
		pathd.o \
		no_s_tests.o \
		bigfile.o \
		servtest.o \
		st_bugs.o \
		st_license.o \
		st_vers.o \
		st_8bit.o \
		red_s_tests.o \
		reptest.o \
		tsclient.o \
		tsclient2.o \
		tscrypt.o \
		tscrypt2.o \
		ut_flex.o \
		utiltest.o 



LIBOBJS =	ts_config.o \
		tsdsapp.o \
		tsdsgen.o \
		ts_dump.o \
		ts_environ.o \
		ts_hostid.o \
		ts_lic_file.o \
		ts_list.o \
		ts_reread.o \
		ts_serv_supp.o \
		ts_utils.o \
		ts_vmsg.o

DAEMONS = demo \
		demof \
		demof2 \
		demo \
		pathd \
		lmgrd

LMGRDS = lmgrd_r  demo_r

CLIENTS = basic_tests \
		bigfile \
		crotest \
		cryptest \
		lock_tests \
		lsapi_tests \
		no_s_tests \
		servtest \
		red_s_tests \
		reptest \
		tsclient \
		tscrypt \
		utiltest  
# timers_test 

EXECS = $(DAEMONS) $(CLIENTS) 

LINTLIB = ../src/llib-llmgr.ln
LCLIENTLIBS = ../src/liblmgr.a ../$(PLATFORM)/libcrvs.a ../$(PLATFORM)/libsb.a 
CLIENTONLY = lm_n.o
CLIENTLIBS = $(LCLIENTLIBS) $(XTRALIB) $(THREADLIB)
LTESTLIBS = ts.a $(LCLIENTLIBS) 
TESTLIBS = ts.a $(CLIENTLIBS) 
DAEMONLIBS = ../app/liblmgr_as.a ../server/liblmgr_s.a 
LLIBS = $(LCLIENTLIBS)
LIBS	= $(LLIBS) $(XTRALIB)   $(THREADLIB)

LD = $(CC)

LINTLIB = ../src/llib-llmgr.ln

all:	$(EXECS) $(LLIBS) $(LMGRDS)

nonstrip:	$(EXECS) $(LLIBS) $(LMGRDS)

daemons:	$(DAEMONS)

clients:	$(CLIENTS)

$(EXECS):	$(LLIBS)

$(LMGRDS):	$(LLIBS)

strip:;	-strip $(EXECS)

ts.a:	$(LIBOBJS) 
	$(AR) cr ts.a $(LIBOBJS)
	-test -f /bin/ranlib && ranlib ts.a || \
	test -f /usr/bin/ranlib && ranlib ts.a || true


basic_tests:	testsuite.h basic_tests.o lm_nold.o $(LTESTLIBS)
	$(LD) -o basic_tests basic_tests.o lm_nold.o -I../h $(TESTLIBS)

cryptapp.o:	$(LTESTLIBS)
	$(CC) -c $(CFLAGS) cryptapp.c

cryptkey.o:	$(LTESTLIBS)
	$(CC) -c $(CFLAGS) cryptkey.c

cryptest:	cryptest.o $(LTESTLIBS) ../utils/lmrand1 \
	cryptapp.o cryptkey.o ts.a tsappfil.o tskeyfil.o
	$(LD) -o cryptest cryptest.o cryptapp.o cryptkey.o tsappfil.o tskeyfil.o $(TESTLIBS) 

crotest:	crotest.o ../src/liblmgr.a $(CLIENTONLY)
	$(LD) -o crotest crotest.o $(CLIENTONLY) $(LCLIENTLIBS) $(XTRALIB) $(THREADLIB)

crotest.o:	crotest.c

demof2:	$(DAEMONLIBS) $(LLIBS) demof2.o lm_n_f2.o
	$(LD) -o demof2 demof2.o lm_n_f2.o $(DAEMONLIBS) $(LIBS)
demof:	$(DAEMONLIBS) $(LLIBS) demof.o
	$(LD) -o demof demof.o lm_nf.o $(DAEMONLIBS) $(LIBS)
	lmstrip demof

demo_add:	$(LLIBS) demo_add.o ts.a
	$(LD) -o demo_add demo_add.o $(LIBS)  ts.a
	strip demo_add

demo:	$(DAEMONLIBS) $(LLIBS) ts.a demo.o lm_n.o
	$(LD) -o demo demo.o lm_n.o $(DAEMONLIBS) $(LIBS) ts.a

jobtests:	jobtests.o $(LTESTLIBS)
		$(LD) $(LDFLAGS) -o jobtests jobtests.o $(TESTLIBS)

lmgrd:	../master/lmgrd
		rm -f lmgrd
		ln -s ../master/lmgrd lmgrd

lmgrd_r:	fakehostname.o ../master/lmgrd
		$(LD) $(LDFLAGS) fakehostname.o \
			-o lmgrd_r $(MASTERDIR)/liblmgrd.a\
			../server/liblmgr_s.a ../src/liblmgr.a $(XTRALIB) $(THREADLIB)
demo_r:	$(DAEMONLIBS) $(LLIBS) demo.o fakehostname.o fake_appinit.o ts.a lm_nold.o
	$(LD) -o demo_r demo.o fakehostname.o  lm_nold.o \
				fake_appinit.o $(DAEMONLIBS) $(LIBS) ts.a

lock_tests:	lock_tests.o $(DAEMONLIBS) $(LLIBS) 
	$(CC) -o lock_tests lock_tests.o ../app/ls_lock.o \
		../app/ls_checkroot.o ../src/liblmgr.a $(XTRALIB) $(THREADLIB)

lsapi_tests:	testsuite.h ../machind/lm_lsapi.h lsapi_tests.o $(LTESTLIBS)
	$(LD) -o lsapi_tests lsapi_tests.o lm_nold.o -I../h $(TESTLIBS)

no_s_tests:	testsuite.h no_s_tests.o lm_nold.o $(LTESTLIBS)
	$(LD) -o no_s_tests no_s_tests.o lm_nold.o -I../h $(TESTLIBS)

one_s_tests:	testsuite.h one_s_tests.o $(LTESTLIBS)
	$(LD) -o one_s_tests one_s_tests.o -I../h $(TESTLIBS)

bigfile:	bigfile.o lm_nold.o $(LTESTLIBS)
	$(LD) -o bigfile bigfile.o lm_nold.o -I../h $(TESTLIBS)

reptest:	reptest.o lm_nold.o $(LTESTLIBS)
	$(LD) -o reptest reptest.o lm_nold.o -I../h $(TESTLIBS)



tskeyfil.o:	tsappfil.o

tsappfil.o:	$(LCLIENTLIBS) ../utils/lmrand1
	rm -f lmappfil.c lmkeyfil.c tsappfil.c tskeyfil.c
	../utils/lmrand1 -filter_gen  0x7135715a 0x9b26c91b 0xb0097e5f -q
	mv lmappfil.c tsappfil.c
	mv lmkeyfil.c tskeyfil.c
	$(CC) -g -c $(CFLAGS) tsappfil.c 
	$(CC) -g -c $(CFLAGS) tskeyfil.c 
	
lmph2app.c:	$(LCLIENTLIBS) ../utils/lmrand1
	rm -rf lmph2???.?
	../utils/lmrand1 -phase2 0x12345678 0x11122233 0x8765abcd -q

lmph2gen.o:	lmph2app.c
	$(CC) -c $(CFLAGS) lmph2gen.c

lmph2app.o:	lmph2app.c
	$(CC) -c $(CFLAGS) lmph2gen.c
	$(CC) -c $(CFLAGS) lmph2app.c

servtest:	../utils/lmrand1 testsuite.h servtest.o st_bugs.o st_vers.o \
		st_8bit.o st_license.o  $(LCLIENTLIBS) tsappfil.o tskeyfil.o \
		$(CLIENTONLY) ts.a lmph2app.o lmph2gen.o servtest.o
	./lmnewgen -o lm_n.c $(SMALL)
	$(CC) -c $(CFLAGS) lm_n.c
	$(LD) -o servtest servtest.o st_bugs.o st_8bit.o st_vers.o  \
		tsappfil.o tskeyfil.o $(CLIENTONLY) lmph2app.o lmph2gen.o \
		st_license.o  -I../h $(TESTLIBS)

red_s_tests:	testsuite.h red_s_tests.o $(LTESTLIBS)
	$(LD) -o red_s_tests red_s_tests.o ../src/lm_nomt.o $(CLIENTONLY) -I../h  $(TESTLIBS) 

pathd:	$(DAEMONLIBS) $(LLIBS) pathd.o lm_npathd.o
	$(LD) -o pathd pathd.o lm_npathd.o $(DAEMONLIBS) $(LIBS)

utiltest:	utiltest.o ut_flex.o lm_nold.o $(LCLIENTLIBS)
	$(LD) $(LDFLAGS) -o utiltest utiltest.o lm_nold.o ut_flex.o $(CLIENTLIBS)
	lmstrip utiltest

utiltest.o:	utiltest.c
		$(CC) -c $(CFLAGS) -DFLEXLM utiltest.c

timers_test:	../src/l_timers.c ../h/l_timers.h ../h/lmachdep.h \
		../machind/lmclient.h ../src/l_select.c ../src/l_malloc.c \
			../src/l_error.c ../src/lm_perror.c  ../src/l_ctype.c
	$(LD) $(XTRACFLAG) -o timers_test -DDEBUG_TIMERS \
			-I../h -I../machind ../src/l_timers.c  ../src/l_mem.c \
			../src/lu_setitimer.c ../src/l_select.c \
			../src/lm_perror.c ../src/l_encode.c ../src/l_malloc.c \
			../src/l_error.c ../src/l_ctype.c $(XTRALIB) $(THREADLIB)

dant:	dant.o $(LTESTLIBS) $(CLIENTONLY)
	$(LD) $(LDFLAGS) -o dant dant.o $(CLIENTONLY) ts.a $(LCLIENTLIBS) $(XTRALIB) $(THREADLIB)

tsclient:	tsclient.o $(LTESTLIBS) $(CLIENTONLY)
	$(LD) $(LDFLAGS) -o tsclient tsclient.o $(CLIENTONLY) ts.a $(LCLIENTLIBS) $(XTRALIB) $(THREADLIB)

tscrypt:	tscrypt.o $(LTESTLIBS)  
	$(LD) $(LDFLAGS) -o tscrypt tscrypt.o $(TESTLIBS)

#$(SRCS):;	sccs get $@

tsclientold:	tsclient.o $(LTESTLIBS) tscryptold
	$(CC) -c $(CFLAGS) -I../h lm_new.c
	$(LD) $(LDFLAGS) -o tsclientold tsclient.o lm_new.o $(TESTLIBS)

tscryptold:	tscrypt.o $(LTESTLIBS)
	../utils/lmrand1 -i demo.c
	$(CC) -c $(CFLAGS) -I../h lmcode.c
	$(CC) ../utils/lmnewgen.o lmcode.o ../src/liblmgr.a -o lmnewgen $(XTRALIB) $(THREADLIB)
	./lmnewgen demo  $(SMALL)
	$(CC) -c $(CFLAGS) -I../h 
	$(LD) $(LDFLAGS) -o tscryptold tscrypt.o $(TESTLIBS)

tsclientnew:	tsclient.o $(LTESTLIBS) tscryptnew
	$(CC) -c $(CFLAGS) -I../h lm_new.c
	$(LD) $(LDFLAGS) -o tsclientnew tsclient.o lm_new.o $(TESTLIBS)

tscryptnew:	tscrypt.o $(LTESTLIBS)
	../utils/lmrand1 -i demo.c
	$(CC) -c $(CFLAGS) -I../h lmcode.c
	$(CC) ../utils/lmnewgen.o lmcode.o ../src/liblmgr.a -o lmnewgen $(XTRALIB) $(THREADLIB)
	./lmnewgen demo $(SMALL)
	$(CC) -c $(CFLAGS) -I../h 
	$(LD) $(LDFLAGS) -o tscryptnew tscrypt.o $(TESTLIBS)

#$(SRCS):;	sccs get $@

clean:
	@echo cleaning `pwd` ...
	@-mkdir savi__
	@mv install?.dat savi__
	@-rm -f $(OBJS) $(LIBOBJS) $(EXECS) ts.a *.out *log license.dat \
	rlicense* opts servtest.err _*  *.o *.lic lm_n*.* \
	lmcode*.* lmrand* tsappfil.c tskeyfil.c cryplicf.c malloc.tst \
	stkey*.* create*.dat lmnewgen* lsr*.c *.err lmgrd_r demo_r *.sav\
	test_flexlmrc rlog* opt* flexlmrc core lmph2*.? log? lm???key.h
	@mv savi__/* .
	@rmdir savi__

cleanexecs:;	rm -f  $(EXECS)  $(LMGRDS)

cleandaemons:;	rm -f  $(DAEMONS) 

cleanclients:;	rm -f  $(CLIENTS) 

#$(OBJS):	../machind/lmclient.h
$(LIBOBJS):	testsuite.h ../machind/lmclient.h



#demo_add.o: demo_add.c ../utils/lmrandom  
#	rm -f xxx.c
##	../utils/lmrandom < demo_add.c > xxx.c
#	$(CC) $(CFLAGS) -c xxx.c
#	rm -f xxx.c demo_add.o
#	mv xxx.o demo_add.o
#
#demo.o: demo.c ../utils/lmrandom
#	rm -f xxx.c
#	../utils/lmrandom < demo.c > xxx.c
#	$(CC) $(CFLAGS) -c xxx.c
#	rm -f xxx.c demo.o
#	mv xxx.o demo.o

demof2.o:	demof2.c  lm_n_f2.o
	$(CC) -c $(CFLAGS) -I../h lsrvendf2.c 
	mv lsrvendf2.o demof2.o

demof.o:	demof.c lm_nf.o
	$(CC) -c $(CFLAGS) -I../h lsrvendf.c 
	mv lsrvendf.o demof.o

demo.o:	$(CLIENTONLY)
	$(CC) -c $(CFLAGS) -I../h lsrvend.c 
	mv lsrvend.o demo.o



lm_n.o:	demo.c code.h  ../utils/lmrand1 ../utils/lmnewgen.o ../machind/lm_code.h \
	$(LCLIENTLIBS)
	../utils/lmrand1 -i demo.c
	$(CC) -c $(CFLAGS) -I../h lmcode.c
	$(CC) ../utils/lmnewgen.o lmcode.o $(LCLIENTLIBS) -o lmnewgen $(XTRALIB) $(THREADLIB)
	./lmnewgen demo  -o lm_n.c $(SMALL)
	$(CC) -c $(CFLAGS) -I../h lm_n.c

lm_n_f2.o:	demof2.c code.h  ../utils/lmrand1 ../utils/lmnewgen.o \
	$(LCLIENTLIBS)
	../utils/lmrand1 -i demof2.c -o lmcodef2.c lsrvendf2.c
	$(CC) -c $(CFLAGS) -I../h lmcodef2.c
	$(CC) ../utils/lmnewgen.o lmcodef2.o $(LCLIENTLIBS) -o lmnewgenf2 $(XTRALIB) $(THREADLIB)
	./lmnewgenf2 demof2 -o lm_n_f2.c $(SMALL)
	$(CC) -c $(CFLAGS) -I../h lm_n_f2.c

lm_nf.o:	demof.c code.h  ../utils/lmrand1 ../utils/lmnewgen.o \
	$(LCLIENTLIBS)
	../utils/lmrand1 -i demof.c -o lmcodef.c lsrvendf.c
	$(CC) -c $(CFLAGS) -I../h lmcodef.c
	$(CC) ../utils/lmnewgen.o lmcodef.o $(LCLIENTLIBS) -o lmnewgenf $(XTRALIB) $(THREADLIB)
	./lmnewgenf demof -o lm_nf.c $(SMALL)
	$(CC) -c $(CFLAGS) -I../h lm_nf.c

lm_nold.o:	demo.c code.h  ../utils/lmrand1 ../utils/lmnewgen.o \
	$(LCLIENTLIBS)
	../utils/lmrand1 -i demo.c
	$(CC) -c $(CFLAGS) -I../h lmcode.c
	$(CC) ../utils/lmnewgen.o lmcode.o $(LCLIENTLIBS) -o lmnewgen $(XTRALIB) $(THREADLIB)
	./lmnewgen demo  -o lm_nold.c $(SMALL)
	$(CC) -c $(CFLAGS) -I../h lm_nold.c

fake_appinit.o:	../app/ls_app_init.c
	$(CC) $(CFLAGS) -URELEASE_VERSION -c ../app/ls_app_init.c
	mv ls_app_init.o fake_appinit.o

lm_npathd.o:	pathd.o

pathd.o:	pathd.c
	../utils/lmrand1 -i pathd.c -o lmcodepathd.c lsrvendpathd.c
	$(CC) -c $(CFLAGS) -I../h lmcodepathd.c
	$(CC) -c $(CFLAGS) -I../h lsrvendpathd.c 
	mv lsrvendpathd.o pathd.o
	rm -f lsrvendpathd.c
	$(CC) ../utils/lmnewgen.o lmcodepathd.o $(LCLIENTLIBS) -o lmnewgenpathd $(XTRALIB) $(THREADLIB)
	./lmnewgenpathd demo -o lm_npathd.c $(SMALL)
	$(CC) -c $(CFLAGS) -I../h lm_npathd.c
	

$(OBJS):	../machind/lmclient.h

basic_tests.o:	testsuite.h code.h basic_tests.c
cryptest.o:	testsuite.h cryptest.c
demof.o:	code.h
inout.o:	code.h inout.c
lock_tests.o:	testsuite.h lock_tests.c
no_s_tests.o:	testsuite.h code.h no_s_tests.c
one_s_tests.o:	testsuite.h code.h one_s_tests.c
servtest.o:	testsuite.h code.h servtest.c
st_bugs.o:	testsuite.h code.h st_bugs.c
st_vers.o:	testsuite.h code.h st_vers.c
st_8bit.o:	testsuite.h code.h st_8bit.c
path2_add.o:	code.h
pathd.o:	code.h
path_tests.o:	testsuite.h code.h path_tests.c
ts_config.o:	code.h ts_config.c
ts_serv_supp.o:	code.h ts_serv_supp.c
ts_dump.o:	ts_dump.c
ts_environ.o:	ts_environ.c
ts_lic_file.o:	ts_lic_file.c
ts_hostid.o:	ts_hostid.c
ts_list.o:	ts_list.c
ts_utils.o:	ts_utils.c
