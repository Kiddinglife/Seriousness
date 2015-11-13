#include "JackieBits.h"

namespace JACKIE_INET
{
	STATIC_FACTORY_DEFINITIONS(JackieBits, JackieBits);

	JackieBits::JackieBits() :
		mBitsAllocSize(BYTES_TO_BITS(JACKIESTREAM_STACK_ALLOC_SIZE)),
		mWritePosBits(0),
		mReadPosBits(0),
		data(mStacBuffer),
		mNeedFree(false),
		mReadOnly(false)
	{
		//memset(data, 0, JACKIESTREAM_STACK_ALLOC_SIZE);
	}

	JackieBits::JackieBits(const BitSize initialBytesToAllocate) :
		mWritePosBits(0),
		mReadPosBits(0),
		mReadOnly(false)
	{
		if (initialBytesToAllocate <= JACKIESTREAM_STACK_ALLOC_SIZE)
		{
			data = mStacBuffer;
			mBitsAllocSize = BYTES_TO_BITS(JACKIESTREAM_STACK_ALLOC_SIZE);
			mNeedFree = false;
			DCHECK_NOTNULL(data);
			//memset(data, 0, JACKIESTREAM_STACK_ALLOC_SIZE);
		}
		else
		{
			data = (UInt8*)jackieMalloc_Ex(initialBytesToAllocate, TRACE_FILE_AND_LINE_);
			mBitsAllocSize = BYTES_TO_BITS(initialBytesToAllocate);
			mNeedFree = true;
			DCHECK_NOTNULL(data);
			//memset(data, 0, initialBytesToAllocate);
		}
	}
	JackieBits::JackieBits(UInt8* src, const ByteSize len, bool copy/*=false*/) :
		mBitsAllocSize(BYTES_TO_BITS(len)),
		mWritePosBits(BYTES_TO_BITS(len)),
		mReadPosBits(0),
		mNeedFree(copy),
		mReadOnly(!copy)
	{
		if (mNeedFree)
		{
			if (len > 0)
			{
				if (len <= JACKIESTREAM_STACK_ALLOC_SIZE)
				{
					data = mStacBuffer;
					mNeedFree = false;
					mBitsAllocSize = BYTES_TO_BITS(JACKIESTREAM_STACK_ALLOC_SIZE);
					DCHECK_NOTNULL(data);
					//memset(data, 0, JACKIESTREAM_STACK_ALLOC_SIZE);
				}
				else
				{
					data = (UInt8*)jackieMalloc_Ex(len, TRACE_FILE_AND_LINE_);
					mNeedFree = true;
					DCHECK_NOTNULL(data);
					//memset(data, 0, len);
				}
				memcpy(data, src, len);
			}
			else
			{
				data = 0;
				mNeedFree = false;
			}
		}
		else
		{
			data = src;
			mNeedFree = false;
		}
	}
	JackieBits::~JackieBits()
	{
		if (mNeedFree && mBitsAllocSize > (JACKIESTREAM_STACK_ALLOC_SIZE << 3))
			jackieFree_Ex(data, TRACE_FILE_AND_LINE_);
		/// realloc and free are more efficient than delete and new  
		/// because it will not call ctor and dtor
	}

	void JackieBits::ReadMiniTo(UInt8* dest, const BitSize bits2Read, const bool isUnsigned)
	{
		UInt32 currByte;
		UInt8 byteMatch;
		UInt8 halfByteMatch;

		if (isUnsigned)
		{
			byteMatch = 0;
			halfByteMatch = 0;
		}
		else
		{
			byteMatch = 0xFF;
			halfByteMatch = 0xF0;
		}

		if (IsBigEndian())
		{
			currByte = (bits2Read >> 3) - 1;
			while (currByte > 0)
			{
				// If we read a 1 then the data is byteMatch.
				bool b;
				ReadTo(b);
				if (b)   // Check that bit
				{
					dest[currByte] = byteMatch;
					currByte--;
				}
				else /// the first byte is not matched 
				{
					// Read the rest of the bytes
					ReadBitsTo(dest, (currByte + 1) << 3);
					return;
				}
			}
		}
		else
		{
			currByte = 0;
			while (currByte < ((bits2Read >> 3) - 1))
			{
				// If we read a 1 then the data is byteMatch.
				bool b;
				ReadTo(b);
				if (b)   // Check that bit
				{
					dest[currByte] = byteMatch;
					currByte++;
				}
				else /// the first byte is not matched 
				{
					// Read the rest of the bytes
					ReadBitsTo(dest, bits2Read);
					return;
				}
			}
		}

		// If this assert is hit the stream wasn't long enough to read from
		DCHECK(GetPayLoadBits() >= 1);

		/// the upper(left aligned) half of the last byte(now currByte == 0) is a 0000
		/// (positive) or 1111 (nagative) write a bit 1 and the remaining 4 bits. 
		bool b;
		ReadTo(b);
		if (b)
		{
			ReadBitsTo(dest + currByte, 4);
			// read the remaining 4 bits
			dest[currByte] |= halfByteMatch;
		}
		else
		{
			ReadBitsTo(dest + currByte, 8);
		}
	}

