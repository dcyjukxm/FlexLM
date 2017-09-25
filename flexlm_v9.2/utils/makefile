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
# $Id: makefile,v 1.54 2003/05/08 19:33:17 sluu Exp $
#


XTRACFLAG = `gplatargs` -DLM_INTERNAL


THREADLIB = `gplatargs -lthreads`
XTRALIB = `gplatargs -l` 

PLAT = `gplatform`

XTRALINTFLAG = `gplatargs`

LINTFLAGS = $(XTRALINTFLAG)

INCFLAGS = -I. -I../h -I../machind -I../master -I../vendor

MAIN = lmutil.o

DEBUGFLAG = -g

CFLAGS = $(INCFLAGS) $(DEBUGFLAG) $(XTRACFLAG)

LD = $(CC)

UTILS = lmborrow \
	lmcksum \
	lmdiag \
	lmdown \
	lmhostid \
	lminstall \
	lmpath \
	lmremove \
	lmreread \
	lmswitchr \
	lmswitch \
	lmstat \
	lmutil \
	lmver 

LIST = lmstrip/lmstrip \
	makekey \
	lmcrypt \
	lmfeats \
	makepkg \
	validdat 

UTILOBJS = lmutil.o \
	lmborrow.o \
	lm_cksum.o    \
	lm_diag.o     \
	lm_diag_ck.o    \
	lm_down.o \
	lm_hostid.o  \
	lm_inst.o \
	lm_lic_info.o   \
	lm_path.o \
	lm_remov.o \
	lm_rerd.o \
	lm_spec.o \
	lm_stat.o \
	lm_swr.o \
	lm_ver.o 

OBJS = makekey.o \
	makepkg.o \
	lmfeats.o \
	lmcrypt.o \
	validdat.o \
	mkborrow.o  \
	$(UTILOBJS)



LIBRARY	= ../src/liblmgr.a \
	../$(PLATFORM)/libcrvs.a \
	../$(PLATFORM)/libsb.a 

UTIL_LIB = liblmutil.a
LIBS = $(LIBRARY) $(XTRALIB) $(LDFLAGS) $(THREADLIB)
SLIBRARY = ../server/liblmgr_s.a
APPLIB = ../app/liblmgr_as.a
LINTLIB = ../src/llib-llmgr.ln

_all:	
	$(MAKE) all CFLAGS="$(CFLAGS)" PLAT="$(PLAT)"

all:	lmrand1 lmnewgen.o utils $(LIST)   

utils:	$(UTILS)

#makekey: makekey.o ../machind/lmclient.h \
#	../vendor/lm_new2.o $(LIBRARY)
#	$(LD) $(DEBUGFLAG) -o makekey makekey.o ../vendor/lm_new2.o $(LIBS)
makekey: makekey.o ../machind/lmclient.h $(LIBRARY)
	$(LD) $(DEBUGFLAG) -o makekey makekey.o $(LIBS)

lmstrip/lmstrip: lmstrip/lmstrip.o 
	$(LD) $(XTRALIB) $(DEBUGFLAG) -o lmstrip/lmstrip lmstrip/lmstrip.o 
	cp lmstrip/lmstrip ../testsuite


makepkg: makepkg.o ../machind/lmclient.h  $(LIBRARY) 
	$(LD) $(DEBUGFLAG) -o makepkg  makepkg.o $(LIBS)

lmrand1:	lmrand1.o $(LIBRARY)
		$(LD) $(DEBUGFLAG) -o lmrand1 lmrand1.o \
			$(LIBS) $(XTRALIB)

lmpath: lmutil
	rm -f lmpath
	ln -s lmutil lmpath


lmborrow: lmutil
	rm -f lmborrow
	ln -s lmutil lmborrow

lmcksum: lmutil
	rm -f lmcksum
	ln -s lmutil lmcksum

lmcrypt.o: ../machind/lmcrypt.c  ../vendor/lmprikey.h 
	$(CC) -c $(CFLAGS) ../machind/lmcrypt.c

../vendor/lmprikey.h: lmrand1 lmnewgen.o ../machind/lm_code.h
	cd ../vendor; $(MAKE) lm_new.o
	

#lmcrypt:	lmcrypt.o ../machind/lm_code.h $(LIBRARY) 
#	$(LD) $(DEBUGFLAG) -o lmcrypt lmcrypt.o $(LIBS)
lmcrypt:	lmcrypt.o $(LIBRARY) 
	$(LD) $(DEBUGFLAG) -o lmcrypt lmcrypt.o $(LIBS)

