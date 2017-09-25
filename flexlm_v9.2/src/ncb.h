/********************************************************************
 *								    *
 *  About this file ...  NCB.H					    *
 *								    *
 *  This file contains information about NCBs and NCB structure     *
 *								    *
 *								    *
 *								    *
 *  History:	Alok Sinha  October, 1991    Created based on Lan   *
 *					     Manager Programmer Tool*
 *					     Kit  (PTK)		    *
 *								    *
 ********************************************************************/

#ifndef NETNCB_INCLUDED

#define NETNCB_INCLUDED 


/**************************************************************** 
 *								*
 *	  	Data structure templates			*
 *								*
 ****************************************************************/

#define NCBNAMSZ	16	  /* absolute length of a net name	*/

/*
 * Network Control Block 
 */

struct ncb {
    unsigned char   ncb_command;	    /* command code		    */
    unsigned char   ncb_retcode;	    /* return code		    */
    unsigned char   ncb_lsn;		    /* local session number	    */
    unsigned char   ncb_num;		    /* number of our network name   */
    char far *	    ncb_buffer; 	    /* address of message buffer    */
    unsigned short  ncb_length; 	    /* size of message buffer	    */
    char	    ncb_callname[NCBNAMSZ]; /* blank-padded name of remote  */
    char	    ncb_name[NCBNAMSZ];     /* our blank-padded netname     */
    unsigned char   ncb_rto;		    /* rcv timeout/retry count	    */
    unsigned char   ncb_sto;		    /* send timeout/sys timeout     */
    unsigned long   ncb_post;		    /* Async notification handle    */
    unsigned char   ncb_lana_num;	    /* lana (adapter) number	    */
    unsigned char   ncb_cmd_cplt;	    /* 0xff => commmand pending     */
    unsigned char   ncb_reserve[14];	    /* reserved, used by BIOS	    */
};	/* ncb */

typedef struct ncb NCB;

/*
 *  Structure returned to the NCB command NCBASTAT is ADAPTER_STATUS followed
 *  by an array of NAME_BUFFER structures.
 */

