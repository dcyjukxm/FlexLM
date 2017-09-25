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
 *	Module: $Id: l_ether.c,v 1.5 2003/04/18 23:48:02 sluu Exp $
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
 *	Module: $Id: l_ether.c,v 1.5 2003/04/18 23:48:02 sluu Exp $
 *
 *	Description: 	Use NDIS or Packet Driver Specification to get
 *			Ethernet Address on Windows.  DOS Protected Mode
 *			Interface (DPMI) is used as the mechanism to accompilsh
 *			Windows/DOS interface.
 *
 *	Chia-Chee Kuan
 *	9/27/94
 *
 *	Last changed:  1/7/97
 *
 */
#include <windows.h>
#include <dos.h>
#include <fcntl.h>
#include <io.h>
#include <string.h>
#include "l_ether.h"
#include "flex_file.h"
#include "odi.h"
#include "ncb.h"
extern BOOL RealInt (BYTE intnum, LPREALMODEREG lpRealModeReg);
extern char * getenv();
#include <stdio.h>
/*
 *	Allocate a protected mode selector, sets its base address and limit.
 */
int alloc_LDT_descriptor(char *seg_base_addr,unsigned seg_limit )
{
	union REGS regs;
	struct SREGS sregs;
	int selector;
    
	regs.x.ax = DPMI_ALLOCATE_LDT;
	regs.x.cx = 1;			// Allocate only one descriptor
    
	segread(&sregs);
	int86x( INT_DPMI, &regs, &regs, &sregs );

	if ( regs.x.cflag )
		return -1;

	selector = regs.x.ax;

	// set the selectors segment base address 
	regs.x.ax = DPMI_SET_SEG_BASE;
	regs.x.bx = selector;
	regs.x.cx = FP_SEG( seg_base_addr ) >> 12;
	regs.x.dx = FP_OFF(seg_base_addr) + (FP_SEG(seg_base_addr) << 4);
    
	segread(&sregs);    
	int86x( INT_DPMI, &regs, &regs, &sregs );
    
	if ( regs.x.cflag )
		return -1;
    
	// set the selector segmenet limit 
	regs.x.ax = DPMI_SET_SEG_LIMIT;
	regs.x.bx = selector;
	regs.x.cx = 0;
	regs.x.dx = seg_limit;		

	segread(&sregs);    
	int86x( INT_DPMI, &regs, &regs, &sregs );
    
	if ( regs.x.cflag )
		return -1;

	return selector;
}

/*
 *	Free a previously allocated LDT descriptor.
 */
int free_LDT_descriptor( unsigned selector )
{
	union REGS regs;
	struct SREGS sregs;
    
	regs.x.ax = DPMI_FREE_LDT;
	regs.x.bx = selector;

	segread(&sregs);    
	int86x( INT_DPMI, &regs, &regs, &sregs );
        return 0;
}

/*
 *	Call a real mode procedure through DPMI.  REALMODEREG sets the
 *	real mode procedure register environment includeing CS:IP.
 *	Return 0 (FALSE) when the call is successful.
 */
DPMI_call_real_mode_proc( REALMODEREG *rmReg )
{
	int rmRegSeg = FP_SEG(rmReg);
	int rmRegOff = FP_OFF(rmReg);
	BOOL dpmi_failed = TRUE;
	
	_asm
	{
		push	es		;save ES:DI
		push	di
				
		mov	es,rmRegSeg	;set ES:DI to real mode register struct
		mov	di,rmRegOff
		mov	ax, 301H	;Call real mode proc with far ret frame
		mov	bx, 0		;no flags
		mov	cx, 0		;no words to be copied to call stack
		int	31H		;Do the DMPI request

		jc	failed
		mov	dpmi_failed,0
failed:
		pop	di		;restore ES:DI
		pop	es
		
	}
	return dpmi_failed;
}

