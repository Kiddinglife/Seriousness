#include "JackieBits.h"
#include "EasyLog.h"

namespace JACKIE_INET
{
	STATIC_FACTORY_DEFINITIONS(JackieBits, JackieBits);

	JackieBits::JackieBits() :
		mBitsAllocCount(BYTES_TO_BITS(JACKIESTREAM_STACK_ALLOC_SIZE)),
		mWritePosBits(0),
		mReadPosBits(0),
		data(mStacBuffer),
		mNeedFree(false)
	{
	}

	JackieBits::JackieBits(const BitSize initialBytesToAllocate)
	{
		mWritePosBits = 0;
		mReadPosBits = 0;

		if( initialBytesToAllocate <= JACKIESTREAM_STACK_ALLOC_SIZE )
		{
			data = mStacBuffer;
			mBitsAllocCount = BYTES_TO_BITS(JACKIESTREAM_STACK_ALLOC_SIZE);
			mNeedFree = false;
		} else
		{
			data = (unsigned char*) jackieMalloc_Ex(initialBytesToAllocate, TRACE_FILE_AND_LINE_);
			mBitsAllocCount = BYTES_TO_BITS(initialBytesToAllocate);
			mNeedFree = true;
			DCHECK_NOTNULL(data);
		}
	}
	JackieBits::JackieBits(unsigned char* src, const BitSize len, bool copy) :
		mBitsAllocCount(BYTES_TO_BITS(len)),
		mWritePosBits(BYTES_TO_BITS(len)),
		mReadPosBits(0),
		mNeedFree(copy)
	{
		if( mNeedFree )
		{
			if( len > 0 )
			{
				if( len <= JACKIESTREAM_STACK_ALLOC_SIZE )
				{
					data = mStacBuffer;
					mNeedFree = false;
					mBitsAllocCount = BYTES_TO_BITS(JACKIESTREAM_STACK_ALLOC_SIZE);
				} else
				{
					data = (unsigned char*) jackieMalloc_Ex(len, TRACE_FILE_AND_LINE_);
					mNeedFree = true;
				}
				memcpy(data, src, len);
			} else
			{
				data = 0;
				mNeedFree = false;
			}
		} else
		{
			data = src;
			mNeedFree = false;
		}
	}
	JackieBits::~JackieBits()
	{
		if( mNeedFree && mBitsAllocCount > ( JACKIESTREAM_STACK_ALLOC_SIZE << 3 ) )
			jackieFree_Ex(data, TRACE_FILE_AND_LINE_);
		/// realloc and free are more efficient than delete and new  
		/// because it will not call ctor and dtor
	}

	/// AddBitsAndReallocate
	void JackieBits::AppendBitsCouldRealloc(const BitSize bits2Append)
	{
		BitSize newBitsAllocCount = bits2Append + mWritePosBits;

		/// see if one or more new bytes need to be allocated
		if( newBitsAllocCount > 0 &&
			( ( mBitsAllocCount - 1 ) >> 3 ) < ( ( newBitsAllocCount - 1 ) >> 3 ) )
		{
			// If this assert hits then we need to specify true for the third parameter in the constructor
			// It needs to reallocate to hold all the data and can't do it unless we allocated to begin with
			// Often hits if you call Write or Serialize on a read-only bitstream
			DCHECK_EQ(mNeedFree, true);

			// Less memory efficient but saves on news and deletes
			/// Cap to 1 meg buffer to save on huge allocations
			newBitsAllocCount = ( bits2Append + mWritePosBits ) << 1;
			if( newBitsAllocCount - ( bits2Append + mWritePosBits ) > 1048576 )
				newBitsAllocCount = bits2Append + mWritePosBits + 1048576;

			// Use realloc and free so we are more efficient than delete and new for resizing
			BitSize bytes2Alloc = BITS_TO_BYTES(newBitsAllocCount);
			if( data == mStacBuffer )
			{
				if( bytes2Alloc > JACKIESTREAM_STACK_ALLOC_SIZE )
				{
					data = (unsigned char *) jackieMalloc_Ex(bytes2Alloc, TRACE_FILE_AND_LINE_);
					memcpy(data, mStacBuffer, BITS_TO_BYTES(mBitsAllocCount));
				}
			} else
			{
				data = (unsigned char*) jackieRealloc_Ex(data, bytes2Alloc, TRACE_FILE_AND_LINE_);
			}

			DCHECK_NOTNULL(data);
		}

		if( newBitsAllocCount > mBitsAllocCount )
		{
			mBitsAllocCount = newBitsAllocCount;
		}
	}

