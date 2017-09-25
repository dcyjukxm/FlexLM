/******************************************************************************
 *
 *           COPYRIGHT (c) 1998, 2003 by Macrovision Corporation.
 *       This software has been provided pursuant to a License Agreement
 *       containing restrictions on its use.  This software contains
 *       valuable trade secrets and proprietary information of 
 *       Macrovision Corporation and is protected by law.  It may 
 *       not be copied or distributed in any form or medium, disclosed 
 *       to third parties, reverse engineered or used in any manner not 
 *       provided for in said License Agreement except with the prior 
 *       written authorization from Macrovision Corporation.
 *
 *****************************************************************************
 *       
 *	Module: $Id: l_instlc.c,v 1.19 2003/05/15 23:28:55 sluu Exp $
 *
 *       Function: l_install_license.c
 *
 *       Description: functions for setting license paths and files 
 *
 *       Parameters:     job- job handle
 *                       feature_line -  feature/Increment line to be 
 *				installed into
 *                       path- path to license file where the license has 
 *				been installed
 *                       action- returns what happened
 *                               1. Put it in existing file
 *                               2. Added a new file
 *       Return: Flexlm error status
 *
 *       B.Eisenberg
 *       10/17/98
 *
 *	Last changed:  12/21/98
 *
 *******************************************************************************/
#if !defined(PC16) && !defined(NLM)  && !defined(OS2)
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lm_attr.h"
#include "lgetattr.h"
#include "../utils/utilscode.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "flex_file.h"
#ifdef WINNT
#include <io.h>
#endif
#ifdef PC
#define access _access
#define unlink _unlink
#endif
#define MAX_VENDORS 100

/*  Structures */

typedef struct reg_paths {
			      struct reg_paths FAR *next;
			      int type;	/* Type of entry */
#define REG_NOTHING	0			/* Nothing */
#define REG_FILE	1			/* (FILE *) */
#define REG_DIRECTORY	2			/* Directory entry */
			      char *filepath;

			    } REG_PATH, FAR *REG_PATH_PTR;


/*  Internal Functions   */
static int path_type lm_args((char * ));
static void add_reg_path 		lm_args ((LM_HANDLE_PTR , char * ,int,
						REG_PATH **));
static int check_code_in_path		lm_args(( LM_HANDLE_PTR, char *,
							char **));
static void free_reg 			lm_args((REG_PATH_PTR ));
static int l_install_feature_text 	lm_args ((LM_HANDLE_PTR ,char *, 
						char *, char [MAX_VENDORS][MAX_DAEMON_NAME +1], int, int,
						REG_PATH_PTR, char *));
static int get_install_directory 	lm_args ((LM_HANDLE_PTR ,char **, int,
						int, REG_PATH_PTR, char [MAX_VENDORS][MAX_DAEMON_NAME +1]));
static int write_license_file 		lm_args ((LM_HANDLE_PTR , char * , 
						char *, char *, char *)); 
static int get_new_job 			lm_args(( LM_HANDLE_PTR , 
						LM_HANDLE_PTR *, char * ));
static int chk_write_ok 		lm_args((LM_HANDLE_PTR, char * ));
static void get_vendor_names_from_file 	lm_args(( LM_HANDLE_PTR, char [MAX_VENDORS][MAX_DAEMON_NAME +1]));
static void add_vendor_name 		lm_args ((LM_HANDLE_PTR, char *, 
							char [MAX_VENDORS][MAX_DAEMON_NAME +1]));
static void update_registry_entry 	lm_args((LM_HANDLE_PTR, char *, 
						char [MAX_VENDORS][MAX_DAEMON_NAME +1]));





/* Internal Definitions */

#define EXISTING_FILE 1
#define ADDED_FILE 2
#define DIRECTORY_PATH  3
#define PORT_AT_HOST_PATH 4
#define FILE_PATH 5
#define NO_PATH  6
/* Global Static Variables */

/* Functions */
API_ENTRY
int
lc_install_license(job,feature_line,path,action)
LM_HANDLE_PTR job;
char * feature_line;
char ** path;
int * action;
{

	if (LM_API_ERR_CATCH) return job->lm_errno;
	LM_API_RETURN(int, 
		l_install_license(job, feature_line, path, action, job->vendor))

}