unsigned pktdrv_access_type(unsigned pktdrv_int)
{
	REALMODEREG rmReg;
	DWORD SegSelector;
	unsigned *pmDosMem;
	unsigned *rmDosMem;

	SegSelector = GlobalDosAlloc( 4 );
	pmDosMem = MK_FP( LOWORD(SegSelector), 0 );
	rmDosMem = MK_FP( HIWORD(SegSelector), 0 );

	memset(&rmReg, 0, sizeof(REALMODEREG));
	rmReg.rmEDX = 0;		/* DL = interface id number*/
	rmReg.rmDS = FP_SEG(rmDosMem); 	/* DS:SI = Packet type template */
	rmReg.rmESI = 0;
	*pmDosMem = 0x0789;		/* an unused Ethernet type number */

	rmReg.rmECX = 2;		/* CX = Length of type */
	rmReg.rmES = 0;			/* ES:DI = Address of receive handler*/
	rmReg.rmEDI = 0;	
	rmReg.rmEBX = ANYTYPE;		/* BX = Type */

	/* AH = ACCESS_TYPE function id, and AL = Class */
	rmReg.rmEAX = (DWORD)((ACCESS_TYPE<<8)|CL_ETHERNET); 
	
	if ( RealInt ((BYTE)pktdrv_int, &rmReg) )
	{
		GlobalDosFree( LOWORD(SegSelector) );
		return LOWORD(rmReg.rmEAX);	/* return the handle */
	}
	else
	{
		GlobalDosFree( LOWORD(SegSelector) );	
		return (unsigned) -1;
	}
}

void
pktdrv_release_type(unsigned pktdrv_int, unsigned handle)
{
	REALMODEREG rmReg;

	memset(&rmReg, 0, sizeof(REALMODEREG));
	rmReg.rmEBX = handle;
	rmReg.rmEAX = (DWORD)(RELEASE_TYPE<<8); /* AH = RELEASE_TYPE */
	
	RealInt((BYTE)pktdrv_int, &rmReg);
}

BOOL
pktdrv_get_address(unsigned pktdrv_int, int handle, char *buff, unsigned len)
{
	REALMODEREG rmReg;
	DWORD SegSelector;
	unsigned *pmDosMem;
	unsigned *rmDosMem;

	/*
	 *	Allocate DOS memory to receive Ethernet address from
	 *	Packet Driver.
	 */
	SegSelector = GlobalDosAlloc( len );
	pmDosMem = MK_FP( LOWORD(SegSelector), 0 );
	rmDosMem = MK_FP( HIWORD(SegSelector), 0 );

	memset(&rmReg, 0, sizeof(REALMODEREG));
	rmReg.rmES = FP_SEG(rmDosMem);
	rmReg.rmEDI = FP_OFF(rmDosMem);
	rmReg.rmECX = len;
	rmReg.rmEBX = handle;
	rmReg.rmEAX = (DWORD)(GET_ADDRESS<<8); /* AH = GET_ADDRESS */
	
	if ( RealInt((BYTE)pktdrv_int, &rmReg) )
	{
		/* DPMI call succeeded. */
		memcpy( buff, pmDosMem, len );
		GlobalDosFree( LOWORD(SegSelector) );
		return TRUE;
	}
	
	GlobalDosFree( LOWORD(SegSelector) );	
	return FALSE;
}
    
/*
 *	Get Ethernet MAC address by using Packet Driver Specification and DPMI.
 */