	void JackieBits::AppendBitsCouldRealloc(const BitSize bits2Append)
	{
		DCHECK_EQ(mReadOnly, false);
		DCHECK_GT(bits2Append, 0);

		BitSize newBitsAllocCount = bits2Append + mWritePosBits;

		/// see if one or more new bytes need to be allocated
		if (newBitsAllocCount > 0 &&
			((mBitsAllocSize - 1) >> 3) < ((newBitsAllocCount - 1) >> 3))
		{
			// If this assert hits then we need to specify copy as true 
			// in @ctor JackieBits(UInt8* src, const ByteSize len, bool copy)
			// It needs to reallocate to hold all the data and can't do it unless we allocated to begin with
			// Often hits if you call Write or Serialize on a read-only bitstream
			//if( mReadOnly )
			//{
			//	JERROR << "this jackie bits is read-only, cannot be written !";
			//	CHECK_EQ(mReadOnly, false);
			//}

			// Less memory efficient but saves on news and deletes
			/// Cap to 1 meg buffer to save on huge allocations
			newBitsAllocCount = (bits2Append + mWritePosBits) << 1;
			if (newBitsAllocCount - (bits2Append + mWritePosBits) > 1048576)
				newBitsAllocCount = bits2Append + mWritePosBits + 1048576;

			// Use realloc and free so we are more efficient than delete and new for resizing
			BitSize bytes2Alloc = BITS_TO_BYTES(newBitsAllocCount);
			if (data == mStacBuffer)
			{
				if (bytes2Alloc > JACKIESTREAM_STACK_ALLOC_SIZE)
				{
					data = (UInt8 *)jackieMalloc_Ex(bytes2Alloc, TRACE_FILE_AND_LINE_);
					memcpy(data, mStacBuffer, BITS_TO_BYTES(mBitsAllocSize));
				}
			}
			else
			{
				data = (UInt8*)jackieRealloc_Ex(data, bytes2Alloc, TRACE_FILE_AND_LINE_);
			}

			DCHECK_NOTNULL(data);
		}

		if (newBitsAllocCount > mBitsAllocSize)
		{
			mBitsAllocSize = newBitsAllocCount;
		}
	}

	void JackieBits::ReadBitsTo(UInt8 *dest, BitSize bits2Read, bool alignRight /*= true*/)
	{
		DCHECK(bits2Read > 0);
		DCHECK(bits2Read <= GetPayLoadBits());
		//if (bits2Read <= 0 || bits2Read > GetPayLoadBits()) return;

		/// get offset that overlaps one byte boudary, &7 is same to %8, but faster
		const BitSize startReadPosBits = mReadPosBits & 7;

		/// byte position where start to read
		ByteSize readPosByte = mReadPosBits >> 3;

		/// if @mReadPosBits is aligned  do memcpy for efficiency
		if (startReadPosBits == 0)
		{
			memcpy(dest, &data[readPosByte], BITS_TO_BYTES(bits2Read));
			mReadPosBits += bits2Read;

			/// if @bitsSize is not multiple times of 8, 
			/// process the last read byte to shit the bits
			BitSize offset = bits2Read & 7;
			if (offset > 0 && alignRight)
			{
				dest[BITS_TO_BYTES(bits2Read) - 1] >>= (8 - offset);
			}
			return;
			// return true;
		}

		BitSize bitsSizeInLastByte;
		BitSize writePosByte = 0;
		memset(dest, 0, BITS_TO_BYTES(bits2Read)); /// Must set all 0 

		/// Read one complete byte each time 
		while (bits2Read > 0)
		{
			readPosByte = mReadPosBits >> 3;

			/// firstly read left-fragment bits in this byte 
			dest[writePosByte] |= (data[readPosByte] << (startReadPosBits));

			/// secondly read right-fragment bits  ( if any ) in this byte
			if (startReadPosBits > 0 && bits2Read > (8 - startReadPosBits))
			{
				dest[writePosByte] |= data[readPosByte + 1] >> (8 - startReadPosBits);
			}

			if (bits2Read >= 8)
			{
				bits2Read -= 8;
				mReadPosBits += 8;
				writePosByte++;
			}
			else
			{
				// Reading a partial byte for the last byte, shift right so the data is aligned on the right
				bitsSizeInLastByte = 8 - bits2Read;
				if (alignRight)  dest[writePosByte] >>= bitsSizeInLastByte;
				writePosByte++;
				bits2Read = 0;
			}
		}
		//return true;
	}