API_ENTRY
int
la_install_license(job,feature_line,path,action, vendor)
LM_HANDLE_PTR job;
char * feature_line;
char ** path;
int * action;
char *vendor;
{
	if (LM_API_ERR_CATCH) return job->lm_errno;
	if (l_getattr(job, LMADMIN_API) != LMADMIN_API_VAL)
        {
                LM_SET_ERRNO(job, LM_FUNCNOTAVAIL, 481, 0);
                LM_API_RETURN(int,LM_FUNCNOTAVAIL)
        }
	LM_API_RETURN(int, l_install_license(job, feature_line, path, action, vendor))
}
/*
 *	return:  0 == success, !=0 failure
 */
API_ENTRY
int
l_install_license(job,feature_line,path,action, vendor)
LM_HANDLE_PTR job;
char * feature_line;
char ** path;
int * action;
char *vendor;
{
  LM_HANDLE_PTR new_job;    
  char vendor_license_file[40];
  char * vendor_path;
  char * vendor_path_copy;
  char * p;
  char * n=0; 
  int path_status;
  char * license_in_string;
  CONFIG * conf;
  int could_not_write=0;
  int feature_not_installed=0;
  char vendor_names[MAX_VENDORS][MAX_DAEMON_NAME + 1]; /* 100 daemon names */
  int file_exists = 0, dir_exists = 0;
  REG_PATH_PTR reg_paths = 0;
  char install_file_path[MAX_CONFIG_LINE+1];

	if (path) *path = 0;
	memset(install_file_path, 0, sizeof(install_file_path));
	memset(vendor_names, 0, sizeof(vendor_names));
	*action=0;
	/* First check to see what is in the path */

        sprintf(vendor_license_file,"%s_LICENSE_FILE",vendor);
	l_uppercase(vendor_license_file);
        if (l_get_registry(job, vendor_license_file, &vendor_path, 0, 0))
        {
                ;
/*      
 *	no vendor path, we need to create a file and then set path to it 
 */
        }
        else
        {

/* 
 *	There is a vendor path, now parse it and see if
 *  	1) a directory exists for the path
 *  	2)  a file exists that we can find a directory to place the new file 
 *	   into.
 */            
                vendor_path_copy=l_malloc(job,strlen(vendor_path)+1);
                n=vendor_path_copy;         /* save value to free later */
                strcpy(vendor_path_copy,vendor_path);

                while (vendor_path_copy)
                {
                        p = vendor_path_copy;
                        vendor_path_copy = l_getnextfile(vendor_path_copy);
                        if (vendor_path_copy != (char *) NULL) 
                        {
                                *vendor_path_copy = '\0';
                                vendor_path_copy++;

                        }
                        path_status=path_type(p);
                        switch (path_status)
                        {
                        
                        case DIRECTORY_PATH:
                                /* this is the best */
                                dir_exists++;
				add_reg_path(job,p,REG_DIRECTORY, &reg_paths);
                                break;  
                        case PORT_AT_HOST_PATH: 
                                /* Ignore, this doesnt help us */
                                break;  
                        case NO_PATH: 
                                /* Ignore, this doesnt help us */
                                break;  

                        case FILE_PATH:
                                /* this is not too bad */ 
                                file_exists++;
				add_reg_path(job, p, REG_FILE, &reg_paths);				
                                break;
                        
                        }             
                }
                lc_free_mem(job,n);


        }
/*  
 *	At this point we have looked at the vendor_license file path and 
 *	it either is empty, or it points to port@host, or it points to 
 *	one or more directories, or it points to one or more files
 */

	license_in_string=l_malloc(job,strlen(feature_line)+100);
	sprintf(license_in_string,"START_LICENSE\n%s\nEND_LICENSE",feature_line);

/*  
 *	First, check to see if this feature is already installed in 
 *	the vendor's path 
 */

	if (get_new_job( job, &new_job, license_in_string ))
	{
	    	free(license_in_string);
		free_reg(reg_paths);
		return 1; /* couldnt get new job */
	}

	if (conf=new_job->line)
	{
		get_vendor_names_from_file(new_job, vendor_names);	    
		do
		{
/*
 *      Now check for match between all features in new file vs all files 
 *	in path For each new feature being added check against what 
 * 	is already there                                             
 */             
			if ( ! check_code_in_path(job,conf->code, path))
			{	
				feature_not_installed++;
/* 
 *	This is the number of features not in the old path  
 *	it was not found in the path , so it will need to be added to a file 
 */
				
			}
		
			if ( conf->next)
			        conf=conf->next;
			else
				conf=0;
                
		} while (conf);
	}
	else
		feature_not_installed++;

/* 
 *	If there are features that are not in the path install the file 
 */
	if ( feature_not_installed )
	{
		if (!l_install_feature_text(job,feature_line, vendor, 
			vendor_names, file_exists, dir_exists, reg_paths, 
			install_file_path))
		{/* could not write into file */
			could_not_write=1;
		}
	
	}
	else
	{
		/* feature already existed, so did not install  */
		*action=EXISTING_FILE;
	}
  	lc_free_job(new_job);    
    	lc_free_mem(job,license_in_string);
	free_reg(reg_paths);
	if ( could_not_write )
	{
		*action=0;
		return 1; /* error */
	}
	if (!(*action))  *action=ADDED_FILE;
	if (*install_file_path && path) 
	{
		*path = l_malloc(job, strlen(install_file_path) + 1);
		strcpy(*path, install_file_path);
	}
	return 0;
	
}
/*
 *  	This function checks the current path to see if it will be 
 *	necessary to add to the path for the new feature that is 
 *	being installed.
 */
