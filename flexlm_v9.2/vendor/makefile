
THREADLIB = `gplatargs -lthreads`
XTRALIB = `gplatargs -l` 
#
#	Use the following XTRALIB definition for sgi machines
#
#XTRALIB = -lbsd

MAIN = ../app/ls_app_main.o

LDFLAGS =
#
#	Use the following LDFLAGS definition for sunOS4
#
#LDFLAGS = -Bstatic

#
#	Use XTRAOBJS to define your object files for initialization 
#	routines, checkout filter, checkin callback, etc.
#
XTRAOBJS = 

XTRACFLAG =

XTRALINTFLAG = 

INCFLAGS = -I../h -I../machind -I../server 

LINTFLAGS = -a -b -h $(XTRALINTFLAG)

DEBUGFLAG = -g

CFLAGS = `gplatargs` $(INCFLAGS) $(DEBUGFLAG) $(XTRACFLAG)

SRCS	= ../machind/lsvendor.c

OBJS	= lsvendor.o 

LD = $(CC)

PROGRAM = demo

LIBS	=	../app/liblmgr_as.a ../server/liblmgr_s.a ../src/liblmgr.a \
	../$(PLATFORM)/libcrvs.a \
		../$(PLATFORM)/libsb.a

LINTLIB =	../app/llib-llmgr_as.ln \
		../server/llib-llmgr_s.ln \
		../src/llib-llmgr.ln

all:	$(PROGRAM) 

$(PROGRAM):	$(XTRAOBJS) $(LIBS) lsvendor.o  
	$(LD) $(LDFLAGS) $(DEBUGFLAG) -o $(PROGRAM) lsvendor.o lm_new.o \
		$(XTRAOBJS) $(LIBS) $(XTRALIB) $(THREADLIB)
	@echo "done"

purify:	$(XTRAOBJS) $(LIBS) lsvendor.o
	@echo -n "Loading $(PROGRAM) ..."
	@purify $(CC) $(LDFLAGS) $(DEBUGFLAG) -o $(PROGRAM) lsvendor.o \
			$(XTRAOBJS) $(LIBS) $(XTRALIB) $(THREADLIB)
	@echo "done"


lint:;	lint $(INCFLAGS) $(LINTFLAGS) $(SRCS) $(LINTLIB)

clean:
	@echo cleaning `pwd` ...
	@rm -f $(OBJS) $(PROGRAM) lmcode.o lm_new* lmnewgen lmcode.c \
	lsrvend.c  *.h

#$(SRCS):;	sccs get $@

$(OBJS):	../machind/lmclient.h ../machind/lsserver.h ../server/ls_glob.h \
				../h/lm_code.h


lsvendor.o:	lm_new.o
	$(CC) -c $(CFLAGS) $(INCFLAGS) lsrvend.c
	mv lsrvend.o lsvendor.o

lm_new.o:	../machind/lsvendor.c ../machind/lm_code.h ../utils/lmrand1 ../utils/lmnewgen.o
	../utils/lmrand1 -i ../machind/lsvendor.c 
	$(CC) -c $(CFLAGS) $(INCFLAGS) lmcode.c
	$(CC) $(CFLAGS) ../utils/lmnewgen.o lmcode.o ../src/liblmgr.a $(LIBS) $(XTRALIB) -o lmnewgen $(THREADLIB)
	rm -f lm_new.c
	lmnewgen demo -o lm_new.c -nocro
	$(CC) -c $(CFLAGS) $(INCFLAGS) lm_new.c

../utils/lmrand1: ../utils/lmrand1.c
	cd ../utils; make lmrand1

../utils/lmnewgen.o: ../utils/lmnewgen.c
	cd ../utils; make lmnewgen.o
