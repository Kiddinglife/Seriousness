﻿#include <stdlib.h>

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

#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define get16bits(d) (*((const unsigned short *) (d)))
#else
#define get16bits(d) ((((unsigned int)(((const unsigned char *)(d))[1])) << 8)\
	+(unsigned int)(((const unsigned char *)(d))[0]) )
#endif

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