static
int
check_code_in_path(job,license_key,license_file)
LM_HANDLE_PTR job;
char * license_key;
char ** license_file;
{

  int found=0;
  CONFIG *conf;

        if (!job->line) l_init_file(job);
        if (conf=job->line)
	{
		do
		{
/*
 *                      Now check for a matching key                                             
*/             
			if (!strcmp(conf->code,license_key) )
			{
				found=1;
				if ( job->lic_files[conf->lf]  && license_file) 
				{
					*license_file = l_malloc(job,
					 strlen(job->lic_files[conf->lf]) + 1);
					strcpy(*license_file, 
						job->lic_files[conf->lf]);
					break;
				}
			}
			if ( conf->next)
				conf=conf->next;
			else
				conf=0;
                
		} while (conf);
	}

        return found;
}
/*
 *  l_install_feature
 *
 *
 */
static 
int
l_install_feature_text(job,file_text, vendor, vendor_names, file_exists,
			dir_exists, reg_paths, install_file_path)
LM_HANDLE_PTR job;
char * file_text;
char *vendor;
char vendor_names[MAX_VENDORS][MAX_DAEMON_NAME + 1]; 
int file_exists;
int dir_exists;
REG_PATH_PTR reg_paths;
char  *install_file_path;
{
 char * install_directory;


	/* First determine directory to place into  */
	if ( !get_install_directory(job,&install_directory, 
			file_exists, dir_exists, reg_paths, vendor_names))
	{
		/* Failed   */
		return 0;
	}
/* 
 *	we now have the directory for the installation  
 * 	First create a file name for placing the license in, and write 
 *	out the file  
 */
	if ( !write_license_file(job, install_directory, install_file_path,
			file_text, vendor) )
	{
		/* Failed */
		return 0;
	}

	return 1; /* success */




}
static int
get_install_directory(job,install_directory, file_exists, dir_exists,
			reg_paths, vendor_names)
