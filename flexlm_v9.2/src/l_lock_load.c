#include "lmachdep.h"
#include "lmclient.h"
#include "l_lock_load.h"
#include "l_prot.h"
#include "lsmaster.h"

/*
 * A pointer through which to call FLEXlock.  If it's set, we'll
 * not try to load the DLL, but call statically.
 */

int (*fl_global_variable_api_entry)() = NULL;

/**********************************************************************************
*
* Data structure used to communicate between FLEXlm and the DLL's.  It's defined
* here rather than in a header file to allow version differences.  The DLL's will
* keep copies of all versions of the structure defined here.  The "size" element
* is set to "sizeof(LM_FL_INTERFACE)" which lets the DLL's find the proper structure
* to use on their end.
*
***********************************************************************************/

typedef struct _LM_FL_INTERFACE
{
	int size;
	int cmd;
	LM_HANDLE_PTR job;
	int (*plc_init)( LM_HANDLE_PTR, char*, VENDORCODE_PTR, LM_HANDLE_PTR_PTR );
	void (*plc_free_job)( LM_HANDLE_PTR );
	int (*plc_get_errno)( LM_HANDLE_PTR );
	char* (*plc_errstring)( LM_HANDLE_PTR );
	int (*plc_set_attr)( LM_HANDLE_PTR, int, LM_A_VAL_TYPE );
	void (*plc_set_errno)( LM_HANDLE_PTR, int );
	int (*plc_hostid)( LM_HANDLE_PTR, int, char* );
	int (*plc_cryptstr)( LM_HANDLE_PTR, char*, char**, VENDORCODE_PTR,  int, char*, char** );
	void (*plc_free_mem)( LM_HANDLE_PTR, char* );
	int (*plc_checkout)( LM_HANDLE_PTR, char* const, char* const, int, int, VENDORCODE_PTR const, int );
	void (*plc_checkin)( LM_HANDLE_PTR, char* const, int );
	CONFIG_PTR (*plc_get_config)( LM_HANDLE_PTR, char* );
	CONFIG_PTR (*plc_auth_data)( LM_HANDLE_PTR, char* );
	long (*plc_expire_days)( LM_HANDLE_PTR, CONFIG_PTR );
	int (*pl_install_license)( LM_HANDLE_PTR, char*, char**, int*, char* );
	int (*pl_init_file)( LM_HANDLE_PTR );
	void* (*pl_malloc)( LM_HANDLE_PTR, size_t );
	HOSTID_PTR (*pl_new_hostid)();
	int (*plc_convert)( LM_HANDLE_PTR, char*, char**, char**, int );
	int (*plc_check_key)( LM_HANDLE_PTR, CONFIG_PTR, VENDORCODE_PTR const );
	int argc;
	void** argv;
} LM_FL_INTERFACE;

/*
 * LM_FL_INTERFACE2 is a better version used, for now, only by FLEXlock.
 * When LCM is converted, then the distinction be removed and the older
 * version can be removed from this file.
 */

typedef struct _DICTIONARY_ENTRY
{
	unsigned long name;
	char *value;
} DICTIONARY_ENTRY;

typedef struct _LM_FL_INTERFACE2
{
	int size;
	int argc;
	void** argv;
	DICTIONARY_ENTRY *dict;
} LM_FL_INTERFACE2;

typedef int (*APIPROC)();

typedef struct _workingdata {
	HINSTANCE hlib;
	APIPROC apiproc;
	CONFIG *pconf;
	int retval;
	char feature[MAX_FEATURE_LEN+1];
} WORKINGDATA;

/************************************************************************************
*
* Static dictionary table used for LM_FL_INTERFACE2
*
*************************************************************************************/