	void JackieBits::ReadTo(float &outFloat, float floatMin, float floatMax)
	{
		DCHECK(floatMax > floatMin);

		UInt16 percentile;
		ReadTo(percentile);

		outFloat = floatMin + ((float)percentile / 65535.0f) * (floatMax - floatMin);
		if (outFloat<floatMin) outFloat = floatMin;
		else if (outFloat>floatMax) outFloat = floatMax;
	}

	void JackieBits::ReadAlignedBytesTo(UInt8 *dest, const ByteSize bytes2Read)
	{
		DCHECK(bytes2Read > 0);
		DCHECK(GetPayLoadBits() >= BYTES_TO_BITS(bytes2Read));
		/// if (bytes2Read <= 0) return;

		/// Byte align
		AlignReadPosBitsToByteBoundary();

		/// Write the data
		memcpy(dest, data + (mReadPosBits >> 3), bytes2Read);
		mReadPosBits += bytes2Read << 3;
	}

	void JackieBits::ReadAlignedBytesTo(Int8 *dest, ByteSize &bytes2Read, const ByteSize maxBytes2Read)
	{
		bytes2Read = ReadBit();
		///ReadMiniTo(bytes2Read);
		if (bytes2Read > maxBytes2Read) bytes2Read = maxBytes2Read;
		if (bytes2Read == 0) return;
		ReadAlignedBytesTo((UInt8*)dest, bytes2Read);
	}

	void JackieBits::ReadAlignedBytesAllocTo(Int8 **dest, ByteSize &bytes2Read, const ByteSize maxBytes2Read)
	{
		jackieFree_Ex(*dest, TRACE_FILE_AND_LINE_);
		*dest = 0;
		bytes2Read = ReadBit();
		///ReadMiniTo(bytes2Read);
		if (bytes2Read > maxBytes2Read) bytes2Read = maxBytes2Read;
		if (bytes2Read == 0) return;
		*dest = (Int8*)jackieMalloc_Ex(bytes2Read, TRACE_FILE_AND_LINE_);
		ReadAlignedBytesTo((UInt8*)*dest, bytes2Read);
	}

