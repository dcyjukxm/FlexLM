#ifndef L_LOCK_LOAD_H
#define L_LOCK_LOAD_H

int lcm_fulfillavailable( LM_HANDLE_PTR job );
int lcm_executefulfill( LM_HANDLE_PTR job, char *feature, char *url, char *lic_location );
int fl_available( LM_HANDLE_PTR job, char *feature, CONFIG **ret_conf );
int fl_install( LM_HANDLE_PTR job, char* feature, CONFIG **ret_conf );
int flapi_available( LM_HANDLE_PTR job, char *feature, CONFIG **ret_conf );
int flapi_use_available( LM_HANDLE_PTR job, char *feature, CONFIG **ret_conf );
int flapi_install( LM_HANDLE_PTR job, char *feature, CONFIG **ret_conf );
void fl_freedyndata(void);

/******************************************************************************
 *
 * FLEXlock Interface 2
 *
 *         T H I S   I S   A   C O P Y !
 *
 * For full comments, see the mirrored file in the FLEXlock or CFC directory.
 *
 * NEVER change any of these values!  The whole idea of the interface is
 * to add values.  This way old things will always be compatible.
 *
 ******************************************************************************/
 
#define LM_FL_ISAVAILABLE_CMD 1
#define LM_FL_INSTALL_CMD 2
#define LM_FL_ISAPIAVAILABLE_CMD 25
#define LM_FL_ISAPIAVAILABLEANDGRANT_CMD 71
#define LM_FL_APIINSTALL_CMD 92
#define LM_FL_FINISHED_CMD 13

#define LM_FL_PiLC_INIT_16		0x04000210
#define LM_FL_PvLC_FREE_JOB_4		0x00000304
#define LM_FL_PiLC_GET_ERRNO_4		0x04000404
#define LM_FL_PpcLC_ERRSTRING_4		0x41000504
#define LM_FL_PiLC_SET_ATTR_12		0x0400060C
#define LM_FL_PvLC_SET_ERRNO_8		0x00000708
#define LM_FL_PiLC_HOSTID_12		0x0400080C
#define LM_FL_PiLC_CRYPTSTR_28		0x0400091C
#define LM_FL_PvLC_FREE_MEM_8		0x00000A08
#define LM_FL_PiLC_CHECKOUT_28		0x04000B1C
#define LM_FL_PvLC_CHECKIN_12		0x00000C0C
#define LM_FL_PCONFIG_PTRLC_GET_CONFIG_8 0x21000D08
#define LM_FL_PCONFIG_PTRLC_AUTH_DATA_8	0x21000E08
#define LM_FL_PlLC_EXPIRE_DAYS_8	0x05000F08
#define LM_FL_PiL_INSTALL_LICENSE_20	0x04001014
#define LM_FL_PiL_INIT_FILE_4		0x04001104
#define LM_FL_PpvL_MALLOC_8		0x40001208
#define LM_FL_PHOSTID_PTRL_NEW_HOSTID_0	0x22001300
#define LM_FL_PiLC_CONVERT_20		0x04001414
#define LM_FL_PiLC_CHECK_KEY_12		0x0400150C
#define LM_FL_PpcL_ASC_HOSTID_8		0x41001608
#define LM_FL_PLM_SERVER_PTRLC_MASTER_LIST_4 0x24001704
#define LM_FL_PpDAEMONL_GET_DLIST_4	0x23001804
#define LM_FL_PCONFIG_PTRLC_NEXT_CONF_12 0x2100190C
#define LM_FL_PppcLC_FEAT_LIST_12	0x4A001A0C
#define LM_FL_PiL_SHUTDOWN_28		0x04001B1C
#define LM_FL_PvLC_FREE_LMGRD_STAT_8	0x00001C08

#endif



