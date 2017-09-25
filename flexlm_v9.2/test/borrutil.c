/******************************************************************************

	    COPYRIGHT (c) 1988, 2003 by Macrovision Corporation.
	This software has been provided pursuant to a License Agreement
	containing restrictions on its use.  This software contains
	valuable trade secrets and proprietary information of 
	Macrovision Corporation and is protected by law.  It may 
	not be copied or distributed in any form or medium, disclosed 
	to third parties, reverse engineered or used in any manner not 
	provided for in said License Agreement except with the prior 
	written authorization from Macrovision Corporation.
 ******************************************************************************
 *	Module: $Id: borrutil.c,v 1.10 2003/01/13 22:46:12 kmaclean Exp $
 *
 *	Description:	Examples of using lc_borrow_xxx()
 *
 *	Last changed:  12/29/98
 *	D. Birns
 *
 */

#include <stdio.h>
#include <time.h>
#include "lmclient.h" 
#include "lm_attr.h"
#ifdef ANSI
#include <stdlib.h>
#include <string.h>
#endif
#ifdef PC
#define LICPATH "license.dat;."
#else
#define LICPATH "@localhost:license.dat:."
#endif /* PC */
static void usage lm_args((void));

#define FEATURE "f1"
#ifdef DEMOF
#include "../testsuite/code.h"
#else
VENDORCODE code;
#endif