BOOL get_pktdrv_ether_addr(char *buff, int len)
{
	int selector_int_table;	// selector for real mode s/w int table
	unsigned pktdrv_int, pktdrv_selector, pktdrv_handle;
	unsigned *cp;
	char *pktdrv_start;
	BOOL bSucceeded;
	
	/*
	 * Allocate LDT descriptor to access real mode s/w interrupt table.
	 * We will search for Packet Driver signature from interrupt
	 * PKTDRV_INT_START to PKTDRV_INT_END.
	 */
	selector_int_table = alloc_LDT_descriptor( (char far *)0,
			   (PKTDRV_INT_END+1)<< 2);
    
	if ( selector_int_table == -1 )
		return FALSE;

	for ( pktdrv_int = PKTDRV_INT_START; pktdrv_int <= PKTDRV_INT_END;
								pktdrv_int++ )
	{
		cp = MK_FP( selector_int_table, (pktdrv_int<<2) );

		pktdrv_selector =
			alloc_LDT_descriptor(MK_FP(*(cp+1), 0),0xffff);

		if ( pktdrv_selector == -1 )
			return FALSE;
	
		FP_SEG( pktdrv_start ) = pktdrv_selector;
		FP_OFF( pktdrv_start ) = *cp;
		if ( !strncmp(PKTDRV_SIGNATURE, (char *)pktdrv_start+3,
				      strlen(PKTDRV_SIGNATURE) ) )
		{
			free_LDT_descriptor( pktdrv_selector );		
			break;
		}
		else
			free_LDT_descriptor( pktdrv_selector );
	}

	free_LDT_descriptor( selector_int_table );

	if ( pktdrv_int > PKTDRV_INT_END )
	{
	       // Packet Driver was not found
	       return FALSE;
        }

	/*
	 * Now we know packet driver is installed at interrupt 'pktdrv_int'.
	 * Get ethernet address by first getting a handle using access_type
	 * function followed by get_address function.
	 */
	pktdrv_handle = pktdrv_access_type( pktdrv_int );
	if ( pktdrv_handle == (unsigned) -1 )
		return FALSE;
	
	bSucceeded = pktdrv_get_address(pktdrv_int, pktdrv_handle, buff, len);
	pktdrv_release_type( pktdrv_int, pktdrv_handle );
	return bSucceeded;	
}

/*
 *	NDIS functions
 */
BindNode *GetBindingStatus( unsigned hProtman )
{
	REALMODEREG rmReg;
	DWORD SegSelector;
	ProtmanReqBlock *pmRB;
	ProtmanReqBlock *rmRB;
	BindNode *pRoot;
    
	/*
	 *	Allocate DOS memory to use for parameter block making Protman
	 *	function call
	 */
	SegSelector = GlobalDosAlloc( sizeof(ProtmanReqBlock) );
	pmRB = MK_FP( LOWORD(SegSelector), 0 );
	rmRB = MK_FP( HIWORD(SegSelector), 0 );

	memset( pmRB, 0, sizeof(ProtmanReqBlock) );
	pmRB->rb_opcode = 9;		/* Get Binding Status OP code */

	/*
	 *	Prepare to call Protman via DOS IOCTL via DPMI
	 */
	memset(&rmReg, 0, sizeof(REALMODEREG));
	rmReg.rmDS = FP_SEG(rmRB);
	rmReg.rmEDX = FP_OFF(rmRB);
	rmReg.rmECX = sizeof(sizeof(ProtmanReqBlock)) ;
	rmReg.rmEBX = hProtman;
	rmReg.rmEAX = (DWORD)((0x44<<8) | 0x2); /* AH = IOCTL = 0x44 */
						/* AL = 'READ' = 0x2 */
	
	if ( !RealInt((BYTE)0x21, &rmReg) )
	{
		GlobalDosFree( FP_SEG(pmRB) );
		return 0;
	}

	if ( pmRB->rb_status )
		return 0;

	pRoot = (BindNode *)pmRB->rb_ptr1;
	GlobalDosFree( FP_SEG(pmRB) );	
		
	return pRoot;
}

/*
 *	Get Ethernet MAC address by using NDIS and DPMI.
 */
