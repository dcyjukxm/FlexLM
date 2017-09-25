XTRACFLAG = -DLM_INTERNAL -DRELEASE_VERSION -DEMBEDDED_FLEXLM -DFLEXLM_ULTRALITE `ul_platargs`

CC = `ul_platargs -compiler`

LDFLAGS = `ul_platargs -l`

#
#	Development
#
SOURCEDIR = /u/gsi/lmgr
FULLFLEXLIBRARY = $(SOURCEDIR)/liblmgr.a
#
#	v6.0 release
#
#SOURCEDIR = /u/releases/flexlm/v6.0
#FULLFLEXLIBRARY = $(SOURCEDIR)/<ARCH>/liblmgr.a

#
#	Use these for local development
#
CCFLAGS = -g
AR = ar
ARFLAGS = cr

INCFLAGS = -I../h -I../machind

#DEBUGFLAG = -g
DEBUGFLAG = 

#CFLAGS = $(INCFLAGS) $(DEBUGFLAG) $(XTRACFLAG)
CFLAGS = $(INCFLAGS) $(DEBUGFLAG) $(XTRACFLAG) $(CCFLAGS)

SRCS	      = lm_checkit.c l_ul_stubs.c

OBJS	      = lm_checkit.o \
		l_key.o \
		l_malloc.o \
		l_ul_stubs.o

LIBRARY	= liblmgr.a

all:	$(LIBRARY)
	@echo "Used compiler \"$(CC)"\"

linksrc:
	rm -f l_crypt.c
	ln -s $(SOURCEDIR)/src/l_crypt.c .
	rm -f l_key.c
	ln -s $(SOURCEDIR)/src/l_key.c .
	rm -f l_malloc.c
	ln -s $(SOURCEDIR)/src/l_malloc.c .
	rm -f l_strkey.c
	ln -s $(SOURCEDIR)/src/l_strkey.c .

$(LIBRARY):	$(OBJS)
	@echo Creating $(LIBRARY) ...
	@$(AR) $(ARFLAGS) $(LIBRARY) $(OBJS)
	ranlib $(LIBRARY)

lmlite:	lmlite.c liblmgr.a ../h/lm_code.h
	$(CC) $(LDFLAGS) -g -o lmlite lmlite.c -I. $(INCFLAGS) liblmgr.a

purelmlite:	lmlite.c liblmgr.a ../h/lm_code.h
	purify cc -g -o lmlite lmlite.c -I. $(INCFLAGS) liblmgr.a

lmlite2:	lmlite2.c liblmgr.a ../h/lm_code.h
	$(CC) -g -o lmlite2 lmlite2.c -I. $(INCFLAGS) liblmgr.a

purelmlite2:	lmlite2.c liblmgr.a ../h/lm_code.h
	purify cc -g -o lmlite2 lmlite2.c -I. $(INCFLAGS) liblmgr.a

lmcrypt: ../machind/lmclient.h lm_code.h $(FULLFLEXLIBRARY)
	cc -g -I. -I/u/gsi/lmgr/h -c lmcrypt.c
	cc -g -o  lmcrypt lmcrypt.o $(FULLFLEXLIBRARY)

clean:;	rm -f $(OBJS) $(LIBRARY) lmlite

#$(SRCS):;	sccs get $@

$(OBJS):	../h/lmachdep.h ../machind/lmclient.h ../h/l_prot.h ../h/l_privat.h

###
lm_checkit.o:	l_strkey.c l_crypt.c ../h/l_strkey.h
