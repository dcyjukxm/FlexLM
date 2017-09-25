/******************************************************************************

	    COPYRIGHT (c) 1996 ,2003 by Macrovision Corporation.
	This software has been provided pursuant to a License Agreement
	containing restrictions on its use.  This software contains
	valuable trade secrets and proprietary information of 
	Macrovision Corporation and is protected by law.  It may 
	not be copied or distributed in any form or medium, disclosed 
	to third parties, reverse engineered or used in any manner not 
	provided for in said License Agreement except with the prior 
	written authorization from Macrovision Corporation.

 *****************************************************************************/
/*	
 *	Module: $Id: lcommdbg.h,v 1.6 2003/01/13 22:13:11 kmaclean Exp $
 *
 *	D. Birns
 *	1/23/95
 *
 *	Last changed:  1/23/96
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
static struct _msgs{
    char 	*transl;
    char	code;
} msgs[] = 
{
	{"LM_BORROW", '!'},
	{"LM_BORROW2", '('},
	{"LM_BUSY", 		'B'},
	{"LM_BUSY_NEW", 	'D'},
	{"LM_CHALLENGE", ')'	},
	{"LM_CHECKIN", 'i'},
	{"LM_CHECKOUT", 'o'},
	{"LM_CLOCKSETTING", '*'},
	{"LM_COMM_UNAVAILABLE", 'E'},
	{"LM_DAEMON", 'b'},
	{"LM_DETACH", 'f'	},
	{"LM_DISOWN", 'c'	},
	{"LM_DOREMOVEALL", 'z'	},
	{"LM_DOTELL", 'x'	},
	{"LM_DUMP", 	'd'},
	{"LM_FEATURE_AVAILABLE", 'F'},
	{"LM_FEATURE_LINE", 	'Y'},
	{"LM_FEATURE_LINE2", 'Z'},
	{"LM_HANDSHAKE", 'a'},
	{"LM_HEARTBEAT", 'g'},
	{"LM_HEARTBEAT_RESP", 'G'},
	{"LM_HELLO", 'h'},
	{"LM_HELLO_THIS_VD", 		'`'},
	{"LM_HOSTID", '~'	},
	{"LM_IS_MASTER", 'e'	},
	{"LM_I_MASTER", 'm'	},
	{"LM_LF_DATA", 	'L'},
	{"LM_LIST ", 'l'},
	{"LM_LOG", 	'j'},
	{"LM_MASTER_READY", 'r'	},
	{"LM_NEED_HOSTID", 	'T'},
	{"LM_NEWSERVER", 'n'	},
	{"LM_NOT_ADMIN", 	'H'	},
	{"LM_NO_CONFIG_FILE", 'C'},
	{"LM_NO_SUCH_FEATURE", '?'},
	{"LM_NO_USER", 	'J'	},
	{"LM_NUSERS", 	'N'},
	{"LM_OK", 		'O'},
	{"LM_ORDER", 'v'	},
	{"LM_PID", 	'p'	},
	{"LM_QUEUED", 	'Q'},
	{"LM_REMOVE", 'u'},
	{"LM_REMOVEH", '#'},
	{"LM_REPFILE", '_'},
	{"LM_REREAD", '&'},
	{"LM_RESTART", 'k'	},
	{"LM_SEND_LF", '$'},
	{"LM_SEND_SEEDS", '-'},
	{"LM_SHELLO", 's'	},
	{"LM_SHELLO_OK", 	'S'},
	{"LM_SHUTDOWN", 'q'},
	{"LM_SINCE", '%'},
	{"LM_SWITCH", 'w'},
	{"LM_SWITCH_REPORT", '^'},
	{"LM_TELLME", 't'},
	{"LM_TOO_SOON", 	'I'	},
	{"LM_TRY_ANOTHER", 	'A'},
	{"LM_USERNAME", 	'U'},
	{"LM_USERNAME2", 	'X'},
	{"LM_U_MASTER", 'y'	},
	{"LM_VD_FEAT_BUNDLE_1", 'M'},
	{"LM_VD_GEN_INFO", 'P'},
	{"LM_VENDOR_DAEMON_INFO", '='	},
	{"LM_VENDOR_MSG", '@'},
	{"LM_VENDOR_RESP", 	'V'},
	{"LM_VHELLO", '+'	},
	{"LM_WHAT", 		'W'},
	0,
	};
