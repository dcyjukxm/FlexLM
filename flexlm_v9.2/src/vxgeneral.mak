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
# $Id: vxgeneral.mak,v 1.3 2002/12/03 21:51:39 kmaclean Exp $
#
# Makefile that holds the general definitions to build the client library 
# for VxWorks.
# This makefile will be included by the platform/cpu specific
# VxWorks makefile.
#
# We assume the following environment variables are set:
#	WIND_BASE
#
# your path must include the gnu tools for the compiler, linker, etc
#

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
EXT		= .out

