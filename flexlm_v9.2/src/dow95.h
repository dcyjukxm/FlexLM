/* dow95.h */

#ifndef _DOW95_H_
#define _DOW95_H_

typedef unsigned char  uchar;
typedef unsigned short ushort;
typedef unsigned int   uint;
typedef unsigned long  ulong;

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#ifdef __cplusplus
extern "C" {
#endif

uchar  OverdriveOn     (void);
void   OverdriveOff    (void);
uchar  ToggleOverdrive (void);
uchar  setup           (uchar);
uchar  dowcheck        (void);
uchar  keyopen         (void);
uchar  keyclose        (void);
uchar  first           (void);
uchar  next            (void);
uchar  gndtest         (void);
uchar  *romdata        (void);
uchar  databyte        (uchar);                                           
uchar  access          (void);
uchar  garbagebag      (uchar *);// returns lastone for SACWD300.dll

#ifdef __cplusplus
}
#endif

#endif /* _DOW95_H_ */

/* dow95.h */