LM_HANDLE_PTR job;
char ** install_directory; 
int file_exists;
int dir_exists;
REG_PATH_PTR reg_paths;
char vendor_names[MAX_VENDORS][MAX_DAEMON_NAME + 1]; 
{
 static char default_directory[256];
 char * ptr;


	/* a directory exists, so find it  */

	if ( dir_exists )
	{
	  REG_PATH_PTR r;
	  r=reg_paths;
	  while (r)
	  {
		if ( r->type==REG_DIRECTORY)
		{
			strcpy(default_directory,r->filepath);
			if ( chk_write_ok(job, default_directory) )
			{
				*install_directory = (char *)default_directory;
				return 1;
			}
			else
			/* could not write in the default directory */
			return 0;



		}
		else
			r=r->next;

	  }


	}

/* a file exists, so find the directory  */

	if ( file_exists)
	{

	  REG_PATH_PTR r;
	  r=reg_paths;
	  while (r)
	  {
		if ( r->type==REG_FILE)
		{
			/* first get the file path */
			strcpy(default_directory,r->filepath);

			/* now strip off the file name  and path terminator */
			ptr=strrchr(default_directory,PATHTERMINATOR);
			if ( NULL == ptr ) /* the path had no path, only the local directory */
			{
				if ( !getcwd( default_directory, 255 ) )
					return 0;
			}
			else
				*ptr=0;
			
			/* next check to see if you can write to this directory */
			if ( chk_write_ok(job, default_directory) )
			{
				*install_directory=default_directory;
				update_registry_entry(job,default_directory,
					vendor_names);
				return 1;
			}
			else
			/* could not write in the default directory */
				return 0;



		}
		else
			r=r->next;

	  }

	}
	/* No path exists in registry, and we didnt have any other directories to write to  */
	strcpy(default_directory,LM_DEFAULT_LICENSE_FILE);
	ptr=strrchr(default_directory,PATHTERMINATOR);
	*ptr=0;
	if ( chk_write_ok(job, default_directory) )
	{
		*install_directory=default_directory;
		update_registry_entry(job,default_directory,
			vendor_names);
		return 1;
	}
	else /* write to LM_DEFAULT_LICENSE_FILE failed */
	{
#ifdef PC
	  char tmpstr[256];
	  char * ptr;
	  	strcpy(tmpstr,LM_DEFAULT_LICENSE_FILE);
		ptr=strrchr(tmpstr,'\\');
		if ( ptr) *ptr=0;
		if ( _mkdir(tmpstr) == 0)
		{
			if ( chk_write_ok(job, default_directory) )
			{
				*install_directory=default_directory;
				update_registry_entry(job,default_directory,
					vendor_names);
				return 1;
			}
		}
		else
			return 0; /* could not create directory c:\flexlm */
#endif /* PC */			
	}
	/* could not find a place to write the new file */
	return 0;
}

static
int
write_license_file (
	LM_HANDLE_PTR	job,
	char *			dir_path,
	char *			file_name_path,
	char *			license_text,
	char *			vendor)
{
  FILE * f;
  char* file_path;
  char vendor_name[32];
  int ret;
  int i;
  long junk;
  int d,m,y;
#ifdef THREAD_SAFE_TIME
  struct tm tst;
#endif
  char * begin_ptr;
  char * end_ptr;
  char line_buffer[2049];
  int file_path_length;	
  /* create file name and see if file exists  */
        strcpy(vendor_name,vendor);
        l_uppercase(vendor_name);

        file_path=l_malloc(job,strlen(dir_path)+40);
        /* create potential file name path vendor.lic   */
#ifdef THREAD_SAFE_TIME
	l_get_date(&d, &m, &y, &junk, &tst);
#else /* !THREAD_SAFE_TIME */
	l_get_date(&d, &m, &y, &junk);
#endif
	y += 1900;
	m += 1;
	sprintf(file_path, "%s%c%s%d%02d%02d.lic",dir_path,PATHTERMINATOR,vendor_name, y, m, d);
	file_path_length=strlen(file_path);
		ret = l_flexAccess(job, file_path, 0);
        if (!ret)
        {
        /* File exists, lets find a different file name */
                int done=0;

                char * ptr;
                i=1;
                while ( !done )
                {

					ptr=file_path + ( file_path_length-4);
                    sprintf(ptr,"%d.lic",i);
                    i++;
					ret = l_flexAccess(job, file_path, 0);
                    if (ret)
						done=1;
            
                }

        }
 

		if (!(f = l_flexFopen(job, file_path, "wb")))
        {
		/* couldnt open file for  write , so punt  */
                lc_free_mem(job,file_path);
                return 0;

        }
/* 	Go through each line and if unix only write lf, if pc, write CR/lf */
	begin_ptr=license_text;
	while ( (end_ptr=strchr(begin_ptr,0x0a )))
	{
		strncpy(line_buffer,begin_ptr,(end_ptr-begin_ptr));
		line_buffer[end_ptr-begin_ptr]='\0';
		if ( line_buffer[strlen(line_buffer)-1]== 0x0d )
			line_buffer[strlen(line_buffer)-1]='\0';
		fwrite(line_buffer,sizeof(char),strlen(line_buffer),f);
#ifdef PC
		fputc('\r',f);
#endif
		fputc('\n',f);
		begin_ptr=end_ptr + 1;

	}
/* 	now go through the last line */
	if ( *begin_ptr )
	{
		strcpy(line_buffer,begin_ptr);

		if ( line_buffer[strlen(line_buffer)-1]== 0x0d )
			line_buffer[strlen(line_buffer)-1]='\0';
		fwrite(line_buffer,sizeof(char),strlen(line_buffer),f);
#ifdef PC
		fputc('\r',f);
#endif
		fputc('\n',f);
		begin_ptr=end_ptr + 1;
	}
/*  	
 *	now add a final lf to make sure that the last line 
 *	has been terminated 
 */

#ifdef PC
	putc('\r',f);
#endif
	fputc('\n',f);
	fclose(f);

        strcpy(file_name_path,file_path);
	free(file_path);

        return 1;       

}


