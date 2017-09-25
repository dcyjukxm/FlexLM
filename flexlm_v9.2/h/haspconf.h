/*
 *  HASPCONF.H - A general configuration header for the HASP C/C++ API.
 *
 *  Determines system configuration in order to declare the hasp() routine
 *  properly, and to define constants needed for memory operations.
 *
 *  Copyright (c) 1996, 1998, 1999 Aladdin Knowledge Systems Ltd.
 */
#if !defined (HASPCONF_H_)
#define HASPCONF_H_

/*
 *  Figure out if a 16-bit or 32-bit compiler is used.
 */

/*
 *  All Win32 compilers define the macro _WIN32 (which either comes with
 *  the compiler or is defined by including windows.h).
 */
#if defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#  if !defined (HASP_WIN32)
#     define HASP_WIN32
#  endif
#  if !defined (HASP_32BIT)
#     define HASP_32BIT
#  endif
#endif

/*
 *  If the above does not succeed, define HASP_32BIT or HASP_16BIT
 *  using "limits.h", which works with all Intel compilers.
 */
#if !defined(HASP_32BIT) && !defined(HASP_16BIT)
#  include <limits.h>
#  if UINT_MAX == USHRT_MAX
#    define HASP_16BIT
#  elseif UINT_MAX == ULONG_MAX
#    define HASP_32BIT
#  endif
#endif


/*
 *  Check if the above methods worked.
 */
#if !defined(HASP_32BIT) && !defined(HASP_16BIT)
#error Unable to recognize 32 bit or 16 bit compiler.
#endif


/*
 *  HASP_FAR is defined instead of just FAR, in order to
 *  avoid name clashes with compiler/system definitions.
 */
#if !defined (HASP_FAR)
#  ifdef HASP_32BIT
#    define HASP_FAR
#  else
#    define HASP_FAR far
#  endif
#endif


#if !defined (HASPAPI)
#  if defined (HASP_DLL)
#    if defined (HASP_32BIT)
#      define HASPAPI __stdcall
#    else
#      define HASPAPI HASP_FAR pascal
#    endif
#  else
#    if defined (HASP_32BIT)
#      define HASPAPI
#    else
#      define HASPAPI __cdecl HASP_FAR
#    endif
#  endif
#endif


/*
 *  Support for block services.
 *
 *  The usage of HASP block services (i.e., ReadBlock, WriteBlock, etc.) is 
 *  different for 16-bit and 32-bit platforms. The following code handles 
 *  this difference.
 */
#define HASP_OFFSET(block)    ((unsigned int)(unsigned long)(block))
#if defined (HASP_16BIT)
#  define HASP_SEGMENT(block) ((unsigned int)((unsigned long)(block)>>16))
#else
#  define HASP_SEGMENT(block) (0)
#endif

#endif /* HASPCONF_H_ */
