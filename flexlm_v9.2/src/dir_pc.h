/******************************************************************************

            COPYRIGHT (c) 1997, 2003  by Macrovision Corporation.
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
 *      
 *	Module: dir_pc.h v1.1
 *      Description:  Macros to get readdir() functions.
 *
 *      C. Mayman
 *      8-Apr-97
 *
 *      Last changed:  9/24/98
 *
 */

#ifndef DIR_PC_H
#define DIR_PC_H
#include "lmachdep.h"

#if defined (PC16) || defined (NLM)
#define MAX_PATH 255
#include <dos.h>
#endif

#ifndef NLM


/*
 * Macros
 */

#undef L_OPENDIR
#undef L_READDIR
#undef L_CLOSEDIR

#define DIR_CMD_DATA PCDirInfo directoryInformation
#define L_OPENDIR(n) opendir_PC( &directoryInformation, (n) )
#define L_READDIR(d) readdir_PC( &directoryInformation, (d) )
#define L_CLOSEDIR(d) closedir_PC( &directoryInformation, (d) )

/*
 * Types.
 */

typedef void *FileData;

typedef struct dirent {
        unsigned short d_namlen;
        char d_name[MAX_PATH + 1];
} s_dirent;

typedef struct _PCDirInfo {
        FileData *fDataPtr;
        s_dirent directoryEntry;
} PCDirInfo;

typedef PCDirInfo DIR;

/*
 * API's
 */

DIR *opendir_PC( PCDirInfo *infoPtr, char *dirname );
struct dirent *readdir_PC( PCDirInfo *infoPtr, DIR *dirp );
int closedir_PC( PCDirInfo *infoPtr, DIR *dirp );

#else 

#include "dirent.h"
#endif /* NLM */
#endif /* DIR_PC_H */

