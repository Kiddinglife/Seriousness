﻿#include "JackieBits.h"

namespace JACKIE_INET
{
	static const BitSize JACKIESTREAM_STACK_ALLOC_BITS_SIZE = BYTES_TO_BITS(JACKIESTREAM_STACK_ALLOC_SIZE);
	STATIC_FACTORY_DEFINITIONS(JackieBits, JackieBits);

	JackieBits::JackieBits() :
		mBitsAllocSize(JACKIESTREAM_STACK_ALLOC_BITS_SIZE),
		mWritingPosBits(0),
		mReadingPosBits(0),
		data(mStacBuffer),
		mNeedFree(false),
		mReadOnly(false)
	{
		//memset(data, 0, JACKIESTREAM_STACK_ALLOC_SIZE); // NO NEED TO SET ALL ZEROS
	}

	JackieBits::JackieBits(const BitSize initialBytesAllocate) :
		mWritingPosBits(0),
		mReadingPosBits(0),
		mReadOnly(false)
	{
		if (initialBytesAllocate <= JACKIESTREAM_STACK_ALLOC_SIZE)
		{
			data = mStacBuffer;
			mBitsAllocSize = JACKIESTREAM_STACK_ALLOC_BITS_SIZE;
			mNeedFree = false;
			DCHECK_NOTNULL(data);
			//memset(data, 0, JACKIESTREAM_STACK_ALLOC_SIZE); // NO NEED TO SET ALL ZEROS
		}
		else
		{
			data = (UInt8*)jackieMalloc_Ex(initialBytesAllocate, TRACE_FILE_AND_LINE_);
			mBitsAllocSize = BYTES_TO_BITS(initialBytesAllocate);
			mNeedFree = true;
			DCHECK_NOTNULL(data);
			//memset(data, 0, initialBytesAllocate); // NO NEED TO SET ALL ZEROS
		}
	}
	JackieBits::JackieBits(UInt8* src, const ByteSize len, bool copy/*=false*/) :
		mBitsAllocSize(BYTES_TO_BITS(len)),
		mWritingPosBits(BYTES_TO_BITS(len)),
		mReadingPosBits(0),
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
					mBitsAllocSize = JACKIESTREAM_STACK_ALLOC_BITS_SIZE;
					DCHECK_NOTNULL(data);
					//memset(data, 0, JACKIESTREAM_STACK_ALLOC_SIZE); // NO NEED TO SET ALL ZEROS
				}
				else
				{
					data = (UInt8*)jackieMalloc_Ex(len, TRACE_FILE_AND_LINE_);
					mNeedFree = true;
					DCHECK_NOTNULL(data);
					//memset(data, 0, len); // NO NEED TO SET ALL ZEROS
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
		if (mNeedFree && mBitsAllocSize > JACKIESTREAM_STACK_ALLOC_BITS_SIZE)
		{
			jackieFree_Ex(data, TRACE_FILE_AND_LINE_);
		}
	}

	void JackieBits::ReadMini(UInt8* dest, const BitSize bits2Read, const bool isUnsigned)
	{
		//JINFO << "get pay loads in read mini " << GetPayLoadBits();
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

		if (!IsBigEndian())
		{
			currByte = (bits2Read >> 3) - 1;
			//JINFO << "currByte" << currByte;
			while (currByte > 0)
			{
				// If we read a 1 then the data is byteMatch.
				bool b;
				Read(b);
				if (b)   // Check that bit
				{
					//JINFO << "matched";
					dest[currByte] = byteMatch;
					currByte--;
				}
				else /// the first byte is not matched 
				{
					//JINFO << "not matched";
					// Read the rest of the bytes
					ReadBits(dest, (currByte + 1) << 3);
					return;
				}
			}
			DCHECK(currByte == 0);
		}
		else
		{
			currByte = 0;
			//JINFO << "currByte" << currByte;
			while (currByte < ((bits2Read >> 3) - 1))
			{
				// If we read a 1 then the data is byteMatch.
				bool b;
				Read(b);
				if (b)   // Check that bit
				{
					//JINFO << "matched";
					dest[currByte] = byteMatch;
					currByte++;
				}
				else /// the first byte is not matched 
				{
					//JINFO << "ReadBits";
					// Read the rest of the bytes
					ReadBits(dest, bits2Read - (currByte << 3));
					return;
				}
			}
		}


		// If this assert is hit the stream wasn't long enough to read from
		DCHECK(GetPayLoadBits() >= 1);

		/// the upper(left aligned) half of the last byte(now currByte == 0) is a 0000
		/// (positive) or 1111 (nagative) write a bit 1 and the remaining 4 bits. 
		bool b;
		Read(b);
		if (b)
		{
			ReadBits(dest + currByte, 4);
			// read the remaining 4 bits
			dest[currByte] |= halfByteMatch;
		}
		else
		{
			ReadBits(dest + currByte, 8);
		}
	}

	void JackieBits::AppendBitsCouldRealloc(const BitSize bits2Append)
	{
		BitSize newBitsAllocCount = bits2Append + mWritingPosBits; /// official
		//BitSize newBitsAllocCount = bits2Append + mWritingPosBits + 1;

		// If this assert hits then we need to specify mReadOnly as false 
		// It needs to reallocate to hold all the data and can't do it unless we allocated to begin with
		// Often hits if you call Write or Serialize on a read-only bitstream
		DCHECK(mReadOnly == false);

		//if (newBitsAllocCount > 0 && ((mBitsAllocSize - 1) >> 3) < // official
		//	((newBitsAllocCount - 1) >> 3)) 

		/// see if one or more new bytes need to be allocated
		if (mBitsAllocSize < newBitsAllocCount)
		{
			// Less memory efficient but saves on news and deletes
			/// Cap to 1 meg buffer to save on huge allocations
			// [11/16/2015 JACKIE]  
			/// fix bug: newBitsAllocCount should plus 1MB if < 1MB, otherwise it should doule itself
			if (newBitsAllocCount > 1048576) /// 1024B*1024 = 1048576B = 1024KB = 1MB
				newBitsAllocCount += 1048576;
			else
				newBitsAllocCount <<= 1;
			// Use realloc and free so we are more efficient than delete and new for resizing
			BitSize bytes2Alloc = BITS_TO_BYTES(newBitsAllocCount);
			if (data == mStacBuffer)
			{
				if (bytes2Alloc > JACKIESTREAM_STACK_ALLOC_SIZE)
				{
					//printf("data == mStacBuffer\n");
					data = (UInt8 *)jackieMalloc_Ex(bytes2Alloc, TRACE_FILE_AND_LINE_);
					if (mWritingPosBits > 0) memcpy(data, mStacBuffer, BITS_TO_BYTES(mBitsAllocSize));
					mNeedFree = true;
				}
			}
			else
			{
				//printf("data != mStacBuffer\n");
				/// if allocate new memory, old data is copied and old memory is frred
				data = (UInt8*)jackieRealloc_Ex(data, bytes2Alloc, TRACE_FILE_AND_LINE_);
				mNeedFree = true;
			}

			DCHECK(data != 0);
		}

		if (newBitsAllocCount > mBitsAllocSize)
			mBitsAllocSize = newBitsAllocCount;
	}

	void JackieBits::ReadBits(UInt8 *dest, BitSize bits2Read, bool alignRight /*= true*/)
	{
		/// Assume bits to write are 10101010+00001111, 
		/// bits2Write = 4, rightAligned = true, and so 
		/// @mWritingPosBits = 5   @startWritePosBits = 5&7 = 5
		/// 
		/// |<-------data[0]------->|     |<---------data[1]------->|        
		///+++++++++++++++++++++++++++++++++++
		/// | 0 |  1 | 2 | 3 |  4 | 5 |  6 | 7 | 8 |  9 |10 |11 |12 |13 |14 | 15 |  src bits index
		///+++++++++++++++++++++++++++++++++++
		/// | 0 |  0 | 0 | 1 |  0 | 0 |  0 | 0 | 0 |  0 |  0 |  0 |  0  |  0 |  0 |   0 |  src  bits in memory
		///+++++++++++++++++++++++++++++++++++
		/// 
		/// start write first 3 bits 101 after shifting to right by , 00000 101 
		/// write result                                                                      00010 101

		DCHECK(bits2Read > 0);
		DCHECK(GetPayLoadBits() >= bits2Read);
		//if (bits2Read <= 0 || bits2Read > GetPayLoadBits()) return;

		/// get offset that overlaps one byte boudary, &7 is same to %8, but faster
		const BitSize startReadPosBits = mReadingPosBits & 7;

		/// byte position where start to read
		ByteSize readPosByte = mReadingPosBits >> 3;

		if (startReadPosBits == 0 && (bits2Read & 7) == 0)
		{
			memcpy(dest, data + (mReadingPosBits >> 3), bits2Read >> 3);
			mReadingPosBits += bits2Read;
			return;
		}

		/// if @mReadPosBits is aligned  do memcpy for efficiency
		if (startReadPosBits == 0)
		{
			memcpy(dest, &data[readPosByte], BITS_TO_BYTES(bits2Read));
			mReadingPosBits += bits2Read;

			/// if @bitsSize is not multiple times of 8, 
			/// process the last read byte to shit the bits
			BitSize offset = bits2Read & 7;
			if (offset > 0)
			{
				if (alignRight)
					dest[BITS_TO_BYTES(bits2Read) - 1] >>= (8 - offset);
				else
					dest[BITS_TO_BYTES(bits2Read) - 1] |= 0;
			}
			return;
			// return true;
		}

		BitSize writePosByte = 0;
		memset(dest, 0, BITS_TO_BYTES(bits2Read)); /// Must set all 0 

		/// Read one complete byte each time 
		while (bits2Read > 0)
		{
			readPosByte = mReadingPosBits >> 3;

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
				mReadingPosBits += 8;
				writePosByte++;
			}
			else
			{
				// Reading a partial byte for the last byte, shift right so the data is aligned on the right
				/*if (alignRight) dest[writePosByte] >>= (8 - bits2Read);*/

				//  [11/16/2015 JACKIE] Add: zero unused bits  
				if (alignRight)
					dest[writePosByte] >>= (8 - bits2Read); /// right align result byte: 0000 1111
				else
					dest[writePosByte] |= 0;  /// left align result byte: 1111 0000 

				///  [11/15/2015 JACKIE] fix bug of not incrementing mReadingPosBits
				mReadingPosBits += bits2Read;
				bits2Read = 0;
			}
		}
		//return true;
	}

	void JackieBits::ReadFloatRange(float &outFloat, float floatMin, float floatMax)
	{
		DCHECK(floatMax > floatMin);

		UInt16 percentile;
		//Read(percentile);
		ReadMini(percentile);

		outFloat = floatMin + ((float)percentile / 65535.0f) * (floatMax - floatMin);
		if (outFloat<floatMin) outFloat = floatMin;
		else if (outFloat>floatMax) outFloat = floatMax;
	}

	void JackieBits::ReadAlignedBytes(UInt8 *dest, const ByteSize bytes2Read)
	{
		DCHECK(bytes2Read > 0);
		DCHECK(GetPayLoadBits() >= BYTES_TO_BITS(bytes2Read));
		/// if (bytes2Read <= 0) return;

		/// Byte align
		AlignReadPosBitsByteBoundary();

		/// read the data
		memcpy(dest, data + (mReadingPosBits >> 3), bytes2Read);
		mReadingPosBits += bytes2Read << 3;
	}

	void JackieBits::ReadAlignedBytes(Int8 *dest, ByteSize &bytes2Read, const ByteSize maxBytes2Read)
	{
		bytes2Read = ReadBit();
		///ReadMini(bytes2Read);
		if (bytes2Read > maxBytes2Read) bytes2Read = maxBytes2Read;
		if (bytes2Read == 0) return;
		ReadAlignedBytes((UInt8*)dest, bytes2Read);
	}

	void JackieBits::ReadAlignedBytesAlloc(Int8 **dest, ByteSize &bytes2Read, const ByteSize maxBytes2Read)
	{
		jackieFree_Ex(*dest, TRACE_FILE_AND_LINE_);
		*dest = 0;
		bytes2Read = ReadBit();
		///ReadMini(bytes2Read);
		if (bytes2Read > maxBytes2Read) bytes2Read = maxBytes2Read;
		if (bytes2Read == 0) return;
		*dest = (Int8*)jackieMalloc_Ex(bytes2Read, TRACE_FILE_AND_LINE_);
		ReadAlignedBytes((UInt8*)*dest, bytes2Read);
	}

	void JackieBits::WriteBits(const UInt8* src, BitSize bits2Write, bool rightAligned /*= true*/)
	{
		/// Assume bits to write are 10101010+00001111, 
		/// bits2Write = 4, rightAligned = true, and so 
		/// @mWritingPosBits = 5   @startWritePosBits = 5&7 = 5
		/// 
		/// |<-------data[0]------->|     |<---------data[1]------->|        
		///+++++++++++++++++++++++++++++++++++
		/// | 0 |  1 | 2 | 3 |  4 | 5 |  6 | 7 | 8 |  9 |10 |11 |12 |13 |14 | 15 |  src bits index
		///+++++++++++++++++++++++++++++++++++
		/// | 0 |  0 | 0 | 1 |  0 | 0 |  0 | 0 | 0 |  0 |  0 |  0 |  0  |  0 |  0 |   0 |  src  bits in memory
		///+++++++++++++++++++++++++++++++++++
		/// 
		/// start write first 3 bits 101 after shifting to right by , 00000 101 
		/// write result                                                                      00010 101

		DCHECK(mReadOnly == false);
		DCHECK(bits2Write > 0);

		//if( mReadOnly ) return false;
		//if( bits2Write == 0 ) return false;

		AppendBitsCouldRealloc(bits2Write);

		/// get offset that overlaps one byte boudary, &7 is same to %8, but faster
		/// @startWritePosBits could be zero
		const BitSize startWritePosBits = mWritingPosBits & 7;

		// If currently aligned and numberOfBits is a multiple of 8, just memcpy for speed
		if (startWritePosBits == 0 && (bits2Write & 7) == 0)
		{
			memcpy(data + (mWritingPosBits >> 3), src, bits2Write >> 3);
			mWritingPosBits += bits2Write;
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
				data[mWritingPosBits >> 3] = dataByte;
			}
			else
			{
				/// startWritePosBits != 0 means there are  overlapped bits to be further 
				/// processed and so we cannot directly write @dataBytedirectly into stream
				/// we have process overlapped bits before writting
				/// firstly write the as the same number of bits from @dataByte intot
				/// @data[mWritePosBits >> 3] to that in the right-half of 
				/// @data[mWritePosBits >> 3]
				data[mWritingPosBits >> 3] |= dataByte >> startWritePosBits;

				/// then to see if we have remaining bits in @dataByte to write 
				/// 1. startWritePosBits > 0 means @data[mWritePosBits >> 3] is a partial byte
				/// 2. bits2Write > ( 8 - startWritePosBits ) means the rest space in 
				/// @data[mWritePosBits >> 3] cannot hold all remaining bits in @dataByte
				/// we have to write these reamining bits to the next byte 
				DCHECK(startWritePosBits > 0);
				if (bits2Write > (8 - startWritePosBits))
				{
					/// write remaining bits into the  byte next to @data[mWritePosBits >> 3]
					data[(mWritingPosBits >> 3) + 1] = (dataByte << (8 - startWritePosBits));
				}
			}

			/// we wrote one complete byte in above codes just now
			if (bits2Write >= 8)
			{
				mWritingPosBits += 8;
				bits2Write -= 8;
			}
			else ///  it is the last (could be partial) byte we wrote in the above codes,
			{
				mWritingPosBits += bits2Write;
				bits2Write = 0;
			}
		}
	}
	void JackieBits::Write(JackieBits *jackieBits, BitSize bits2Write)
	{
		DCHECK(mReadOnly == false);
		DCHECK(bits2Write > 0);
		DCHECK(bits2Write <= jackieBits->GetPayLoadBits());

		///?
		/// test new  implementation 20 seconds
		/// write some bits that makes jackieBits->mReadingPosBits aligned to next byte boudary
		AppendBitsCouldRealloc(bits2Write);
		BitSize numberOfBitsMod8 = (jackieBits->mReadingPosBits & 7);
		BitSize newBits2Read = 8 - numberOfBitsMod8;
		if (newBits2Read > 0)
		{
			while (newBits2Read-- > 0)
			{
				numberOfBitsMod8 = mWritingPosBits & 7;
				if (numberOfBitsMod8 == 0)
				{
					/// see if this src bit  is 1 or 0, 0x80 (16)= 128(10)= 10000000 (2)
					if ((jackieBits->data[jackieBits->mReadingPosBits >> 3] &
						(0x80 >> (jackieBits->mReadingPosBits & 7))))
						// Write 1
						data[mWritingPosBits >> 3] = 0x80;
					else
						data[mWritingPosBits >> 3] = 0;
				}
				else
				{
					/// see if this src bit  is 1 or 0, 0x80 (16)= 128(10)= 10000000 (2)
					if ((jackieBits->data[jackieBits->mReadingPosBits >> 3] &
						(0x80 >> (jackieBits->mReadingPosBits & 7))))
					{
						/// set dest bit to 1 if the src bit is 1,do-nothing if the src bit is 0
						data[mWritingPosBits >> 3] |= 0x80 >> (numberOfBitsMod8);
					}
					else
					{
						data[mWritingPosBits >> 3] |= 0;
					}
				}

				jackieBits->mReadingPosBits++;
				mWritingPosBits++;
			}
			bits2Write -= newBits2Read;
		}
		// call WriteBits() for efficient  because it writes one byte from src at one time much faster
		DCHECK((jackieBits->mReadingPosBits & 7) == 0);
		WriteBits(&jackieBits->data[jackieBits->mReadingPosBits >> 3], bits2Write, false);
		jackieBits->mReadingPosBits += bits2Write;
		///?

		/// test Old Implementation 25 seconds
		//// ? official way
		//AppendBitsCouldRealloc(bits2Write);
		//BitSize numberOfBitsMod8 = (jackieBits->mReadingPosBits & 7);
		/// write all bytes for efficiency
		//if (numberOfBitsMod8 == 0 && (mWritingPosBits & 7) == 0)
		//{
		//	int readOffsetBytes = jackieBits->mReadingPosBits >> 3;
		//	int numBytes = bits2Write >> 3;
		//	memcpy(data + (mWritingPosBits >> 3),
		//		jackieBits->data + readOffsetBytes, numBytes);
		//	bits2Write -= BYTES_TO_BITS(numBytes);
		//	jackieBits->mReadingPosBits = BYTES_TO_BITS(numBytes + readOffsetBytes);
		//	mWritingPosBits += BYTES_TO_BITS(numBytes);
		//}
		/// write remaining bits one by one
		//while (bits2Write-- > 0 && jackieBits->GetPayLoadBits() >= 1)
		//while (bits2Write-- > 0)
		//{
		//	numberOfBitsMod8 = mWritingPosBits & 7;
		//	if (numberOfBitsMod8 == 0)
		//	{
		//		/// see if this src bit  is 1 or 0, 0x80 (16)= 128(10)= 10000000 (2)
		//		if ((jackieBits->data[jackieBits->mReadingPosBits >> 3] &
		//			(0x80 >> (jackieBits->mReadingPosBits & 7))))
		//			// Write 1
		//			data[mWritingPosBits >> 3] = 0x80;
		//		else
		//			data[mWritingPosBits >> 3] = 0;
		//	}
		//	else
		//	{
		//		/// see if this src bit  is 1 or 0, 0x80 (16)= 128(10)= 10000000 (2)
		//		if ((jackieBits->data[jackieBits->mReadingPosBits >> 3] &
		//			(0x80 >> (jackieBits->mReadingPosBits & 7))))
		//		{
		//			/// set dest bit to 1 if the src bit is 1,do-nothing if the src bit is 0
		//			data[mWritingPosBits >> 3] |= 0x80 >> (numberOfBitsMod8);
		//		}
		//		else
		//		{
		//			data[mWritingPosBits >> 3] |= 0;
		//		}
		//	}
		//	jackieBits->mReadingPosBits++;
		//	mWritingPosBits++;
		//}
		//// ? official way
	}

	void JackieBits::WriteFloatRange(float src, float floatMin, float floatMax)
	{
		DCHECK_GT(floatMax, floatMin);
		DCHECK_LE(src, floatMax + .001);
		DCHECK_GE(src, floatMin - .001);

		float percentile = 65535.0f * ((src - floatMin) / (floatMax - floatMin));
		if (percentile < 0.0f) percentile = 0.0;
		if (percentile > 65535.0f) percentile = 65535.0f;
		//Write((UInt16)percentile);
		WriteMini((UInt16)percentile);
	}

	void JackieBits::WriteMini(const UInt8* src, const BitSize bits2Write, const bool isUnsigned)
	{
		static bool truee = true;
		static bool falsee = false;

		ByteSize currByte;
		UInt8 byteMatch = isUnsigned ? 0 : 0xFF; /// 0xFF=255=11111111

		if (!IsBigEndian())
		{
			//JINFO << "little endian ";
			/// get the highest byte with highest index  PCs
			currByte = (bits2Write >> 3) - 1;

			///  high byte to low byte, 
			/// if high byte is a byteMatch then write a 1 bit.
			/// Otherwise write a 0 bit and then write the remaining bytes
			while (currByte > 0)
			{
				///  If high byte is byteMatch (0 or 0xff)
				/// then it would have the same value shifted
				if (src[currByte] == byteMatch)
				{
					//JINFO << "write match " << byteMatch;
					Write(truee);
					currByte--;
				}
				else /// the first byte is not matched
				{
					//JINFO << "write not match " << byteMatch;
					Write(falsee);
					// Write the remainder of the data after writing bit false
					WriteBits(src, (currByte + 1) << 3, true);
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

			///  high byte to low byte, 
			/// if high byte is a byteMatch then write a 1 bit.
			/// Otherwise write a 0 bit and then write the remaining bytes
			while (currByte < ((bits2Write >> 3) - 1))
			{
				///  If high byte is byteMatch (0 or 0xff)
				/// then it would have the same value shifted
				if (src[currByte] == byteMatch)
				{
					JINFO << "write match " << byteMatch;
					Write(truee);
					currByte++;
				}
				else /// the first byte is not matched
				{
					JINFO << "write not match " << byteMatch;
					Write(falsee);
					// Write the remainder of the data after writing bit false
					WriteBits(src + currByte, bits2Write - (currByte << 3), true);
					return;
				}
			}
			/// make sure we are now on the lowest byte (index highest)
			DCHECK(currByte == ((bits2Write >> 3) - 1));
		}

		/// last byte
		if ((isUnsigned && (src[currByte] & 0xF0) == 0x00) ||
			(!isUnsigned && (src[currByte] & 0xF0) == 0xF0))
		{/// the upper(left aligned) half of the last byte(now currByte == 0) is a 0000 (positive) or 1111 (nagative)
			/// write a bit 1 and the remaining 4 bits. 
			//JINFO << "match four zeros" << byteMatch;
			Write(truee);
			WriteBits(src + currByte, 4, true);
			//JINFO << "get pay loads " << GetPayLoadBits();
		}
		else
		{/// write a 0 and the remaining 8 bites.
			//JINFO << "match one zeros" << byteMatch;
			Write(falsee);
			WriteBits(src + currByte, 8, true);
			//JINFO << "get pay loads " << GetPayLoadBits();
		}
	}

	void JackieBits::WriteAlignedBytes(const UInt8 *src, const ByteSize numberOfBytesWrite)
	{
		AlignWritePosBits2ByteBoundary();
		Write((Int8*)src, numberOfBytesWrite);
	}

	void JackieBits::WriteAlignedBytes(const UInt8 *src, const ByteSize bytes2Write, const ByteSize maxBytes2Write)
	{
		if (src == 0 || bytes2Write == 0)
		{
			//  [11/15/2015 JACKIE] this is ooficial impl whixch will waste 4 bits
			// actually we only nned to send one bit
			///WriteMini(bytes2Write);
			WriteBitZero();
			return;
		}
		WriteMini(bytes2Write);
		WriteAlignedBytes(src, bytes2Write < maxBytes2Write ?
		bytes2Write : maxBytes2Write);
	}

	void JackieBits::PadZero2LengthOf(UInt32 bytes)
	{
		Int32 numWrite = bytes - GetWrittenBytesCount();
		if (numWrite > 0)
		{
			AlignWritePosBits2ByteBoundary();
			AppendBitsCouldRealloc(BYTES_TO_BITS(numWrite));
			memset(data + (mWritingPosBits >> 3), 0, numWrite);
			mWritingPosBits += BYTES_TO_BITS(numWrite);
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
		PrintBit(out, mWritingPosBits, data);
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
		PrintHex(out, mWritingPosBits, data);
		printf_s("%s\n", out);
	}

}