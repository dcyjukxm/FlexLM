/****************************************************************************
*
* (C) Unpublished Copyright Novell, Inc. All Rights Reserved.
* (c) Copyright.  1988-1990 WATCOM Systems Inc. All rights reserved.
*
* No part of this file may be duplicated, revised, translated,
* localized or modified in any manner or compiled, linked or 
* uploaded or downloaded to or from any computer system without
* the prior written consent of Novell, Inc.
*
* NetWare C NLM Runtime Library source code
*
*****************************************************************************/

#ifndef _SYS_STAT_H_INCLUDED

#include <sys\types.h>
#include <stdlib.h>

struct stat 
   {
   dev_t          st_dev;        /* volume number of volume file resides on  */
   ino_t          st_ino;        /* NW386 directory entry number             */
   unsigned short st_mode;       /* emulated file mode                       */
   short          st_nlink;      /* # of hard links, always 1                */
   unsigned long  st_uid;        /* object id of owner                       */
   short          st_gid;        /* group-id, always 0                       */
   dev_t          st_rdev;       /* device type, always 0                    */
   off_t          st_size;       /* total file size - files only             */
   time_t         st_atime;      /* last access date - files only            */
   time_t         st_mtime;      /* last modify date and time                */
   time_t         st_ctime;      /* Under POSIX is last status change time   */
                                 /*  under NW386, is creation date/time      */
   time_t         st_btime;      /* last archived date and time              */
   unsigned long  st_attr;       /* file attributes                          */
   unsigned long  st_archivedID; /* user/object ID that last archived file   */
   unsigned long  st_updatedID;  /* user/object ID that last updated file    */
                                 /* inherited rights mask                    */
   unsigned short st_inheritedRightsMask; 
                                 /* namespace file was created in            */
   unsigned char  st_originatingNameSpace;
                                 /* ASCIIZ filename                          */
   unsigned char  st_name[_MAX_NAME]; 
   };

/*----------------------------------------------------------------------------*
 *   File attribute constants for st_attr field                               *
 *----------------------------------------------------------------------------*/
#define _A_NORMAL           0x00 /* Normal file - read/write permitted       */
#define _A_RDONLY           0x01 /* Read-only file                           */
#define _A_HIDDEN           0x02 /* Hidden file                              */
#define _A_SYSTEM           0x04 /* System file                              */
#define _A_EXECUTE          0x08 /* Execute only file                        */
#define _A_SUBDIR           0x10 /* Subdirectory                             */
#define _A_ARCH             0x20 /* Archive file                             */
#define _A_SHARE            0x80 /* Used for compatibility with DOS software */
                                 /*  that is not network aware               */

/* Extended attributes:     (NYI means Not Yet Implemented)                  */

#define _A_TRANS      0x00001000 /* Transactional - file will use TTS        */
#define _A_READAUD    0x00004000 /* Read audit - NYI in NW386 V3.1           */
#define _A_WRITAUD    0x00008000 /* Write audit - NYI in NW386 V3.1          */

#define _A_IMMPURG    0x00010000 /* Immediate purge                          */
#define _A_NORENAM    0x00020000 /* Mac rename inhibit                       */
#define _A_NODELET    0x00040000 /* Mac delete inhibit                       */
#define _A_NOCOPY     0x00080000 /* Mac copy inhibit                         */


/*                       Unix permission stuff                               */

#define S_IFMT          0170000 /* type of file                              */
#define S_IFDIR         0040000 /* directory                                 */
#define S_IFCHR         0020000 /* character special file                    */
#define S_IFREG         0100000 /* regular                                   */

#define S_ISDIR( m )    (((m) & S_IFMT) == S_IFDIR)
#define S_ISCHR( m )    (((m) & S_IFMT) == S_IFCHR)
#define S_ISREG( m )    (((m) & S_IFMT) == S_IFREG)
/* the following two are not possible on DOS or NetWare                      */
#define S_ISBLK( m )    0
#define S_ISFIFO( m )   0

/* owner permission                                                          */
#define S_IRWXU         0000700
#define S_IRUSR         0000400
#define S_IWUSR         0000200
#define S_IXUSR         0000100
#define S_IREAD         0000400
#define S_IWRITE        0000200
#define S_IEXEC         0000100

/* group permission.  same as owner's                                        */
#define S_IRWXG         0000070
#define S_IRGRP         0000040
#define S_IWGRP         0000020
#define S_IXGRP         0000010

/* other permission.  same as owner's                                        */
#define S_IRWXO         0000007
#define S_IROTH         0000004
#define S_IWOTH         0000002
#define S_IXOTH         0000001

/* setuid, setgid, and sticky.  always false                                 */
#define S_ISUID         0004000
#define S_ISGID         0002000
#define S_ISVTX         0001000

#ifdef __cplusplus
extern "C" {
#endif

int   fstat(
         int,
         struct stat * );

int   stat(
         const char *,
         struct stat * );

#ifdef __cplusplus
}
#endif

#define _SYS_STAT_H_INCLUDED
#endif  /* _SYS_STAT_H_INCLUDED */
