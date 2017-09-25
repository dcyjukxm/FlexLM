XTRACFLAG =  

CC = cc

LIBRARY = liblmgr.a

CCFLAGS = -g
DEBUGFLAG = 
CFLAGS = -I../h $(DEBUGFLAG) $(XTRACFLAG) $(CCFLAGS)

SRCS	      = lmlite.c

lmlite:	lmlite.c liblmgr.a lm_code.h
	$(CC) -g -o lmlite lmlite.c -I. liblmgr.a

purelmlite:	lmlite.c liblmgr.a ../h/lm_code.h
	purify cc -g -o lmlite lmlite.c -I. liblmgr.a

clean:;	rm -f lmlite