BOOL get_ndis_ether_addr(char *buff, int len)
{
	int hProtman;		/* protocol manager file handle */
	BindNode *rmBindNode, *pmBindNode;
	CCT *rmCCT, *pmCCT;
	MacSST *rmMacSST, *pmMacSST;
	
	/*
	 *	(1) Open Protocol Manager as a device
	 */
	if ( (hProtman = l_flexOpen( "PROTMAN$", _O_RDONLY|_O_BINARY)) == -1 )
		return FALSE;

	/* Get current binding status from Protocol Manager */
	if ( !(rmBindNode=GetBindingStatus(hProtman)) )
		return FALSE;

	/* Set up pmBindNode ptr which we can use in Windows protected mode. */
	FP_SEG(pmBindNode) = alloc_LDT_descriptor(
		(char *)((DWORD)rmBindNode&(DWORD)0xffff0000), 0xffff );
	FP_OFF(pmBindNode) = FP_OFF(rmBindNode);

	/* Get the real mode address of the first Mac module */
	rmBindNode = pmBindNode->bn_down;
	free_LDT_descriptor( FP_SEG(pmBindNode) );

	if ( !rmBindNode )
		return FALSE;
	
	/* Convert again the rmBindNode address to protected address */
	FP_SEG(pmBindNode) = alloc_LDT_descriptor(
		(char *)((DWORD)rmBindNode&(DWORD)0xffff0000), 0xffff );
	FP_OFF(pmBindNode) = FP_OFF(rmBindNode);

	/* Get CCT pointer from the BindNode */
	rmCCT = pmBindNode->bn_CCT;
	free_LDT_descriptor( FP_SEG(pmBindNode) );

	if ( !rmCCT )
		return FALSE;
	
	/* Convert real mode CCT address to protected mode address */
	FP_SEG(pmCCT) = alloc_LDT_descriptor(
		(char *)((DWORD)rmCCT&(DWORD)0xffff0000), 0xffff );
	FP_OFF(pmCCT) = FP_OFF(rmCCT);

	/* Get Mac Service Specific Table from CCT */
	rmMacSST = (MacSST *)pmCCT->cct_ssct;
	free_LDT_descriptor( FP_SEG(pmCCT) );

	if ( !rmMacSST )
		return FALSE;
	
	/* Convert the read mode MacSST address to protect mode */
	FP_SEG(pmMacSST) = alloc_LDT_descriptor(
		(char *)((DWORD)rmMacSST&(DWORD)0xffff0000), 0xffff );
	FP_OFF(pmMacSST) = FP_OFF(rmMacSST);

	memcpy( buff, pmMacSST->sst_perm_addr, len );
	free_LDT_descriptor( FP_SEG(pmMacSST) );
	
	close( hProtman );
	return TRUE;
}

/*
 *	Get Ethernet MAC address by using ODI Specification and DPMI.
 */
