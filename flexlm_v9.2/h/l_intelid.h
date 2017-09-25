#ifndef L_INTELID_H
#define L_INTELID_H

/*
 * Types
 */

typedef int (*LM_GET_PSN_PROC)( unsigned long*, unsigned long*, unsigned long* );

#if 0
typedef struct {
	unsigned long high;
	unsigned long mid;
	unsigned long low;
} LM_INTEL_CPU_ID;

/*
 * API
 */

HOSTID* l_intelid lm_args(( LM_HANDLE_PTR, int ));
HOSTID* l_intel_all lm_args(( LM_HANDLE_PTR, int, LM_GET_PSN_PROC ));
#endif


#endif