lmdiag: lmutil ../src/lm_ckout.c
	rm -f lmdiag
	ln -s lmutil lmdiag

lminstall: lmutil
	rm -f lminstall
	ln -s lmutil lminstall

lmdown: lmutil
	rm -f lmdown
	ln -s lmutil lmdown

lmfeats:	lmfeats.o ../vendor/lmprikey.h ../vendor/lm_new.o $(LIBRARY)
	$(LD) $(DEBUGFLAG) -o lmfeats lmfeats.o ../vendor/lm_new.o $(LIBS)

../vendor/lm_new.o:	lmrand1 lmnewgen.o
	cd ../vendor; $(MAKE) lm_new.o; $(MAKE) 

lmhostid: lmutil
	rm -f lmhostid
	ln -s lmutil lmhostid

lmremove: lmutil
	rm -f lmremove
	ln -s lmutil lmremove

lmreread: lmutil
	rm -f lmreread
	ln -s lmutil lmreread

lmstat: lmutil
	rm -f lmstat
	ln -s lmutil lmstat

lmswitch: lmutil
	rm -f lmswitch
	ln -s lmutil lmswitch

lmswitchr: lmutil
	rm -f lmswitchr
	ln -s lmutil lmswitchr

lmutil: $(UTIL_LIB) $(LIBRARY)
	$(LD) $(DEBUGFLAG) -o lmutil $(UTIL_LIB) $(LIBS)

$(UTIL_LIB):	$(UTILOBJS)
	$(AR) r $(UTIL_LIB) $(UTILOBJS)
	-test -f /bin/ranlib && ranlib $(UTIL_LIB) || \
	test -f /usr/bin/ranlib && ranlib $(UTIL_LIB) || true

purify: lmutil.o $(LIBRARY)
	purify $(LD) $(DEBUGFLAG) -o lmutil lmutil.o 

lmver: lmutil
	rm -f lmver
	ln -s lmutil lmver

validdat:	validdat.o $(LIBRARY)
	$(LD) $(DEBUGFLAG) -o validdat validdat.o $(LIBS)


mkborrow:	mkborrow.o  $(LIBRARY) $(SLIBRARY) $(APPLIB)
	-$(LD) $(DEBUGFLAG) -o mkborrow mkborrow.o $(APPLIB) $(SLIBRARY) $(LIBS)


clean:
	@echo cleaning `pwd` ...
	@rm -f $(LIST) $(OBJS) $(UTIL_LIB) lm_af.o lm_licf.o \
	lmrandom.o lmrandomf.o lmrandomf2.o lmrand1 lmrand1.o lmnewgen \
	lmnewgen.o lmrandom lmrandomf lmrandomf2 lmutil lmcode.c \
		lmstrip/lmstrip.o lm_new* *.a

lint:;	lint $(LINTFLAGS) $(INCFLAGS) validdat.c $(LINTLIB)
	lint $(LINTFLAGS) $(INCFLAGS) lmutil.c $(LINTLIB)

lmutil.o: lmutil.c utilscode.h ../machind/lmclient.h ../h/lm_comm.h ../machind/lsmaster.h

$(OBJS): ../machind/lmclient.h ../h/l_prot.h ../h/l_privat.h

$(UTILOBJS):	lmutil.h

lmnewgen.o:	../machind/lmclient.h ../h/l_prot.h ../h/l_privat.h	 \
		../h/l_borrow.h



lsvendor.o:	../vendor/lsvendor.c

lmstrip/lmstrip.o: ../machind/lmstrip.c
	$(CC) -c $(CFLAGS) -o lmstrip/lmstrip.o ../machind/lmstrip.c

makekey.o: ../machind/makekey.c ../vendor/lmprikey.h
	$(CC) -c $(CFLAGS) ../machind/makekey.c

makepkg.o: ../machind/makepkg.c ../vendor/lmprikey.h
	$(CC) -c $(CFLAGS) ../machind/makepkg.c

lmfeats.o: ../machind/lmfeats.c 
	$(CC) -c $(CFLAGS) ../machind/lmfeats.c

validdat.o: ../machind/validdat.c
	$(CC) -c $(CFLAGS) ../machind/validdat.c

lmutil.o: lmutil.c
lm_diag_ck.o: ../src/lm_ckout.c
lmnewgen.o:	../h/l_rand.h ../h/l_borrow.h


