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
#include "EasyLog.h"

// MSWin uses _copysign, others use copysign...
#ifndef _WIN32
#define _copysign copysign
#endif

namespace JACKIE_INET
{

	/// This class allows you to write and read native types as a string of bits.  
	class JACKIE_EXPORT JackieBits
	{
		private:
		BitSize mBitsAllocSize;
		BitSize mWritePosBits;
		BitSize mReadPosBits;
		UInt8 *data;
		// true if @data is pointint to heap-memory pointer, 
		// false if it is stack-memory  pointer
		bool mNeedFree;
		// true if writting not allowed in which case all write functions will not work
		// false if writting is allowed 
		bool mReadOnly;
		UInt8 mStacBuffer[JACKIESTREAM_STACK_ALLOC_SIZE];

		public:
		STATIC_FACTORY_DECLARATIONS(JackieBits);

		/// Getters and Setters
		BitSize WritePosBits() const { return mWritePosBits; }
		BitSize WritePosByte() const { return BITS_TO_BYTES(mWritePosBits); }
		BitSize ReadPosBits() const { return mReadPosBits; }
		UInt8 * Data() const { return data; }
		void WritePosBits(BitSize val) { mWritePosBits = val; }
		void ReadPosBits(BitSize val) { mReadPosBits = val; }
		void BitsAllocSize(BitSize val) { mBitsAllocSize = val; }

		///========================================
		/// @Param [in] [ BitSize initialBytesToAllocate]:
		/// the number of bytes to pre-allocate.
		/// @Remarks:
		/// Create the JackieBits, with some number of bytes to immediately
		/// allocate. There is no benefit to calling this, unless you know exactly
		/// how many bytes you need and it is greater than 256.
		/// @Author mengdi[Jackie]
		///========================================
		JackieBits(const BitSize initialBytesToAllocate);

		///========================================
		/// @Brief  Initialize by immediately setting the +data to a predefined pointer.
		/// @Access  public  
		/// @Param [in] [UInt8 * src]  
		/// @Param [in] [const  ByteSize len]  unit of byte
		/// @Param [in] [bool copy]  
		/// true to make an deep copy of the @src . 
		/// false to just save a pointer to the @src.
		/// @Remarks
		/// 99% of the time you will use this function to read Packet:;data, 
		/// in which case you should write something as follows:
		/// JACKIE_INET::JackieStream js(packet->data, packet->length, false);
		/// @Author mengdi[Jackie]
		///========================================
		JackieBits(UInt8* src, const ByteSize len, bool copy = false);
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
		/// @brief 
		/// reallocates (if necessary) in preparation of writing @bits2Append
		/// all internal status will not be changed like @mWritePosBits and so on
		/// @access  public  
		/// @notice  
		/// It is caller's reponsibility to ensure 
		/// @param bits2Append > 0 and @param mReadOnly is false
		/// @author mengdi[Jackie]
		///========================================
		void AppendBitsCouldRealloc(const BitSize bits2Append);

		///========================================
		/// @func   ReadBitsTo
		/// @brief   Read numbers of bit into dest array
		/// @access public
		/// @param [out] [unsigned UInt8 * dest]  The destination array
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
		bool ReadBitsTo(UInt8 *dest, BitSize bitsRead, bool alignRight = true);

		///========================================
		/// @func  WriteBitsFrom 
		/// @brief  write @bitsCount number of bits into @input
		/// @access      public  
		/// @param [in] [const UInt8 * src] source array
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
		bool WriteBitsFrom(const UInt8* src, BitSize bits2Write, bool rightAligned = true);


		///========================================
		/// @func WriteFrom 
		/// @access  public  
		/// @brief write an array or raw data in bytes. NOT do endian swapp.
		/// default is right aligned[true]
		/// @author mengdi[Jackie]
		///========================================
		inline bool WriteFrom(const Int8* src, const ByteSize bytes2Write)
		{
			DCHECK_EQ(mReadOnly, false);
			if( mReadOnly ) return false;
			if( bytes2Write == 0 ) return false;

			return WriteBitsFrom((const UInt8*) src, BYTES_TO_BITS(bytes2Write), true);
		}