static DICTIONARY_ENTRY inf2_dict[] = {
	{ LM_FL_PiLC_INIT_16,			(char*) lc_init },
	{ LM_FL_PvLC_FREE_JOB_4,		(char*) lc_free_job },
	{ LM_FL_PiLC_GET_ERRNO_4,		(char*) lc_get_errno },
	{ LM_FL_PpcLC_ERRSTRING_4,		(char*) lc_errstring },
	{ LM_FL_PiLC_SET_ATTR_12,		(char*) lc_set_attr },
	{ LM_FL_PvLC_SET_ERRNO_8,		(char*) lc_set_errno },
	{ LM_FL_PiLC_HOSTID_12,			(char*) lc_hostid },
	{ LM_FL_PiLC_CRYPTSTR_28,		(char*) lc_cryptstr },
	{ LM_FL_PvLC_FREE_MEM_8,		(char*) lc_free_mem },
	{ LM_FL_PiLC_CHECKOUT_28,		(char*) lc_checkout },
	{ LM_FL_PvLC_CHECKIN_12,		(char*) lc_checkin },
	{ LM_FL_PCONFIG_PTRLC_GET_CONFIG_8,	(char*) lc_get_config },
	{ LM_FL_PCONFIG_PTRLC_AUTH_DATA_8,	(char*) lc_auth_data },
	{ LM_FL_PlLC_EXPIRE_DAYS_8,		(char*) lc_expire_days },
	{ LM_FL_PiL_INSTALL_LICENSE_20,		(char*) l_install_license },
	{ LM_FL_PiL_INIT_FILE_4,		(char*) l_init_file },
	{ LM_FL_PpvL_MALLOC_8,			(char*) l_malloc },
	{ LM_FL_PHOSTID_PTRL_NEW_HOSTID_0,	(char*) l_new_hostid },
	{ LM_FL_PiLC_CONVERT_20,		(char*) lc_convert },
	{ LM_FL_PiLC_CHECK_KEY_12,		(char*) lc_check_key },
	{ LM_FL_PpcL_ASC_HOSTID_8,		(char*) l_asc_hostid },
	{ LM_FL_PLM_SERVER_PTRLC_MASTER_LIST_4,	(char*) lc_master_list },
	{ LM_FL_PpDAEMONL_GET_DLIST_4,		(char*) l_get_dlist },
	{ LM_FL_PCONFIG_PTRLC_NEXT_CONF_12,	(char*) lc_next_conf },
	{ LM_FL_PppcLC_FEAT_LIST_12,		(char*) lc_feat_list },
	{ LM_FL_PiL_SHUTDOWN_28,		(char*) l_shutdown },
	{ LM_FL_PvLC_FREE_LMGRD_STAT_8,		(char*) lc_free_lmgrd_stat },
	{ 0, NULL }
};

/************************************************************************************
*
* Initializes the Inteface structure.  The FLEXlm versions of LCM and FLEXlock DLL's
* include no FLEXlm code themselves.  Rather, then dynamically bind to the callers
* FLEXlm library using the LM_FL_INTERFACE structure.  This keeps the DLL's small,
* and makes the communication more obscure (to prevent cracking).
*
************************************************************************************/

static void InitLmInterface( LM_FL_INTERFACE *I )
{
	I->size = sizeof(LM_FL_INTERFACE);
	I->plc_init = lc_init;
	I->plc_free_job = lc_free_job;
	I->plc_get_errno = lc_get_errno;
	I->plc_errstring = lc_errstring;
	I->plc_set_attr = lc_set_attr;
	I->plc_set_errno = lc_set_errno;
	I->plc_hostid = lc_hostid;
	I->plc_cryptstr = lc_cryptstr;
	I->plc_free_mem = lc_free_mem;
	I->plc_checkout = lc_checkout;
	I->plc_checkin = lc_checkin;
	I->plc_get_config = lc_get_config;
	I->plc_auth_data = lc_auth_data;
	I->plc_expire_days = lc_expire_days;
	I->pl_install_license = l_install_license;
	I->pl_init_file = l_init_file;
	I->pl_malloc = l_malloc;
	I->pl_new_hostid = l_new_hostid;
	I->plc_convert = lc_convert;
	I->plc_check_key = lc_check_key;
}

/************************************************************************************
*
* LCM fulfillment interface
*
************************************************************************************/

static WORKINGDATA Lcmdata;

int lcm_fulfillavailable( LM_HANDLE_PTR job )
{
	//
	// Load the library if it's not already loaded.
	//

	if( NULL == Lcmdata.hlib )
	{
		Lcmdata.hlib = LoadLibrary( "lcmflxA.dll" );
		if( NULL == Lcmdata.hlib )
			return 0;

		Lcmdata.apiproc = (APIPROC) GetProcAddress( Lcmdata.hlib, "LCM_FlexFulfill" );
		if( NULL == Lcmdata.apiproc )
		{
			FreeLibrary( Lcmdata.hlib );
			Lcmdata.hlib = NULL;
			return 0;
		}
	}

	return 1;
}

int lcm_executefulfill( LM_HANDLE_PTR job, char *feature, char *url, char *lic_location )
{
	LM_FL_INTERFACE Ilcm;
	void *argv[10];
	int retval;

	if( NULL == Lcmdata.apiproc )
		return 0;

	//
	// Initialize the api structure, then add in the parts of the
	// structure we're manipulating.
	//

	InitLmInterface( &Ilcm );

	Ilcm.job = job;

	argv[0] = feature;
	argv[1] = url;
	argv[2] = lic_location;
	argv[3] = &retval;

	Ilcm.argv = argv;
	Ilcm.argc = 4;

	//
	// Call the LCM API
	//

	if( !(*Lcmdata.apiproc)( &Ilcm ) )
		return 0;

	return retval;
}

/************************************************************************************
*
* FLEXlock Interface
*
************************************************************************************/

