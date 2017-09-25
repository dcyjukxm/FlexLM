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
# $Id: vx_ppc860.mak,v 1.1 2002/12/11 18:41:02 kmaclean Exp $
#
# Makefile that holds the definitions to build the client library for VxWorks.
# This makefile will be included by the real makefile
#
# only the things specific to VxWorks simpc are defined here .
# General VxWorks settings are in vxgeneral.mak
#
# We assume the following environment variables are set:
#	WIND_BASE
#
# your path must include the gnu tools for the compiler, linker, etc
#
# GPLATFORM is used to make the cpu specific directory and to include the proper
# cpu specific makefile. As well as in the code.
# In the older makefiles this is derived by calling the gplatform script
#
# the main makefile should be called with GPLATFORM=vx_simpc

# VxWorks CPU identifier
CPU		= PPC860
# tell the compiler which cpu to build for
CPUFLAG		= -c -mcpu=860
# used as the suffix for the gnu tool names
REAL_TOOL	= ppc	
VXLDFLAGS	= 

include vxgeneral.mak
