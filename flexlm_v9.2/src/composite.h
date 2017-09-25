/******************************************************************************

	    COPYRIGHT (c) 2003 by Macrovision Corporation
	This software has been provided pursuant to a License Agreement
	containing restrictions on its use.  This software contains
	valuable trade secrets and proprietary information of 
	Macrovision Corporation and is protected by law.  It may 
	not be copied or distributed in any form or medium, disclosed 
	to third parties, reverse engineered or used in any manner not 
	provided for in said License Agreement except with the prior 
	written authorization from Macrovision Corporation

 *****************************************************************************/
/*	
 *	Module: $Id: composite.h,v 1.2 2003/03/10 17:24:21 sluu Exp $
 *
 *	Description:  Composite hostid data structures.
 *
 *	Last changed:  06 Mar 2003
 *	J. Wong
 *
 */
#ifndef INCLUDE_COMPOSITE_H
#define INCLUDE_COMPOSITE_H

#include "lmclient.h"

typedef struct _lm_composite_info{
	int id_type;                    /* HOSTID_ defines */
	int weight;                     /* how important is this id */
	char *id_mask;			/* wildcards to match host id */
} LM_COMPOSITEID_INFO;

typedef struct _composit_id_info {
     int id_count;
     LM_COMPOSITEID_INFO *info_list;
} COMPOSITE_ID_INFO;     

#endif
