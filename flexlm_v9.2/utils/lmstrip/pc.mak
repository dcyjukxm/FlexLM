#########################################################################
#            COPYRIGHT (c) 2003 by Macrovision Corporation.
#        This software has been provided pursuant to a License Agreement
#        containing restrictions on its use.  This software contains
#        valuable trade secrets and proprietary information of 
#        Macrovision Corporation and is protected by law.  It may 
#        not be copied or distributed in any form or medium, disclosed 
#        to third parties, reverse engineered or used in any manner not 
#        provided for in said License Agreement except with the prior 
#        written authorization from Macrovision Corporation.
##########################################################################
#       
#       Module: $Id: pc.mak,v 1.2 2003/05/01 00:03:55 kmaclean Exp $
#
#       Description:  Builds the lmstrip program
#
#
#*****************************************************************************

#****************************
# Platform setup
#****************************

!include "..\..\h\pc.mak"

!ifdef DEBUG
DEBUG_CFLAGS = /Od /Zi
DEBUG_LDFLAGS = /DEBUG
!else
DEBUG_FLAGS = /Ox
DEBUG_LDFLAGS = /INCREMENTAL:NO
!endif

HFILE_DIR = ..\..\h
INCS = /I $(HFILE_DIR) /I ..\..\machind

CFLAGS = /nologo /W3 /D_CONSOLE /DWIN32 /DPC $(DEBUG_CFLAGS)  $(INCS)
LDFLAGS =  $(DEBUG_LDFLAGS)

#
# Compilation rules
#
.cpp.obj:
        $(CC) $(CFLAGS) /c $<
.c.obj:
        $(CC) $(CFLAGS) /c $<

#****************************
# File lists
#****************************

OUTPUTS =       lmstrip.exe 

LMSTRIP_OBJS =  lmstrip.obj lm_objfilecoff.obj lm_objfilewinpe.obj lm_objfilecommon.obj lm_stripstrings.obj lm_objfilelib.obj

#****************************
# General Targets
#****************************

all :  $(OUTPUTS)
        @echo !--------------------------------------
        @echo ! Done building 
        @echo !--------------------------------------

lmstrip.exe : $(LMSTRIP_OBJS)
        $(LD) $(LDFLAGS) /out:$@ $**

$(LMSTRIP_OBJS):  $(HFILE_DIR)\lm_objfilecoff.h \
		$(HFILE_DIR)\lm_objfilewinpe.h \
		$(HFILE_DIR)\lm_objfilecommon.h  \
		$(HFILE_DIR)\lm_objfilelib.h  \
		$(HFILE_DIR)\lm_strip.h

clean :
        @echo Cleaning up 
	-del $(OUTPUTS)
        -del $(LMSTRIP_OBJS)
	-del *.pdb *.ilk *.suo *.opt

