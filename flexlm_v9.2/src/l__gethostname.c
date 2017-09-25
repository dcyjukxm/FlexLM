/******************************************************************************
#
#           COPYRIGHT (c) 1994, 2003  by Macrovision Corporation.
#       This software has been provided pursuant to a License Agreement
#       containing restrictions on its use.  This software contains
#       valuable trade secrets and proprietary information of 
#       Macrovision Corporation and is protected by law.  It may 
#       not be copied or distributed in any form or medium, disclosed 
#       to third parties, reverse engineered or used in any manner not 
#       provided for in said License Agreement except with the prior 
#       written authorization from Macrovision Corporation.
#
#*****************************************************************************
#       
#       Module: $Id: l__gethostname.c,v 1.2 2003/01/13 22:41:56 kmaclean Exp $
#
#       Function:
#
#       Description: 
#
#       Parameters:
#
#       Return:
#
#       Blane Eisenberg
#       1/17/93
#
#       Last changed:  9/25/98
#
*/


#include <winsock2.h>
int PASCAL FAR flex_gethostname (char FAR * name, int namelen);

int PASCAL FAR l__gethostname(char FAR * name, int namelen)
{
        return flex_gethostname(name,namelen);
}