	void JackieBits::WriteBitsFrom(const UInt8* src, BitSize bits2Write, bool rightAligned /*= true*/)
	{
		DCHECK_EQ(mReadOnly, false);
		DCHECK_GT(bits2Write, 0);
		//if( mReadOnly ) return false;
		//if( bits2Write == 0 ) return false;

		AppendBitsCouldRealloc(bits2Write);

		/// get offset that overlaps one byte boudary, &7 is same to %8, but faster
		/// @startWritePosBits could be zero
		const BitSize startWritePosBits = mWritePosBits & 7;

		// If currently aligned and numberOfBits is a multiple of 8, just memcpy for speed
		if (startWritePosBits == 0 && (bits2Write & 7) == 0)
		{
			memcpy(data + (mWritePosBits >> 3), src, bits2Write >> 3);
			mWritePosBits += bits2Write;
			return;
		}

		UInt8 dataByte;
		//const UInt8* inputPtr = src;

		while (bits2Write > 0)
		{
			dataByte = *(src++);

			/// if @dataByte is the last byte to write, we have to convert this byte into 
			/// stream internal data by shifting the bits in this last byte to left-aligned
			if (bits2Write < 8 && rightAligned) dataByte <<= 8 - bits2Write;

			/// The folowing if-else block will write one byte each time
			if (startWritePosBits == 0)
			{
				/// startWritePosBits == 0  means there are no overlapped bits to be further 
				/// processed and so we can directly write @dataByte into stream
				data[mWritePosBits >> 3] = dataByte;
			}
			else
			{
				/// startWritePosBits != 0 means there are  overlapped bits to be further 
				/// processed and so we cannot directly write @dataBytedirectly into stream
				/// we have process overlapped bits before writting

				/// firstly write the as the same number of bits from @dataByte intot
				/// @data[mWritePosBits >> 3] to that in the right-half of 
				/// @data[mWritePosBits >> 3]
				data[mWritePosBits >> 3] |= dataByte >> startWritePosBits;

				/// then to see if we have remaining bits in @dataByte to write 
				/// 1. startWritePosBits > 0 means @data[mWritePosBits >> 3] is a partial byte
				/// 2. bits2Write > ( 8 - startWritePosBits ) means the rest space in 
				/// @data[mWritePosBits >> 3] cannot hold all remaining bits in @dataByte
				/// we have to write these reamining bits to the next byte 
				if (startWritePosBits > 0 && bits2Write > (8 - startWritePosBits))
				{
					/// write remaining bits into the  byte next to @data[mWritePosBits >> 3]
					data[(mWritePosBits >> 3) + 1] = (dataByte << (8 - startWritePosBits));
				}
			}

			/// we wrote one complete byte in above codes just now
			if (bits2Write >= 8)
			{
				mWritePosBits += 8;
				bits2Write -= 8;
			}
			else ///  it is the last (could be partial) byte we wrote in the above codes,
			{
				mWritePosBits += bits2Write;
				bits2Write = 0;
			}
		}
	}
	void JackieBits::WriteFrom(JackieBits *jackieBits, BitSize bits2Write)
	{
		DCHECK_EQ(mReadOnly, false);
		DCHECK_GT(bits2Write, 0);
		DCHECK_GE(bits2Write, jackieBits->GetPayLoadBits());
		//if( mReadOnly ) return;
		//if( bits2Write == 0 ) return;
		//if( bits2Write > jackieBits->GetPayLoadBits() ) return;

		/// if numberOfBitsMod8 == 0, we call WriteBits() directly for efficiency
		BitSize numberOfBitsMod8 = (jackieBits->mReadPosBits & 7);
		if (numberOfBitsMod8 == 0)
		{
			return this->WriteBitsFrom(jackieBits->data + (jackieBits->mReadPosBits >> 3),
				bits2Write, false);
		}

		/// if numberOfBitsMod8 > 0, this means there are bits to write in the byte of
		/// @jackieBits->data[[jackieBits->mReadPosBits >> 3]]
		AppendBitsCouldRealloc(bits2Write);
		BitSize partialBits2Write = (8 - numberOfBitsMod8);

		while (partialBits2Write-- > 0)
		{
			numberOfBitsMod8 = mWritePosBits & 7;
			if ((jackieBits->data[jackieBits->mReadPosBits >> 3] &
				(0x80 >> (jackieBits->mReadPosBits & 7))))
				data[mWritePosBits >> 3] |= 0x80 >> (numberOfBitsMod8); // Write bit 1
			else // write bit 0
				data[mWritePosBits >> 3] = (data[mWritePosBits >> 3]
				>> (8 - numberOfBitsMod8)) << (8 - numberOfBitsMod8);
			jackieBits->mReadPosBits++;
			mWritePosBits++;
			bits2Write--;
		}

		/// after writting partial bits, numberOfBitsMod8 must be zero
		/// we can now safely call WriteBits() for further process
		DCHECK_EQ((jackieBits->mReadPosBits & 7), 0);
		return this->WriteBitsFrom(jackieBits->data + (jackieBits->mReadPosBits >> 3),
			bits2Write, false);

		//AppendBitsCouldRealloc(bits2Write);
		///// write all bytes for efficiency
		//if( numberOfBitsMod8 == 0 && ( mWritePosBits & 7 ) == 0 )
		//{
		//	int readOffsetBytes = jackieBits->mReadPosBits >> 3;
		//	int numBytes = bits2Write >> 3;
		//	memcpy(data + ( mWritePosBits >> 3 ),
		//		jackieBits->Data() + readOffsetBytes, numBytes);
		//	bits2Write -= BYTES_TO_BITS(numBytes);
		//	jackieBits->mReadPosBits = BYTES_TO_BITS(numBytes + readOffsetBytes);
		//	mWritePosBits += BYTES_TO_BITS(numBytes);
		//}

		///// write remaining bits one by one
		///*	bool isOne;*/
		//while( bits2Write-- > 0 && jackieBits->mReadPosBits + 1 <=
		//	jackieBits->mWritePosBits )
		//{
		//	numberOfBitsMod8 = mWritePosBits & 7;
		//	//if( numberOfBitsMod8 == 0 )
		//	//{
		//	/// see if this src bit  is 1 or 0, 0x80 (16)= 128(10)= 10000000 (2)
		//	//if( ( jackieBits->data[jackieBits->mReadPosBits >> 3] &
		//	//	( 0x80 >> ( jackieBits->mReadPosBits & 7 ) ) ) )
		//	//	// Write 1
		//	//	data[mWritePosBits >> 3] = 0x80;
		//	//else
		//	//	data[mWritePosBits >> 3] = 0;
		//	//} else
		//	//{
		//	//	/// see if this src bit  is 1 or 0, 0x80 (16)= 128(10)= 10000000 (2)
		//	//	if( ( jackieBits->data[jackieBits->mReadPosBits >> 3] &
		//	//		( 0x80 >> ( jackieBits->mReadPosBits & 7 ) ) ) )
		//	//	{
		//	//		/// set dest bit to 1 if the src bit is 1,do-nothing if the src bit is 0
		//	//		data[mWritePosBits >> 3] |= 0x80 >> ( numberOfBitsMod8 );
		//	//	}
		//	//}

		//	if( ( jackieBits->data[jackieBits->mReadPosBits >> 3] &
		//		( 0x80 >> ( jackieBits->mReadPosBits & 7 ) ) ) )
		//		data[mWritePosBits >> 3] |= 0x80 >> ( numberOfBitsMod8 ); // Write bit 1
		//	else // write bit 0
		//		data[mWritePosBits >> 3] = ( data[mWritePosBits >> 3]
		//		>> ( 8 - numberOfBitsMod8 ) ) << ( 8 - numberOfBitsMod8 );
		//	jackieBits->mReadPosBits++;
		//	mWritePosBits++;
		//}
	}