static WORKINGDATA FlApiData;
static LM_FL_INTERFACE2 FlInterface;
static void* Argv[10];

static void callbackA( void* data, CONFIG* confptr )
{
	WORKINGDATA *p = (WORKINGDATA*) data;

	p->pconf = confptr;
}

static void callbackB( void* data, int i )
{
	WORKINGDATA *p = (WORKINGDATA*) data;

	p->retval = i;
}

static int fl_transact( int cmd, LM_HANDLE_PTR job, char *feature, CONFIG **ret_conf )
{
	int i;

	/*
	 * In the DLL, we try to load the library.  In the static
	 * library we just call through a pointer.
	 */

#if defined(FLEX_STATIC)

		if( NULL != fl_global_variable_api_entry )
			FlApiData.apiproc = (APIPROC) fl_global_variable_api_entry;
		else 
		{	// try to load DLL anyway
			FlApiData.hlib = LoadLibrary( "flckflxA.dll" );
			if( NULL == FlApiData.hlib )
				return 0;

			FlApiData.apiproc = (APIPROC) GetProcAddress(
						FlApiData.hlib,
						"FL_FlexAPI" );
			if( NULL == FlApiData.apiproc )
			{
				FreeLibrary( FlApiData.hlib );
				FlApiData.hlib = NULL;
				return 0;
			}
		}

#else

		if( NULL == FlApiData.hlib )
		{
			FlApiData.hlib = LoadLibrary( "flckflxA.dll" );
			if( NULL == FlApiData.hlib )
				return 0;

			FlApiData.apiproc = (APIPROC) GetProcAddress(
						FlApiData.hlib,
						"FL_FlexAPI" );
			if( NULL == FlApiData.apiproc )
			{
				FreeLibrary( FlApiData.hlib );
				FlApiData.hlib = NULL;
				return 0;
			}
		}
#endif

	/*
	 * Initialize the api structure.  We set up the arguments as if
	 * the API were:
	 *
	 * void FL_FlexAPI(
	 *		int command,
	 *		LM_HANDLE*,
	 *		APIPROC cbA,
	 *		APIPROC cbB,
	 *		char* feature,
	 *		void *callbackData );
	 *
	 * It's just that the arguments are passed as part of LM_FL_INTERFACE2
	 * and not actually to the routine.
	 */

	strcpy( FlApiData.feature, feature );

	i = 0;
	Argv[i++] = (void*) cmd;
	Argv[i++] = (void*) job;
	Argv[i++] = (void*) callbackA;
	Argv[i++] = (void*) callbackB;
	Argv[i++] = (void*) FlApiData.feature;
	Argv[i++] = (void*) &FlApiData;

	memset( &FlInterface, 0, sizeof(FlInterface) );
	FlInterface.size = sizeof(FlInterface);
	FlInterface.argc = i;
	FlInterface.argv = Argv;
	FlInterface.dict = inf2_dict;

	/*
	 * Call the FLEXlock API
	 */

	if( !(*FlApiData.apiproc)( &FlInterface ) )
		return 0;

	/*
	 * Return back proper values.
	 */

	*ret_conf = FlApiData.pconf;
	return FlApiData.retval;
}

int fl_available( LM_HANDLE_PTR job, char *feature, CONFIG **ret_conf )
{
	return fl_transact( LM_FL_ISAVAILABLE_CMD, job, feature, ret_conf );
}

int fl_install( LM_HANDLE_PTR job, char* feature, CONFIG **ret_conf )
{
	return fl_transact( LM_FL_INSTALL_CMD, job, feature, ret_conf );
}

int flapi_available( LM_HANDLE_PTR job, char *feature, CONFIG **ret_conf )
{
	return fl_transact( LM_FL_ISAPIAVAILABLE_CMD, job, feature, ret_conf );
}

int flapi_use_available( LM_HANDLE_PTR job, char *feature, CONFIG **ret_conf )
{
	return fl_transact( LM_FL_ISAPIAVAILABLEANDGRANT_CMD, job, feature, ret_conf );
}

int flapi_install( LM_HANDLE_PTR job, char *feature, CONFIG **ret_conf )
{
	return fl_transact( LM_FL_APIINSTALL_CMD, job, feature, ret_conf );
}

void 
fl_freedyndata(void)
{
	CONFIG *confPtr;
	/*
	 * Always give the library a call when we're about to do
	 * this.  If we're in a STATIC build, then there will
	 * be no unload to the DLL, so the library needs to
	 * know this is happening.
	 */

	fl_transact( LM_FL_FINISHED_CMD, NULL, "", &confPtr );

	if( NULL != FlApiData.hlib )
	{
		FreeLibrary( FlApiData.hlib );
		FlApiData.hlib = NULL;
	}
}



