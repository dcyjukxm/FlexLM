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
# $Id: vx_simpc_example.mak,v 1.2 2002/12/09 23:25:41 kmaclean Exp $
#
# Makefile that holds the definitions to build the Ultra lite example program.
#
# This makefile assumes the WIND_BASE environment variable is set to the 
# tornado development directory.
#
# Run the make from the ulite subdirectory of the platform directory.
# ie. vx_simpc/ulite
#
# To make the ultra lite version:
#	make -f example.mak 
#

# for a different VxWorks platform change these four values
GPLATFORM = vx_simpc
# VxWorks CPU identifier
CPU		= SIMNT
# tell the compiler which cpu to build for
CPUFLAG		= -mpentium
# used as the suffix for the gnu tool names
REAL_TOOL	= simpc	


include $(GPLATFORM).mak

TOOL            = gnu
TARGET_DFLAGS	= -DVXWORKS -DCPU=$(CPU) -DTOOL_FAMILY=$(TOOL) -DTOOL=$(TOOL)
TARGET_INCFLAGS	= -I$(WIND_BASE)/target/h 
DEBUGFLAGS	= -g  -O0
OPTIMIZEFLAGS	= -O2
TARGET_CFLAGS	= $(CPUFLAG) -ansi -fno-builtin -fno-defer-pop 
AR              = ar$(REAL_TOOL)
AS              = cc$(REAL_TOOL)
CC              = cc$(REAL_TOOL)
ARFLAGS		= crus
CPP             = cc$(REAL_TOOL) -E -P
LD              = ld$(REAL_TOOL)
LDFLAGS		= 	
RANLIB		= ranlib$(REAL_TOOL)
EXEEXT		= .out
LIBEXT		= .a

BASEDIR = ../

INCFLAGS	= -I$(BASEDIR). -I$(BASEDIR)../h -I$(BASEDIR)../machind $(TARGET_INCFLAGS)
DFLAGS		= $(TARGET_DFLAGS) -DEMBEDDED_FLEXLM -DFLEXLM_ULTRALITE 
CFLAGS		= $(INCFLAGS) $(DFLAGS) $(DEBUGFLAGS) $(TARGET_CFLAGS)

EXAMPLE	= lmlite$(EXEEXT)

LIBRARY	= libulitelmgr$(LIBEXT)

all:	$(EXAMPLE)
	@echo $(EXAMPLE) build done.

%.out : %.o
	ccsimpc -r -nostdlib $< $(LIBRARY) -o partialImage.o 
	nmsimpc -g partialImage.o $< | wtxtcl C:\Tornado2.2\host\src\hutils\munch.tcl -c $(REAL_TOOL) > ctdt.c
	ccsimpc -c -fdollars-in-identifiers $(CFLAGS) ctdt.c -o ctdt.o
	ccsimpc -r -nostdlib -Wl,--force-stabs-reloc $(DEBUGFLAGS) partialImage.o ctdt.o -o $@

%.o : %.c
	$(CC) $(CFLAGS) -o $@ $<
