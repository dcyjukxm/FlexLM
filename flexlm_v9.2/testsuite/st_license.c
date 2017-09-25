/******************************************************************************

            COPYRIGHT (c) 1997, 2003 by Macrovision Corporation.
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
 *	Module: $Id: st_license.c,v 1.16 2003/03/07 23:20:39 kmaclean Exp $
 *
 *
 *	Function:  st_license.c
 *      B. Eisenberg
 *      11/98
 *
 *	Last changed:  12/15/98
 *
 */
 
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lmpolicy.h"
#include "lm_attr.h"
#include "testsuite.h"
#ifdef FREE_VERSION
#include "free.h"
LM_CODE(code, ENCRYPTION_SEED1, ENCRYPTION_SEED2, VENDOR_KEY1,
        VENDOR_KEY2, VENDOR_KEY3, VENDOR_KEY4, VENDOR_KEY5);
#define SIG VENDOR_KEY5
#else
#include "code.h"
#define TEST_DEMOF
#endif
#include "l_prot.h"
#ifdef UNIX
#include <sys/types.h>
#include <sys/stat.h>
#endif /* UNIX */
#ifdef PC                
#include <io.h>             
#include <direct.h>
#include <time.h>
#ifndef WINNT
#include <stdlib.h>
#ifndef OS2
#include <lzexpand.h>
#endif /* OS2 */
#endif /* WINNT */          
#endif /* PC */
#ifndef WINNT
#include <string.h>
#endif


#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/file.h>   /* for X_OK */
#if defined (MOTO_88K) || defined (sco)
#include <sys/unistd.h> /* for X_OK */
#endif
#ifdef SVR4
#include <unistd.h>
#endif
#include "testsuite.h"
extern char *delim;
extern LM_HANDLE *job[];
extern LM_VD_FEATURE_INFO fi;
extern LM_VD_GENERIC_INFO gi;
extern int bugnumber;
extern CONFIG *conf, *pos;
extern VENDORCODE vc;
extern char hostid [];
extern char *replace();

#ifdef VMS
#define PORT 200
#define RPORT -1
#else
#define PORT 2837
#define RPORT PORT
#endif /* VMS */


char save_string[2049];
char save_registry[2049];
extern FILE *ofp;

static
void
_lm_init(vendor_id, vendor_key, job_id, line)
char *vendor_id;		/* vendor ID */
VENDORCODE *vendor_key;		/* Vendor's encryption code */
LM_HANDLE **job_id;
int line;
{
  int rc;
  extern LM_HANDLE *main_job;

	
	if ((rc = lc_init(main_job, vendor_id, vendor_key, job_id)) && 
						rc != LM_DEMOKIT) 	
	{
		fprintf(ofp, "error line %d: %s\n", line, lc_errstring(*job_id));
	}
	st_set_attr(*job_id, LM_A_LONG_ERRMSG, (LM_A_VAL_TYPE) 0, __LINE__);
	st_set_attr(*job_id, LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE) 0, __LINE__);
}

static 
test_license_file( job,  license_file)
LM_HANDLE_PTR job;
char * license_file;
{
  char test_string[256];
  char * lic_file;

#undef getenv
        
        lic_file=getenv(license_file);
        if (lic_file )
                strcpy(save_string,lic_file);
        unsetenv(license_file);
        if (lc_set_registry(job, license_file, 0))
        {
                
                fprintf(ofp, 
		"Line %d---An error occured when deleteing the registry key \n",
			__LINE__);
        }
        lic_file=l_getenv(job, license_file);
        if ( lic_file  )
        {       
                sprintf(test_string,
			"LINE  %d ---error in l_getenv %s",
			__LINE__,license_file);
                fprintf(ofp, "%s\n",test_string);
        }
        sprintf(test_string,"%s=test",license_file);
        setenv(license_file, "test");
        
        lic_file=l_getenv(job, license_file);
        if ( !lic_file && strlen(lic_file) !=4 )
        {       
                sprintf(test_string,"LINE  %d ---error in l_getenv  %s",
					__LINE__,license_file);
                fprintf(ofp, "%s\n",test_string);
                if ( strcmp("test",lic_file) )
                {
                        sprintf(test_string,
			"LINE  %d ---error in getting registry entry %s",
			__LINE__,license_file);
                        fprintf(ofp, "%s\n",test_string);     
                }

        }
        if (lc_set_registry(job, license_file, "TouchDown"))
                fprintf(ofp, 
		"Line %d---An error occured when deleteing the registry key \n",
			__LINE__);
        lic_file=l_getenv(job, license_file);
        if ( !lic_file && strlen(lic_file) !=15 )
        {       
                fprintf(ofp, "LINE  %d ---error in l_getenv \n",__LINE__);
                if ( strcmp("test;TouchDown",lic_file) )
                        fprintf(ofp, "LINE  %d ---error in l_getenv\n",
							__LINE__);


        }
        unsetenv(license_file);
        lic_file=l_getenv(job, license_file);
        if ( !lic_file && strlen(lic_file) !=9 )
        {       
                fprintf(ofp, "LINE  %d ---error in l_getenv \n",
								__LINE__);
                if ( strcmp("TouchDown",lic_file) )
                        fprintf(ofp, "LINE  %d ---error in l_getenv\n",
								__LINE__);


        }
        lc_set_registry(job, license_file, 0);


}