BOOL get_odi_ether_addr(char *buff, int len)
{
	struct MlidConfigTab	*rmMlidConfTabPtr, *pmMlidConfTabPtr;
	FUNCTION_PTR		 MlidCtrlEntryFunc;
	struct LslEntryTable	*pmLslEntryTab, *rmLslEntryTab;
	unsigned index;
	REALMODEREG rmReg;
	DWORD SegSelector;
	FUNCTION_PTR  *tmp;
	
	/*
	 *	(0) Allocate LslEntryTab in DOS real mode accessible area.
	 *	    Set the protected and real mode pointer pmLslEntryTab
	 *	    and rmLslEntryTab.
	 */
	SegSelector = GlobalDosAlloc( sizeof(struct LslEntryTable) );
	pmLslEntryTab = MK_FP( LOWORD(SegSelector), 0 );
	rmLslEntryTab = MK_FP( HIWORD(SegSelector), 0 );

	
	/*
	 *	(1) Sarch for LSL.COM and its init entry function address
	 */
	memset(&rmReg, 0, sizeof(REALMODEREG));	
	for ( index = 0xC000; index <= 0xC0ff; index++ )
	{
		rmReg.rmEAX = index;
	
		RealInt((BYTE)0x2f, &rmReg);	
		if ( LOBYTE(LOWORD(rmReg.rmEAX)) == 0xff )
		{
			/*
			 *	Found something here, check its signature.
			 *	Real mode address for LSL signature is in
			 *	real mode register ES:SI
			 */
			int  sig_selector; 

			/* create a selector to access to signature */
			sig_selector = 
			    alloc_LDT_descriptor(MK_FP(LOWORD(rmReg.rmES),0),
								0xffff);

			if ( !sig_selector )
				continue;
	
			if ( !stricmp(MK_FP(sig_selector,LOWORD(rmReg.rmESI)),
							      "LINKSUP$" ) )
			{
				/*
				 *	Signature matched!  Get LSL init
				 *	fucntion address.
				 */			
				pmLslEntryTab->l_init_func =
					MK_FP(rmReg.rmES, LOWORD(rmReg.rmEBX));
				free_LDT_descriptor( sig_selector );		
				break;
			}
			else
				free_LDT_descriptor( sig_selector );
		}
	}

	if ( index > 0xC0ff )
	{
		/* LSL not found! */
		GlobalDosFree( LOWORD(SegSelector) );
		return FALSE;
	}

	/*
	 *	(2) Fill in the rest of the LSL entry pointer table by using
	 *	    the LSL initialization function address retrieved in (1)
	 */
	memset(&rmReg, 0, sizeof(REALMODEREG));	
	rmReg.rmEBX = 2;			/* 2 = GET_ENTRY_POINTS     */
	rmReg.rmES = FP_SEG(rmLslEntryTab);	/* ES:DI ->LSL Entry Table  */
	rmReg.rmEDI = FP_OFF(rmLslEntryTab);
	rmReg.rmDS  = FP_SEG(rmLslEntryTab);	/* DS:SI ->l_prot_stk_func */
	tmp = &(rmLslEntryTab->l_prot_stack_func);
	rmReg.rmESI = FP_OFF(tmp);	      
	rmReg.rmCS = FP_SEG(pmLslEntryTab->l_init_func);	/* Set CS:IP */
	rmReg.rmIP = FP_OFF(pmLslEntryTab->l_init_func);		     
	
	/*
	 *	DO DPMI call to LSL init function
	 */
	if ( DPMI_call_real_mode_proc( &rmReg ) )
	{
		GlobalDosFree( LOWORD(SegSelector) );	
		return FALSE;
	}

	/*
	 *	(3) Get MLID Control entry point using the l_prot_stack_func
	 *	    in LslEntryTab retrieved in (2)
	 */
	memset(&rmReg, 0, sizeof(REALMODEREG));
	rmReg.rmEBX = 18;			/* 18 = GETMLIDCONTROLENTRY */
	rmReg.rmEDX = 0x12;
	rmReg.rmCS = FP_SEG(pmLslEntryTab->l_prot_stack_func);/*Set CS:IP */
	rmReg.rmIP = FP_OFF(pmLslEntryTab->l_prot_stack_func);
	
	/*
	 *	DO DPMI call to LSL init function
	 */
	if ( DPMI_call_real_mode_proc( &rmReg ) )
	{
		GlobalDosFree( LOWORD(SegSelector) );	
		return FALSE;
	}

	if ( LOWORD(rmReg.rmEAX) )
	{
		GlobalDosFree( LOWORD(SegSelector) );	
		return FALSE;
	}

	FP_SEG(MlidCtrlEntryFunc) = LOWORD(rmReg.rmES);
	FP_OFF(MlidCtrlEntryFunc) = LOWORD(rmReg.rmESI);	

	/*
	 *	(4) Get MLID configuration table by using the
	 *	    MlidCtrlEntryFunc retrieved in (3).
	 */
	memset(&rmReg, 0, sizeof(REALMODEREG));	/* BX=0=GETMLIDCONFIGURATION*/
	

	rmReg.rmEDX = 0x12;
	rmReg.rmCS = FP_SEG(MlidCtrlEntryFunc);		/*Set CS:IP */
	rmReg.rmIP = FP_OFF(MlidCtrlEntryFunc);
	
	/*
	 *	DO DPMI call to LSL init function
	 */
	if ( DPMI_call_real_mode_proc( &rmReg ) )
	{
		GlobalDosFree( LOWORD(SegSelector) );	
		return FALSE;
	}

	if ( LOWORD(rmReg.rmEAX) )
	{
		GlobalDosFree( LOWORD(SegSelector) );	
		return FALSE;
	}

	FP_SEG(rmMlidConfTabPtr) = LOWORD(rmReg.rmES);
	FP_OFF(rmMlidConfTabPtr) = LOWORD(rmReg.rmESI);

	if ( !rmMlidConfTabPtr )
	{
		GlobalDosFree( LOWORD(SegSelector) );	
		return FALSE;	
	}

	/*
	 *	(5) Convert the real mode address rmMlidConfTabPtr to
	 *	    the protected mode address, which can then be used
	 *	    to access MAC address in the table pointed to by this ptr.
	 */
	FP_SEG(pmMlidConfTabPtr) =
		alloc_LDT_descriptor(MK_FP(FP_SEG(rmMlidConfTabPtr),0),
								0xffff);

	if ( !FP_SEG(pmMlidConfTabPtr) )
	{
		GlobalDosFree( LOWORD(SegSelector) );		
		return (FALSE);
	}
	
	FP_OFF(pmMlidConfTabPtr) = FP_OFF(rmMlidConfTabPtr);
	memcpy( buff, (char *)pmMlidConfTabPtr->m_NodeAddr, len );	
	
	/*
	 *	Ethernet address successfully retrieved
	 */
	free_LDT_descriptor( FP_SEG(pmMlidConfTabPtr) );
	GlobalDosFree( LOWORD(SegSelector) );		
	return (TRUE);

}


