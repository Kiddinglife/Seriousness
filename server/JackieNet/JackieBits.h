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
	class JACKIE_EXPORT JackieBits
	{
		/*private:*/
		public:
		BitSize mBitsAllocCount;
		BitSize mWritePosBits;
		BitSize mReadPosBits;
		unsigned  char *data;
		/// true if the internal buffer is copy of the data passed to the constructor
		bool mNeedFree;
		unsigned  char mStacBuffer[JACKIESTREAM_STACK_ALLOC_SIZE];

		public:
		STATIC_FACTORY_DECLARATIONS(JackieBits);

		/// Getters and Setters
		BitSize WritePosBits() const { return mWritePosBits; }
		BitSize WritePosByte() const { return BITS_TO_BYTES(mWritePosBits); }
		void WritePosBits(BitSize val) { mWritePosBits = val; }
		BitSize ReadPosBits() const { return mReadPosBits; }
		void ReadPosBits(BitSize val) { mReadPosBits = val; }
		unsigned char * Buffer() const { return data; }
		void Buffer(unsigned unsigned char * val) { data = val; }

		///========================================
		/// @Param [in] [ BitSize initialBytesToAllocate]:
		/// the number of bytes to pre-allocate.
		/// @Remarks:
		/// Create the bitstream, with some number of bytes to immediately
		/// allocate. There is no benefit to calling this, unless you know exactly
		/// how many bytes you need and it is greater than 256.
		/// @Author mengdi[Jackie]
		///========================================
		JackieBits(const BitSize initialBytesToAllocate);

		///========================================
		/// @Brief  Initialize by immediately setting the +data to a predefined pointer.
		/// @Access  public  
		/// @Param [in] [unsigned char * data]  
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
		JackieBits(unsigned char* data, const BitSize len, bool copy);
		JackieBits();
		~JackieBits();

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
		/// @func AppendBitsCouldRealloc 
		/// @brief reallocates (if necessary) in preparation of writing @bits2Append
		/// @access  public  
		/// @author mengdi[Jackie]
		///========================================
		void AppendBitsCouldRealloc(const BitSize bits2Append);

		///========================================
		/// @func   ReadBits
		/// @brief   Read numbers of bit into dest array
		/// @access public
		/// @param [out] [unsigned unsigned char * dest]  The destination array
		/// @param [in] [BitSize bitsRead] The number of bits to read
		/// @param [in] [const bool alignRight]  If true bits will be right aligned
		/// @returns bool True if [bitsRead] number of bits are read
		/// @remarks
		/// 1.jackie stream internal data are aligned to the left side of byte boundary.
		/// 2.user data are aligned to the right side of byte boundary.
		/// @notice
		/// 1.use True to read to user data 
		/// 2.use False to read this stream to another stream 
		/// @author mengdi[Jackie]
		///========================================
		bool ReadBits(unsigned char *dest, BitSize bitsRead, bool alignRight = true);

		///========================================
		/// @func  WriteBits 
		/// @brief  write @bitsCount number of bits into @input
		/// @access      public  
		/// @param [in] [const unsigned char * src] source array
		/// @param [in] [BitSize bits2Write] the number of bits to write
		/// @param [in] [bool rightAligned] if true particial bits will be right aligned
		/// @returns void
		/// @remarks
		/// 1.jackie stream internal data are aligned to the left side of byte boundary.
		/// 2.user data are aligned to the right side of byte boundary.
		/// @notice
		/// 1.Use true to write user data to jackie stream 
		/// 2.Use False to write this jackie stream internal data to another stream
		/// @Author mengdi[Jackie]
		///========================================
		void WriteBits(const unsigned char* src, BitSize bits2Write, bool rightAligned = true);

		/// Can only print 4096 size of unsigned char no materr is is bit or byte
		/// mainly used for dump binary data
		void PrintBit(void);
		static void PrintBit(char* outstr, BitSize bitsPrint, unsigned char* src);
		void PrintHex(void);
		static void PrintHex(char* outstr, BitSize bitsPrint, unsigned char* src);
	};
}
#endif  //__BITSTREAM_H__
