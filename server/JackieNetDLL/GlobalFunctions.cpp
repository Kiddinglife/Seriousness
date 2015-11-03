#include <stdlib.h>

#if !defined(_WIN32)
#include <stdint.h>
#endif
#include <iostream>

#include "GlobalFunctions.h"
#include "BasicTypes.h"
#include "SockOSIncludes.h"
#include "WSAStartupSingleton.h"
#include "NetTime.h"

#undef get16bits

#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) 
|| defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define get16bits(d) (*((const unsigned short *) (d)))
#else
#define get16bits(d) ((((unsigned int)(((const unsigned char *)(d))[1])) << 8)\
	+(unsigned int)(((const unsigned char *)(d))[0]) )
#endif

/////////////////////////////// SuperFastHash ////////////////////////////////
static const int INCREMENTAL_READ_BLOCK = 65536;
unsigned long  SuperFastHash(const char * data, int length)
{
	// All this is necessary or the hash does not match SuperFastHashIncremental
	int bytesRemaining = length;
	unsigned int lastHash = length;
	int offset = 0;
	while( bytesRemaining >= INCREMENTAL_READ_BLOCK )
	{
		lastHash = SuperFastHashIncremental(data + offset, INCREMENTAL_READ_BLOCK, lastHash);
		bytesRemaining -= INCREMENTAL_READ_BLOCK;
		offset += INCREMENTAL_READ_BLOCK;
	}
	if( bytesRemaining > 0 )
	{
		lastHash = SuperFastHashIncremental(data + offset, bytesRemaining, lastHash);
	}
	return lastHash;

	//	return SuperFastHashIncremental(data,len,len);
}
unsigned long SuperFastHashIncremental(const char * data, int len, unsigned int lastHash)
{
	unsigned int hash = (unsigned int) lastHash;
	unsigned int tmp;
	int rem;

	if( len <= 0 || data == NULL ) return 0;

	rem = len & 3;
	len >>= 2;

	/* Main loop */
	for( ; len > 0; len-- )
	{
		hash += get16bits(data);
		tmp = ( get16bits(data + 2) << 11 ) ^ hash;
		hash = ( hash << 16 ) ^ tmp;
		data += 2 * sizeof(unsigned short);
		hash += hash >> 11;
	}

	/* Handle end cases */
	switch( rem )
	{
		case 3: hash += get16bits(data);
			hash ^= hash << 16;
			hash ^= data[sizeof(unsigned short)] << 18;
			hash += hash >> 11;
			break;
		case 2: hash += get16bits(data);
			hash ^= hash << 11;
			hash += hash >> 17;
			break;
		case 1: hash += *data;
			hash ^= hash << 10;
			hash += hash >> 1;
	}

	/* Force "avalanching" of final 127 bits */
	hash ^= hash << 3;
	hash += hash >> 5;
	hash ^= hash << 4;
	hash += hash >> 17;
	hash ^= hash << 25;
	hash += hash >> 6;

	return (unsigned int) hash;

}
unsigned long SuperFastHashFile(const char * filename)
{
	FILE *fp = fopen(filename, "rb");
	if( fp == 0 )
		return 0;
	unsigned int hash = SuperFastHashFilePtr(fp);
	fclose(fp);
	return hash;
}
unsigned long SuperFastHashFilePtr(FILE *fp)
{
	fseek(fp, 0, SEEK_END);
	int length = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	int bytesRemaining = length;
	unsigned int lastHash = length;
	char readBlock[INCREMENTAL_READ_BLOCK];
	while( bytesRemaining >= ( int ) sizeof(readBlock) )
	{
		fread(readBlock, sizeof(readBlock), 1, fp);
		lastHash = SuperFastHashIncremental(readBlock, ( int ) sizeof(readBlock), lastHash);
		bytesRemaining -= ( int ) sizeof(readBlock);
	}
	if( bytesRemaining > 0 )
	{
		fread(readBlock, bytesRemaining, 1, fp);
		lastHash = SuperFastHashIncremental(readBlock, bytesRemaining, lastHash);
	}
	return lastHash;
}
/////////////////////////////// SuperFastHash ////////////////////////////////


///////////////////////// DomainIPAddr ///////////////////////////
bool isDomainIPAddr(const char *host)
{
	unsigned int i = 0;
	while( host[i] )
	{
		// IPV4: natpunch.jenkinssoftware.com
		// IPV6: fe80::7c:31f7:fec4:27de%14
		if( ( host[i] >= 'g' && host[i] <= 'z' ) ||
			( host[i] >= 'A' && host[i] <= 'Z' ) )
			return true;
		++i;
	}
	return false;
}
char* Itoa(int value, char* result, int base)
{
	// check that the base if valid
	if( base < 2 || base > 16 )
	{
		*result = 0;
		return result;
	}

	char* out = result;
	int quotient = value;
	int absQModB;

	do
	{
		// KevinJ - get rid of this dependency
		//*out = "0123456789abcdef"[ std::abs( quotient % base ) ];
		absQModB = quotient % base;
		if( absQModB < 0 )
		{
			absQModB = -absQModB;
		}
		*out = "0123456789abcdef"[absQModB];
		++out;
		quotient /= base;
	} while( quotient );

	// Only apply negative sign for base 10
	if( value < 0 && base == 10 ) *out++ = '-';

	// KevinJ - get rid of this dependency
	// std::reverse( result, out );
	*out = 0;

	// KevinJ - My own reverse code
	char *start = result;
	char temp;
	out--;
	while( start < out )
	{
		temp = *start;
		*start = *out;
		*out = temp;
		start++;
		out--;
	}

	return result;
}
///////////////////////// DomainIPAddr ///////////////////////////