	void JackieBits::WriteFrom(float src, float floatMin, float floatMax)
	{
		DCHECK_GT(floatMax, floatMin);
		DCHECK_LE(src, floatMax + .001);
		DCHECK_GE(src, floatMin - .001);

		float percentile = 65535.0f * ((src - floatMin) / (floatMax - floatMin));
		if (percentile < 0.0f) percentile = 0.0;
		if (percentile > 65535.0f) percentile = 65535.0f;
		WriteFrom((UInt16)percentile);
	}

	void JackieBits::WriteMiniFrom(const UInt8* src, const BitSize bits2Write, const bool isUnsigned)
	{
		static bool truee = true;
		static bool falsee = false;

		ByteSize currByte;
		UInt8 byteMatch;

		if (!IsBigEndian())
		{

			/// get the highest byte with highest index  PCs
			currByte = (bits2Write >> 3) - 1;
			byteMatch = isUnsigned ? 0 : 0xFF; /// 0xFF=255=11111111

			/// From high byte to low byte, 
			/// if high byte is a byteMatch then write a 1 bit.
			/// Otherwise write a 0 bit and then write the remaining bytes
			while (currByte > 0)
			{
				///  If high byte is byteMatch (0 or 0xff)
				/// then it would have the same value shifted
				if (src[currByte] == byteMatch)
				{
					WriteFrom(truee);
					currByte--;
				}
				else /// the first byte is not matched
				{
					WriteFrom(falsee);
					// Write the remainder of the data after writing bit false
					WriteBitsFrom(src, (currByte + 1) << 3, true);
					return;
				}
			}
			/// make sure we are now on the lowest byte (index 0)
			DCHECK(currByte == 0);
		}
		else
		{
			/// get the highest byte with highest index  PCs
			currByte = 0;
			byteMatch = isUnsigned ? 0 : 0xFF; /// 0xFF=255=11111111

			/// From high byte to low byte, 
			/// if high byte is a byteMatch then write a 1 bit.
			/// Otherwise write a 0 bit and then write the remaining bytes
			while (currByte < ((bits2Write >> 3) - 1))
			{
				///  If high byte is byteMatch (0 or 0xff)
				/// then it would have the same value shifted
				if (src[currByte] == byteMatch)
				{
					WriteFrom(truee);
					currByte++;
				}
				else /// the first byte is not matched
				{
					WriteFrom(falsee);
					// Write the remainder of the data after writing bit false
					WriteBitsFrom(src, bits2Write, true);
					return;
				}
			}
			/// make sure we are now on the lowest byte (index highest)
			DCHECK(currByte == ((bits2Write >> 3) - 1));
		}

		if ((isUnsigned && (src[currByte] & 0xF0) == 0x00) ||
			(!isUnsigned && (src[currByte] & 0xF0) == 0xF0))
		{/// the upper(left aligned) half of the last byte(now currByte == 0) is a 0000 (positive) or 1111 (nagative)
			/// write a bit 1 and the remaining 4 bits. 
			WriteFrom(truee);
			WriteBitsFrom(src + currByte, 4, true);
		}
		else
		{/// write a 0 and the remaining 8 bites.
			WriteFrom(falsee);
			WriteBitsFrom(src + currByte, 8, true);
		}
	}

