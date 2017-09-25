XTRACFLAG = `gplatargs`

THREADLIB=`gplatargs -lthreads`
XTRALIB = `gplatargs -l` 

INCFLAGS = -I../h -I../machind 

DEBUGFLAG = -g

CFLAGS = $(INCFLAGS) $(DEBUGFLAG) $(XTRACFLAG)

LDFLAGS = $(DEBUGFLAG)

OBJS = \
	client.o	\
	dlog.o		\
	dump.o		\
	ether_id.o	\
	get_config.o	\
	get_id.o	\
	host.o		\
	laspi.o		\
	lmclient.o	\
	lmflex.o	\
	lmsimple.o	\
	mail_test.o	\
	master_list.o	\
	monthtest.o	\
	replog.o	\
	server_dump.o 	\
	test_date.o	\
	test_slwg.o 	\
	vdinfo.o

SRCS = \
	dlog.c		\
	dump.c		\
	ether_id.c	\
	get_config.c	\
	get_id.c	\
	host.c		\
	laspi.c		\
	../machind/lmclient.c	\
	../machind/lmflex.c	\
	../machind/lmsimple.c	\
	mail_test.c	\
	master_list.c	\
	monthtest.c	\
	replog.c	\
	server_dump.c	\
	test_date.c	\
	test_slwg.c 	\
	vdinfo.c


STANDARD =	lmflex vdinfo lmsimple lmclient  

EXECS = $(STANDARD) c2 hclient dlog ether_id get_id lister lsapi mail_test \
	monthtest multiclient next_config \
	server_dump test_server_timeout timer_test x vdinfo lmclient \
	lmsimple


LIBRARY	= ../src/liblmgr.a ../$(PLATFORM)/libsb.a \
	../$(PLATFORM)/libcrvs.a 
LIBS = $(LIBRARY) $(XTRALIB)  $(THREADLIB)
SERVERLIBRARY	= ../server/liblmgr_s.a
APPSERVERLIBRARY = ../app/liblmgr_as.a
LINTLIB = ../src/llib-llmgr.ln

LD = $(CC)

all:	$(STANDARD)

everything:	$(EXECS)

c2:	c2.o $(LIBRARY)
	$(LD) $(LDFLAGS) -o c2 c2.o $(LIBS)

c3:	c3.o $(LIBRARY)
	$(LD) $(LDFLAGS) -o c3 c3.o $(LIBS)

replog:	replog.o $(LIBRARY) 
	$(LD) $(LDFLAGS) -o replog replog.o $(LIBS)

flush:	flush.o $(LIBRARY) ../vendor/lm_new.o
	$(LD) $(LDFLAGS) -o flush flush.o ../vendor/lm_new.o $(LIBS)

lmflex.o:	../machind/lmflex.c
	$(CC) -c $(CFLAGS) ../machind/lmflex.c

lmflex:	lmflex.o $(LIBRARY) ../vendor/lm_new.o 
	$(LD) $(LDFLAGS) -o lmflex lmflex.o ../vendor/lm_new.o $(LIBS)

lmsimple.o:	../machind/lmsimple.c
	$(CC) -c $(CFLAGS) ../machind/lmsimple.c

lmsimple:	lmsimple.o $(LIBRARY) ../vendor/lm_new.o
	$(LD) $(LDFLAGS) -o lmsimple lmsimple.o ../vendor/lm_new.o $(LIBS)

borrutil:	borrutil.o $(LIBRARY) ../vendor/lm_new.o
	$(LD) $(LDFLAGS) -o borrutil borrutil.o ../vendor/lm_new.o $(LIBS)

lmclient.o:	../machind/lmclient.c
	$(CC) -c $(CFLAGS) ../machind/lmclient.c

lmclient:	lmclient.o $(LIBRARY) ../vendor/lm_new.o
	$(LD) $(LDFLAGS) -o lmclient lmclient.o ../vendor/lm_new.o $(LIBS)

client:	client.o $(LIBRARY) 
	$(LD) $(LDFLAGS) -o client client.o $(LIBS)

dlog:	dlog.o $(LIBRARY)
	$(LD) -o dlog dlog.o $(LIBS)

ether_id: ether_id.o $(LIBRARY)
	$(LD) $(LDFLAGS) -o ether_id ether_id.o $(LIBS)

