/*
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef GlobalFunctions_H_
#define GlobalFunctions_H_

#include <stdio.h>
#include "DLLExport.h"
#include "BasicTypes.h"

/////////////////////////////////////////// SuperFastHash //////////////////////////////////////////////
/// From http://www.azillionmonkeys.com/qed/hash.html
/// Author of main code is Paul Hsieh. I just added some convenience functions
/// Also note http://burtleburtle.net/bob/hash/doobs.html, which shows that this is 20%
/// faster than the one on that page but has more collisions
JACKIE_EXPORT extern  unsigned long  SuperFastHash(const char * data, int length);
JACKIE_EXPORT extern  unsigned long  SuperFastHashIncremental(const char * data, int len, unsigned int lastHash);
JACKIE_EXPORT extern  unsigned long  SuperFastHashFile(const char * filename);
JACKIE_EXPORT extern  unsigned long  SuperFastHashFilePtr(FILE *fp);
//////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////
/// Return false if Numeric IP address. Return true if domain NonNumericHostString
JACKIE_EXPORT extern  bool  isDomainIPAddr(const char *host);
//////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////////
/// Fast itoa from http://www.jb.man.ac.uk/~slowe/cpp/itoa.html for Linux since it seems like
/// Linux doesn't support this function. I modified it to remove the std dependencies.
JACKIE_EXPORT extern char*  Itoa(int value, char* result, int base);
//////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////// DomainNameToIP /////////////////////////////////////////
/// Every platform except windows store 8 and native client supports Berkley sockets
#if !defined(WINDOWS_STORE_RT)
JACKIE_EXPORT extern void DomainNameToIP_Berkley_IPV4And6(const char *domainName, char ip[65]);
JACKIE_EXPORT extern void DomainNameToIP_Berkley_IPV4(const char *domainName, char ip[65]);
#else
JACKIE_EXPORT extern void DomainNameToIP_Non_Berkley(const char *domainName, char ip[65]);
#endif ///  !defined(WINDOWS_STORE_RT)
JACKIE_EXPORT inline  extern void  DomainNameToIP(const char *domainName, char ip[65])
{
#if defined(WINDOWS_STORE_RT)
	return DomainNameToIP_Non_Berkley(domainName, ip);
#elif defined(__native_client__)
	return DomainNameToIP_Berkley_IPV4And6(domainName, ip);
#elif defined(_WIN32)
	return DomainNameToIP_Berkley_IPV4And6(domainName, ip);
#else
	return DomainNameToIP_Berkley_IPV4And6(domainName, ip);
#endif
}
//////////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////// JACKIE_Sleep /////////////////////////////////////////
JACKIE_EXPORT extern void  JACKIE_Sleep(unsigned int ms);
//////////////////////////////////////////////////////////////////////////////////////////////////////

inline JACKIE_EXPORT extern UInt64 JackieAlignBinary(UInt64 ptr, unsigned char alignment)
{
	unsigned int const tmp = alignment - 1;
	return ( ptr + tmp ) & ( ~tmp );
}
/// Return the next address aligned to a required boundary
inline JACKIE_EXPORT extern char* JackiePointerAlignBinary(char const * ptr, unsigned char alignment)
{
	return reinterpret_cast<char *> (JackieAlignBinary(reinterpret_cast<UInt64> ( ptr ), 
		alignment) );
}
/// Return the next address aligned to a required boundary
inline JACKIE_EXPORT extern char* JackiePointerAlignBinary(unsigned char const * ptr, unsigned char alignment)
{
	return JackiePointerAlignBinary(reinterpret_cast<char const *> ( ptr ), alignment);
}
#endif