	void JackieBits::WriteAlignedBytesFrom(const UInt8 *src, const ByteSize numberOfBytesToWrite)
	{
		AlignWritePosBits2ByteBoundary();
		WriteBytesFrom((Int8*)src, numberOfBytesToWrite);
	}

	void JackieBits::WriteAlignedBytesFrom(const UInt8 *src, const ByteSize bytes2Write, const ByteSize maxBytes2Write)
	{
		if (src == 0 || bytes2Write == 0)
		{
			///WriteMiniFrom(bytes2Write);
			WriteBitZero();
			return;
		}
		WriteMiniFrom(bytes2Write);
		WriteAlignedBytesFrom(src, bytes2Write < maxBytes2Write ?
		bytes2Write : maxBytes2Write);
	}

	void JackieBits::PadZeroAfterAlignedWRPos(UInt32 bytes)
	{
		Int32 numToWrite = bytes - WritePosBytes();
		if (numToWrite > 0)
		{
			AlignWritePosBits2ByteBoundary();
			AppendBitsCouldRealloc(BYTES_TO_BITS(numToWrite));
			memset(data + (mWritePosBits >> 3), 0, numToWrite);
			mWritePosBits += BYTES_TO_BITS(numToWrite);
		}
	}

	void JackieBits::PrintBit(char* out, BitSize mWritePosBits, UInt8* mBuffer)
	{
		printf_s("%s[%dbits %dbytes]:\n", "BitsDumpResult",
			mWritePosBits, BITS_TO_BYTES(mWritePosBits));
		if (mWritePosBits <= 0)
		{
			strcpy(out, "no bits to print\n");
			return;
		}
		int strIndex = 0;
		int inner;
		int stopPos;
		int outter;
		int  len = BITS_TO_BYTES(mWritePosBits);

		for (outter = 0; outter < len; outter++)
		{
			if (outter == len - 1)
				stopPos = 8 - (((mWritePosBits - 1) & 7) + 1);
			else
				stopPos = 0;

			for (inner = 7; inner >= stopPos; inner--)
			{
				if ((mBuffer[outter] >> inner) & 1)
					out[strIndex++] = '1';
				else
					out[strIndex++] = '0';
			}
			out[strIndex++] = '\n';
		}

		out[strIndex++] = '\n';
		out[strIndex++] = 0;
	}
	void JackieBits::PrintBit(void)
	{
		char out[4096 * 8];
		PrintBit(out, mWritePosBits, data);
		printf_s("%s\n", out);
	}
	void JackieBits::PrintHex(char* out, BitSize mWritePosBits, UInt8* mBuffer)
	{
		printf_s("%s[%d bytes]:\n", "HexDumpResult", BITS_TO_BYTES(mWritePosBits));
		if (mWritePosBits <= 0)
		{
			strcpy(out, "no bytes to print\n");
			return;
		}
		for (BitSize Index = 0; Index < BITS_TO_BYTES(mWritePosBits); Index++)
		{
			sprintf_s(out + Index * 3, BITS_TO_BYTES(mWritePosBits), "%02x ", mBuffer[Index]);
		}

	}
	void JackieBits::PrintHex(void)
	{
		char out[4096];
		PrintHex(out, mWritePosBits, data);
		printf_s("%s\n", out);
	}

}