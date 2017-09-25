/******************************************************************************

	    COPYRIGHT (c) 1990, 2003 by Macrovision Corporation.
	This software has been provided pursuant to a License Agreement
	containing restrictions on its use.  This software contains
	valuable trade secrets and proprietary information of 
	Macrovision Corporation and is protected by law.  It may 
	not be copied or distributed in any form or medium, disclosed 
	to third parties, reverse engineered or used in any manner not 
	provided for in said License Agreement except with the prior 
	written authorization from Macrovision Corporation.

 *****************************************************************************/
/******************************************************************************
 *
 *
 *	NOTE:	The purchase of FLEXlm source does not give the purchaser
 *
 *		the right to run FLEXlm on any platform of his choice.
 *
 *		Modification of this, or any other file with the intent 
 *
 *		to run on an unlicensed platform is a violation of your 
 *
 *		license agreement with Macrovision Corporation. 
 *
 *
 *****************************************************************************/
/*	
 *	Module: $Id: l_ether.h,v 1.3 2003/01/13 23:27:04 kmaclean Exp $
 *
 *	Description: 	Use NDIS or Packet Driver Specification to get
 *			Ethernet Address on Windows.
 *
 *	Chia-Chee Kuan
 *	9/27/94
 *
 *	Last changed:  10/3/94
 *
 */

/*
 *	DPMI definitions
 */
#define INT_DPMI	0x31	/* Software interrupt number for DPMI */

/* ID's for DPMI function calls */
#define	DPMI_ALLOCATE_LDT			0x0000
#define	DPMI_FREE_LDT				0x0001
#define DPMI_SEG_TO_DESC			0x0002
#define DPMI_SET_SEG_BASE			0x0007
#define DPMI_SET_SEG_LIMIT			0x0008

/*
 *	Standard DPMI real mode register structure.
 */
typedef struct tagREALMODEREG {
    DWORD rmEDI, rmESI, rmEBP, Reserved, rmEBX, rmEDX, rmECX, rmEAX;
    WORD  rmCPUFlags, rmES, rmDS, rmFS, rmGS, rmIP, rmCS, rmSP, rmSS;
} REALMODEREG, FAR *LPREALMODEREG;


/*
 *	Packet Driver Definitions
 */
#define PKTDRV_SIGNATURE	"PKT DRVR"	/* Packet driver signature */
#define PKTDRV_INT_START	0x60
#define PKTDRV_INT_END		0x80

/* Packet driver interface types */
#define	ANYTYPE		0xFFFF

/* Packet driver function call numbers. From Appendix B. */
#define	DRIVER_INFO		1
#define	ACCESS_TYPE		2
#define	RELEASE_TYPE		3
#define	SEND_PKT		4
#define	TERMINATE		5
#define	GET_ADDRESS		6
#define	RESET_INTERFACE		7
#define GET_PARAMETERS		10
#define AS_SEND_PKT		11
#define	SET_RCV_MODE		20
#define	GET_RCV_MODE		21
#define	SET_MULTICAST_LIST	22
#define	GET_MULTICAST_LIST	23
#define	GET_STATISTICS		24
#define SET_ADDRESS		25

/* Packet driver interface classes */
#define	CL_NONE		0
#define	CL_ETHERNET	1
#define	CL_PRONET_10	2
#define	CL_IEEE8025	3
#define	CL_OMNINET	4
#define	CL_APPLETALK	5
#define	CL_SERIAL_LINE	6
#define	CL_STARLAN	7
#define	CL_ARCNET	8
#define	CL_AX25		9
#define	CL_KISS		10
#define CL_IEEE8023	11
#define CL_FDDI 	12
#define CL_INTERNET_X25 13
#define CL_LANSTAR	14
#define CL_SLFP 	15
#define	CL_NETROM	16
#define NCLASS		17

/*
 *	NDIS Portion
 */

/*
 *	Parameter block for making Protocol Manager Calls
 */
typedef struct ReqBlock {
	unsigned rb_opcode;
	unsigned rb_status;
	char	 *rb_ptr1;
	char	 *rb_ptr2;
	unsigned rb_word;
} ProtmanReqBlock;

/* Common Characteristic Table (CCT) */
typedef struct CommonCharTable {
	unsigned	cct_tab_size;	/* size of the table 		*/
	unsigned	cct_level;	/* level of the table		*/
	unsigned	cct_level_sub;	/* level of the subtable	*/
	unsigned char 	cct_major_ver;	/* major version number		*/
	unsigned char 	cct_minor_ver;	/* minor version number		*/
	unsigned long	cct_func_flags;	/* module function flags	*/
	char 		cct_name[16];	/* module name			*/
	unsigned char	cct_level_up;	/* prot level at upper boundary */
	unsigned char 	cct_type_up;	/* type of IF at upper boundary */
	unsigned char 	cct_level_low;	/* prot level at lower boundary */
	unsigned char 	cct_type_low;	/* type of IF at lower boundary */
	unsigned	cct_module_id;	/* module ID			*/
	unsigned	cct_module_ds;	/* module DS			*/
	void		(*cct_sys_req)();	/* system request dispatch entry*/
	char		*cct_ssct;	/* service specific char table  */
	char		*cct_ssst;	/* service specific status table*/
	char		*cct_udt;	/* upper dispatch table		*/
	char		*cct_ldt;	/* lower dispatch table		*/
	char		*cct_nul1; 	/* resvered			*/
	char		*cct_nul2; 	/* resvered			*/
} CCT;


/* MAC Service Specific Characteristic table  */
typedef struct MacServiceSpecTable {
	unsigned	sst_tab_size;	/* size of the table 		*/
	char		sst_type[16];	/* type name of MAC		*/
	unsigned	sst_len_addr;	/* length of station address	*/
	char unsigned	sst_perm_addr[16]; /* permanent station address */
	char unsigned	sst_curr_addr[16]; /* current station address	*/
	long		sst_func_addr;	/* current functional address	*/
	char		*sst_multicast;	/* multicast address list	*/
	unsigned long	sst_link_speed;	/* link speed bits/sec		*/
	unsigned long	sst_flags;	/* service flags		*/
	unsigned	sst_max_frame;	/* max frame size		*/
	unsigned long	sst_tx_buf;	/* transmission buffer capicity */
	unsigned	sst_tx_block;	/* tx buffer block size		*/
	unsigned long	sst_rv_buf;	/* reception buffer capicity	*/
	unsigned	sst_rv_block;	/* reception block size		*/
	char		sst_ieee_vcode[3]; /* IEEE vender code 		*/
	char		sst_v_adp_code; /* vender adapter code		*/
	char		*sst_v_desc;	/* vender adapter description	*/
	unsigned	sst_int_level;	/* interrupt level		*/
} MacSST;

/*
 *	Bind Status structure
 */
typedef struct BindNode
{
	CCT		*bn_CCT;
	struct BindNode *bn_down;
	struct BindNode *bn_right;
} BindNode;

