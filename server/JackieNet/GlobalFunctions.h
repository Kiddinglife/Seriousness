/*
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef GlobalFunctions_H_
#define GlobalFunctions_H_

#include <stdio.h>
#include "DLLExport.h"

/////////////////////////////////////////// SuperFastHash //////////////////////////////////////////////
/// From http://www.azillionmonkeys.com/qed/hash.html
/// Author of main code is Paul Hsieh. I just added some convenience functions
/// Also note http://burtleburtle.net/bob/hash/doobs.html, which shows that this is 20%
/// faster than the one on that page but has more collisions
extern  unsigned long JACKIE_EXPORT SuperFastHash(const char * data, int length);
extern  unsigned long JACKIE_EXPORT SuperFastHashIncremental(const char * data,
	int len, unsigned int lastHash);
extern  unsigned long JACKIE_EXPORT SuperFastHashFile(const char * filename);
extern  unsigned long JACKIE_EXPORT SuperFastHashFilePtr(FILE *fp);
//////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////
/// Return false if Numeric IP address. Return true if domain NonNumericHostString
extern  bool JACKIE_EXPORT isDomainIPAddr(const char *host);
//////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////////
/// Fast itoa from http://www.jb.man.ac.uk/~slowe/cpp/itoa.html for Linux since it seems like
/// Linux doesn't support this function. I modified it to remove the std dependencies.
extern char* JACKIE_EXPORT Itoa(int value, char* result, int base);
//////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////// DomainNameToIP /////////////////////////////////////////
/// Every platform except windows store 8 and native client supports Berkley sockets
#if !defined(WINDOWS_STORE_RT)
extern void DomainNameToIP_Berkley_IPV4And6(const char *domainName, char ip[65]);
extern void DomainNameToIP_Berkley_IPV4(const char *domainName, char ip[65]);
#else
extern void DomainNameToIP_Non_Berkley(const char *domainName, char ip[65]);
#endif ///  !defined(WINDOWS_STORE_RT)
inline extern void JACKIE_EXPORT DomainNameToIP(const char *domainName, char ip[65])
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
extern void JACKIE_EXPORT JACKIE_Sleep(unsigned int ms);
//////////////////////////////////////////////////////////////////////////////////////////////////////


#endif
