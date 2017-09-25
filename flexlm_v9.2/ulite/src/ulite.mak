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
# $Id: ulite.mak,v 1.3 2002/12/09 23:25:41 kmaclean Exp $
#
# Makefile that holds the definitions to build the Ultra lite client library.
# This is the new way to do things. Add new platforms here not to 'makefile' 
# or 'ul_platargs'
#
# To make the ultra lite version:
#	make -f ulite.mak GPLATFORM=xyz
# 
# where xyz is one of the known platform names. Previously retrieved
# from the gplatform script. Using the script doesn't really work here
# because mostly the ultra lite is cross compiled so gplatform
# will not return the correct GPLATFORM.
#
# To make a release and the binary kit:
#	make -f ulite.mak GPLATFORM=xyz release.
# This will create a directory named ulite_$(GPLATOFRM) and place the kit
# in there. Still need to copy the releasenotes and manual in though.
#
# platforms supported and tested of 12/9/02:
#	vx_simpc - VxWorks simulator on the PC
#
# To add new platforms:
#	1. Create a new ($GPLATFORM).mak file to hold the compile flags etc.
#
# This makefile works with GNU make
#


include $(GPLATFORM).mak

INCFLAGS	= -I. -I../h -I../machind -I../flexsrc $(TARGET_INCFLAGS)
DFLAGS		= -DGPLATFORM="\"$(GPLATFORM)\"" $(TARGET_DFLAGS) -DLM_INTERNAL -DEMBEDDED_FLEXLM -DFLEXLM_ULTRALITE 

CFLAGS		= $(INCFLAGS) $(DFLAGS) $(DEBUGFLAGS) $(TARGET_CFLAGS)

EXAMPLESRC	= lmlite.c
SRCS		= lm_checkit.c l_ul_stubs.c
OBJS		= lm_checkit$(OBJEXT) \
	  		../flexsrc/l_key$(OBJEXT) \
	  		../flexsrc/l_malloc$(OBJEXT) \
	  		l_ul_stubs$(OBJEXT)

LIBRARY	= libulitelmgr$(LIBEXT)
EXAMPLE_EXE = lmlite$(EXEEXT)

# where and what is the release build
RELEASELIST = $(LIBRARY) $(EXAMPLESRC) $(GPLATFORM)_example.mak 
PLATFORMDIR = ../ulite_$(GPLATFORM)

# a few tools. These work on unix
# To build on the PC it is expected that the unix versins of these tools are installed
RM		= rm -f
COPY		= cp
MKDIR		= mkdir

# Default rule
all:	$(LIBRARY)
	@echo build $(LIBRARY) done

.PHONY : release
release:
	$(MAKE) -f ulite.mak DEBUGFLAGS="$(OPTIMIZEFLAGS) -DRELEASE_VERSION" real_release

.PHONY : real_release
real_release: $(LIBRARY)
	-$(MKDIR) $(PLATFORMDIR)
	$(COPY) $(RELEASELIST) $(PLATFORMDIR)
#	$(COPY) $(GPLATFORM)_releasenotes.txt $(PLATFORMDIR)
	@echo Release build Done. $(PLATFORMDIR)	
	
$(LIBRARY):   $(OBJS)
	@echo Creating $(LIBRARY) ...
	$(AR) $(ARFLAGS) $(LIBRARY) $(OBJS)
	$(RANLIB) $(LIBRARY)

.PHONY : clean
clean:;	$(RM) $(OBJS) $(LIBRARY) $(EXAMPLE_EXE)

$(OBJS): ../h/lmachdep.h ../machind/lmclient.h ../h/l_prot.h ../h/l_privat.h

%.o : %.c
	$(CC) $(CFLAGS) -o $@ $<

%.obj : %.c
	$(CC) $(CFLAGS) /Fo$@ $<

###
lm_checkit$(OBJEXT):	../flexsrc/l_crypt.c ../h/l_strkey.h ../flexsrc/l_strkey.c 