/////////////////////////////////////// DomainNameToIP /////////////////////////////////////////
/// Every platform except windows store 8 and native client supports Berkley sockets
#if !defined(WINDOWS_STORE_RT)
void DomainNameToIP_Berkley_IPV4And6(const char *domainName,
	char ip[65])
{
#if NET_SUPPORT_IPV6 == 1
	struct addrinfo hints, *res, *p;
	int status;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
	hints.ai_socktype = SOCK_DGRAM;

	WSAStartupSingleton::AddRef();
	if( ( status = getaddrinfo(domainName, NULL, &hints, &res) ) != 0 )
	{
		memset(ip, 0, 65 * sizeof(char));
		WSAStartupSingleton::Deref();
		return;
	}
	WSAStartupSingleton::Deref();

	p = res;
	// 	for(p = res;p != NULL; p = p->ai_next) {
	void *addr;
	//		char *ipver;

	// get the pointer to the address itself,
	// different fields in IPv4 and IPv6:
	if( p->ai_family == AF_INET )
	{
		struct sockaddr_in *ipv4 = ( struct sockaddr_in * )p->ai_addr;
		addr = &( ipv4->sin_addr );
		strcpy(ip, inet_ntoa(ipv4->sin_addr));
	} else
	{
		// TODO - test
		struct sockaddr_in6 *ipv6 = ( struct sockaddr_in6 * )p->ai_addr;
		addr = &( ipv6->sin6_addr );
		// inet_ntop function does not exist on windows
		// http://www.mail-archive.com/users@ipv6.org/msg02107.html
		getnameinfo(( struct sockaddr * )ipv6, sizeof(struct sockaddr_in6), ip, 1, NULL, 0, NI_NUMERICHOST);
	}
	freeaddrinfo(res); // free the linked list
	//	}
#else
	DomainNameToIP_Berkley_IPV4(domainName, ip);
#endif // #if NET_SUPPORT_IPV6==1
}
void DomainNameToIP_Berkley_IPV4(const char *domainName,
	char ip[65])
{
	static struct in_addr addr;
	memset(&addr, 0, sizeof(in_addr));

	WSAStartupSingleton::AddRef();
	// Use inet_addr instead? What is the difference?
	struct hostent * phe = gethostbyname(domainName);
	WSAStartupSingleton::Deref();

	if( phe == 0 || phe->h_addr_list[0] == 0 )
	{
		printf("Yow! Bad host lookup.\n");
		memset(ip, 0, 65 * sizeof(char));
		return;
	}

	if( phe->h_addr_list[0] == 0 )
	{
		memset(ip, 0, 65 * sizeof(char));
		return;
	}

	memcpy(&addr, phe->h_addr_list[0], sizeof(struct in_addr));
	strcpy(ip, inet_ntoa(addr));
}
#else
void DomainNameToIP_Non_Berkley(const char *domainName, char ip[65])
{
	//to-do
}
#endif ///  !defined(WINDOWS_STORE_RT)
/////////////////////////////////////// DomainNameToIP /////////////////////////////////////////


//////////////////////////////// JACKIE_Sleep /////////////////////////
#if defined(_WIN32)
#include "WindowsIncludes.h" // Sleep
#else
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
pthread_mutex_t fakeMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t fakeCond = PTHREAD_COND_INITIALIZER;
#endif

#if defined(WINDOWS_PHONE_8) || defined(WINDOWS_STORE_RT)
#include "../DependentExtensions/WinPhone8/ThreadEmulation.h"
using namespace ThreadEmulation;
#endif
void  JackieSleep(unsigned int ms)
{
#if defined(_WIN32)
	Sleep(ms);
#else
	/// Single thread sleep code thanks to Furquan Shaikh, 
	/// http://somethingswhichidintknow.blogspot.com/2009/09/sleep-in-pthread.html
	///  Modified slightly from the original
	struct timespec timeToWait;
	struct timeval now;
	int rt;

	gettimeofday(&now, NULL);

	long seconds = ms / 1000;
	long nanoseconds = ( ms - seconds * 1000 ) * 1000000;
	timeToWait.tv_sec = now.tv_sec + seconds;
	timeToWait.tv_nsec = now.tv_usec * 1000 + nanoseconds;

	if( timeToWait.tv_nsec >= 1000000000 )
	{
		timeToWait.tv_nsec -= 1000000000;
		timeToWait.tv_sec++;
	}

	pthread_mutex_lock(&fakeMutex);
	rt = pthread_cond_timedwait(&fakeCond, &fakeMutex, &timeToWait);
	pthread_mutex_unlock(&fakeMutex);
#endif
}
//////////////////////////////// JACKIE_Sleep /////////////////////////