hclient: lmflex.o $(LIBRARY)
	$(LD) $(LDFLAGS) -o hclient lmflex.o $(LIBS) ../server/ls_gethost.o

dump:	dump.o $(LIBRARY)
	$(LD) $(LDFLAGS) -o dump dump.o $(LIBS)

exinstal:	exinstal.o $(LIBRARY) ../vendor/lm_new.o
	$(LD) $(LDFLAGS) -o exinstal ../vendor/lm_new.o exinstal.o $(LIBS)

licstring:	licstring.o $(LIBRARY) ../vendor/lm_new.o
	$(LD) $(LDFLAGS) -o licstring licstring.o ../vendor/lm_new.o $(LIBS)

#dump.o:	dump.c
#	$(CC) -c $(CFLAGS) -DLM_INTERNAL dump.c

get_config:	get_config.o $(LIBRARY)
	$(LD) $(LDFLAGS) -o get_config get_config.o $(LIBS)

get_id:	get_id.o $(LIBRARY)
	$(LD) $(LDFLAGS) -o get_id get_id.o $(LIBS)

host:	host.o $(LIBRARY)
	$(LD) $(LDFLAGS) -o host host.o $(LIBS)

lister: lister.o $(LIBRARY)
	$(LD) $(LDFLAGS) -o lister lister.o $(LIBS)

lsapi:	lsapi.o $(LIBRARY)
	$(LD) $(LDFLAGS) -o lsapi lsapi.o $(LIBS)

mail_test:	mail_test.o $(LIBRARY)
	$(LD) $(LDFLAGS) -o mail_test mail_test.o $(SERVERLIBRARY) $(LIBS)

master_list:	master_list.o $(LIBRARY)
	$(LD) $(LDFLAGS) -o master_list master_list.o $(LIBS)

monthtest:	monthtest.o $(LIBRARY)
	$(LD) $(LDFLAGS) -o monthtest monthtest.o $(LIBS)

multiclient:	multiclient.o $(LIBRARY)
	$(LD) $(LDFLAGS) -o multiclient multiclient.o $(LIBS)

next_config:	next_config.o $(LIBRARY)
	$(LD) $(LDFLAGS) -o next_config next_config.o $(LIBS)

server_dump:	server_dump.o $(LIBRARY)
	$(LD) $(LDFLAGS) -o server_dump server_dump.o $(LIBS)

test_date:	test_date.o $(LIBRARY)
	$(LD) $(LDFLAGS) -o test_date test_date.o $(LIBS)

test_server_timeout: test_server_timeout.o $(LIBRARY)
	$(LD) $(LDFLAGS) -o test_server_timeout test_server_timeout.o $(LIBS)

test_slwg:	test_slwg.o $(LIBRARY)
	$(LD) $(LDFLAGS) -o test_slwg test_slwg.o $(LIBS)

timer_test:	timer_test.o $(LIBRARY)
	$(LD) $(LDFLAGS) -o timer_test timer_test.o $(LIBS)

vdinfo:	vdinfo.o $(LIBRARY) ../vendor/lm_new.o
	$(LD) $(LDFLAGS) -o vdinfo vdinfo.o ../vendor/lm_new.o $(LIBS) 

x:	x.o $(LIBRARY)
	$(LD) $(LDFLAGS) -o x x.o $(LIBS)

lint:;	lint $(INCFLAGS) ../machind/lmflex.c $(LINTLIB)

clean:	
	@echo cleaning `pwd` ...
	@rm -f $(OBJS) $(EXECS)

$(OBJS):	../machind/lmclient.h

#$(SRCS):;	sccs get $@

lmflex.o: ../machind/lmflex.c ../h/lm_code.h
dump.o: dump.c ../h/lm_code.h
ether_id.o: ether_id.c ../h/lm_code.h
get_config.o: get_config.c ../h/lm_code.h
get_sprog.o: get_sprog.c ../h/lm_code.h
host.o: host.c ../h/lm_code.h
mail_test.o: mail_test.c ../h/lm_code.h
master_list.o: master_list.c ../h/lm_code.h
test_date.o: test_date.c ../h/lm_code.h
test_slwg.o: test_slwg.c ../h/lm_code.h
replog.o:	replog.c replog.h
