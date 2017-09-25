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
/*	
 *	Module: $Id: test_slwg.c,v 1.2 2003/01/13 22:46:13 kmaclean Exp $
 *
 *	Description: 	Test the Software Licensing Working Group calls 
 *
 *	M. Christiano
 *	6/18/90
 *
 *	Last changed:  10/18/95
 *
 */

#if DOESNT_WORK_MAY_18
#include <stdio.h>
#include "lmachdep.h"
#include "lmclient.h"
#include "lm_code.h"
#include "lm_attr.h"
#include "lm_slwg.h"
LM_DATA_STATIC;

LM_CODE(code, ENCRYPTION_CODE_1, ENCRYPTION_CODE_2, VENDOR_KEY1, VENDOR_KEY2, 
						VENDOR_KEY3, VENDOR_KEY4);

main()
{
  int job_id;
  int rc;
  char *msg = "hi there, this is Software License Working Group compatibility";
  char *license_handle;
  int queue_position;
  int check_interval = 100;
  int max_entries = 1;
  int total_licenses;
  int queued = 0;	/* Don't return queued licenses on lb_get_cur_users() */
  LM_USERS *entries;
  LM_USERS *next;
  int number_of_entries;

	printf("ls_init...");
	fflush(stdout);
	rc = ls_init(VENDOR_NAME, &code, &job_id);
	if (rc) lm_perror("ls_init");
	else printf("OK\n");

	printf("lb_reqeust...");
	fflush(stdout);
	rc = lb_request(job_id, "f1", (double) 1.0, 1, 0, check_interval, 
							&license_handle);
	if (rc) lm_perror("lb_request");
	else printf("OK\n");

	printf("lb_check_wait...");
	fflush(stdout);
	rc = lb_check_wait(license_handle, check_interval, &queue_position);
	if (rc) lm_perror("lb_check_wait");
	printf("queue position is %d\n", queue_position);

	printf("lb_confirm...");
	fflush(stdout);
	rc = lb_confirm(license_handle, check_interval);
	if (rc) lm_perror("ls_log_message");
	else printf("OK\n");

	printf("ls_log_message...");
	fflush(stdout);
	rc = ls_log_message(job_id, strlen(msg), msg);
	if (rc) lm_perror("ls_log_message");
	else printf("OK\n");

	next = LB_START;
	printf("lb_get_cur_users...\n");
	fflush(stdout);
	number_of_entries = max_entries;
	while (number_of_entries == max_entries)
	{
	  rc = lb_get_cur_users(job_id, "f1", (double) 1.0, &next, max_entries,
			queued, &total_licenses, &number_of_entries, &entries);
	  if (rc) lm_perror("lb_get_cur_users");
	  else if (number_of_entries > 0)
	  {
		printf("  lb_get_cur_users returns %d entries:", 
				number_of_entries);
		printf(" %s/%s %d licenses, total: %d licenses\n",
				entries->name, entries->node, entries->nlic,
				total_licenses);
	  }
	}

	printf("lb_release...");
	fflush(stdout);
	rc = lb_release(license_handle);
	if (rc) lm_perror("lb_release");
	else printf("OK\n");

/*
 *	Unsupported calls
 */
	rc = um_put_record();
	if (rc != NOFEATURE) lm_perror("um_put_record");
	rc = um_get_record();
	if (rc != NOFEATURE) lm_perror("um_get_record");
	rc = um_delete_records();
	if (rc != NOFEATURE) lm_perror("um_delete_records");
	rc = um_purge_records();
	if (rc != NOFEATURE) lm_perror("um_purge_records");
	rc = um_undelete_records();
	if (rc != NOFEATURE) lm_perror("um_undelete_records");

/*
 *	Terminate licensing
 */

	rc = ls_terminate(job_id);
	if (rc) lm_perror("ls_terminate");
}
#else
main() { puts("needs to be fixed!");}
#endif /* DOESNT_WORK */

