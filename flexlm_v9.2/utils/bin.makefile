SRCDIR = ../machind

INCFLAGS = -I$(SRCDIR) -I.

THREADLIB =  

XTRALIB =  

XTRACFLAG = 

#
#	Use XTRAOBJS to define your object files for vendor daemon
#	initialization routines, checkout filter, checkin callback, etc.
#
XTRAOBJS = 

LDFLAGS =

LINTFLAGS = -a -b -h

CFLAGS = -c -g $(INCFLAGS) $(XTRACFLAG)

SRCS	= $(SRCDIR)/lsvendor.c

STRIP = strip

OBJS =  makekey.o \
	makepkg.o \
	lmclient.o \
	lmcrypt.o \
	lmfeats.o \
	lmstrip.o \
	lsvendor.o 

DAEMON = demo

EXECS = makekey makepkg lmclient lmcrypt lmstrip lmflex


CLIENTLIB	= liblmgr.a libcrvs.a libsb.a $(BORROWOBJ)
LIBS		= liblmgr_as.a liblmgr_s.a $(CLIENTLIB)

LINTLIB =	llib-llmgr.a.ln

UTILS = lmhostid lmdown lminstall lmremove \
	lmreread  lmswitchr lmstat lmdiag lmver lmpath lmborrow lmswitch

all: $(EXECS) $(DAEMON) utils 



lmnewgen:	../machind/lsvendor.c ../machind/lm_code.h
	./lmrand1 -i $(SRCDIR)/lsvendor.c
	$(CC) -c $(CFLAGS) lmcode.c
	$(CC) lmnewgen.o lmcode.o $(CLIENTLIB) $(XTRALIB) $(THREADLIB) \
						-o lmnewgen

daemon:	$(DAEMON)

$(DAEMON):	$(XTRAOBJS) $(LIBS) $(SRCDIR)/lsserver.h $(SRCDIR)/lm_code.h lmstrip lm_new.o 
	$(MAKE) lm_new.o
	$(CC) -c $(CFLAGS) lsrvend.c 
	mv lsrvend.o lsvendor.o
	rm -f lsrvend.c
	$(CC) $(LDFLAGS) -o $(DAEMON) lsvendor.o lm_new.o $(XTRAOBJS) \
				$(LIBS) $(XTRALIB) $(THREADLIB)
	$(STRIP) $(DAEMON)
	./lmstrip $(DAEMON)
	rm -f lm_new.o

lm_new.o: $(SRCDIR)/lsvendor.c $(SRCDIR)/lm_code.h $(SRCDIR)/lmclient.h \
	$(CLIENTLIB)
	./lmrand1 -i $(SRCDIR)/lsvendor.c
	$(CC) -c $(CFLAGS)  lmcode.c
	$(CC) lmnewgen.o lmcode.o $(CLIENTLIB) $(XTRALIB) $(THREADLIB) \
						-o lmnewgen
	rm -f lm_new.c
	./lmnewgen $(DAEMON) -o lm_new.c
	$(CC) -c $(CFLAGS)  lm_new.c

makekey:	$(SRCDIR)/makekey.c $(SRCDIR)/lm_code.h \
		$(SRCDIR)/lmclient.h $(CLIENTLIB) lmprikey.h
	$(CC) $(CFLAGS) $(SRCDIR)/makekey.c
	$(CC) -o makekey makekey.o $(CLIENTLIB) $(XTRALIB) $(THREADLIB)
	rm makekey.o

makepkg:	$(SRCDIR)/makepkg.c $(SRCDIR)/lm_code.h  lmprikey.h \
				$(SRCDIR)/lmclient.h $(CLIENTLIB)
	$(CC) $(CFLAGS) $(SRCDIR)/makepkg.c
	$(CC) -o makepkg makepkg.o $(CLIENTLIB) $(XTRALIB) $(THREADLIB)

lmcrypt: $(SRCDIR)/lmcrypt.c \
		$(SRCDIR)/lmclient.h $(SRCDIR)/lm_code.h lmprikey.h $(CLIENTLIB)
	$(CC) $(CFLAGS) $(SRCDIR)/lmcrypt.c
	$(CC) -o lmcrypt lmcrypt.o $(CLIENTLIB) $(XTRALIB) $(THREADLIB)

lmprikey.h: lm_new.o

validdat:	$(SRCDIR)/validdat.c  $(CLIENTLIB)
	$(CC) $(CFLAGS) $(SRCDIR)/validdat.c
	$(CC) -o validdat validdat.o $(CLIENTLIB) $(XTRALIB) $(THREADLIB)
	rm validdat.o

lmclient:	$(SRCDIR)/lmclient.c  $(SRCDIR)/lm_code.h $(CLIENTLIB) lmstrip \
			lm_new.o
	$(MAKE) lm_new.o
	$(CC) $(CFLAGS) $(SRCDIR)/lmclient.c
	$(CC) -o lmclient lmclient.o lm_new.o $(CLIENTLIB) $(XTRALIB) \
						$(THREADLIB)
	rm lmclient.o
	$(STRIP) lmclient
	./lmstrip lmclient
	rm -f lm_new.o