static int
get_new_job( job, job_ptr, license_file )
LM_HANDLE_PTR  job;
LM_HANDLE_PTR * job_ptr;
char * license_file;
{
  int ret;

        if (ret=l_init(job, "lmgrd", &code, job_ptr, 0))
		return ret;
	l_set_attr(*job_ptr,LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1);
	l_set_attr(*job_ptr,LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)license_file);
	l_init_file(*job_ptr);
	return 0;

}

static int
path_type(p)
char * p;
{
  int ret=0;
  struct stat s;

	if (strchr(p,'@'))
		ret=PORT_AT_HOST_PATH;
	else
	{
		if ( !l_flexStat(NULL, p,&s) )
		{
			if (!L_S_ISDIR(s.st_mode) )
		    		ret=FILE_PATH;
			else
				ret=DIRECTORY_PATH;
			
		}
		else
			ret=NO_PATH;



	}
	return ret;		
}

static void
add_reg_path(job,p,type, reg_paths)
LM_HANDLE_PTR  job;
char * p;
int type;
REG_PATH **reg_paths;
{
  REG_PATH_PTR r,last;
	for (last=*reg_paths; last && last->next; last = last->next) ;

	r=l_malloc(job,sizeof(REG_PATH));
	r->next=0;
	r->type=type;
	r->filepath=l_malloc(job,strlen(p)+1);
	strcpy(r->filepath,p);
	
	if (last)
		last->next=r;
	else
		*reg_paths=r;

}

static void
free_reg(reg_paths)
REG_PATH_PTR reg_paths;

{
  REG_PATH_PTR r,next;
	r=reg_paths;
	for (r = reg_paths; r; r = next)
	{
		next = r->next;
		free(r->filepath);
		free(r);
	}

}

static int
chk_write_ok(
	LM_HANDLE *	job,
	char *		directory)
{
  char * ptr = NULL;
  char direct_path[MAX_LONGNAME_SIZE] = {'\0'};	/* LONGNAME */ /* OVERRUN */
  FILE * f;

#ifdef PC
	ptr=mktemp("FLXXXXXX");
#else
	ptr = "ChKwRtOk";
#endif
	sprintf(direct_path,"%s%c%s",directory, PATHTERMINATOR, ptr);
	if (!(f = l_flexFopen(job, direct_path, "w")))
	{
		return 0; /* could not write into directory */
	}
	else
	{
		fclose(f);
		l_flexUnlink(job, direct_path);
	}
	return 1;


}

static void
get_vendor_names_from_file(job, vendor_names)
LM_HANDLE_PTR  job;
char vendor_names[MAX_VENDORS][MAX_DAEMON_NAME + 1]; 
{
  CONFIG * conf;

	if (conf=job->line)
	{

		do
		{
			add_vendor_name(job, conf->daemon, vendor_names);
		
			if ( conf->next)
			        conf=conf->next;
			else
				conf=0;
                
		} while (conf);
	}


    
}

static void
add_vendor_name(job, vendor_name, vendor_names)
LM_HANDLE_PTR  job;
char * vendor_name;
char vendor_names[MAX_VENDORS][MAX_DAEMON_NAME + 1]; 
{
  int i;
  int found=0;
	for (i=0; (i < MAX_VENDORS) && *vendor_names[i]; i++)
	{
		if (L_STREQ(vendor_names[i],vendor_name))
		{
			found = 1;
			break;
		}
	}
	
	if ( !found && (i < MAX_VENDORS))
		l_zcp(vendor_names[i],vendor_name, MAX_DAEMON_NAME);

}

static void
update_registry_entry(job,path, vendor_names)
LM_HANDLE_PTR job;
char * path;
char vendor_names[MAX_VENDORS][MAX_DAEMON_NAME + 1]; 
{
  int i;
	for (i=0;(i < MAX_VENDORS) && *vendor_names[i] ; i++)
	{
		l_update_license_file(job,  path, vendor_names[i]);
	}
}


#endif
