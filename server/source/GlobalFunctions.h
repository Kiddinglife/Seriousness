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



//////////////////////////////////////////////////////////////////////////////////////////////////////
/// Fast itoa from http://www.jb.man.ac.uk/~slowe/cpp/itoa.html for Linux since it seems like
/// Linux doesn't support this function. I modified it to remove the std dependencies.
JACKIE_EXPORT extern char*  Itoa(int value, char* result, int base);
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
