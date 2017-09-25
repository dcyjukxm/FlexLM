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
 *	Module: $Id: odi.h,v 1.3 2003/01/13 23:27:15 kmaclean Exp $
 *
 *	Description: Structures and constants used for getting Ethernet
 *		     address by use Novell Open Datalink Interface
 *		     (ODI) specification.
 *
 *	Chia-Chee Kuan
 *	2/2/95
 *
 *	Last changed:  2/2/95
 *
 */


struct MlidConfigTab {
	BYTE	m_signature[26];	
	BYTE	m_major_ver;
	BYTE	m_minor_ver;
	BYTE	m_NodeAddr[6];
	UINT	m_md_flags;		
	UINT	m_bd_num;
	UINT	m_bd_instance;
	UINT	m_MaxDataSize;
	UINT	m_BestDataSize;
	UINT	m_WrstDataSize;
	BYTE *	m_CardName;
	BYTE *	m_ShortName;
	BYTE *	m_MediaType;
	UINT	reserved1;      
	UINT	m_MediaID;
	UINT	m_TransportTime;
	DWORD	m_SourceRouteHandler;
	UINT	m_LookAheadSize;
	UINT	m_LineSpeed;
	char	m_reserved2[8];
	BYTE	m_m_MajorVersion;
	BYTE	m_m_MinorVersion;
	UINT	m_flags;
	UINT	m_SendRetries;
	char *	m_Link;
	UINT	m_ShareFlag;
	UINT	m_Slot;
	UINT	m_IOAddress1;
	UINT	m_IORange1;
	UINT	m_IOAddress2;
	UINT	m_IORange2;
	DWORD	m_MemoryAddress1;
	UINT	m_MemorySize1;
	DWORD	m_MemoryAddress2;
	UINT	m_MemorySize2;
	BYTE	m_IntLine1;
	BYTE	m_IntLine2;
	BYTE	m_DMALine1;
	BYTE	m_DMALine2;
};

typedef int  (*FUNCTION_PTR)();

struct LslEntryTable {
	FUNCTION_PTR	l_init_func;
	FUNCTION_PTR	l_mlid_supp_func;
	FUNCTION_PTR	l_prot_stack_func;
	FUNCTION_PTR	l_gen_serv_func;
};