void
main(argc, argv)
int argc;
char **argv;
{
  LM_HANDLE *lm_job;
  int i;
  char *device =0;
  int print_device = 1;


	for (i = 0; i < argc; i++)
	{
		if (!strncmp(argv[i], "-d", 2))
		{
		        print_device = 0;
			i++;
			if (i >= argc) usage();
			device = malloc(strlen(argv[i]) + 1);
			strcpy(device, argv[i]);
		}
	}
	if (argc < 2)
	{
		usage();
	}
#ifdef DEMOF

	if (LM_DEFAULT_SEEDS != lc_init((LM_HANDLE *)0, "demof", &codef, &lm_job))	
#else
	if (lc_new_job((LM_HANDLE *)0, 0, &code, &lm_job))
#endif
	
	{
		lc_perror(lm_job, "lc_new_job failed");
		exit(lc_get_errno(lm_job));
	}
	if (!strcmp(argv[1], "init"))
	{
		if (lc_borrow_init(lm_job, LM_BORROW_METER, &device))
		{
			lc_perror(lm_job, "lc_borrow_init failed");
			exit(lc_get_errno(lm_job));
		}
	}
	if (!strcmp(argv[1], "uninit"))
	{
		if (lc_borrow_uninit(lm_job, LM_BORROW_METER, device))
		{
			lc_perror(lm_job, "lc_borrow_uninit failed");
			exit(lc_get_errno(lm_job));
		}
	}
	else if (!strcmp(argv[1], "read"))
	{
	  int counter = 1;
	  int val;
	  unsigned int lock;
		if (argc >= 3) sscanf(argv[2], "%d", &counter);
		if (lc_borrow_get_counter(lm_job, LM_BORROW_METER, &device, counter, &val, &lock))
		{
			lc_perror(lm_job, "lc_borrow_get_counter failed");
			exit(lc_get_errno(lm_job));
		}
		printf("counter %d: val = %d, lock = %u\n",
			counter, val, lock);
	}
	else if (!strcmp(argv[1], "make"))
	{
	  int counter = 1;
	  int val;
	  unsigned int lock;
		if (argc >= 3) sscanf(argv[2], "%d", &counter);
		if (argc >= 4) sscanf(argv[3], "%d", &val);
		if (lc_borrow_make_counter(lm_job, LM_BORROW_METER, &device, counter, val, &lock))
		{
			lc_perror(lm_job, "lc_borrow_get_counter failed");
			exit(lc_get_errno(lm_job));
		}
		printf("counter %d: val = %d, lock = %u\n",
			counter, val, lock);
	}
	else if (!strcmp(argv[1], "update"))
	{
	  int counter = 1;
	  int increment;
	  unsigned int lock;
	  char *update_key;
	  char *sn;
		if (argc >= 6) 
		{
			sn = argv[2];
			sscanf(argv[3], "%d ", &counter);
			sscanf(argv[4], "%d ", &increment);
			sscanf(argv[5], "%u ", &lock);
		}
		else usage();
		if (lc_borrow_update_key(lm_job, LM_BORROW_METER, sn, counter, increment,
                                lock, &update_key))
		{
			lc_perror(lm_job, "lc_borrow_get_counter failed");
			exit(lc_get_errno(lm_job));
		}
		printf("update key: %s\n", update_key);
		free(update_key);
	}
	else if (!strcmp(argv[1], "increment"))
	{
	 char *update_key;
	  int counter = 1;
	  int increment;
		if (argc >= 5) 
		{
			sscanf(argv[2], "%d ", &counter);
			sscanf(argv[3], "%d ", &increment);
			update_key = argv[4];
		}
		else usage();
		if (lc_borrow_increment(lm_job, LM_BORROW_METER, &device, counter, increment, 
				update_key))
		{
			lc_perror(lm_job, "lc_borrow_get_counter failed");
			exit(lc_get_errno(lm_job));
		}
	}
	else if (!strcmp(argv[1], "free"))
	{
	  int counter;
		if (argc >= 3) 
		{
			sscanf(argv[2], "%d ", &counter);
		}
		if (lc_borrow_free_counter(lm_job, LM_BORROW_METER, &device, counter))
		{
			lc_perror(lm_job, "lc_borrow_get_counter failed");
			exit(lc_get_errno(lm_job));
		}
	}
	else if (!strcmp(argv[1], "borrow"))
	{
	  char license[MAX_CONFIG_LINE + 1];

		*license = 0;
		if (argc < 3) usage();
		if ( lc_borrow(lm_job, LM_BORROW_METER, &device, argv[2], "1.0",
			&code, license, 0, 0, 0, 0))
		{
			lc_perror(lm_job, "lc_borrow failed");
			exit(lc_get_errno(lm_job));
		}
		printf("borrowed %s\n", license);
	}
	else if (!strcmp(argv[1], "unborrow") ||
			!strcmp(argv[1], "return"))
	{
	  char *port_at_host = 0;
	  char *feature = 0;
	  int counter = 0;

		if (argc == 3 ) 
		{
		        feature = argv[2];
		}
		else if (argc ==5 ) 
		{
                        port_at_host = argv[3];
                        counter = atoi(argv[4]);
                }
                else usage();
		if (lc_unborrow(lm_job, LM_BORROW_METER, &device, feature,
				port_at_host, counter, 0, 0))
		{
			lc_perror(lm_job, "lc_unborrow failed");
			exit(lc_get_errno(lm_job));
		}
		if (argc == 3)
                        printf("returned %s\n", feature);
                else
                        printf("returned METER:%s:%d to %s\n", device, counter,
                                port_at_host);
	}
	else if (!strcmp(argv[1], "list"))
	{
	  LM_METERS *d;
		if (lc_borrow_find_meter(lm_job, &d ))
		{
		
			lc_perror(lm_job, "lc_borrow_find_meter failed");
			exit(0);
		}
		printf("%-25.25s%s\n", "Borrow meter", "Vendor name");
		printf("%-25.25s%s\n\n", "____________", "___________");
		while (d)
		{
			printf("%-25.25s%s\n",d->device, d->vendor);
			d=d->next;

		}
	}
	else if (!strcmp(argv[1], "dump"))
	{
	  long pos = 0;
	  int counter, val;
	  unsigned int lock;
	  int stat;
	  char *sn;
	  char *vendor;
		if (lc_borrow_sn(lm_job, LM_BORROW_METER, &device, &sn))
		{
			lc_perror(lm_job, "lc_borrow_sn failed");
			exit(0);
		}
		lc_borrow_vendor(lm_job, LM_BORROW_METER,
			&device,&vendor);
		printf("sn=%s device=%s vendor=%s\n", sn, 
			device , vendor);
		free(vendor);
		printf("%-11s%-11s%-11s\n", "Counter", "Value", 
			"Lock");
		while (!(stat = lc_borrow_dump(lm_job, 
				LM_BORROW_METER, device, 
				&counter, &val, &lock, &pos)))
			printf("%-11d%-11d%-11u\n", counter, 
				val, lock);
		if (stat < 0) 
			lc_perror(lm_job, 
				"lc_borrow_dump failed");
		free(sn);
	}
	else usage();

	if (print_device && device && *device) printf("device: %s\n", device);
	lc_free_job(lm_job);
	if (device) free(device);
        exit(0);
}
static
void
usage()
{
	printf("usage:  borrow command [-d device]{\n\
\tcommand is one of:\n\
\tinit | \n\
\tread counter |\n\
\tmake counter val |\n\
\tupdate sernum counter incr lock\n\
\tincrement counter incr update-key\n\
\tfree counter\n\
\tborrow feature\n\
\tunborrow feature\n\
\tunborrow port@host counter\n\
\tdump\n\
\tlist\n\
\t}\n");
	exit(1);
}