		/// \brief Write one JackieBits to another.
		/// \param[in] bits2Write bits to write
		/// \param JackieBits the JackieBits to copy from
		bool WriteFrom(JackieBits *jackieBits, BitSize bits2Write);
		inline bool WriteFrom(JackieBits &jackieBits, BitSize bits2Write)
		{
			return WriteFrom(&jackieBits, bits2Write);
		}
		inline bool WriteFrom(JackieBits *jackieBits)
		{
			return WriteFrom(jackieBits, jackieBits->GetPayLoadBits());
		}
		inline bool WriteFrom(JackieBits &jackieBits) { return WriteFrom(&jackieBits); }

		///========================================
		/// @func WriteBitZero 
		/// @access  public  
		/// @notice @mReadOnly must be false
		/// @author mengdi[Jackie]
		///========================================
		inline bool WriteBitZero(void)
		{
			DCHECK_EQ(mReadOnly, false);
			if( mReadOnly ) return false;

			AppendBitsCouldRealloc(1);
			BitSize shit = 8 - ( mWritePosBits & 7 );
			data[mWritePosBits >> 3] = ( ( data[mWritePosBits >> 3] >> shit ) << shit );
			mWritePosBits++;
			return true;

			//AppendBitsCouldRealloc(1);
			// New bytes need to be zeroed
			//if( ( mWritePosBits & 7 ) == 0 ) data[mWritePosBits >> 3] = 0;
			//mWritePosBits++;
		}

		///========================================
		/// @func WriteBitOne 
		/// @access  public  
		/// @notice @mReadOnly must be false
		/// @author mengdi[Jackie]
		///========================================
		inline bool WriteBitOne(void)
		{
			DCHECK_EQ(mReadOnly, false);
			if( mReadOnly ) return false;

			AppendBitsCouldRealloc(1);
			BitSize shit = mWritePosBits & 7;
			data[mWritePosBits >> 3] |= 0x80 >> shit; // Write bit 1
			mWritePosBits++;
			return true;
			//AddBitsAndReallocate(1);
			//BitSize_t numberOfBitsMod8 = mWritePosBits & 7;
			//if( numberOfBitsMod8 == 0 )
			//	data[mWritePosBits >> 3] = 0x80;
			//else
			//	data[mWritePosBits >> 3] |= 0x80 >> ( numberOfBitsMod8 ); // Set the bit to 1
			//mWritePosBits++;
		}

		/// \brief Align the next write and/or read to a byte boundary.  
		/// \details This can be used to 'waste' bits to byte align for efficiency reasons It
		/// can also be used to force coalesced bitstreams to start on byte
		/// boundaries so so WriteAlignedBits and ReadAlignedBits both
		/// calculate the same offset when aligning.

		///========================================
		/// @func AlignWritePosBits2ByteBoundary 
		/// @brief align @mWritePosBits to a byte boundary. 
		/// @access  public  
		/// @notice
		/// this can be used to 'waste' bits to byte align for efficiency reasons It
		/// can also be used to force coalesced bitstreams to start on byte
		/// boundaries so so WriteAlignedBits and ReadAlignedBits both
		/// calculate the same offset when aligning.
		/// @author mengdi[Jackie]
		///========================================
		inline void AlignWritePosBits2ByteBoundary(void)
		{
			mWritePosBits += 8 - ( ( ( mWritePosBits - 1 ) & 7 ) + 1 );
		}

		///========================================
		/// @func WriteAlignedBytesFrom 
		/// @brief  align the bitstream to the byte boundary and 
		/// then write the specified number of bytes.  
		/// @access  public  
		/// @param [in] [const UInt8 * src]  
		/// @param [in] [const ByteSize numberOfBytesToWrite]  
		/// @returns [void]
		/// @notice this is faster than WriteBits() but
		/// wastes the bits to do the alignment and requires you to call
		/// ReadAlignedBits() at the corresponding read position.
		/// @author mengdi[Jackie]
		///========================================
		void WriteAlignedBytesFrom(const UInt8 *src, const ByteSize numberOfBytesToWrite);

		/// Can only print 4096 size of UInt8 no materr is is bit or byte
		/// mainly used for dump binary data
		void PrintBit(void);
		static void PrintBit(char* outstr, BitSize bitsPrint, UInt8* src);
		void PrintHex(void);
		static void PrintHex(char* outstr, BitSize bitsPrint, UInt8* src);
	};
}
#endif  //__BITSTREAM_H__
