/*
*  NativeTypes.h
*  This source code is licensed under the BSD-style license found in the
*  LICENSE file in the root directory of this source tree. An additional grant
*  of patent rights can be found in the PATENTS file in the same directory.
*/
#ifndef  BasicTypes_H_
#define BasicTypes_H_

#if defined(__GNUC__) || defined(__GCCXML__) || defined(__SNC__) || defined(__S3E__)

#include <stdint.h>
typedef int8_t                     Int8;
typedef uint8_t                   UInt8;

typedef int16_t                   Int16;
typedef uint16_t                 UInt16;

typedef int32_t                   Int32;
typedef uint32_t                 UInt32;

typedef int64_t                   Int64;
typedef uint64_t                 UInt64;

//#elif !defined(_STDINT_H) && !defined(_SN_STDINT_H) && !defined(_SYS_STDINT_H_) && !defined(_STDINT) && !defined(_MACHTYPES_H_) && !defined(_STDINT_H_)
#else
typedef char            Int8;
typedef unsigned char       UInt8;

typedef short          Int16;
typedef unsigned short     UInt16;

typedef __int32				      Int32;
typedef unsigned __int32    UInt32;

#if defined(_MSC_VER) && _MSC_VER < 1300
typedef unsigned __int64  UInt64;
typedef signed __int64   	  Int64;
#else
typedef long long   	          Int64;
typedef unsigned long long int    UInt64;
#endif


#endif

#endif
