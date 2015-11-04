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
		/*private:*/
		public:
		BitSize bitsAllocCount;
		BitSize mWritePosBits;
		BitSize mReadPosBits;
		char *base;
		/// true if the internal buffer is copy of the data passed to the constructor
		bool mUseHeapBuf;
		char mStacBuffer[JACKIESTREAM_STACK_ALLOC_SIZE];

		public:
		STATIC_FACTORY_DECLARATIONS(JackieStream);

		/// Getters and Setters
		BitSize WritePosBits() const { return mWritePosBits; }
		BitSize WritePosByte() const { return BITS_TO_BYTES(mWritePosBits); }
		void WritePosBits(BitSize val) { mWritePosBits = val; }
		BitSize ReadPosBits() const { return mReadPosBits; }
		void ReadPosBits(BitSize val) { mReadPosBits = val; }
		char * Buffer() const { return base; }
		void Buffer(char * val) { base = val; }

		///========================================
		/// @Param [in] [ BitSize initialBytesToAllocate]:
		/// the number of bytes to pre-allocate.
		/// @Remarks:
		/// Create the bitstream, with some number of bytes to immediately
		/// allocate. There is no benefit to calling this, unless you know exactly
		/// how many bytes you need and it is greater than 256.
		/// @Author mengdi[Jackie]
		///========================================
		JackieStream(const BitSize initialBytesToAllocate);

		///========================================
		/// @Brief  Initialize by immediately setting the +data to a predefined pointer.
		/// @Access  public  
		/// @Param [in] [char * data]  
		/// @Param [in] [const  BitSize len]  unit of byte
		/// @Param [in] [bool copy]  
		/// true to make an deep copy of the +data . 
		/// false to just save a pointer to the +data.
		/// @Remarks
		/// 99% of the time you will use this function to read Packet:;data, 
		/// in which case you should write something as follows:
		/// JACKIE_INET::JackieStream js(packet->data, packet->length, false);
		/// @Author mengdi[Jackie]
		///========================================
		JackieStream(char* data, const BitSize len, bool copy);
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

		BitSize GetPayLoadBits(void) const { return mWritePosBits - mReadPosBits; }

		///========================================
		/// @Function ReadBits
		/// @Brief   Read numbers of bit into dest array
		/// @Access public
		/// @Param [out] [unsigned char * dest]  The destination array
		/// @Param [in] [BitSize bitsRead] The number of bits to read
		/// @Param [in] [const bool alignRight]  If true bits will be right aligned
		/// @Returns bool True if [bitsRead] number of bits are read
		/// @Remarks
		/// @Notice
		/// [alignBitsToRight] should be set to True to convert internal
		/// bitstream data to user data or False if you used [WriteBits()] with 
		/// [rightAlignedBits] false
		/// @Author mengdi[Jackie]
		///========================================
		bool ReadBits(char *dest, BitSize bitsRead, bool alignRight = true);

		///========================================
		/// @Function  WriteBits 
		/// @Brief  write @bitsCount number of bits into @input
		/// @Access      public  
		/// @Param [in] [const char * src] source array
		/// @Param [in] [BitSize bitsSize] the number of bits to write
		/// @Param [in] [bool rightAligned] if true particial bits will be right aligned
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
		void WriteBits(const char* src, BitSize bitsSize, bool rightAligned = true);

		/// Can only print 4096 size of char no materr is is bit or byte
		/// mainly used for dump binary data
		void PrintBit(void);
		static void PrintBit(char* outstr, BitSize bitsPrint, char* src);
		void PrintHex(void);
		static void PrintHex(char* outstr, BitSize bitsPrint, char* src);
	};
}
#endif  //__BITSTREAM_H__
