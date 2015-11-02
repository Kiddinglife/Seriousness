//  [11/2/2015 mengdi]
#ifndef __BITSTREAM_H__
#define __BITSTREAM_H__

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <float.h>
#include <string.h>
#include <math.h>

#if defined(_WIN32)
#include "WindowsIncludes.h"
#endif

#include "DLLExport.h"
#include "OverrideMemory.h"
#include "SockOSIncludes.h"
#include "DefaultNetDefines.h"
#include "NetTypes.h"

// MSWin uses _copysign, others use copysign...
#ifndef _WIN32
#define _copysign copysign
#endif

namespace JACKIE_INET
{

	/// This class allows you to write and read native types as a string of bits.  
	class JACKIE_EXPORT JackieStream
	{
		private:
		unsigned int bitsAllocCount;
		unsigned int mWritePosBits;
		unsigned int mReadPosBits;
		char *mBuffer;
		/// true if the internal buffer is copy of the data passed to the constructor
		bool mUseHeapBuf;
		char mStacBuffer[JACKIESTREAM_STACK_ALLOC_SIZE];

		public:
		STATIC_FACTORY_DECLARATIONS(JackieStream);

		/// Getters and Setters
		unsigned int WritePosBits() const { return mWritePosBits; }
		unsigned int WritePosByte() const { return BITS_TO_BYTES(mWritePosBits); }
		void WritePosBits(unsigned int val) { mWritePosBits = val; }
		unsigned int ReadPosBits() const { return mReadPosBits; }
		void ReadPosBits(unsigned int val) { mReadPosBits = val; }
		char * Buffer() const { return mBuffer; }
		void Buffer(char * val) { mBuffer = val; }

		///========================================
		/// @Param [in] [ unsigned int initialBytesToAllocate]:
		/// the number of bytes to pre-allocate.
		/// @Remarks:
		/// Create the bitstream, with some number of bytes to immediately
		/// allocate. There is no benefit to calling this, unless you know exactly
		/// how many bytes you need and it is greater than 256.
		/// @Author mengdi[Jackie]
		///========================================
		JackieStream(const unsigned int initialBytesToAllocate);

		///========================================
		/// @Brief  Initialize by immediately setting the +data to a predefined pointer.
		/// @Access  public  
		/// @Param [in] [char * data]  
		/// @Param [in] [const  unsigned int len]  unit of byte
		/// @Param [in] [bool copy]  
		/// true to make an deep copy of the +data . 
		/// false to just save a pointer to the +data.
		/// @Remarks
		/// 99% of the time you will use this function to read Packet:;data, 
		/// in which case you should write something as follows:
		/// JACKIE_INET::JackieStream js(packet->data, packet->length, false);
		/// @Author mengdi[Jackie]
		///========================================
		JackieStream(char* data, const unsigned int len, bool copy);
		JackieStream();
		~JackieStream();

		///========================================
		/// @Function  Reuse 
		/// @Brief  Resets for reuse.
		/// @Access  public  
		/// @Notice
		/// Do NOT reallocate memory because JackieStream is used
		/// to serialize/deserialize a buffer. Reallocation is a dangerous 
		/// operation (may result in leaks).
		/// @Author mengdi[Jackie]
		///========================================
		void Reuse(void) { mWritePosBits = mReadPosBits = 0; }

		unsigned int  GetPayLoadLen(void) const { return mWritePosBits - mReadPosBits; }

		///========================================
		/// @Function ReadBits
		/// @Brief   Read numbers of bit into dest array
		/// @Access public
		/// @Param [out] [unsigned char * dest]  The destination array
		/// @Param [in] [unsigned int bitsCount] The number of bits to read
		/// @Param [in] [const bool alignBits2Right]  If true bits will be right aligned
		/// @Returns bool True if [bitsCount] number of bits are read
		/// @Remarks
		/// @Notice
		/// [alignBitsToRight] should be set to True to convert internal
		/// bitstream data to user data or False if you used [WriteBits()] with 
		/// [rightAlignedBits] false
		/// @Author mengdi[Jackie]
		///========================================
		bool ReadBits(unsigned char *dest, unsigned int bitsCount,
			const bool alignBits2Right = true);

		///========================================
		/// @Function  WriteBits 
		/// @Brief  write @bitsCount number of bits into @input
		/// @Access      public  
		/// @Param [in] [const char * src] source array
		/// @Param [in] [unsigned int bitsCount] the number of bits to write
		/// @Param [in] [bool rightAlignedBits] if true particial bits will be right aligned
		/// @Returns void
		/// @Remarks
		/// @Notice
		/// in the case of a partial byte, the bits are aligned
		/// from the right (bit 0) rather than the left (as in the normal
		/// internal representation) You would set this to true when
		/// writing user data, and false when copying bitstream data, such
		/// as writing one bitstream to another.
		/// @Author mengdi[Jackie]
		///========================================
		void WriteBits(const char* src, unsigned int  bitsCount, bool rightAlignedBits = true);

		/// Can only print 4096 size of char no materr is is bit or byte
		/// mainly used for dump binary data
		void PrintBit(void);
		void PrintBit(char* out);
		void PrintHex(void);
		void PrintHex(char* out);
	};
}
#endif  //__BITSTREAM_H__
