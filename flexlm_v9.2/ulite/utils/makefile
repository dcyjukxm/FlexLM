FREL = /u/releases/flexlm/v6.0
PLAT = `gplatform`

LIB = $(FREL)/$(PLAT)/liblmgr.a
MYLIB = ../src/liblmgr.a
INC = $(FREL)/h

lmkey:
	cc -DLM_INTERNAL -g -o lmkey lmkey.c $(LIB) -I. -I$(INC)

lmckout:
	cc -g -o lmckout lmckout.c $(MYLIB) -I. -I$(INC)