lmsimple: $(SRCDIR)/lmsimple.c  $(SRCDIR)/lm_code.h $(CLIENTLIB) lmstrip \
			lm_new.o
	$(MAKE) lm_new.o
	$(CC) $(CFLAGS) $(SRCDIR)/lmsimple.c
	$(CC) -o lmsimple lmsimple.o lm_new.o $(CLIENTLIB) $(XTRALIB) \
						$(THREADLIB)
	rm lmsimple.o
	$(STRIP) lmsimple
	./lmstrip lmsimple
	rm -f lm_new.o

lmflex:	$(SRCDIR)/lmflex.c  $(SRCDIR)/lm_code.h $(CLIENTLIB) lmstrip lm_new.o
	$(MAKE) lm_new.o
	$(CC) $(CFLAGS) $(SRCDIR)/lmflex.c
	$(CC) -o lmflex lmflex.o lm_new.o $(CLIENTLIB) $(XTRALIB) $(THREADLIB)
	rm lmflex.o
	$(STRIP) lmflex
	./lmstrip lmflex
	rm -f lm_new.o

lmfeats: $(SRCDIR)/lmfeats.c $(SRCDIR)/lm_code.h $(CLIENTLIB)  lm_new.o
	$(CC) $(CFLAGS) $(SRCDIR)/lmfeats.c
	$(CC) -o lmfeats lmfeats.o lm_new.o $(CLIENTLIB) $(XTRALIB) $(THREADLIB)
	rm lmfeats.o

lmstrip: $(SRCDIR)/lmstrip.c
	$(CC) $(CFLAGS) $(SRCDIR)/lmstrip.c
	$(CC) -o lmstrip lmstrip.o $(XTRALIB) $(THREADLIB)
	rm lmstrip.o


lmutil:	liblmutil.a liblmgr.a
	$(CC) -o lmutil liblmutil.a $(CLIENTLIB) $(XTRALIB) $(THREADLIB)
	$(STRIP) lmutil

lmgrd:	liblmgrd.a liblmgr_s.a liblmgr.a
	$(CC) -o lmgrd liblmgrd.a liblmgr_s.a liblmgr.a $(XTRALIB) $(THREADLIB)
	$(STRIP) lmgrd

shared_object:  liblmgr_pic.a lm_new.o
	rm -rf sotmp liblmgr.so ;\
	mkdir sotmp ;\
	cd sotmp ;\
	ar x ../liblmgr_pic.a ;\
	cp ../lm_new.o . ;\
	ld -shared -o ../liblmgr.so *.o ;\
	cd .. ;\
	rm -rf sotmp

lint:;	lint $(INCFLAGS) $(LINTFLAGS) $(SRCS) $(LINTLIB)

clean:;	rm -f $(OBJS) $(EXECS) $(DAEMON) $(UTILS) lmcode.c lmcode.o \
	lmnewgen lsrvend.c lmflex lm_new* *.h




lmcrypt.o:	$(SRCDIR)/lmcrypt.c $(SRCDIR)/lm_code.h $(SRCDIR)/lmclient.h
	$(CC) $(CFLAGS) $(SRCDIR)/lmcrypt.c



utils:	
	-rm -f lmhostid lmver lmdown lmremove lmreread lmswitchr \
		lmstat lmdiag lminstall lmpath lmborrow lmswitch
	ln lmutil lmhostid
	ln lmutil lmver
	ln lmutil lmdown
	ln lmutil lmremove 
	ln lmutil lmreread 
	ln lmutil lmswitchr 
	ln lmutil lmstat 
	ln lmutil lmdiag
	ln lmutil lminstall
	ln lmutil lmpath
	ln lmutil lmborrow
	ln lmutil lmswitch

oldnames_comment:
	@echo Many filenames were shortened in v5.
	@echo If you want links to the old pre-v5 names, please type
	@echo 	\'make oldnames\'

oldnames:
	rm -f create_license; ln makekey create_license
	rm -f create_pkg; ln makepkg create_pkg
	rm -f lmcrypter; ln lmcrypt lmcrypter	
	rm -f lmfeatset; ln lmfeats lmfeatset
	-(\
		cd ../machind; \
		rm -f ls_vendor.c; ln lsvendor.c ls_vendor.c; \
		rm -f create_lic.c; ln makekey.c create_lic.c; \
		rm -f create_pkg.c; ln makepkg.c create_pkg.c; \
		rm -f lm_client.h; ln lmclient.h lm_client.h; \
		rm -f lm_errors.h; ln lmerrors.h lm_errors.h; \
		rm -f lm_hosttype.h; ln	lmhtype.h lm_hosttype.h; \
		rm -f lmcrypter.c; ln lmcrypt.c lmcrypter.c; \
		rm -f ls_feature.h; ln lsfeatur.h ls_feature.h; \
		rm -f ls_master.h; ln lsmaster.h ls_master.h; \
		rm -f ls_server.h;  ln lsserver.h ls_server.h; \
		rm -f ls_vendor.c; ln lsvendor.c ls_vendor.c; \
	)

eval:
	rm -f lmprikey.h $(UTILS) lmcrypt.o lmpubkey.h lsvendor.o lm_new.c \
			makepkg makepkg.o  lmcode.c lmcode.o