/*
 *      Get Ethernet MAC address by using Netbios Specification and DPMI.
 */
BOOL get_netbios_ether_addr(char *buff, int len)
{


typedef struct _ADAPTER_STATUS {
    unsigned char   adapter_address[6];
    unsigned char   rev_major;
    unsigned char   reserved0;
    unsigned char   adapter_type;
    unsigned char   rev_minor;
    WORD    duration;
    WORD    frmr_recv;
    WORD    frmr_xmit;

    WORD    iframe_recv_err;

    WORD    xmit_aborts;
    DWORD   xmit_success;
    DWORD   recv_success;


   WORD    iframe_xmit_err;

    WORD    recv_buff_unavail;
    WORD    t1_timeouts;
    WORD    ti_timeouts;
    DWORD   reserved1;
    WORD    free_ncbs;
    WORD    max_cfg_ncbs;
    WORD    max_ncbs;
    WORD    xmit_buf_unavail;
    WORD    max_dgram_size;
    WORD    pending_sess;
    WORD    max_cfg_sess;
    WORD    max_sess;
    WORD    max_sess_pkt_size;
    WORD    name_count;
} ADAPTER_STATUS, *PADAPTER_STATUS;

typedef struct _NAME_BUFFER {
    unsigned char   name[NCBNAMSZ];
    unsigned char   name_num;
    unsigned char   name_flags;

} NAME_BUFFER, *PNAME_BUFFER;


 typedef struct _ASTAT_
 {
        ADAPTER_STATUS adapt;
        NAME_BUFFER     NameBuff[30];
} ASTAT, * PASTAT ;

ASTAT *pmAdapter, *rmAdapter ;




        REALMODEREG rmReg;
        DWORD SegSelector,SegSelector1;
        NCB *pmNCB;
        NCB *rmNCB;

    int i;

  /*check if netbios installed*/

	memset(&rmReg,0,sizeof(REALMODEREG));
        rmReg.rmEAX = 0x355c; /* DOS interrupt to get int vector */

   if( !        RealInt((BYTE)0x21, &rmReg))
                {
                return FALSE;  /* NetBIOS not Installed */
                }

        /*
         *      Allocate DOS memory to use for NetBios parameter block
         *
         */
        if ( !(SegSelector = GlobalDosAlloc( sizeof(NCB) )))
                        return FALSE;

        pmNCB = MK_FP( LOWORD(SegSelector), 0 );
        rmNCB = MK_FP( HIWORD(SegSelector), 0 );

        memset( pmNCB, 0, sizeof(NCB) );
        /*
         *      Allocate DOS memory to use for NetBios Adapter parameter block
         *
         */
        if      (!(SegSelector1 = GlobalDosAlloc( sizeof(ASTAT))) )
               {
                GlobalDosFree( FP_SEG(pmNCB) );
                return FALSE;
                }
        pmAdapter = MK_FP( LOWORD(SegSelector1), 0 );
        rmAdapter = MK_FP( HIWORD(SegSelector1), 0 );

/* now check for the first adapter address that does not */
/* return an error */


for ( i=0;i<=255;i++)
{

        memset( pmAdapter, 0, sizeof(ASTAT) );

        pmNCB->ncb_command = NCBASTAT;
        pmNCB->ncb_buffer= (char far *) rmAdapter     ;
        pmNCB->ncb_length= sizeof(ASTAT);
        pmNCB->ncb_lana_num=i;
                        /* Get COmmands OP code */
	  strcpy(pmNCB->ncb_callname,"*               ");

        memset(&rmReg, 0, sizeof(REALMODEREG));
        rmReg.rmES = FP_SEG(rmNCB);
        rmReg.rmEBX = FP_OFF(rmNCB);

        if ( !RealInt((BYTE)0x5C, &rmReg) )
        {
                GlobalDosFree( FP_SEG(pmNCB) );
                GlobalDosFree( FP_SEG(pmAdapter) );
		return FALSE;
       }
        // if the return code is good, and the address is not 0, and the address
	//  is not DEST == PPP adapter
	 if (pmNCB->ncb_retcode == NRC_GOODRET &&
                pmAdapter->adapt.adapter_address &&
//                *(pmAdapter->adapt.adapter_address) &&
//                *(pmAdapter->adapt.adapter_address +1) &&
                strncmp("DEST",pmAdapter->adapt.adapter_address,4))

                break;   // We found a valid address !!

}
// Now copy address from structure

        if ( i== 256 )
        {   // We Did not find a valid Ethernet Adapter Address
                GlobalDosFree( FP_SEG(pmNCB) );
                GlobalDosFree( FP_SEG(pmAdapter) );
                return FALSE;
        }


        memcpy(buff,pmAdapter->adapt.adapter_address,len );



//Now Reset the adapter
  /*
        pmNCB->ncb_command = NCBRESET;
        pmNCB->ncb_buffer= (char far *) rmAdapter     ;
        pmNCB->ncb_length= sizeof(ASTAT);
        pmNCB->ncb_lana_num=i;

        memset(&rmReg, 0, sizeof(REALMODEREG));
        rmReg.rmES = FP_SEG(rmNCB);
        rmReg.rmEBX = FP_OFF(rmNCB);

        if ( !RealInt((BYTE)0x5C, &rmReg) )
        {   // error in Real Mode Interrupt Call
                GlobalDosFree( FP_SEG(pmNCB) );
                GlobalDosFree( FP_SEG(pmAdapter) );
                return FALSE;
        }


 */

        GlobalDosFree( FP_SEG(pmNCB) );
        GlobalDosFree( FP_SEG(pmAdapter) );
         return TRUE   ;
}