static 
int 
get_license_file(feature_line,file_name)
char * feature_line;
char * file_name;
{
int bytes_read;
FILE * f=fopen(file_name,"rb");
        if (f)
        {
                bytes_read=fread(feature_line,sizeof(char),2048,f);
                
                fclose(f);
                if ( bytes_read)
                {
                        feature_line[bytes_read]='\0';
                        return 0;
                }
                else
                {
                        return 1;
                }

        }
        else
                return 1;

}



test48()
{
  char feature[MAX_FEATURE_LEN+1]; 
  LM_HANDLE *job;
  char ** featlist;
  char temp_buf[256];
  char buffer[2049];
  char * t;
  CONFIG * conf; 
  int ret;
  char license[2049];
  char * lic_file = 0;
  char  save_string[2049];
  char  temp_buf2[2049];
  char  save_license_file_path[2049];
  char term[2];
  char * ptr;
  FILE * f;
  CONFIG * pos=0;
  int num;
  extern char servtest_lic[];

  char * path = 0;
  char * path2 = 0;
  char * path3 = 0;
  char * path4 = 0;
  char feature_line[2049];
 int action;

#ifdef UNIX
	{
	 struct stat statbuf;
		if (stat("/usr/local/flexlm/licenses", &statbuf))
		{
			fprintf(ofp, "No /usr/local/flexlm/licenses directory.");
			fprintf(ofp, "Skipping this test.");
			return 0;
		}
	}
#endif
	serv_log( "48 -- License file install api \n");

	_lm_init("demo", &code, &job, __LINE__); 



        save_license_file_path[0]=0;
	lc_free_job(job);
	_lm_init("demo", &code, &job, __LINE__);

	/*  First make sure vendor_license_file is not set */
        lc_get_registry(job,"DEMO_LICENSE_FILE",&lic_file);
        

        if ( lic_file && strlen(lic_file) !=0 )
        {
                if (lc_set_registry(job, "DEMO_LICENSE_FILE", 0))
                        fprintf(ofp, 
		"Line %d---An error occured when deleteing the registry key \n",
						__LINE__);

        }
        /* now check to make sure it was reset */
	
        if (!lc_get_registry(job,"DEMO_LICENSE_FILE",&lic_file))
                fprintf(ofp, 
	"Line %d ---error in getting registry entry DEMO_LICENSE_FILE\n",
						__LINE__);

        if ( lic_file && strlen(lic_file) !=0 )
        {
                fprintf(ofp, " error- registry was not cleared \n");
        
        }
        /* Now check that you can set the registry */
        license[0]=0;
        getcwd(license,2048);
        if ( strlen(license))
        {
                if (lc_set_registry(job, "DEMO_LICENSE_FILE", &license[0]))
                        fprintf(ofp, 
		"Line %d --- An error occured when setting the registry key \n",
					__LINE__);
        
                if (lc_get_registry(job,"DEMO_LICENSE_FILE",&lic_file))
                        fprintf(ofp, 
	"LINE  %d ---error in getting registry entry DEMO_LICENSE_FILE\n",
							__LINE__);

                if (strcmp(license,lic_file ))
                        fprintf(ofp, 
"LINE %d ---error in setting registry, registry get didnt match results\n",
						__LINE__);

        }

/* check the l_getenv function  */
/* first try it for a non-LICENSE_FILE FUNCTION */
        if (lc_set_registry(job, "XYZZY", 0))
                fprintf(ofp, 
		"Line %d --- An error occured when setting the registry key \n",
							__LINE__);

        setenv("XYZZY","1234");
        lic_file=l_getenv(job, "XYZZY");
        if ( !lic_file && strlen(lic_file) !=4 )
        {       
                fprintf(ofp, 
		"LINE  %d ---error in getting registry entry XYZZY\n",
							__LINE__);
                if ( strcmp("1234",lic_file) )
                        fprintf(ofp, 
			"LINE  %d ---error in getting registry entry XYZZY\n",
						__LINE__);


        }
        unsetenv("XYZZY"); /* blane, this is behavior, not unix */
        lic_file=l_getenv(job, "XYZZY");
        if ( lic_file  && *lic_file)
        {       
                fprintf(ofp, 
		"LINE  %d ---error in getting registry entry XYZZY\n Got %s\n",
						__LINE__,lic_file);


        }
        
        /* now try the registry part of it */
        if (lc_set_registry(job, "XYZZY", "5678"))
                fprintf(ofp, 
		"Line %d --- An error occured when setting the registry key \n",
						__LINE__);

        lic_file=l_getenv(job, "XYZZY");
        if ( !lic_file && strlen(lic_file) !=4 )
        {       
                fprintf(ofp, 
		"LINE  %d ---error in getting registry entry XYZZY\n",
								__LINE__);
                if ( strcmp("5678",lic_file) )
                        fprintf(ofp, 
			"LINE  %d ---error in getting registry entry XYZZY\n",
								__LINE__);


        }
        /* Clean up xyzzy  */
        unsetenv("XYZZY");
        if (lc_set_registry(job, "XYZZY", 0))
                fprintf(ofp, 
		"Line %d --- An error occured when setting the registry key \n",
					__LINE__);
        /* clean up DEMO_LICENSE_FILE */
        if (lc_set_registry(job, "DEMO_LICENSE_FILE", 0))
                fprintf(ofp, 
		"Line %d---An error occured when deleteing the registry key \n",
							__LINE__);

        /* now try a lm_license_file type */
        test_license_file(job, "LM_LICENSE_FILE");

        test_license_file(job,"DEMO_LICENSE_FILE");

        /* Test lc_update_license_path */
        /* First check degenerate cases */
        /* no path */
        ret=lc_install_license_path(job,0,"demo");
        if ( !ret)
                fprintf(ofp, "LINE  %d ---error in lc_install_license_path \n",
								__LINE__);

        /* no vendor name */
        getcwd(license,2048);

        ret=lc_install_license_path(job,license,0);
        if ( ret)
                fprintf(ofp, "LINE  %d ---error in lc_install_license_path \n",
								__LINE__);


        /* no behavior */
	lc_set_attr(job, LM_A_BEHAVIOR_VER, (LM_A_VAL_TYPE)LM_BEHAVIOR_V2);
        
        ret=lc_install_license_path(job,license,"demo");
        if ( !ret)
                fprintf(ofp, "LINE  %d ---error in lc_install_license_path \n",
								__LINE__);
	lc_set_attr(job, LM_A_BEHAVIOR_VER, (LM_A_VAL_TYPE)LM_BEHAVIOR_V7);
/* Now try ones that work  */
/* first try a one path file  */
        
        ret=lc_install_license_path(job,license,"demo");
        if ( ret)
                fprintf(ofp, "LINE  %d ---error in lc_install_license_path \n",
								__LINE__);
        lc_get_registry(job,"DEMO_LICENSE_FILE",&lic_file);
        if (strcmp(license,lic_file))
                fprintf(ofp, "LINE  %d ---error in lc_install_license_path \n",
								__LINE__);

        if (lc_set_registry(job, "DEMO_LICENSE_FILE", 0))
                fprintf(ofp, 
		"Line %d --- An error occured when setting the registry key \n",
								__LINE__);

        term[0]=PATHSEPARATOR;
        term[1]=0;


/* next try a multiple path file path */
        strcat(license,term);
#ifdef PC
        lic_file="c:\\bin;c:\\windows;c:\\";
#else
	lic_file = "/usr:/usr/tmp:/bin:/usr/bin";
#endif
        strcat(license,lic_file);
        ret=lc_install_license_path(job,license,"demo");
        if ( ret)
                fprintf(ofp, "LINE  %d ---error in lc_install_license_path \n",
								__LINE__);
        lc_get_registry(job,"DEMO_LICENSE_FILE",&lic_file);
        if (!l_keyword_eq(job, license,lic_file))
                fprintf(ofp, "LINE  %s%d expected %s, got %s\n",
								__FILE__, __LINE__, lic_file, license);

        if (lc_set_registry(job, "DEMO_LICENSE_FILE", 0))
                fprintf(ofp, 
		"Line %d --- An error occured when setting the registry key \n",
								__LINE__);



/* next try concatenating the path */
        /* set DEMO_LICENSE_FILE to the current directory */
        getcwd(license,2048);
        if (lc_set_registry(job, "DEMO_LICENSE_FILE", license))
                fprintf(ofp, 
		"Line %d --- An error occured when setting the registry key \n",
								__LINE__);
        
        /* find the default directory */
        strcpy(temp_buf,LM_DEFAULT_LICENSE_FILE);
        ptr=strrchr(temp_buf,PATHTERMINATOR);
        *ptr=0;

        ret=lc_install_license_path(job,temp_buf,"demo");
        if ( ret)
                fprintf(ofp, "LINE  %d ---error in lc_install_license_path \n",
								__LINE__);
        lc_get_registry(job,"DEMO_LICENSE_FILE",&lic_file);
        sprintf(buffer,"%s%s%s",temp_buf,term,license);
        if (strcmp(buffer,lic_file))
                fprintf(ofp, "LINE  %d ---error in lc_install_license_path \n",
								__LINE__);

        if (lc_set_registry(job, "DEMO_LICENSE_FILE", 0))
                fprintf(ofp, 
		"Line %d --- An error occured when setting the registry key \n",
								__LINE__);
#if 0
/* next try pre v5.12  */
	lc_set_attr(job, LM_A_BEHAVIOR_VER, (LM_A_VAL_TYPE)LM_BEHAVIOR_V5);
        ret=lc_install_license_path(job,license,"demo");
        if (! ret)
                fprintf(ofp, "LINE  %d ---error in lc_install_license_path \n",
								__LINE__);
/* next try v5.12 */
	lc_set_attr(job, LM_A_BEHAVIOR_VER, (LM_A_VAL_TYPE)LM_BEHAVIOR_V5_1);


        ret=lc_install_license_path(job,license,"demo");
        if ( !ret)
                fprintf(ofp, "LINE  %d ---error in lc_install_license_path \n",
								__LINE__);
	lc_set_attr(job, LM_A_BEHAVIOR_VER, (LM_A_VAL_TYPE)LM_BEHAVIOR_V7);
        lc_get_registry(job,"LM_LICENSE_FILE",&lic_file);
        if (lic_file && strcmp(license,lic_file))
                fprintf(ofp, "LINE  %d ---error in lc_install_license_path \n",
								__LINE__);

#endif
        if (lc_set_registry(job, "LM_LICENSE_FILE", 0))
                fprintf(ofp, 
		"Line %d --- An error occured when setting the registry key \n",								__LINE__);

        lc_free_job(job);


/************* Finshed with l_getenv ********************/

/****** Now test lc_install_license *****************  */

/*** Parameter Checking???*******/

/* First Try with no existing path */
	_lm_init("demo", &code, &job, __LINE__);

        if (get_license_file(feature_line,"installa.dat"))
                fprintf(ofp, "Error reading file installa.dat at line %d\n",								__LINE__);
        getcwd(license,2048);
        ret =lc_install_license(job, feature_line, &path, &action);
        if (ret )
                fprintf(ofp, 
		"Line %d -- Error in lc_install_license Function, failed \n",								__LINE__);

        if ( action !=2 )
                fprintf(ofp, 
	"Line %d -- Error in lc_install_license action parameter wrong\n",
								__LINE__);

        if ( path )
	{
                strcpy(save_license_file_path,path);
		lc_free_mem(job, path);
		path = 0;
	}

        lc_get_registry(job,"DEMO_LICENSE_FILE",&lic_file);

        /* find the default directory */
        strcpy(temp_buf,LM_DEFAULT_LICENSE_FILE);
        ptr=strrchr(temp_buf,PATHTERMINATOR);
        *ptr=0;
        if (!lic_file || strcmp(temp_buf,lic_file))
                fprintf(ofp, 
			"LINE  %d ---error in path for license installed \n",
								__LINE__);
        
        
        lc_free_job(job);
	
	_lm_init("demo", &code, &job, __LINE__);
	
	if ( lc_checkout(job, "l_install_test" , "1.0", 1, LM_CO_NOWAIT, &code, 
                                                                LM_DUP_NONE))
        {
                fprintf(ofp, 
		"Line %d -- Error in lc_install_license, checkout failed \n",
								__LINE__);
                lc_perror(job,"l_install_license_failed");
        }
        lc_free_job(job);
/* Next Try with one existing path */
	_lm_init("demo", &code, &job, __LINE__);
	
	if (get_license_file(feature_line,"installa.dat"))
                fprintf(ofp, "Error reading file installa.dat at line %d\n",
								__LINE__);

        ret =lc_install_license(job, feature_line, &path2, &action);
        if (ret)
                fprintf(ofp, "Line %d -- Error in lc_install_license \n",
								__LINE__);


        lc_free_job(job);
	_lm_init("demo", &code, &job, __LINE__);
	
	if ( lc_checkout(job, "l_install_test" , "1.0", 1, LM_CO_NOWAIT, &code, 
                                                                LM_DUP_NONE))
        {
                fprintf(ofp, 
		"Line %d -- Error in lc_install_license, checkout failed \n",
							__LINE__);
                lc_perror(job,"l_install_license_failed");
        }
        lc_free_job(job);
	_lm_init("demo", &code, &job, __LINE__);

/* next try with multiple existing paths */     
        lc_get_registry(job,"DEMO_LICENSE_FILE",&lic_file);
        getcwd(license,2048);
        sprintf(temp_buf2,"%s%s%s%s%s%s%s",license,term,lic_file,term,"@speedy",term,LM_DEFAULT_LICENSE_FILE);
        if (lc_set_registry(job, "DEMO_LICENSE_FILE",temp_buf2 ))
                fprintf(ofp, "Line %d --- An error occured when setting the registry key \n",__LINE__);

        lc_free_job(job);
	_lm_init("demo", &code, &job, __LINE__);

        if (get_license_file(feature_line,"installb.dat"))
                fprintf(ofp, "Error reading file installb.dat at line %d\n",
								__LINE__);

        ret =lc_install_license(job, feature_line, &path3, &action);
        if (ret)
                fprintf(ofp, "Line %d -- Error in lc_install_license \n",
								__LINE__);
	
        lc_free_job(job);
	
	_lm_init("demo", &code, &job, __LINE__);

	
        if ( lc_checkout(job, "l_install_test2" , "1.0", 1, LM_CO_NOWAIT, &code, 
                                                                LM_DUP_NONE))
        {
                fprintf(ofp, 
		"Line %d -- Error in lc_install_license, checkout failed \n",
								__LINE__);
                lc_perror(job,"l_install_license_failed");
        }
        lc_free_job(job);

        if (path3) unlink(path3);

        if (save_license_file_path[0]) unlink(save_license_file_path);

        *save_license_file_path = 0;

/* Try with multiple Vendors  */
	_lm_init("demo", &code, &job, __LINE__);
        if (lc_set_registry(job, "DEMO_LICENSE_FILE", 0))
                fprintf(ofp, 
		"Line %d --- An error occured when setting the registry key \n",
								__LINE__);

        lc_free_job(job);
	_lm_init("demo", &code, &job, __LINE__);

	
        if (get_license_file(feature_line,"installc.dat"))
                fprintf(ofp, "Error reading file installc.dat at line %d\n",
								__LINE__);

        ret =lc_install_license(job, feature_line, &path4, &action);
        if (ret)
                fprintf(ofp, "Line %d -- Error in lc_install_license \n",
								__LINE__);
	else if (path4)  
        { 
                lc_free_mem(job, path4); path4 = 0 ;
        }


        lc_free_job(job);
	_lm_init("demo", &code, &job, __LINE__);

	if ( lc_checkout(job, "l_install_test2" , "1.0", 1, LM_CO_NOWAIT, &code, 
                                                                LM_DUP_NONE))
        {
                fprintf(ofp, 
		"Line %d -- Error in lc_install_license, checkout failed \n",
								__LINE__);
                lc_perror(job,"l_install_license_failed");
        }

        if (lc_set_registry(job, "DEMOF_LICENSE_FILE", 0))
                fprintf(ofp, 
		"Line %d --- An error occured when setting the registry key \n",
								__LINE__);

/* try old version */

	/* only effect was tried in the above code */

/* Done with tests */
        if (lc_set_registry(job, "DEMO_LICENSE_FILE", 0))
                fprintf(ofp, 
	"Line %d --- An error occured when setting the registry key \n",									__LINE__);

        if (lc_set_registry(job, "LM_LICENSE_FILE", 0))
                fprintf(ofp, 
	"Line %d --- An error occured when setting the registry key \n", __LINE__);
#ifdef PC
	{
	  char *tmp;
                tmp = (char *)malloc(strlen("LM_LICENSE_FILE") + 
                                strlen(&servtest_lic[0]) + 3);
                sprintf(tmp, "LM_LICENSE_FILE=%s", servtest_lic);
                _putenv(tmp);
		free(tmp);
	}
#else
	setenv("LM_LICENSE_FILE", servtest_lic);
#endif
        lc_free_job(job);
	if (path) unlink(path);
	if (path2) {unlink(path2); free(path2);}
	if (path3) {unlink(path3); free(path3);}
	if (path4) unlink(path4);
        return 0;

}
