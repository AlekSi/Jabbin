#ifndef RTPTYPES_H

#define RTPTYPES_H

#include <sys/types.h>

#if defined(_MSC_VER)
typedef unsigned long u_int32_t;
typedef unsigned short u_int16_t;
typedef unsigned char u_int8_t;
typedef long int32_t;
#else
#ifndef HAVE_U_INT32_T
typedef uint32_t		   u_int32_t;
#define HAVE_U_INT32_T
#endif

#ifndef HAVE_U_INT16_T
typedef uint16_t           u_int16_t;
#define HAVE_U_INT16_T
#endif

#ifndef HAVE_U_INT8_T
typedef uint8_t           u_int8_t;
#define HAVE_U_INT8_T
#endif

#ifndef HAVE_INT32_T
typedef int32_t           int32_t;
#define HAVE_INT32_T
#endif
#endif


#endif // RTPTYPES_H