typedef struct _ADAPTER_STATUS {
    unsigned char   adapter_address[6];
    unsigned char   rev_major;
    unsigned char   reserved0;
    unsigned char   adapter_type;
    unsigned char    rev_minor;
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




struct findbuf {
    unsigned short  usNumResponding;
    unsigned char   reserved;
    unsigned char   ucStatus;
    unsigned char   ucLanHeaderLength;
};
typedef struct findbuf FINDBUF;

#define MAX_HEADER_LEN	 33
#define MAX_NODE	 5
#define FIND_BUFFER_LENGTH ( sizeof ( FINDBUF) + MAX_HEADER_LEN * MAX_NODE)

/**************************************************************** 
 *								*
 *	  	Special values and constants			*
 *								*
 ****************************************************************/

/*
 *	NCB Command codes
 */

#define NCBCALL 	0x10		/* NCB CALL			    */
#define NCBLISTEN	0x11		/* NCB LISTEN			    */
#define NCBHANGUP	0x12		/* NCB HANG UP			    */
#define NCBSEND 	0x14		/* NCB SEND			    */
#define NCBRECV 	0x15		/* NCB RECEIVE			    */
#define NCBRECVANY	0x16		/* NCB RECEIVE ANY		    */
#define NCBCHAINSEND	0x17		/* NCB CHAIN SEND		    */
#define NCBDGSEND	0x20		/* NCB SEND DATAGRAM		    */
#define NCBDGRECV	0x21		/* NCB RECEIVE DATAGRAM 	    */
#define NCBDGSENDBC	0x22		/* NCB SEND BROADCAST DATAGRAM	    */
#define NCBDGRECVBC	0x23		/* NCB RECEIVE BROADCAST DATAGRAM   */
#define NCBADDNAME	0x30		/* NCB ADD NAME 		    */
#define NCBDELNAME	0x31		/* NCB DELETE NAME		    */
#define NCBRESET	0x32		/* NCB RESET			    */
#define NCBASTAT	0x33		/* NCB ADAPTER STATUS		    */
#define NCBSSTAT	0x34		/* NCB SESSION STATUS		    */
#define NCBCANCEL	0x35		/* NCB CANCEL			    */
#define NCBADDGRNAME	0x36		/* NCB ADD GROUP NAME		    */
#define NCBUNLINK	0x70		/* NCB UNLINK			    */
#define NCBSENDNA	0x71		/* NCB SEND NO ACK		    */
#define NCBCHAINSENDNA	0x72		/* NCB CHAIN SEND NO ACK	    */
#define NCBFINDNAME	0x78		/* NCB FIND NAME		    */

#define ASYNCH		0x80		/* high bit set == asynchronous     */

/*
 *	NCB Return codes
 */

#define NRC_GOODRET	0x00	/* good return				     */
#define NRC_BUFLEN	0x01	/* illegal buffer length		     */
#define NRC_BFULL	0x02	/* buffers full, no receive issued	     */
#define NRC_ILLCMD	0x03	/* illegal command			     */
#define NRC_CMDTMO	0x05	/* command timed out			     */
#define NRC_INCOMP	0x06	/* message incomplete, issue another command */
#define NRC_BADDR	0x07	/* illegal buffer address		     */
#define NRC_SNUMOUT	0x08	/* session number out of range		     */
#define NRC_NORES	0x09	/* no resource available		     */
#define NRC_SCLOSED	0x0a	/* session closed			     */
#define NRC_CMDCAN	0x0b	/* command cancelled			     */
#define NRC_DMAFAIL	0x0c	/* PC DMA failed			     */
#define NRC_DUPNAME	0x0d	/* duplicate name			     */
#define NRC_NAMTFUL	0x0e	/* name table full			     */
#define NRC_ACTSES	0x0f	/* no deletions, name has active sessions    */
#define NRC_INVALID	0x10	/* name not found or no valid name	     */
#define NRC_LOCTFUL	0x11	/* local session table full		     */
#define NRC_REMTFUL	0x12	/* remote session table full		     */
#define NRC_ILLNN	0x13	/* illegal name number			     */
#define NRC_NOCALL	0x14	/* no callname				     */
#define NRC_NOWILD	0x15	/* cannot put * in NCB_NAME		     */
#define NRC_INUSE	0x16	/* name in use on remote adapter	     */
#define NRC_NAMERR	0x17	/* called name cannot == name or name #      */
#define NRC_SABORT	0x18	/* session ended abnormally		     */
#define NRC_NAMCONF	0x19	/* name conflict detected		     */
#define NRC_IFBUSY	0x21	/* interface busy, IRET before retrying      */
#define NRC_TOOMANY	0x22	/* too many commands outstanding, retry later*/
#define NRC_BRIDGE	0x23	/* ncb_bridge field not 00 or 01	     */
#define NRC_CANOCCR	0x24	/* command completed while cancel occurring  */
#define NRC_RESNAME	0x25	/* reserved name specified		     */
#define NRC_CANCEL	0x26	/* command not valid to cancel		     */
#define NRC_MULT	0x33	/* multiple requests for same session	     */
#define NRC_MAXAPPS	0x36	/* max number of applications exceeded	     */
#define NRC_NORESOURCES 0x38	/* requested resources are not available     */
#define NRC_SYSTEM	0x40	/* system error 			     */
#define NRC_ROM 	0x41	/* ROM checksum failure 		     */
#define NRC_RAM 	0x42	/* RAM test failure			     */
#define NRC_DLF 	0x43	/* digital loopback failure		     */
#define NRC_ALF 	0x44	/* analog loopback failure		     */
#define NRC_IFAIL	0x45	/* interface failure			     */
#define NRC_ADPTMALFN	0x50	/* network adapter malfunction		     */

#define NRC_PENDING	0xff	/* asynchronous command is not yet finished  */



/*
 *	Maximum datagram size
 */

#define MAX_DG_SIZE 512

/*
 *  OS/2 NetBios calls
 */
#if defined(OS2)
/*
 *	Open modes for NetBiosOpen.
 */

#define	NB_REGULAR	1
#define	NB_PRIVILEGED	2
#define	NB_EXCLUSIVE	3

unsigned far pascal
  NetBiosOpen ( char far *	     pszDevName,
                char far *           pszReserved,
                unsigned short       usOpenOpt,
                unsigned short far * phDevName );

unsigned far pascal
  NetBiosClose ( unsigned short hDevName,
                 unsigned short usReserved );


unsigned far pascal
  NetBiosSubmit ( unsigned short   hDevName,
                  unsigned short   usNcbOpt,
                  struct ncb far * pNCB );

#endif /* (OS/2)	  */

#endif /* NETNCB_INCLUDED */