static HINSTANCE hInst=0;
typedef char * (FAR * getaddtype)(int *);
static  char * ( FAR *get_mac_addresses)(int * );
  BOOL get_sun_ether_addr(char *buff, int len)
{
 char * ether;
int i=6;
        if ( hInst < HINSTANCE_ERROR )
                {
 
        /*
         * Instruct Windows not to display the annoying error msg
         * when LoadLibrary() call fails to find the DLL.
         */
	                SetErrorMode( SEM_NOOPENFILEERRORBOX );
 
        // Do this only if WINSOCK.DLL is found!
       		         if ( (hInst = LoadLibrary( "DNET.Dll" ))<
						HINSTANCE_ERROR )
                         {
                                return FALSE;
 
                         }
#pragma warning (disable: 4113)
#pragma warning (disable: 4047)
	                if(!(get_mac_addresses= (getaddtype)
				 GetProcAddress(hInst,"_get_mac_address" )))
       		                         return FALSE;
               }
 
        ether= (*get_mac_addresses)(&i);
        memcpy(buff,ether,i );
        return TRUE;
 }

BOOL get_pc_ether_addr(char *buff, int len)
{
	union REGS regs;
	struct SREGS sregs;

	/*
	 *	Make sure that we do not call DPMI in real mode.
	 */
	regs.x.ax = 0x1686;
	segread(&sregs);        
	int86x( 0x2f, &regs, &regs, &sregs );
	if ( regs.x.ax )
	{
	    // AX != 0 indicates that CPU in real mode or virtual 86 mode.
	    return FALSE;
	}
        if (get_sun_ether_addr(buff, len) )
                {
                return TRUE;
                }

	if ( get_odi_ether_addr(buff, len) )
		return TRUE;
	
	if ( get_pktdrv_ether_addr(buff, len) )
		return TRUE;

	if ((!getenv("FLEXLM_NO_NDIS")) &&  get_ndis_ether_addr(buff, len) )	/* overrun checked */
		return TRUE;

	if ( !getenv("FLEXLM_NO_NETBIOS") )	/* overrun checked */
		return get_netbios_ether_addr(buff,len);

	return FALSE;
}

