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

		///========================================
		/// @access public 
		/// @author mengdi[Jackie]
		///========================================
		BitSize GetPayLoadBits(void) const { return mWritePosBits - mReadPosBits; }

		/// \brief Align the next read to a byte boundary.  
		/// \details This can be used to 'waste' bits to byte align for efficiency reasons It
		/// can also be used to force coalesced bitstreams to start on byte
		/// boundaries so so WriteAlignedBits and ReadAlignedBits both
		/// calculate the same offset when aligning.
		///========================================
		/// @method AlignReadPosBitsToByteBoundary
		/// @access public 
		/// @returns void
		/// @param [in] void
		/// @brief align the next read to a byte boundary.  
		/// @notice
		/// this can be used to 'waste' bits to byte align for efficiency reasons It
		/// can also be used to force coalesced bitstreams to start on byte
		/// boundaries so so WriteAlignedBits and ReadAlignedBits both
		/// calculate the same offset when aligning.
		/// @see
		///========================================
		inline void AlignReadPosBitsToByteBoundary(void)
		{
			mReadPosBits += 8 - (((mReadPosBits - 1) & 7) + 1);
		}

		///========================================
		/// @func   ReadBitsTo
		/// @brief   Read numbers of bit into dest array
		/// @access public
		/// @param [out] [unsigned UInt8 * dest]  The destination array
		/// @param [in] [BitSize bitsRead] The number of bits to read
		/// @param [in] [const bool alignRight]  If true bits will be right aligned
		/// @returns void
		/// @remarks
		/// 1.jackie stream internal data are aligned to the left side of byte boundary.
		/// 2.user data are aligned to the right side of byte boundary.
		/// @notice
		/// 1.use True to read to user data 
		/// 2.use False to read this stream to another stream 
		/// @author mengdi[Jackie]
		///========================================
		void ReadBitsTo(UInt8 *dest, BitSize bitsRead, bool alignRight = true);

		///========================================
		/// @method ReadTo
		/// @access public 
		/// @returns void
		/// @param [in] IntegralType & outTemplateVar
		/// @brief Read any integral type from a bitstream.  
		/// Define DO_NOT_SWAP_ENDIAN if you need endian swapping.
		/// @notice
		/// @see
		///========================================
		template <class IntegralType>
		inline void ReadTo(IntegralType &dest)
		{
			if (sizeof(IntegralType) == 1)
				ReadBitsTo((UInt8*)&dest, sizeof(IntegralType) * 8, true);
			else
			{
#ifndef DO_NOT_SWAP_ENDIAN
				if (DoEndianSwap())
				{
					UInt8 output[sizeof(IntegralType)];
					ReadBitsTo(output, BYTES_TO_BITS(sizeof(IntegralType)), true);
					ReverseBytes(output, (UInt8*)&dest, sizeof(IntegralType));
				}
				else
				{
					ReadBitsTo((UInt8*)&dest, BYTES_TO_BITS(sizeof(IntegralType)), true);
				}
#else
				ReadBitsTo((UInt8*)&dest,BYTES_TO_BITS(sizeof(IntegralType)), true);
#endif
			}
		}

		///========================================
		/// @method ReadTo
		/// @access public 
		/// @returns void
		/// @param [in] bool & dest The value to read
		/// @brief  Read a bool from a bitstream.
		/// @notice
		/// @see
		///========================================
		template <>
		inline void ReadTo(bool &dest)
		{
			DCHECK(GetPayLoadBits() >= 1);
			if (GetPayLoadBits() < 1) return;

			// Is it faster to just write it out here?
			data[mReadPosBits >> 3] & (0x80 >> (mReadPosBits & 7)) ?
				dest = true : dest = false;

			// Has to be on a different line for Mac
			mReadPosBits++;
		}

		///========================================
		/// @method ReadTo
		/// @access public 
		/// @returns void
		/// @param [in] JackieAddress & dest The value to read
		/// @brief Read a JackieAddress from a bitstream.
		/// @notice
		/// @see
		///========================================
		template <>
		inline void ReadTo(JackieAddress &dest)
		{
			UInt8 ipVersion;
			ReadTo(ipVersion);
			if (ipVersion == 4)
			{
				dest.address.addr4.sin_family = AF_INET;
				// Read(var.binaryAddress);
				// Don't endian swap the address or port
				UInt32 binaryAddress;
				ReadBitsTo((UInt8*)& binaryAddress,
					BYTES_TO_BITS(sizeof(binaryAddress)), true);
				// Unhide the IP address, done to prevent routers from changing it
				dest.address.addr4.sin_addr.s_addr = ~binaryAddress;
				ReadBitsTo((UInt8*)& dest.address.addr4.sin_port, BYTES_TO_BITS(sizeof(dest.address.addr4.sin_port)), true);
				dest.debugPort = ntohs(dest.address.addr4.sin_port);
			}
			else
			{
#if NET_SUPPORT_IPV6==1
				ReadBitsTo((UInt8*)&dest.address.addr6, BYTES_TO_BITS(sizeof(dest.address.addr6)), true);
				dest.debugPort = ntohs(dest.address.addr6.sin6_port);
				//return b;
#else
				//return false;
#endif
			}
		}

		///========================================
		/// @func ReadTo 
		/// @brief read three bytes into stream
		/// @access  public  
		/// @param [in] [const UInt24 & inTemplateVar]  
		/// @return [void] 
		/// @remark
		/// @notice will align @mReadPosBIts to byte-boundary internally
		/// @see  AlignReadPosBitsToByteBoundary()
		/// @author mengdi[Jackie]
		///========================================
		template <>
		inline void ReadTo(UInt24 &dest)
		{
			AlignReadPosBitsToByteBoundary();
			if (GetPayLoadBits() < 24) return;

			if (!IsBigEndian())
			{
				((UInt8 *)&dest.val)[0] = data[(mReadPosBits >> 3) + 0];
				((UInt8 *)&dest.val)[1] = data[(mReadPosBits >> 3) + 1];
				((UInt8 *)&dest.val)[2] = data[(mReadPosBits >> 3) + 2];
				((UInt8 *)&dest.val)[3] = 0;
			}
			else
			{
				((UInt8 *)&dest.val)[3] = data[(mReadPosBits >> 3) + 0];
				((UInt8 *)&dest.val)[2] = data[(mReadPosBits >> 3) + 1];
				((UInt8 *)&dest.val)[1] = data[(mReadPosBits >> 3) + 2];
				((UInt8 *)&dest.val)[0] = 0;
			}

			mReadPosBits += 24;
		}

		template <>
		inline void ReadTo(JackieGUID &dest)
		{
			return ReadTo(dest.g);
		}

		///========================================
		/// @brief Read any integral type from a bitstream.  
		/// @details If the written value differed from the value 
		/// compared against in the write function,
		/// var will be updated.  Otherwise it will retain the current value.
		/// ReadDelta is only valid from a previous call to WriteDelta
		/// @param[in] outTemplateVar The value to read
		///========================================
		template <class IntegralType>
		inline void ReadChangedTo(IntegralType &dest)
		{
			bool dataWritten;
			ReadTo(dataWritten);
			if (dataWritten) ReadTo(dest);
		}

		///========================================
		/// \brief Read a bool from a bitstream.
		/// \param[in] outTemplateVar The value to read
		///========================================
		template <>
		inline void ReadChangedTo(bool &dest)
		{
			return ReadTo(dest);
		}

		///========================================
		/// @Brief Assume the input source points to a compressed native type. 
		/// Decompress and read it.
		///========================================
		void ReadMiniTo(UInt8* dest, const BitSize bits2Read, const bool isUnsigned);

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

		/// \brief Read any integral type from a bitstream.  
		/// \details Undefine DO_NOT_SWAP_ENDIAN if you need endian swapping.
		/// For floating point, this is lossy, using 2 bytes for a float and 4 for a double.  The range must be between -1 and +1.
		/// For non-floating point, this is lossless, but only has benefit if you use less than half the bits of the type
		/// If you are not using DO_NOT_SWAP_ENDIAN the opposite is true for types larger than 1 byte
		/// \param[in] outTemplateVar The value to read
		template <class IntegralType>
		inline void ReadMiniTo(IntegralType &dest)
		{
			if (sizeof(dest) == 1)
				ReadMiniTo((UInt8*)&dest, BYTES_TO_BITS(sizeof(IntegralType)), true);
			else
			{
#ifndef DO_NOT_SWAP_ENDIAN
				if (DoEndianSwap())
				{
					UInt8 output[sizeof(IntegralType)];
					ReadMiniTo((UInt8*)output, BYTES_TO_BITS(sizeof(IntegralType)), true);
					ReverseBytes(output, (UInt8*)&dest, sizeof(IntegralType));
				}
				else
				{
					ReadMiniTo((UInt8*)& dest, BYTES_TO_BITS(sizeof(IntegralType)), true);
				}
#else
				ReadMiniTo((UInt8*)& dest, BYTES_TO_BITS(sizeof(IntegralType)), true);
#endif
			}
		}

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
		void WriteBitsFrom(const UInt8* src, BitSize bits2Write, bool rightAligned = true);


		///========================================
		/// @func WriteFrom 
		/// @access  public  
		/// @brief write an array or raw data in bytes. NOT do endian swapp.
		/// default is right aligned[true]
		/// @author mengdi[Jackie]
		///========================================
		inline void WriteBytesFrom(const Int8* src, const ByteSize bytes2Write)
		{
			WriteBitsFrom((UInt8*)src, BYTES_TO_BITS(bytes2Write), true);
		}

		/// @brief Write one JackieBits to another.
		/// @param[in] [bits2Write] bits to write
		/// @param[in] [JackieBits] the JackieBits to copy from
		void WriteFrom(JackieBits *jackieBits, BitSize bits2Write);
		inline void WriteFrom(JackieBits &jackieBits, BitSize bits2Write)
		{
			WriteFrom(&jackieBits, bits2Write);
		}
		inline void WriteFrom(JackieBits *jackieBits)
		{
			WriteFrom(jackieBits, jackieBits->GetPayLoadBits());
		}
		inline void WriteFrom(JackieBits &jackieBits) { WriteFrom(&jackieBits); }

		///========================================
		/// @func WriteBitZero 
		/// @access  public  
		/// @notice @mReadOnly must be false
		/// @author mengdi[Jackie]
		///========================================
		inline void WriteBitZero(void)
		{
			DCHECK_EQ(mReadOnly, false);

			AppendBitsCouldRealloc(1);
			BitSize shit = 8 - (mWritePosBits & 7);
			data[mWritePosBits >> 3] = ((data[mWritePosBits >> 3] >> shit) << shit);
			mWritePosBits++;

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
		inline void WriteBitOne(void)
		{
			DCHECK_EQ(mReadOnly, false);

			AppendBitsCouldRealloc(1);
			BitSize shit = mWritePosBits & 7;
			data[mWritePosBits >> 3] |= 0x80 >> shit; // Write bit 1
			mWritePosBits++;

			//AddBitsAndReallocate(1);
			//BitSize_t numberOfBitsMod8 = mWritePosBits & 7;
			//if( numberOfBitsMod8 == 0 )
			//	data[mWritePosBits >> 3] = 0x80;
			//else
			//	data[mWritePosBits >> 3] |= 0x80 >> ( numberOfBitsMod8 ); // Set the bit to 1
			//mWritePosBits++;
		}

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
			mWritePosBits += 8 - (((mWritePosBits - 1) & 7) + 1);
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
		/// wastes the bits to do the alignment for @mWritePosBits and
		/// requires you to call ReadAlignedBits() at the corresponding 
		/// read position.
		/// @author mengdi[Jackie]
		///========================================
		void WriteAlignedBytesFrom(const UInt8 *src, const ByteSize numberOfBytesToWrite);

		///========================================
		/// @func WriteFrom 
		/// @brief write a float into 2 bytes, spanning the range 
		/// between @param[floatMin] and @param[floatMax]
		/// @access  public  
		/// @param [in] [float src]  value to write into stream
		/// @param [in] [float floatMin] Predetermined mini value of f
		/// @param [in] [float floatMax] Predetermined max value of f
		/// @return bool
		/// @notice calculate the proparation of the @src 
		/// @author mengdi[Jackie]
		///========================================
		void WriteFrom(float src, float floatMin, float floatMax);

		/// \brief Write any integral type to a bitstream.  
		/// \details Undefine __BITSTREAM_NATIVE_END if you need endian swapping.
		/// \param[in] inTemplateVar The value to write

		///========================================
		/// @func WriteFrom 
		/// @brief write any integral type to a bitstream.  
		/// @access  public  
		/// @param [in] [const templateType & src] 
		/// it is user data that is right aligned in default
		/// @return void
		/// @notice will swap endian internally 
		/// if DO_NOT_SWAP_ENDIAN not defined
		/// @author mengdi[Jackie]
		///========================================
		template <class IntergralType>
		void WriteFrom(const IntergralType &src)
		{
			if (sizeof(IntergralType) == 1)
			{
				WriteBitsFrom((UInt8*)&src, BYTES_TO_BITS(sizeof(IntergralType)), true);
			}
			else
			{
#ifndef DO_NOT_SWAP_ENDIAN
				if (DoEndianSwap())
				{
					/// Reverse Bytes
					//UInt8 result[sizeof(IntergralType)];
					//for( UInt32 i = 0; i < sizeof(IntergralType); i++ )
					//{
					//	result[i] = ( (UInt8*) src )[sizeof(IntergralType) - i - 1];
					//}
					UInt8 output[sizeof(IntergralType)];
					ReverseBytes((UInt8*)&src, output, sizeof(IntergralType));
					WriteBitsFrom(output, BYTES_TO_BITS(sizeof(IntergralType)), true);
				}
				else
#endif
					WriteBitsFrom((UInt8*)&src, BYTES_TO_BITS(sizeof(IntergralType)), true);
			}
		}

		///========================================
		/// @func WriteFrom 
		/// @access  public  
		/// @brief Write a bool to a bitstream.
		/// @param [in] [const bool & src] The value to write
		/// @return [bool] true succeed, false failed
		/// @author mengdi[Jackie]
		///========================================
		template <>
		inline void WriteFrom(const bool &src)
		{
			if (src == true)
				WriteBitOne();
			else
				WriteBitZero();
		}

		///========================================
		/// @func WriteFrom 
		/// @brief write a JackieAddress to stream
		/// @access  public  
		/// @param [in] [const JackieAddress & src]  
		/// @return [bool]  true succeed, false failed
		/// @remark
		/// @notice  will not endian swap the address or port
		/// @author mengdi[Jackie]
		///========================================
		template <>
		inline void WriteFrom(const JackieAddress &src)
		{
			UInt8 version = src.GetIPVersion();
			WriteFrom(version);

			if (version == 4)
			{
				/// Hide the address so routers don't modify it
				JackieAddress addr = src;
				UInt32 binaryAddress = ~src.address.addr4.sin_addr.s_addr;
				UInt16 p = addr.GetPortNetworkOrder();
				// Don't endian swap the address or port
				WriteBitsFrom((UInt8*)&binaryAddress,
					BYTES_TO_BITS(sizeof(binaryAddress)), true);
				WriteBitsFrom((UInt8*)&p, BYTES_TO_BITS(sizeof(p)), true);
			}
			else
			{
#if NET_SUPPORT_IPV6 == 1
				// Don't endian swap
				WriteBitsFrom((UInt8*) &src.address.addr6, 
					BYTES_TO_BITS(sizeof(src.address.addr6)), true);
#endif
			}
		}


		///========================================
		/// @func WriteFrom 
		/// @brief write three bytes into stream
		/// @access  public  
		/// @param [in] [const UInt24 & inTemplateVar]  
		/// @return [void]  true succeed, false failed
		/// @remark
		/// @notice will align write-position to byte-boundary internally
		/// @see  AlignWritePosBits2ByteBoundary()
		/// @author mengdi[Jackie]
		///========================================
		template <>
		inline void WriteFrom(const UInt24 &inTemplateVar)
		{
			AlignWritePosBits2ByteBoundary();
			AppendBitsCouldRealloc(BYTES_TO_BITS(3));

			if (!IsBigEndian())
			{
				data[(mWritePosBits >> 3) + 0] = ((UInt8 *)&inTemplateVar.val)[0];
				data[(mWritePosBits >> 3) + 1] = ((UInt8 *)&inTemplateVar.val)[1];
				data[(mWritePosBits >> 3) + 2] = ((UInt8 *)&inTemplateVar.val)[2];
			}
			else
			{
				data[(mWritePosBits >> 3) + 0] = ((UInt8 *)&inTemplateVar.val)[3];
				data[(mWritePosBits >> 3) + 1] = ((UInt8 *)&inTemplateVar.val)[2];
				data[(mWritePosBits >> 3) + 2] = ((UInt8 *)&inTemplateVar.val)[1];
			}

			mWritePosBits += BYTES_TO_BITS(3);
		}


		///========================================
		/// @func WriteFrom 
		/// @access  public  
		/// @param [in] [const JackieGUID & inTemplateVar]  
		/// @return void
		/// @author mengdi[Jackie]
		///========================================
		template <>
		inline void WriteFrom(const JackieGUID &inTemplateVar)
		{
			WriteFrom(inTemplateVar.g);
		}

		///========================================
		/// @func WriteChangedFrom 
		/// @brief write any changed integral type to a bitstream.
		/// @access  public  
		/// @param [in] const templateType & latestVal 
		/// @param [in] const templateType & lastVal  
		/// @return void 
		/// @notice 
		/// If the current value is different from the last value
		/// the current value will be written.  Otherwise, a single bit will be written
		/// @author mengdi[Jackie]
		///========================================
		template <class templateType>
		inline void WriteChangedFrom(const templateType &latestVal,
			const templateType &lastVal)
		{
			if (latestVal == lastVal)
			{
				WriteFrom(false);
			}
			else
			{
				WriteFrom(true);
				WriteFrom(latestVal);
			}
		}


		///========================================
		/// @func WriteChangedFrom 
		/// @brief write a bool delta. Same thing as just calling Write
		/// @access  public  
		/// @param [in] const bool & currentValue  
		/// @param [in] const bool & lastValue  
		/// @return void 
		/// @author mengdi[Jackie]
		///========================================
		template <>
		inline void WriteChangedFrom(const bool &currentValue,
			const bool &lastValue)
		{
			(void)lastValue;
			WriteFrom(currentValue);
		}

		/// \brief WriteDelta when you don't know what the last value is, or there is no last value.
		/// \param[in] currentValue The current value to write

		///========================================
		/// @func WriteChangedFrom 
		/// @brief 
		/// writeDelta when you don't know what the last value is, or there is no last value.
		/// @access  public  
		/// @param [in] const templateType & currentValue  
		/// @return void  
		/// @author mengdi[Jackie]
		///========================================
		template <class templateType>
		inline void WriteChangedFrom(const templateType &currentValue)
		{
			WriteFrom(true);
			WriteFrom(currentValue);
		}

		///========================================
		/// @func WriteMiniChangedFrom 
		/// @brief write any integral type to a bitstream.  
		/// @access  public  
		/// @param [in] const templateType & currVal 
		/// The current value to write 
		/// @param [in] const templateType & lastValue  
		///  The last value to compare against
		/// @return void 
		/// @notice
		/// If the current value is different from the last value. the current
		/// value will be written.  Otherwise, a single bit will be written
		/// For floating point, this is lossy, using 2 bytes for a float and 
		/// 4 for a double. The range must be between -1 and +1.
		/// For non-floating point, this is lossless, but only has benefit
		/// if you use less than half the bits of the type
		/// If you are not using DO_NOT_SWAP_ENDIAN the opposite is
		/// true for types larger than 1 byte
		/// @author mengdi[Jackie]
		///========================================
		template <class templateType>
		inline void WriteMiniChangedFrom(const templateType&currVal,
			const templateType &lastValue)
		{
			if (currVal == lastValue)
			{
				WriteFrom(false);
			}
			else
			{
				WriteFrom(true);
				WriteMiniFrom(currVal);
			}
		}

		///======================================
		/// @brief Write a bool delta.  Same thing as just calling Write
		/// @param[in] currentValue The current value to write
		/// @param[in] lastValue The last value to compare against
		///======================================
		template <>
		inline void WriteMiniChangedFrom(const bool &currentValue, const bool&
			lastValue)
		{
			(void)lastValue;
			WriteFrom(currentValue);
		}

		///=============================
		/// @brief Same as WriteMiniChangedFrom() 
		/// when we have an unknown second parameter
		///==============================
		template <class templateType>
		inline void WriteMiniChangedFrom(const templateType &currentValue)
		{
			WriteFrom(true);
			WriteMiniFrom(currentValue);
		}

		///========================================
		/// @func WriteMiniFrom 
		/// @access  public  
		/// @param [in] const UInt8 * src  
		/// @param [in] const BitSize bits2Write  write size in bits
		/// @param [in] const bool isUnsigned  
		/// @return void 
		/// @notice 
		/// this function assumes that @src points to a native type,
		/// compress and write it.
		/// 在字节内部，一个字节的二进制排序，不存在大小端问题。
		/// 就和平常书写的一样，先写高位，即低地址存储高位。
		/// 如char a=0x12.存储从低位到高位就为0001 0010
		/// @author mengdi[Jackie]
		///========================================
		void WriteMiniFrom(const UInt8* src, const BitSize bits2Write,
			const bool isUnsigned);

		///========================================
		/// @func WriteMiniFrom 
		/// @brief Write any integral type to a bitstream.  
		/// @access  public  
		/// @param [in] const IntergralType & src  
		/// @return void 
		/// @notice
		/// 1.Undefine DO_NOT_SWAP_ENDIAN if you need endian swapping.
		/// 2.For floating point, this is lossy, using 2 bytes for a float and 4 for 
		/// a double.  The range must be between -1 and +1.
		/// 3.For non-floating point, this is lossless, but only has benefit 
		/// if you use less than half the bits of the type
		/// 4.If you are not using DO_NOT_SWAP_ENDIAN the opposite is true 
		/// for types larger than 1 byte
		/// @author mengdi[Jackie]
		///========================================
		template <class IntergralType>
		inline void WriteMiniFrom(const IntergralType &src)
		{
			if (sizeof(src) == 1)
			{
				WriteMiniFrom((UInt8*)& src, sizeof(templateType) << 3, true);
				return;
			}
#ifndef DO_NOT_SWAP_ENDIAN
			if (DoEndianSwap())
			{
				UInt8 output[sizeof(templateType)];
				ReverseBytes((UInt8*)&src, output, sizeof(templateType));
				WriteMiniFrom(output, sizeof(templateType) << 3, true);
			}
			else
			{
				WriteMiniFrom((UInt8*)&src, sizeof(templateType) << 3, true);
			}
#else
			WriteMiniFrom((UInt8*)&src, sizeof(templateType) << 3, true);
#endif
		}
		template <> inline void WriteMiniFrom(const JackieAddress &src)
		{
			WriteFrom(src);
		}
		template <> inline void WriteMiniFrom(const JackieGUID &src)
		{
			WriteFrom(src);
		}
		template <> inline void WriteMiniFrom(const UInt24 &var)
		{
			WriteFrom(var);
		}
		template <> inline void WriteMiniFrom(const bool &src)
		{
			WriteFrom(src);
		}
		///@notice only For values between -1 and 1
		template <> inline void WriteMiniFrom(const float &src)
		{
			DCHECK(src > -1.01f && src < 1.01f);
			float varCopy = src;
			if (varCopy < -1.0f) varCopy = -1.0f;
			if (varCopy > 1.0f) varCopy = 1.0f;
			WriteFrom((UInt16)((varCopy + 1.0f)*32767.5f));
		}
		///@notice For values between -1 and 1
		template <> inline void WriteMiniFrom(const double &src)
		{
			DCHECK(src > -1.01f && src < 1.01f);
			double varCopy = src;
			if (varCopy < -1.0f) varCopy = -1.0f;
			if (varCopy > 1.0f) varCopy = 1.0f;
			WriteFrom((UInt32)((varCopy + 1.0)*2147483648.0));
		}

		template <class destType, class srcType >
		void WriteCasted(const srcType &value)
		{
			destType val = (destType)value;
			WriteFrom(val);
		}

		///==================================
		/// @method WriteBitsFromIntegerRange
		/// @access public 
		/// @returns void
		/// @param [in] const templateType value 
		/// value Integer value to write, which should be
		/// between @param mini and @param max
		/// @param [in] const templateType mini
		/// @param [in] const templateType max
		/// @param [in] bool allowOutsideRange
		/// If true, all sends will take an extra bit,
		/// however value can deviate from outside \a minimum and \a maximum.
		/// If false, will assert if the value deviates. 
		/// This should match the corresponding value passed to Read().
		/// @brief 
		/// given the minimum and maximum values for an integer type, 
		/// figure out the minimum number of bits to represent the range
		/// Then write only those bits
		/// @notice
		/// a static is used so that the required number of bits for
		/// (maximum-minimum pair) is only calculated once. 
		/// This does require that @param mini and @param max are fixed 
		/// values for a given line of code for the life of the program
		/// @see
		///===================================
		template <class IntegerType> void WriteFromIntegerRange(
			const IntegerType value,
			const IntegerType mini,
			const IntegerType max,
			bool allowOutsideRange = false)
		{
			static int requiredBits = BYTES_TO_BITS(sizeof(IntegerType)) -
				GetLeadingZeroSize(IntegerType(max - mini));
			WriteFromIntegerRange(value, mini, max, requiredBits, allowOutsideRange);
		}

		template <class IntegerType>
		void WriteFromIntegerRange(
			const IntegerType value,
			const IntegerType minimum,
			const IntegerType maximum,
			const int requiredBits,
			bool allowOutsideRange = false)
		{
			DCHECK(max >= mini);
			DCHECK(allowOutsideRange == true ||
				(value >= minimum && value <= maximum));

			if (allowOutsideRange)
			{
				if (value <mini || value>max)  ///< out of range
				{
					WriteFrom(true);
					WriteFrom(value);
					return;
				}
				WriteFrom(false); ///< inside range
			}

			templateType valueBeyondMini = value - mini;
			if (IsBigEndian())
			{/// because bits are written from low byte to high byte
				/// so we have to reverse the bytes to put the low bits in the low memory
				/// address to ensure they are written correctly
				UInt8 output[sizeof(templateType)];
				ReverseBytes((UInt8*)&valueBeyondMini, output, sizeof(templateType));
				WriteBitsFrom(output, requiredBits);
			}
			else
			{
				WriteBitsFrom((UInt8*)&valueBeyondMini, requiredBits);
			}
		}

		///========================================
		/// @method WriteNormVector
		/// @access public 
		/// @returns void
		/// @param [in] templateType x
		/// @param [in] templateType y
		/// @param [in] templateType z
		/// @brief
		/// Write a normalized 3D vector, using (at most) 4 bytes + 3 bits
		/// instead of 12 - 24 bytes. Accurate to 1/32767.5.
		/// @notice
		/// Will further compress y or z axis aligned vectors.
		/// templateType for this function must be a float or double
		/// @see
		///========================================
		template <class templateType> void WriteNormVector(
			templateType x,
			templateType y,
			templateType z)
		{
			DCHECK(
				x <= 1.01 &&
				y <= 1.01 &&
				z <= 1.01 &&
				x >= -1.01 &&
				y >= -1.01 &&
				z >= -1.01);
			WriteFrom((float)x, -1.0f, 1.0f);
			WriteFrom((float)y, -1.0f, 1.0f);
			WriteFrom((float)z, -1.0f, 1.0f);
		}

		// templateType for this function must be a float or double
		///========================================
		/// @method WriteVector
		/// @access public 
		/// @returns void
		/// @param [in] templateType x
		/// @param [in] templateType y
		/// @param [in] templateType z
		/// @brief Write a vector, using 10 bytes instead of 12.
		/// @notice
		/// Loses accuracy to about 3/10ths and only saves 2 bytes, 
		/// so only use if accuracy is not important
		/// @see
		///========================================
		template <class templateType> void WriteVector(
			templateType x,
			templateType y,
			templateType z)
		{
			templateType magnitude = sqrt(x * x + y * y + z * z);
			WriteFrom((float)magnitude);
			if (magnitude > 0.00001f)
			{
				WriteMiniFrom((float)(x / magnitude));
				WriteMiniFrom((float)(y / magnitude));
				WriteMiniFrom((float)(z / magnitude));
				//	Write((unsigned short)((x/magnitude+1.0f)*32767.5f));
				//	Write((unsigned short)((y/magnitude+1.0f)*32767.5f));
				//	Write((unsigned short)((z/magnitude+1.0f)*32767.5f));
			}
		}

		///========================================
		/// @method WriteNormQuat
		/// @access public 
		/// @returns void
		/// @param [in] templateType w
		/// @param [in] templateType x
		/// @param [in] templateType y
		/// @param [in] templateType z
		/// @brief 
		/// Write a normalized quaternion in 6 bytes + 4 bits instead of 16 bytes.  
		/// Slightly lossy.
		/// @notice
		/// templateType for this function must be a float or double
		/// @see
		///========================================
		template <class templateType> void WriteNormQuat(
			templateType w,
			templateType x,
			templateType y,
			templateType z)
		{
			WriteFrom((bool)(w < 0.0));
			WriteFrom((bool)(x < 0.0));
			WriteFrom((bool)(y < 0.0));
			WriteFrom((bool)(z < 0.0));
			WriteFrom((unsigned short)(fabs(x)*65535.0));
			WriteFrom((unsigned short)(fabs(y)*65535.0));
			WriteFrom((unsigned short)(fabs(z)*65535.0));
			// Leave out w and calculate it on the target
		}

		///========================================
		/// @method WriteOrthMatrix
		/// @access public 
		/// @returns void
		/// @param [in] templateType m00
		/// @param [in] templateType m01
		/// @param [in] templateType m02
		/// @param [in] templateType m10
		/// @param [in] templateType m11
		/// @param [in] templateType m12
		/// @param [in] templateType m20
		/// @param [in] templateType m21
		/// @param [in] templateType m22
		/// @brief
		/// Write an orthogonal matrix by creating a quaternion, 
		/// and writing 3 components of the quaternion in 2 bytes each.
		/// @notice
		/// Lossy, although the result is renormalized
		/// Use 6 bytes instead of 36
		/// templateType for this function must be a float or double
		/// @see
		///========================================
		template <class templateType> void WriteOrthMatrix(
			templateType m00, templateType m01, templateType m02,
			templateType m10, templateType m11, templateType m12,
			templateType m20, templateType m21, templateType m22)
		{
			double qw;
			double qx;
			double qy;
			double qz;

			// Convert matrix to quat
			// http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/
			float sum;
			sum = 1 + m00 + m11 + m22;
			if (sum < 0.0f) sum = 0.0f;
			qw = sqrt(sum) / 2;
			sum = 1 + m00 - m11 - m22;
			if (sum < 0.0f) sum = 0.0f;
			qx = sqrt(sum) / 2;
			sum = 1 - m00 + m11 - m22;
			if (sum < 0.0f) sum = 0.0f;
			qy = sqrt(sum) / 2;
			sum = 1 - m00 - m11 + m22;
			if (sum < 0.0f) sum = 0.0f;
			qz = sqrt(sum) / 2;
			if (qw < 0.0) qw = 0.0;
			if (qx < 0.0) qx = 0.0;
			if (qy < 0.0) qy = 0.0;
			if (qz < 0.0) qz = 0.0;
			qx = _copysign((double)qx, (double)(m21 - m12));
			qy = _copysign((double)qy, (double)(m02 - m20));
			qz = _copysign((double)qz, (double)(m10 - m01));

			WriteNormQuat(qw, qx, qy, qz);
		}

		inline static int GetLeadingZeroSize(Int8 x)
		{
			return GetLeadingZeroSize((UInt8)x);
		}
		inline static  int GetLeadingZeroSize(UInt8 x)
		{
			UInt8 y;
			int n;

			n = 8;
			y = x >> 4;  if (y != 0) { n = n - 4;  x = y; }
			y = x >> 2;  if (y != 0) { n = n - 2;  x = y; }
			y = x >> 1;  if (y != 0) return n - 2;
			return (int)(n - x);
		}
		inline static int GetLeadingZeroSize(Int16 x)
		{
			return GetLeadingZeroSize((UInt16)x);
		}
		inline static  int GetLeadingZeroSize(UInt16 x)
		{
			UInt16 y;
			int n;

			n = 16;
			y = x >> 8;  if (y != 0) { n = n - 8;  x = y; }
			y = x >> 4;  if (y != 0) { n = n - 4;  x = y; }
			y = x >> 2;  if (y != 0) { n = n - 2;  x = y; }
			y = x >> 1;  if (y != 0) return n - 2;
			return (int)(n - x);
		}
		inline static  int GetLeadingZeroSize(Int32 x)
		{
			return GetLeadingZeroSize((UInt32)x);
		}
		inline static  int GetLeadingZeroSize(UInt32 x)
		{
			UInt32 y;
			int n;

			n = 32;
			y = x >> 16;  if (y != 0) { n = n - 16;  x = y; }
			y = x >> 8;  if (y != 0) { n = n - 8;  x = y; }
			y = x >> 4;  if (y != 0) { n = n - 4;  x = y; }
			y = x >> 2;  if (y != 0) { n = n - 2;  x = y; }
			y = x >> 1;  if (y != 0) return n - 2;
			return (int)(n - x);
		}
		inline static  int GetLeadingZeroSize(Int64 x)
		{
			return GetLeadingZeroSize((UInt64)x);
		}
		inline static  int GetLeadingZeroSize(UInt64 x)
		{
			UInt64 y;
			int n;

			n = 64;
			y = x >> 32;  if (y != 0) { n = n - 32;  x = y; }
			y = x >> 16;  if (y != 0) { n = n - 16;  x = y; }
			y = x >> 8;  if (y != 0) { n = n - 8;  x = y; }
			y = x >> 4;  if (y != 0) { n = n - 4;  x = y; }
			y = x >> 2;  if (y != 0) { n = n - 2;  x = y; }
			y = x >> 1;  if (y != 0) return n - 2;
			return (int)(n - x);
		}

		inline static bool DoEndianSwap(void)
		{
#ifndef DO_NOT_SWAP_ENDIAN
			return IsNetworkOrder() == false;
#else
			return false;
#endif
		}
		inline static bool IsNetworkOrder(void)
		{
			static int a = 0x01;
			return *((char*)& a) != 1;
		}
		inline static bool IsBigEndian(void) { return IsNetworkOrder(); }
		inline static void ReverseBytes(UInt8 *src, UInt8 *dest, const UInt32 length)
		{
			JINFO << "ReverseBytes";
			for (UInt32 i = 0; i < length; i++)
			{
				dest[i] = src[length - i - 1];
			}
		}

		/// Can only print 4096 size of UInt8 no materr is is bit or byte
		/// mainly used for dump binary data
		void PrintBit(void);
		static void PrintBit(char* outstr, BitSize bitsPrint, UInt8* src);
		void PrintHex(void);
		static void PrintHex(char* outstr, BitSize bitsPrint, UInt8* src);
	};
}
#endif  //__BITSTREAM_H__
