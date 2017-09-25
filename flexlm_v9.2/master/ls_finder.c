/******************************************************************************

	    COPYRIGHT (c) 1994, 2003 by Macrovision Corporation.
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
 *	Module: $Id: ls_finder.c,v 1.8 2003/04/29 21:46:25 sluu Exp $
 *
 *	Function:	Serves the "license path" name for FLEXlm clients.
 *
 *	Description: 	This module implements the "license finder" function
 *			in lmgrd.  It binds port "FLEXlm" (or 744 if FLEXlm
 *			is not found), then listens and processes requests from
 *			clients.  
 *
 *	Parameters:	(char *) config - Parameter file.
 *
 *	Return:		None - never returns.
 *
 *	M. Christiano
 *	1/17/93
 *
 *	Last changed:  10/27/96
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lsserver.h"
#include "ls_sprot.h"
#include "lsmaster.h"
#include "l_m_prot.h"
#include "flex_file.h"
#ifndef NO_UIO_H
#include <sys/uio.h>
#endif
#ifdef USE_WINSOCK
#include <winsock.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
/*- #include <netinet/in.h> 	 Included in lsserver.h */
#include <netdb.h>
#endif
#include <errno.h>

extern LM_SOCKET ls_make_tcp_socket();
extern DAEMON *master_daemons;
extern LM_QUORUM quorum;        /* The LM_QUORUM in this server */

#if !defined (htons) && !defined(i386) && !defined(OSF)	&& !defined(WINNT)
			/*- Sun systems make this a macro! */
extern u_short htons();
#endif

#ifdef USE_WINSOCK
#define lm_nofile 0
#endif

static void ls_read_finder_config lm_args((char *config));
static int nrequests;

void
ls_finder(char *		config,
		  SELECT_MASK *	select_mask)
{
  unsigned short port;
  int iport;
  LM_SOCKET s;
  int nrdy;
  char hostname[MAX_HOSTNAME+1];
  SELECT_MASK ready_mask;
  COMM_ENDPOINT endpoint; 
  
/*
 *	First, initialize a few things that lmgrd normally has set up
 */
	(void) strcpy(lm_job->vendor, "l_finder");
	if (master_daemons)
		l_free_daemon_list(lm_job, master_daemons);
	if (master_daemons)
		l_free_daemon_list(lm_job, master_daemons);
	master_daemons = (DAEMON *) NULL;
	quorum.list = (LM_SERVER *) NULL;
	quorum.count = 0;
	quorum.quorum = 0;
	quorum.master = 0;
	(void) gethostname(hostname, MAX_HOSTNAME);
/*
 *	Now, get our port and bind it
 */
	port = l_finder_port();
	iport = port;
	LOG(("License finder started on %s (port %d)\n", hostname, iport));
	LOG(("Using configuration file %s\n", config));

	endpoint.transport = LM_TCP;	
	endpoint.transport_addr.port = (int) ntohs((unsigned short)iport); 
	s = ls_socket(hostname, &endpoint, 0);	

/* 
 *	Now, read the config file 
 */

	ls_read_finder_config(config);

	MASK_CREATE(*select_mask);
	MASK_CREATE(ready_mask);
	MASK_INIT(*select_mask, s);  /* select the socket */
/*
 *	Process commands
 */
	nrequests = 0;
	while (1)
	{
		MASK_COPY(ready_mask, *select_mask);
		nrdy = l_select(lm_nofile, (int *)ready_mask, 0, 0, 0);
		(void) ls_m_process(ready_mask, nrdy, *select_mask, s, 0,
						(DAEMON **) NULL, hostname);
	}
}

static char *lfpath;

char *
ls_get_lfpath(type, name, node, display, vendor)
char *type;
char *name;
char *node;
char *display;
char *vendor;
{
#ifdef lint
	LOG(("Request for \"%s\" from client \"%s\", node \"%s\",\n",
						type, name, node));
	LOG(("  display \"%s\", vendor: \"%s\"\n", display, vendor));
	LOG(("   returns: \"%s\"\n", lfpath));
#endif /* lint */
	nrequests++;
	if (nrequests && ((nrequests % 100) == 0))
	{
		LOG(("%d requests processed.\n", nrequests));
	}
	return(lfpath);
}

static
void
ls_read_finder_config(char * config)
{
	FILE * f = NULL;
  LICENSE_FILE lf;
  char data[MAX_CONFIG_LINE+1], data2[MAX_CONFIG_LINE+1], 
	junk[MAX_CONFIG_LINE+1];
  int nflds;
#ifdef PC
  char *conf_file_content;
  int conf_file_size;
#endif

	f = l_flexFopen(lm_job, config, "r");

	if (f == (FILE *) NULL)
	{
		LOG(("Error opening finder config. file \"%s\"\n", config));
		(void) exit(1);
	}
	
#ifdef PC
	/*
	 *	Read the whole configuration file in memory because
	 * 	we can't pass file pointer to l_lfgets(), which is
	 * 	crossing DLL interface on Windows and NT.
	 */
	fseek(f, 0L, 2);	/* Seek to the end */
	conf_file_size = ftell(f);
	if (!(conf_file_content=malloc(conf_file_size)))
	{
		LOG(("Configuration file too big. malloc(%d) failed\n", conf_file_size));
		(void) exit(1);
	}
	
	fseek(f, 0L, 0);	/* Seek to the beginning */ 
	fread(conf_file_content, 1, conf_file_size, f);		/* overrun threat */
	
	memset(&lf, 0, sizeof(lf));
	lf.type = LF_STRING_PTR;
	lf.ptr.str.s = lf.ptr.str.cur = conf_file_content;
#else /* PC */
	lf.type = LF_FILE_PTR;
	lf.ptr.f = f;
#endif /* PC */

	while (l_lfgets(lm_job, data, MAX_CONFIG_LINE, &lf, 0) != (char *) NULL)
	{
		nflds = sscanf(data, "DATA %s %s", junk, data2);		/* overrun threat */
		if (nflds > 1)
		{
			lfpath = (char *)malloc(strlen(data2) + 1);
			(void) strcpy(lfpath, data2);
			break;
		}
	}

#ifdef PC
	free( (void *)conf_file_content );
#endif /* PC */
	
	(void) fclose(f);
}