	bool JackieBits::ReadBits(unsigned char *dest, BitSize bits2Read, bool alignRight /*= true*/)
	{
		DCHECK_GT(bits2Read, 0);

		if( bits2Read <= 0 ) return false;
		if( bits2Read > ( mWritePosBits - mReadPosBits ) ) return false;

		/// get offset that overlaps one byte boudary, &7 is same to %8, but faster
		const BitSize startReadPosBits = mReadPosBits & 7;

		/// byte position where start to read
		ByteSize readPosByte = mReadPosBits >> 3;

		/// if @mReadPosBits is aligned and @bitsSize is multiple times of 8, 
		/// do memcpy for efficiency
		if( startReadPosBits == 0 && ( bits2Read & 7 ) == 0 )
		{
			memcpy(dest, &data[readPosByte], bits2Read >> 3);
			mReadPosBits += bits2Read;
			return true;
		}

		BitSize bitsSizeInLastByte;
		BitSize writePosByte = 0;
		memset(dest, 0, BITS_TO_BYTES(bits2Read)); /// Must set all 0 

		/// Read one complete byte each time 
		while( bits2Read > 0 )
		{
			readPosByte = mReadPosBits >> 3;

			/// firstly read left-fragment bits in this byte 
			dest[writePosByte] |= ( data[readPosByte] << ( startReadPosBits ) );

			/// secondly read right-fragment bits ( if any ) in this byte
			if( startReadPosBits > 0 && bits2Read > ( 8 - startReadPosBits ) )
			{
				dest[writePosByte] |= data[readPosByte + 1] >> ( 8 - startReadPosBits );
			}

			if( bits2Read >= 8 )
			{
				bits2Read -= 8;
				mReadPosBits += 8;
				writePosByte++;
			} else
			{
				// Reading a partial byte for the last byte, shift right so the data is aligned on the right
				bitsSizeInLastByte = 8 - bits2Read;
				if( alignRight )  dest[writePosByte] >>= bitsSizeInLastByte;
				writePosByte++;
				bits2Read = 0;
			}
		}
		return true;
	}

	void JackieBits::WriteBits(const unsigned char* src, BitSize bits2Write, bool rightAligned /*= true*/)
	{
		AppendBitsCouldRealloc(bits2Write);

	}

	void JackieBits::PrintBit(char* out, BitSize mWritePosBits, unsigned char* mBuffer)
	{
		printf_s("%s[%dbits %dbytes]:\n", "BitsDumpResult",
			mWritePosBits, BITS_TO_BYTES(mWritePosBits));

		int strIndex = 0;
		int inner;
		int stopPos;
		int outter;
		int  len = BITS_TO_BYTES(mWritePosBits);

		for( outter = 0; outter < len; outter++ )
		{
			if( outter == len - 1 )
				stopPos = 8 - ( ( ( mWritePosBits - 1 ) & 7 ) + 1 );
			else
				stopPos = 0;

			for( inner = 7; inner >= stopPos; inner-- )
			{
				if( ( mBuffer[outter] >> inner ) & 1 )
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
	void JackieBits::PrintHex(char* out, BitSize mWritePosBits, unsigned char* mBuffer)
	{
		printf_s("%s[%d bytes]:\n", "HexDumpResult", BITS_TO_BYTES(mWritePosBits));
		for( BitSize Index = 0; Index < BITS_TO_BYTES(mWritePosBits); Index++ )
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