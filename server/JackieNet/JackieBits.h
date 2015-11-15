//  [11/2/2015 mengdi]
#ifndef __JACKIE_BITS_H__
#define __JACKIE_BITS_H__

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
		/// the value of @mWritePosBits always reprsents 
		/// the position where a bit is going to be written(not been written yet)
		/// the value of @mReadPosBits always reprsents 
		/// the position where a bit is going to be read(not been read yet)
		/// both of them will start to cout at index of 0, 
		/// so mWritePosBits = 2 means the first 2 bits has been written, the third bit is
		/// being written (not been written yet)
		/// so mReadPosBits = 2 means the first 2 bits has been read, the third bit is
		/// being read (not been read yet)
		/// |<-------data[0]------->|     |<---------data[0]------->|           
		///+++++++++++++++++++++++++++++++++++
		/// | 0 |  1 | 2 | 3 |  4 | 5 |  6 | 7 | 8 |  9 |10 |11 |12 |13 |14 | 15 |   bit index
		///+++++++++++++++++++++++++++++++++++
		/// | 0 |  0 | 0 | 1 |  0 | 0 |  0 | 0 | 1 |  0 |  0 |  0 |  0  |  0 |  0 |   0 |  bit in memory
		///+++++++++++++++++++++++++++++++++++
		///              ^                                                       ^
		/// for example, @mWritePosBits = 12, @mReadPosBits = 2,
		/// base on draw above, all the unwritten bits are 0000 (index 12,13,14,15), 
		/// all the unread bits are 10 bits (0100001000, index 2,3,4,5,6,7,8,9,10,11),
		/// we can calculate:
		/// @the index of the byte that @mReadPosBits points to is 
		/// 0 = @mReadPosBits >> 3 = 2/8 = index 0, data[0]
		/// @the index of the byte that @mWritePosBits points to is 
		/// 1 = @mWritePosBits >> 3 = 12/8 = index 1, data[1]
		/// @the offset for mReadPosBits to the byte boundary = mReadPosBits mod 8
		/// =  mReadPosBits & 7 = 2 &7 = 2 bits (00, index 0,1) 
		/// @the offset for mWritePosBits to the byte boundary  = mWritePosBits mod 8 
		/// =  mWritePosBits & 7 = 12 &7 = 4 bits (1000, index 8,9,10,11) 
		/// @BITS_TO_BYTES(mWritePosBits[bit at index 12 is exclusive ]) 
		/// = (12+7) >> 3 = 19/8 = 2( also is the number of written bytes)
		/// BITS_TO_BYTES(8[bit at index 8 is exclusive ])  = 
		/// (8+7)>>3 = 15/8 = 1 ( also is the number of written bytes)
		/// 
	private:
		BitSize mBitsAllocSize;
		BitSize mWritingPosBits;
		BitSize mReadingPosBits;
		UInt8 *data;
		/// true if @data is pointint to heap-memory pointer, 
		/// false if it is stack-memory  pointer
		bool mNeedFree;
		/// true if writting not allowed in which case all write functions will not work
		/// false if writting is allowed 
		bool mReadOnly;
		UInt8 mStacBuffer[JACKIESTREAM_STACK_ALLOC_SIZE];

		JackieBits(const JackieBits &invalid)
		{
			(void)invalid;
			CHECK(0);
		}
		JackieBits& operator = (const JackieBits& invalid)
		{
			(void)invalid;
			CHECK(0);
			static JackieBits i;
			return i;
		}

	public:
		STATIC_FACTORY_DECLARATIONS(JackieBits);

		/// @Param [in] [ BitSize initialBytesAllocate]:
		/// the number of bytes to pre-allocate.
		/// @Remarks:
		/// Create the JackieBits, with some number of bytes to immediately
		/// allocate. There is no benefit to calling this, unless you know exactly
		/// how many bytes you need and it is greater than 256.
		/// @Author mengdi[Jackie]
		JackieBits(const BitSize initialBytesAllocate);

		/// @brief  Initialize by setting the @data to a predefined pointer.
		/// @access  public  
		/// @param [in] [UInt8 * src]  
		/// @param [in] [const  ByteSize len]  unit of byte
		/// @param [in] [bool copy]  
		/// true to make an deep copy of the @src . 
		/// false to just save a pointer to the @src.
		/// @remarks
		/// 99% of the time you will use this function to read Packet:;data, 
		/// in which case you should write something as follows:
		/// JACKIE_INET::JackieStream js(packet->data, packet->length, false);
		/// @author mengdi[Jackie]
		JackieBits(UInt8* src, const ByteSize len, bool copy = false);

		JackieBits();
		~JackieBits();


		/// @ brief Use Mini write and read internally 
		template <class templateType>
		JackieBits& operator<<(templateType& c)
		{
			WriteMini(c);
			return *this;
		}
		template <class templateType>
		JackieBits& operator>>(templateType& c)
		{
			ReadMini(c);
			return *this;
		}

		template <>
		JackieBits& operator<<(JackieBits& c)
		{
			Write(c);
			return *this;
		}

		/// Getters and Setters
		BitSize WritePosBits() const { return mWritingPosBits; }
		BitSize WritePosByte() const { return BITS_TO_BYTES(mWritingPosBits); }
		BitSize ReadPosBits() const { return mReadingPosBits; }
		UInt8 * Data() const { return data; }
		void Data(UInt8* val){ data = val; mReadOnly = true; }
		void WritePosBits(BitSize val) { mWritingPosBits = val; }
		void ReadPosBits(BitSize val) { mReadingPosBits = val; }
		void BitsAllocSize(BitSize val) { mBitsAllocSize = val; }

		/// @Brief  Resets for reuse.
		/// @Access  public  
		/// @Notice
		/// Do NOT reallocate memory because JackieStream is used
		/// to serialize/deserialize a buffer. Reallocation is a dangerous 
		/// operation (may result in leaks).
		/// @author mengdi[Jackie]
		inline void Reset(void) { mWritingPosBits = mReadingPosBits = 0; }

		///@brief Sets the read pointer back to the beginning of your data.
		/// @access public
		/// @author mengdi[Jackie]
		inline void ResetReadPosBits(void)
		{
			mReadingPosBits = 0;
		}

		/// @brief Sets the write pointer back to the beginning of your data.
		/// @access public
		/// @author mengdi[Jackie]
		inline void ResetWritePosBits(void)
		{
			mWritingPosBits = 0;
		}

		/// @brief this is good to call when you are done with the stream to make
		/// sure you didn't leave any data left over void
		/// should hit if reads didn't match writes
		/// @access public
		/// @author mengdi[Jackie]
		inline void AssertStreamEmpty(void)
		{
			CHECK(mReadingPosBits == mWritingPosBits);
		}

		///@brief payload are actually the unread bits
		/// @access public 
		inline BitSize GetPayLoadBits(void) const
		{
			return mWritingPosBits - mReadingPosBits;
		}

		///@brief the number of bytes needed to  hold all the written bits 
		/// @access public 
		///@notice
		/// particial byte is also accounted and the bit at index @param 
		/// mWritePosBits is exclusive). 
		/// if mWritingPosBits =12, will need 2 bytes to hold 12 written bits (6 bits wasted)
		/// if mWritingPosBits = 8, will need 1 byte to hold 8 written bits (0 bits wasted)
		/// @author mengdi[Jackie]
		inline ByteSize GetWrittenBytesCount(void) const
		{
			return BITS_TO_BYTES(mWritingPosBits);
		}

		/// @brief get the number of written bits
		/// will return same value to that of WritePosBits()
		/// @access public
		/// @author mengdi[Jackie]
		inline BitSize GetWrittenBitsCount(void) const { return mWritingPosBits; }

		/// @method SerializeFloat16
		/// @access public 
		/// @returns void
		/// @param [in] bool writeBitstream
		/// writeBitstream true to write from your data to this bitstream. 
		/// False to read from this bitstream and write to your data
		/// @param [in] float & inOutFloat  The float to write
		/// @param [in] float floatMin Predetermined minimum value of f
		/// @param [in] float floatMax Predetermined maximum value of f
		/// @brief Serialize a float into 2 bytes, spanning the range 
		/// between @param floatMin and @param floatMax
		inline void SerializeFloatRange16Bits(bool writeBitstream, float &inOutFloat,
			float floatMin, float floatMax)
		{
			if (writeBitstream)
				WriteFloatRange(inOutFloat, floatMin, floatMax);
			else
				ReadFloatRange(inOutFloat, floatMin, floatMax);
		}
		//inline void SerializeDouble32Bits(bool writeBitstream, double &inOutFloat,
		//	double floatMin, double floatMax)
		//{
		//	if (writeBitstream)
		//		Write(inOutFloat, floatMin, floatMax);
		//	else
		//		Read(inOutFloat, floatMin, floatMax);
		//}

		/// @method Serialize
		/// @access public 
		/// @brief  
		/// bidirectional serialize/deserialize an array or casted stream or raw data.  
		/// This does NOT do endian swapping.
		/// @param[in] writeBitstream 
		/// true to write from your data to this bitstream.  
		/// false to read from this bitstream and write to your data
		/// @param[in] inOutByteArray a byte buffer
		/// @param[in] numberOfBytes the size of @a input in bytes
		/// @return void
		inline void Serialize(bool writeBitstream, Int8* inOutByteArray,
			const UInt32 numberOfBytes)
		{
			if (writeBitstream)
				Write(inOutByteArray, numberOfBytes);
			else
				Read(inOutByteArray, numberOfBytes);
		}

		/// @method Serialize
		/// @access public 
		/// @returns void
		/// @param [in] bool writeBitstream
		/// true to write from your data to this bitstream.
		/// false to read from this bitstream and write to your data
		/// @param [in] templateType & inOutTemplateVar The value to write
		/// @brief bidirectional serialize/deserialize any integral type to/from a bitstream. 
		/// undefine DO_NOT_SWAP_ENDIAN if you need endian swapping.
		/// for float and double,  use SerializeMini()
		template <class templateType>
		inline void Serialize(bool writeBitstream,
			templateType &inOutTemplateVar)
		{
			if (writeBitstream)
				Write(inOutTemplateVar);
			else
				Read(inOutTemplateVar);
		}

		/// @method SerializeChangedValue
		/// @access public 
		/// @param[in] writeBitstream 
		/// true to write from your data to this bitstream.  
		/// false to read from this bitstream and write to your data
		/// @param[in] inOutCurrentValue The current value to write
		/// @param[in] lastValue The last value to compare against.  
		/// only used if @a writeBitstream is true.
		/// @return void
		/// @brief Bidirectional serialize/deserialize any integral type to/from a bitstream. 
		/// @notice 
		/// If the current value is different from the last value
		/// the current value will be written. Otherwise, a single bit will be written
		template <class IntegralType>
		inline void SerializeChangedValue(bool writeBitstream, IntegralType &inOutCurrentValue, const IntegralType &lastValue)
		{
			if (writeBitstream)
				WriteChangedValue(inOutCurrentValue, lastValue);
			else
				ReadChangedValue(inOutCurrentValue);
		}

		/// @method SerializeChangedValue
		/// @access public 
		/// @param[in] inOutCurrentValue The current value to write
		/// @return void
		/// @brief  when you don't know what the last value is, or there is no last value.
		/// @param[in] writeBitstream
		/// true to write from your data to this bitstream.  
		/// false to read from this bitstream and write to your data
		template <class IntegralType>
		inline void SerializeChangedValue(bool writeBitstream, IntegralType &Value)
		{
			if (writeBitstream)
				WriteChangedValue(Value);
			else
				ReadChangedValue(Value);
		}


		/// @method SerializeMini
		/// @access public 
		/// @returns void
		/// @template BasicType all integral types plus all floating types
		/// @param [in] bool writeBitstream
		/// @param [in] templateType & inOutTemplateVar
		/// @brief  
		/// bidirectional serialize/deserialize any basic type to/from a bitstream.
		/// undefine DO_NOT_SWAP_ENDIAN if you need endian swapping.
		/// @notice in case of floating type, 
		/// for floating-point part, this is lossy, using 2 bytes for a float and 4 for a double.
		/// the range must be between -1 and +1. For non-floating-point part, 
		/// this is lossless, but only has benefit if you use less than half the bits of the type.
		/// if you are not using DO_NOT_SWAP_ENDIAN the opposite is true for types 
		/// larger than 1 byte
		template <class BasicType>
		inline void SerializeMini(bool writeBitstream, BasicType &inOutTemplateVar)
		{
			if (writeBitstream)
				WriteMini(inOutTemplateVar);
			else
				ReadMini(inOutTemplateVar);
		}

		/// @method SerializeMini
		/// @access public 
		/// @returns void
		/// @template BasicType all integral types plus all floating types
		/// @param[in] writeBitstream true to write from your data to this bitstream.  
		/// false to read from this bitstream and write to your data
		/// @param[in] currValue The current value to be written or to be read
		/// @param[in] lastValue The last value to compare against to be written  
		/// Only used if @param writeBitstream is true.
		/// @brief 
		/// Bidirectional serialize/deserialize any integral type to/from a bitstream.  
		/// If the current value is different from the last value
		/// the current value will be written.  Otherwise, a single bit will be written
		/// @notice
		/// For floating point, this is lossy, using 2 bytes for a float and 4 for a double.  
		/// The range must be between -1 and +1.
		/// For non-floating point, this is lossless, but only has benefit if you use less than
		/// half the bits of the type.  If you are not using DO_NOT_SWAP_ENDIAN the 
		/// opposite is true for types larger than 1 byte
		template <class BasicType>
		inline void SerializeMiniChangedValue(bool writeBitstream,
			BasicType &currValue, const BasicType &lastValue)
		{
			if (writeBitstream)
				WriteMiniChanged(currValue, lastValue);
			else
				ReadMiniChanged(currValue);
		}

		template <class BasicType>
		inline void SerializeMiniChangedValue(bool writeBitstream,
			BasicType &currValue)
		{
			if (writeBitstream)
				WriteMiniChanged(currValue);
			else
				ReadMiniChanged(currValue);
		}

		/// @method SerializeCasted
		/// @access public 
		/// @returns void
		/// @template serializationType all integral types plus all floating types dest type
		/// @template sourceType all integral types plus all floating types src type
		/// @param [in] bool writeBitstream
		/// true to write from your data to this bitstream.  
		/// false to read from this bitstream and write to your data
		/// @param [in] sourceType & value
		/// @brief Serialize one type casted to another (smaller) type, to save bandwidth
		/// serializationType should be uint8, uint16, uint24, or uint32
		/// @use
		/// would use 1 byte to write what would otherwise be an integer(4 or 8 bytes)
		/// int num=53; SerializeCasted<uint8>(true, num); 
		/// uint8 val; SerializeCasted<uint8>(false, val);
		template <class serializationType, class sourceType >
		void SerializeCasted(bool writeBitstream, sourceType &value)
		{
			if (writeBitstream)
				WriteCasted<serializationType>(value);
			else
				ReadCasted<serializationType>(value);
		}

		/// @method SerializeIntegerRange
		/// @access public 
		/// @returns void
		/// @param [in] bool writeBitstream 
		/// true to write from your data to this bitstream.
		/// false to read from this bitstream and write to your data
		/// @param [in] templateType & value  Integer value to write, 
		/// which should be between @paramminimum and @param maximum
		/// @param [in] const templateType minimum best to use global const
		/// @param [in] const templateType maximum best to use global const
		/// @param [in] bool allowOutsideRange
		/// If true, all sends will take an extra bit, however value can deviate
		/// from outside @param minimum and @param maximum.
		/// If false, will assert if the value deviates
		/// @brief Given the minimum and maximum values for an integer type,
		/// figure out the minimum number of bits to represent the range
		/// Then serialize only those bits, smaller the difference is, less bits to use,
		/// no matter how big the max or mini is, best to send and recv huge numbers,
		/// like 666666666, SerializeMini() will not work well in such case,
		/// @notice
		/// A static is used so that the required number of bits for
		/// (maximum - minimum) is only calculated once.This does require that
		/// @param minimum and @param maximum are fixed values for a 
		/// given line of code  for the life of the program
		/// @use 
		/// const uint64 MAX_VALUE = 1000000000;
		/// const uint64 Mini_VALUE = 999999900;
		/// uint64 currVal = 999999966; 
		/// SerializeIntegerRange(true, currVal, MAX_VALUE, Mini_VALUE);
		/// uint64 Val;
		/// SerializeIntegerRange(fals, Val, MAX_VALUE, Mini_VALUE);
		/// the sample above will use 7 bits (128) instead of 8 bytes
		/// if you use SerializeMini(), will also use 8 bytes for no all zero byte to compress 
		template <class IntegerType>
		void SerializeIntegerRange(bool writeBitstream,
			IntegerType &value,
			const IntegerType minimum,
			const IntegerType maximum,
			bool allowOutsideRange = false)
		{
			//int requiredBits = BYTES_TO_BITS(sizeof(templateType)) -
			//	GetLeadingZeroSize(templateType(maximum - minimum));
			//SerializeBitsIntegerRange(writeBitstream,
			//	value,
			//	minimum,
			//	maximum,
			//	requiredBits,
			//	allowOutsideRange);
			if (writeBitstream)
				WriteIntegerRange(value, minimum, maximum, allowOutsideRange);
			else
				ReadIntegerRange(value, minimum, maximum, allowOutsideRange);
		}
		/// \param[in] requiredBits Primarily for internal use, called from above function() after calculating number of bits needed to represent maximum-minimum
		//template <class templateType>
		//bool SerializeIntegerRange(bool writeBitstream, templateType &value, const templateType minimum, const templateType maximum, const int requiredBits, bool allowOutsideRange = false)
		//{
		//	if (writeBitstream)
		//		WriteIntegerRange(value, minimum, maximum,
		//		requiredBits, allowOutsideRange);
		//	else
		//		ReadIntegerRange(value, minimum, maximum, 
		//		requiredBits, allowOutsideRange);
		//}

		/// @method SerializeNormVector
		/// @access public 
		/// @returns void
		/// @param [in] bool writeBitstream
		/// true to write from your data to this bitstream.  
		/// false to read from this bitstream and write to your data
		/// @param [in] templateType & x
		/// @param [in] templateType & y
		/// @param [in] templateType & z
		/// @brief bidirectional serialize/deserialize a normalized 3D vector,
		/// using (at most) 4 bytes + 3 bits instead of 12-24 bytes. 
		/// will further compress y or z axis aligned vectors.
		/// Accurate to 1/32767.5.
		/// @notice
		/// templateType for this function must be a float or double
		template <class templateType>
		void SerializeNormVector(bool writeBitstream,
			templateType &x, templateType &y, templateType &z)
		{
			if (writeBitstream)
				WriteNormVector(x, y, z);
			else
				ReadNormVector(x, y, z);
		}


		/// @method SerializeVector
		/// @access public 
		/// @returns void
		/// @param [in] bool writeBitstream
		/// true to write from your data to this bitstream.  
		/// false to read from this bitstream and write to your data
		/// @param [in] templateType & x
		/// @param [in] templateType & y
		/// @param [in] templateType & z
		/// @brief 
		/// bidirectional serialize/deserialize a vector, using 10 bytes instead of 12.
		/// loses accuracy to about 3 / 10ths and only saves 2 bytes, 
		/// so only use if accuracy is not important.
		/// @notice
		/// templateType for this function must be a float or double
		template <class templateType>
		void SerializeVector(bool writeBitstream,
			templateType &x, templateType &y, templateType &z)
		{
			if (writeBitstream)
				WriteVector(x, y, z);
			else
				ReadVector(x, y, z);
		}


		/// @method SerializeNormQuat
		/// @access public 
		/// @returns void
		/// @param [in] bool writeBitstream
		/// true to write from your data to this bitstream.  
		/// false to read from this bitstream and write to your data
		/// @param [in] templateType & x
		/// @param [in] templateType & y
		/// @param [in] templateType & z
		/// @brief 
		/// bidirectional serialize / deserialize a normalized quaternion
		/// in 6 bytes + 4 bits instead of 16 bytes.Slightly lossy.
		/// @notice
		/// templateType for this function must be a float or double
		template <class templateType>
		void SerializeNormQuat(bool writeBitstream,
			templateType &w, templateType &x, templateType &y, templateType &z)
		{
			if (writeBitstream)
				WriteNormQuat(w, x, y, z);
			else
				ReadNormQuat(w, x, y, z);
		}


		/// @method SerializeOrthMatrix
		/// @access public 
		/// @returns void
		/// @param [in] bool writeBitstream
		/// true to write from your data to this bitstream.  
		/// false to read from this bitstream and write to your data
		/// @param [in] templateType & x
		/// @param [in] templateType & y
		/// @param [in] templateType & z
		/// @brief 
		/// bidirectional serialize / deserialize an orthogonal matrix by creating a 
		/// quaternion, and writing 3 components of the quaternion in 2 bytes each.
		/// use 6 bytes instead of 36, 
		/// @notice
		/// templateType for this function must be a float or double
		/// lossy, although the result is renormalized
		template <class templateType>
		void SerializeOrthMatrix(
			bool writeBitstream,
			templateType &m00, templateType &m01, templateType &m02,
			templateType &m10, templateType &m11, templateType &m12,
			templateType &m20, templateType &m21, templateType &m22)
		{
			if (writeBitstream)
				WriteOrthMatrix(m00, m01, m02, m10, m11, m12, m20, m21, m22);
			else
				ReadOrthMatrix(m00, m01, m02, m10, m11, m12, m20, m21, m22);
		}


		/// @method SerializeBits
		/// @access public 
		/// @returns void
		/// @param [in] bool writeBitstream
		/// true to write from your data to this bitstream.
		/// false to read from this bitstream and write to your data
		/// @param [in] UInt8 * inOutByteArray
		/// @param [in] const BitSize numberOfBitsSerialize
		/// @param [in] const bool rightAlignedBits
		/// @brief
		/// @notice
		/// from the right (bit 0) rather than the left (as in the normal
		/// internal representation) You would set this to true when
		/// writing user data, and false when copying bitstream data, such
		/// as writing one bitstream to another
		/// @remarks
		/// right aligned data means in the case of a partial byte, 
		/// the bits are aligned right
		/// @see
		void SerializeBits(bool writeBitstream,
			UInt8* inOutByteArray,
			const BitSize numberOfBitsSerialize,
			const bool rightAlignedBits = true)
		{
			if (writeBitstream)
				WriteBits(inOutByteArray,
				numberOfBitsSerialize, rightAlignedBits);
			else
				ReadBits(inOutByteArray,
				numberOfBitsSerialize, rightAlignedBits);
		}


		/// @method AlignReadPosBitsByteBoundary
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
		inline void AlignReadPosBitsByteBoundary(void)
		{
			mReadingPosBits += 8 - (((mReadingPosBits - 1) & 7) + 1);
		}


		/// @method Read
		/// @access public 
		/// @returns void
		/// @param [in] Int8 * output 
		/// The result byte array. It should be larger than @em numberOfBytes.
		/// @param [in] const unsigned int numberOfBytes  The number of byte to read
		/// @brief Read an array of raw data or casted stream of byte.
		/// @notice The array is raw data. 
		/// There is no automatic endian conversion with this function
		void Read(Int8* output, const unsigned int numberOfBytes)
		{
			ReadBits((UInt8*)output, BYTES_TO_BITS(numberOfBytes));
		}


		/// @func   ReadBits
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
		void ReadBits(UInt8 *dest, BitSize bitsRead, bool alignRight = true);


		/// @method Read
		/// @access public 
		/// @returns void
		/// @param [in] IntegralType & outTemplateVar
		/// @brief Read any integral type from a bitstream.  
		/// Define DO_NOT_SWAP_ENDIAN if you need endian swapping.
		template <class IntegralType>
		inline void Read(IntegralType &dest)
		{
			if (sizeof(IntegralType) == 1)
				ReadBits((UInt8*)&dest, sizeof(IntegralType) * 8, true);
			else
			{
#ifndef DO_NOT_SWAP_ENDIAN
				if (DoEndianSwap())
				{
					UInt8 output[sizeof(IntegralType)];
					ReadBits(output, BYTES_TO_BITS(sizeof(IntegralType)), true);
					ReverseBytes(output, (UInt8*)&dest, sizeof(IntegralType));
				}
				else
				{
					ReadBits((UInt8*)&dest, BYTES_TO_BITS(sizeof(IntegralType)), true);
				}
#else
				ReadBits((UInt8*)&dest, BYTES_TO_BITS(sizeof(IntegralType)), true);
#endif
			}
		}


		/// @method Read
		/// @access public 
		/// @returns void
		/// @param [in] bool & dest The value to read
		/// @brief  Read a bool from a bitstream.
		template <>
		inline void Read(bool &dest)
		{
			DCHECK(GetPayLoadBits() >= 1);
			//if (GetPayLoadBits() < 1) return;
			// Has to be on a different line for Mac
			// Is it faster to just write it out here?
			dest = (data[mReadingPosBits >> 3] & (0x80 >> (mReadingPosBits & 7))) != 0;
			mReadingPosBits++;
		}


		/// @method Read
		/// @access public 
		/// @returns void
		/// @param [in] JackieAddress & dest The value to read
		/// @brief Read a JackieAddress from a bitstream.
		template <>
		inline void Read(JackieAddress &dest)
		{
			UInt8 ipVersion;
			Read(ipVersion);
			if (ipVersion == 4)
			{
				dest.address.addr4.sin_family = AF_INET;
				// Read(var.binaryAddress);
				// Don't endian swap the address or port
				UInt32 binaryAddress;
				ReadBits((UInt8*)& binaryAddress,
					BYTES_TO_BITS(sizeof(binaryAddress)), true);
				// Unhide the IP address, done to prevent routers from changing it
				dest.address.addr4.sin_addr.s_addr = ~binaryAddress;
				ReadBits((UInt8*)& dest.address.addr4.sin_port, BYTES_TO_BITS(sizeof(dest.address.addr4.sin_port)), true);
				dest.debugPort = ntohs(dest.address.addr4.sin_port);
			}
			else
			{
#if NET_SUPPORT_IPV6==1
				ReadBits((UInt8*)&dest.address.addr6, BYTES_TO_BITS(sizeof(dest.address.addr6)), true);
				dest.debugPort = ntohs(dest.address.addr6.sin6_port);
				//return b;
#else
				//return false;
#endif
			}
		}


		/// @func Read 
		/// @brief read three bytes into stream
		/// @access  public  
		/// @param [in] [const UInt24 & inTemplateVar]  
		/// @return [void] 
		/// @remark
		/// @notice will align @mReadPosBIts to byte-boundary internally
		/// @see  AlignReadPosBitsByteBoundary()
		/// @author mengdi[Jackie]
		template <>
		inline void Read(UInt24 &dest)
		{
			AlignReadPosBitsByteBoundary();
			if (GetPayLoadBits() < 24) return;

			if (!IsBigEndian())
			{
				((UInt8 *)&dest.val)[0] = data[(mReadingPosBits >> 3) + 0];
				((UInt8 *)&dest.val)[1] = data[(mReadingPosBits >> 3) + 1];
				((UInt8 *)&dest.val)[2] = data[(mReadingPosBits >> 3) + 2];
				((UInt8 *)&dest.val)[3] = 0;
			}
			else
			{
				((UInt8 *)&dest.val)[3] = data[(mReadingPosBits >> 3) + 0];
				((UInt8 *)&dest.val)[2] = data[(mReadingPosBits >> 3) + 1];
				((UInt8 *)&dest.val)[1] = data[(mReadingPosBits >> 3) + 2];
				((UInt8 *)&dest.val)[0] = 0;
			}

			mReadingPosBits += 24;
		}

		template <>
		inline void Read(JackieGUID &dest)
		{
			return Read(dest.g);
		}


		/// @brief Read any integral type from a bitstream.  
		/// @details If the written value differed from the value 
		/// compared against in the write function,
		/// var will be updated.  Otherwise it will retain the current value.
		/// ReadDelta is only valid from a previous call to WriteDelta
		/// @param[in] outTemplateVar The value to read
		template <class IntegralType>
		inline void ReadChangedValue(IntegralType &dest)
		{
			bool dataWritten;
			Read(dataWritten);
			if (dataWritten) Read(dest);
		}


		/// @brief Read a bool from a bitstream.
		/// @param[in] outTemplateVar The value to read
		template <>
		inline void ReadChangedValue(bool &dest)
		{
			return Read(dest);
		}


		/// @Brief Assume the input source points to a compressed native type. 
		/// Decompress and read it.
		void ReadMini(UInt8* dest, const BitSize bits2Read, const bool isUnsigned);


		/// @method ReadMini
		/// @access public 
		/// @returns void
		/// @param [in] IntegralType & dest
		/// @brief Read any integral type from a bitstream.  
		/// @notice
		/// For floating point, this is lossy, using 2 bytes for a float and 4 for
		/// a double.  The range must be between -1 and +1.
		/// For non-floating point, this is lossless, but only has benefit if you 
		/// use less than half the bits of the type
		template <class IntegralType>
		inline void ReadMini(IntegralType &dest)
		{
			ReadMini((UInt8*)&dest, BYTES_TO_BITS(sizeof(IntegralType)), true);
			//			if (sizeof(dest) == 1)
			//				ReadMini((UInt8*)&dest, BYTES_TO_BITS(sizeof(IntegralType)), true);
			//			else
			//			{
			//#ifndef DO_NOT_SWAP_ENDIAN
			//				if (DoEndianSwap())
			//				{
			//					//UInt8 output[sizeof(IntegralType)];
			//					//ReadMini((UInt8*)output, BYTES_TO_BITS(sizeof(IntegralType)), true);
			//					//ReverseBytes(output, (UInt8*)&dest, sizeof(IntegralType));
			//					ReadMini((UInt8*)&dest, BYTES_TO_BITS(sizeof(IntegralType)), true);
			//				}
			//				else
			//				{
			//					ReadMini((UInt8*)& dest, BYTES_TO_BITS(sizeof(IntegralType)), true);
			//				}
			//#else
			//				ReadMini((UInt8*)& dest, BYTES_TO_BITS(sizeof(IntegralType)), true);
			//#endif
			//			}
		}
		template <> inline void ReadMini(JackieAddress &dest)
		{
			UInt8 ipVersion;
			ReadMini(ipVersion);
			if (ipVersion == 4)
			{
				dest.address.addr4.sin_family = AF_INET;
				// Read(var.binaryAddress);
				// Don't endian swap the address or port
				UInt32 binaryAddress;
				ReadMini(binaryAddress);
				// Unhide the IP address, done to prevent routers from changing it
				dest.address.addr4.sin_addr.s_addr = ~binaryAddress;
				ReadMini(dest.address.addr4.sin_port);
				dest.debugPort = ntohs(dest.address.addr4.sin_port);
			}
			else
			{
#if NET_SUPPORT_IPV6==1
				ReadMini(dest.address.addr6);
				dest.debugPort = ntohs(dest.address.addr6.sin6_port);
				//return b;
#else
				//return false;
#endif
			}
		}
		template <> inline void ReadMini(UInt24 &dest)
		{

			ReadMini(dest.val);
		}
		template <> inline void ReadMini(JackieGUID &dest)
		{
			ReadMini(dest.g);
		}
		template <> inline void ReadMini(bool &dest)
		{
			Read(dest);
		}
		template <> /// For values between -1 and 1
		inline void ReadMini(float &dest)
		{
			UInt16 compressedFloat;
			ReadMini(compressedFloat);
			dest = ((float)compressedFloat / 32767.5f - 1.0f);
		}
		template <> /// For values between -1 and 1
		inline void ReadMini(double &dest)
		{
			UInt32 compressedFloat;
			ReadMini(compressedFloat);
			dest = ((double)compressedFloat / 2147483648.0 - 1.0);
		}


		/// @access public 
		/// @brief
		template <class srcType, class destType >
		void ReadCasted(destType &value)
		{
			srcType val;
			Read(val);
			value = (destType)val;
		}

		/// @method ReadMiniChanged
		/// @access public 
		/// @returns void
		/// @param [in] templateType & dest
		/// @brief Read any integral type from a bitstream.  
		/// If the written value differed from the value compared against in 
		/// the write function, var will be updated.  Otherwise it will retain the
		/// current value. the current value will be updated.
		/// @notice
		/// For floating point, this is lossy, using 2 bytes for a float and 4 for a 
		/// double.  The range must be between -1 and +1. For non-floating point, 
		/// this is lossless, but only has benefit if you use less than half the bits of the
		/// type.  If you are not using DO_NOT_SWAP_ENDIAN the opposite is true for
		/// types larger than 1 byte
		/// @see
		template <class IntegralType>
		inline void ReadMiniChanged(IntegralType &dest)
		{
			bool dataWritten;
			Read(dataWritten);
			if (dataWritten) ReadMini(dest);
		}


		/// @brief Read a bool from a bitstream.
		/// @param[in] outTemplateVar The value to read
		template <>
		inline void ReadMiniChanged(bool &dest)
		{
			Read(dest);
		}

		template <class IntegerType>
		void ReadIntegerRange(
			IntegerType &value,
			const IntegerType minimum,
			const IntegerType maximum,
			bool allowOutsideRange)
		{
			/// get the high byte bits size
			int requiredBits = BYTES_TO_BITS(sizeof(IntegerType)) - GetLeadingZeroSize(IntegerType(maximum - minimum));
			ReadIntegerRange(value, minimum, maximum, requiredBits, allowOutsideRange);
		}


		/// @method ReadBitsIntegerRange
		/// @access public 
		/// @returns void
		/// @param [in] templateType & value
		/// @param [in] const templateType minimum
		/// @param [in] const templateType maximum
		/// @param [in] const int requiredBits the value of bits to read
		/// @param [in] bool allowOutsideRange 
		/// if true, we directly read it
		/// @brief
		/// @notice
		/// This is how we write value
		/// Assume@valueBeyondMini's value is 0x000012
		///------------------> Memory Address
		///+++++++++++
		///| 00 | 00 | 00 | 12 |  Big Endian
		///+++++++++++
		///+++++++++++
		///| 12 | 00 | 00 | 00 |  Little Endian 
		///+++++++++++
		/// so for big endian, we need to reverse byte so that
		/// the high byte of 0x00 that was put in low address can be written correctly
		/// for little endian, we do nothing.
		/// After reverse bytes:
		///+++++++++++
		///| 12 | 00 | 00 | 00 |  Big Endian 
		///+++++++++++
		///+++++++++++
		///| 12 | 00 | 00 | 00 |  Little Endian 
		///+++++++++++
		/// When reading it, we have to reverse it back fro big endian
		/// we do nothing for little endian.
		/// @see
		template <class templateType>
		void ReadIntegerRange(
			templateType &value,
			const templateType minimum,
			const templateType maximum,
			const int requiredBits,
			bool allowOutsideRange)
		{
			DCHECK(maximum >= minimum);

			if (allowOutsideRange)
			{
				bool isOutsideRange;
				Read(isOutsideRange);
				if (isOutsideRange)
				{
					Read(value);
					return;
				}
			}

			value = 0;
			ReadBits((UInt8*)&value, requiredBits, true);
			if (IsBigEndian())
			{
				value >>= (BYTES_TO_BITS(sizeof(value)) - requiredBits);
			}
			value += minimum;

			//UInt8 output[sizeof(templateType)] = { 0 };
			//ReadBits(output, requiredBits, true);
			//if (IsBigEndian()) ReverseBytes(output, sizeof(output));
			//memcpy(&value, output, sizeof(output));
			//value += minimum;

		}


		/// @method Read
		/// @access public 
		/// @returns void
		/// @param [in] float & outFloat The float to read
		/// @param [in] float floatMin  Predetermined minimum value of f
		/// @param [in] float floatMax Predetermined maximum value of f
		/// @brief
		/// Read a float into 2 bytes, spanning the range between
		/// @param floatMin and @param floatMax
		/// @notice
		/// @see
		void ReadFloatRange(float &outFloat, float floatMin, float floatMax);


		/// @brief Read bytes, starting at the next aligned byte. 
		/// @details Note that the modulus 8 starting offset of the sequence
		/// must be the same as was used with WriteBits. This will be a problem
		/// with packet coalescence unless you byte align the coalesced packets.
		/// @param[in] dest The byte array larger than @em numberOfBytesRead
		/// @param[in] bytes2Read The number of byte to read from the internal state
		/// @return true if there is enough byte.
		void ReadAlignedBytes(UInt8 *dest, const ByteSize bytes2Read);


		/// @brief Reads what was written by WriteAlignedBytes.
		/// @param[in] inOutByteArray The data
		/// @param[in] maxBytesRead Maximum number of bytes to read
		/// @return true on success, false on failure.
		void ReadAlignedBytes(Int8 *dest, ByteSize &bytes2Read,
			const ByteSize maxBytes2Read);


		/// @method ReadAlignedBytesAlloc
		/// @access public 
		/// @returns void
		/// @param [in] Int8 * * dest  will be deleted if it is not a pointer to 0
		/// @param [in] ByteSize & bytes2Read
		/// @param [in] const ByteSize maxBytes2Read
		/// @brief  Same as ReadAlignedBytesSafe() but allocates the memory
		/// for you using new, rather than assuming it is safe to write to
		void ReadAlignedBytesAlloc(Int8 **dest, ByteSize &bytes2Read, const ByteSize maxBytes2Read);

		// @brief return 1 if the next data read is a 1, 0 if it is a 0
		///@access public 
		inline UInt32 ReadBit(void)
		{
			UInt32 result = ((data[mReadingPosBits >> 3] & (0x80 >> (mReadingPosBits & 7))) != 0) ? 1 : 0;
			mReadingPosBits++;
			return result;
		}

		/// @access public
		/// @brief Read a normalized 3D vector, using (at most) 4 bytes 
		/// + 3 bits instead of 12-24 bytes.  
		/// @details Will further compress y or z axis aligned vectors.
		/// Accurate to 1/32767.5.
		/// @param[in] x x
		/// @param[in] y y
		/// @param[in] z z
		/// @return void
		/// @notice templateType for this function must be a float or double
		template <class templateType>
		void ReadNormVector(templateType &x, templateType &y, templateType &z)
		{
			ReadFloatRange(x, -1.0f, 1.0f);
			ReadFloatRange(y, -1.0f, 1.0f);
			ReadFloatRange(z, -1.0f, 1.0f);
		}


		/// @brief Read 3 floats or doubles, using 10 bytes, 
		/// where those float or doubles comprise a vector.
		/// @details Loses accuracy to about 3/10ths and only saves 2 bytes, 
		/// so only use if accuracy is not important.
		/// @param[in] x x
		/// @param[in] y y
		/// @param[in] z z
		/// @return void
		/// @notice templateType for this function must be a float or double
		template <class templateType>
		void ReadVector(templateType &x, templateType &y, templateType &z)
		{
			float magnitude;
			Read(magnitude);

			if (magnitude > 0.00001f)
			{
				//float cx = 0.0f, cy = 0.0f, cz = 0.0f;
				//ReadMini(cx);
				//ReadMini(cy);
				//ReadMini(cz);
				//x = cx;
				//y = cy;
				//z = cz;
				//x *= magnitude;
				//y *= magnitude;
				//z *= magnitude;

				ReadMini(x);
				ReadMini(y);
				ReadMini(z);
				x *= magnitude;
				y *= magnitude;
				z *= magnitude;
			}
			else
			{
				x = 0.0;
				y = 0.0;
				z = 0.0;
			}
		}

		/// @brief Read a normalized quaternion in 6 bytes + 4 bits instead of 16 bytes.
		/// @param[in] w w
		/// @param[in] x x
		/// @param[in] y y
		/// @param[in] z z
		/// @return void
		/// @notice templateType for this function must be a float or double
		template <class templateType>
		bool ReadNormQuat(templateType &w, templateType &x, templateType &y, templateType &z)
		{
			bool cwNeg = false, cxNeg = false, cyNeg = false, czNeg = false;
			Read(cwNeg);
			Read(cxNeg);
			Read(cyNeg);
			Read(czNeg);

			UInt16 cx, cy, cz;
			ReadMini(cx);
			ReadMini(cy);
			ReadMini(cz);

			// Calculate w from x,y,z
			x = (templateType)(cx / 65535.0);
			y = (templateType)(cy / 65535.0);
			z = (templateType)(cz / 65535.0);

			if (cxNeg) x = -x;
			if (cyNeg) y = -y;
			if (czNeg) z = -z;

			float difference = 1.0f - x*x - y*y - z*z;
			if (difference < 0.0f) difference = 0.0f;

			w = (templateType)(sqrt(difference));
			if (cwNeg) w = -w;
		}



		/// @brief Read an orthogonal matrix from a quaternion, 
		/// reading 3 components of the quaternion in 2 bytes each and
		/// extrapolatig the 4th.
		/// @details Use 6 bytes instead of 36
		/// Lossy, although the result is renormalized
		/// @return true on success, false on failure.
		///@notice templateType for this function must be a float or double
		template <class templateType>
		void ReadOrthMatrix(
			templateType &m00, templateType &m01, templateType &m02,
			templateType &m10, templateType &m11, templateType &m12,
			templateType &m20, templateType &m21, templateType &m22)
		{
			float qw, qx, qy, qz;
			ReadNormQuat(qw, qx, qy, qz);

			// Quat to orthogonal rotation matrix
			// http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionMatrix/index.htm
			double sqw = (double)qw*(double)qw;
			double sqx = (double)qx*(double)qx;
			double sqy = (double)qy*(double)qy;
			double sqz = (double)qz*(double)qz;
			m00 = (templateType)(sqx - sqy - sqz + sqw); // since sqw + sqx + sqy + sqz =1
			m11 = (templateType)(-sqx + sqy - sqz + sqw);
			m22 = (templateType)(-sqx - sqy + sqz + sqw);

			double tmp1 = (double)qx*(double)qy;
			double tmp2 = (double)qz*(double)qw;
			m10 = (templateType)(2.0 * (tmp1 + tmp2));
			m01 = (templateType)(2.0 * (tmp1 - tmp2));

			tmp1 = (double)qx*(double)qz;
			tmp2 = (double)qy*(double)qw;
			m20 = (templateType)(2.0 * (tmp1 - tmp2));
			m02 = (templateType)(2.0 * (tmp1 + tmp2));
			tmp1 = (double)qy*(double)qz;
			tmp2 = (double)qx*(double)qw;
			m21 = (templateType)(2.0 * (tmp1 + tmp2));
			m12 = (templateType)(2.0 * (tmp1 - tmp2));
		}


		/// @func AppendBitsCouldRealloc 
		/// @brief 
		/// reallocates (if necessary) in preparation of writing @bits2Append
		/// all internal status will not be changed like @mWritePosBits and so on
		/// @access  public  
		/// @notice  
		/// It is caller's reponsibility to ensure 
		/// @param bits2Append > 0 and @param mReadOnly is false
		/// @author mengdi[Jackie]
		void AppendBitsCouldRealloc(const BitSize bits2Append);


		/// @func  WriteBits 
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
		void WriteBits(const UInt8* src, BitSize bits2Write, bool rightAligned = true);

		/// @func Write
		/// @access  public  
		/// @brief write an array or raw data in bytes.
		/// NOT do endian swapp.
		/// default is right aligned[true]
		/// @author mengdi[Jackie]
		inline void Write(const Int8* src, const ByteSize bytes2Write)
		{
			WriteBits((UInt8*)src, BYTES_TO_BITS(bytes2Write), true);
		}

		/// @brief Write one JackieBits to another.
		/// @param[in] [bits2Write] bits to write
		/// @param[in] [JackieBits] the JackieBits to copy from
		void Write(JackieBits *jackieBits, BitSize bits2Write);
		inline void Write(JackieBits &jackieBits, BitSize bits2Write)
		{
			Write(&jackieBits, bits2Write);
		}
		inline void Write(JackieBits *jackieBits)
		{
			Write(jackieBits, jackieBits->GetPayLoadBits());
		}
		inline void Write(JackieBits &jackieBits) { Write(&jackieBits); }

		/// @method WritePtr
		/// @access public 
		/// @returns void
		/// @param [in] IntergralType * src pointing to the value to write
		/// @brief  
		/// write the dereferenced pointer to any integral type to a bitstream.  
		/// Undefine DO_NOT_SWAP_ENDIAN if you need endian swapping.
		template <class IntergralType>
		void WritePtr(IntergralType *src)
		{
			if (sizeof(IntergralType) == 1)
				WriteBits((UInt8*)src, BYTES_TO_BITS(sizeof(IntergralType)), true);
			else
			{
#ifndef DO_NOT_SWAP_ENDIAN
				if (DoEndianSwap())
				{
					UInt8 output[sizeof(IntergralType)];
					ReverseBytes((UInt8*)src, output, sizeof(IntergralType));
					WriteBits((UInt8*)output, BYTES_TO_BITS(sizeof(IntergralType)), true);
				}
				else
#endif
					WriteBits((UInt8*)src, BYTES_TO_BITS(sizeof(IntergralType)), true);
			}
		}

		/// @func WriteBitZero 
		/// @access  public  
		/// @notice @mReadOnly must be false
		/// @author mengdi[Jackie]
		inline void WriteBitZero(void)
		{
			DCHECK(mReadOnly == false);

			//AppendBitsCouldRealloc(1);
			//BitSize shit = 8 - (mWritingPosBits & 7);
			//data[mWritingPosBits >> 3] = ((data[mWritingPosBits >> 3] >> shit) << shit);
			//mWritingPosBits++;

			AppendBitsCouldRealloc(1);
			/// New bytes need to be zeroed
			if ((mWritingPosBits & 7) == 0) data[mWritingPosBits >> 3] = 0;
			mWritingPosBits++;
		}

		/// @func WriteBitOne 
		/// @access  public  
		/// @notice @mReadOnly must be false
		/// @author mengdi[Jackie]
		inline void WriteBitOne(void)
		{
			DCHECK(mReadOnly == false);
			AppendBitsCouldRealloc(1);

			// Write bit 1
			BitSize shift = mWritingPosBits & 7;
			shift == 0 ? data[mWritingPosBits >> 3] = 0x80 :
				data[mWritingPosBits >> 3] |= 0x80 >> shift;
			mWritingPosBits++;
		}


		/// @func AlignWritePosBits2ByteBoundary 
		/// @brief align @mWritePosBits to a byte boundary. 
		/// @access  public  
		/// @notice
		/// this can be used to 'waste' bits to byte align for efficiency reasons It
		/// can also be used to force coalesced bitstreams to start on byte
		/// boundaries so so WriteAlignedBits and ReadAlignedBits both
		/// calculate the same offset when aligning.
		/// @author mengdi[Jackie]
		inline void AlignWritePosBits2ByteBoundary(void)
		{
			mWritingPosBits += 8 - (((mWritingPosBits - 1) & 7) + 1);
		}

		/// @func WriteAlignedBytes 
		/// @brief  align the bitstream to the byte boundary and 
		/// then write the specified number of bytes.  
		/// @access  public  
		/// @param [in] [const UInt8 * src]  
		/// @param [in] [const ByteSize numberOfBytesWrite]  
		/// @returns [void]
		/// @notice this is faster than WriteBits() but
		/// wastes the bits to do the alignment for @mWritePosBits and
		/// requires you to call ReadAlignedBits() at the corresponding 
		/// read position.
		/// @author mengdi[Jackie]
		void WriteAlignedBytes(const UInt8 *src, const ByteSize numberOfBytesWrite);


		/// @brief Aligns the bitstream, writes inputLength, and writes input. 
		/// @access  public  
		/// @param[in] inByteArray The data
		/// @param[in] inputLength the size of input.
		/// @param[in] maxBytesWrite max bytes to write
		/// @notice Won't write beyond maxBytesWrite
		void WriteAlignedBytes(const UInt8 *src, const ByteSize bytes2Write,
			const ByteSize maxBytes2Write);


		/// @func Write 
		/// @brief write a float into 2 bytes, spanning the range, 
		/// between @param[floatMin] and @param[floatMax]
		/// @access  public  
		/// @param [in] [float src]  value to write into stream
		/// @param [in] [float floatMin] Predetermined mini value of f
		/// @param [in] [float floatMax] Predetermined max value of f
		/// @return bool
		/// @notice 
		/// @author mengdi[Jackie]
		void WriteFloatRange(float src, float floatMin, float floatMax);


		/// @func Write 
		/// @brief write any integral type to a bitstream.  
		/// @access  public  
		/// @param [in] [const templateType & src] 
		/// it is user data that is right aligned in default
		/// @return void
		/// @notice will swap endian internally 
		/// if DO_NOT_SWAP_ENDIAN not defined
		/// @author mengdi[Jackie]
		template <class IntergralType>
		void Write(const IntergralType &src)
		{
			if (sizeof(IntergralType) == 1)
			{
				WriteBits((UInt8*)&src, BYTES_TO_BITS(sizeof(IntergralType)), true);
			}
			else
			{
#ifndef DO_NOT_SWAP_ENDIAN
				if (DoEndianSwap())
				{
					UInt8 output[sizeof(IntergralType)];
					ReverseBytes((UInt8*)&src, output, sizeof(IntergralType));
					WriteBits(output, BYTES_TO_BITS(sizeof(IntergralType)), true);
				}
				else
#endif
					WriteBits((UInt8*)&src, BYTES_TO_BITS(sizeof(IntergralType)), true);
			}
		}


		/// @func Write 
		/// @access  public  
		/// @brief Write a bool to a bitstream.
		/// @param [in] [const bool & src] The value to write
		/// @return [bool] true succeed, false failed
		/// @author mengdi[Jackie]
		template <>
		inline void Write(const bool &src)
		{
			if (src == true)
				WriteBitOne();
			else
				WriteBitZero();
		}

		/// @func Write 
		/// @brief write a JackieAddress to stream
		/// @access  public  
		/// @param [in] [const JackieAddress & src]  
		/// @return [bool]  true succeed, false failed
		/// @remark
		/// @notice  will not endian swap the address or port
		/// @author mengdi[Jackie]
		template <>
		inline void Write(const JackieAddress &src)
		{
			UInt8 version = src.GetIPVersion();
			Write(version);

			if (version == 4)
			{
				/// Hide the address so routers don't modify it
				JackieAddress addr = src;
				UInt32 binaryAddress = ~src.address.addr4.sin_addr.s_addr;
				UInt16 p = addr.GetPortNetworkOrder();
				// Don't endian swap the address or port
				WriteBits((UInt8*)&binaryAddress,
					BYTES_TO_BITS(sizeof(binaryAddress)), true);
				WriteBits((UInt8*)&p, BYTES_TO_BITS(sizeof(p)), true);
			}
			else
			{
#if NET_SUPPORT_IPV6 == 1
				// Don't endian swap
				WriteBits((UInt8*)&src.address.addr6,
					BYTES_TO_BITS(sizeof(src.address.addr6)), true);
#endif
			}
		}

		/// @func Write 
		/// @brief write three bytes into stream
		/// @access  public  
		/// @param [in] [const UInt24 & inTemplateVar]  
		/// @return [void]  true succeed, false failed
		/// @remark
		/// @notice will align write-position to byte-boundary internally
		/// @see  AlignWritePosBits2ByteBoundary()
		/// @author mengdi[Jackie]
		template <>
		inline void Write(const UInt24 &inTemplateVar)
		{
			AlignWritePosBits2ByteBoundary();
			AppendBitsCouldRealloc(BYTES_TO_BITS(3));

			if (!IsBigEndian())
			{
				data[(mWritingPosBits >> 3) + 0] = ((UInt8 *)&inTemplateVar.val)[0];
				data[(mWritingPosBits >> 3) + 1] = ((UInt8 *)&inTemplateVar.val)[1];
				data[(mWritingPosBits >> 3) + 2] = ((UInt8 *)&inTemplateVar.val)[2];
			}
			else
			{
				data[(mWritingPosBits >> 3) + 0] = ((UInt8 *)&inTemplateVar.val)[3];
				data[(mWritingPosBits >> 3) + 1] = ((UInt8 *)&inTemplateVar.val)[2];
				data[(mWritingPosBits >> 3) + 2] = ((UInt8 *)&inTemplateVar.val)[1];
			}

			mWritingPosBits += BYTES_TO_BITS(3);
		}

		/// @func Write 
		/// @access  public  
		/// @param [in] [const JackieGUID & inTemplateVar]  
		/// @return void
		/// @author mengdi[Jackie]
		template <>
		inline void Write(const JackieGUID &inTemplateVar)
		{
			Write(inTemplateVar.g);
		}


		/// @func WriteChanged 
		/// @brief write any changed integral type to a bitstream.
		/// @access  public  
		/// @param [in] const templateType & latestVal 
		/// @param [in] const templateType & lastVal  
		/// @return void 
		/// @notice 
		/// If the current value is different from the last value
		/// the current value will be written.  Otherwise, a single bit will be written
		/// @author mengdi[Jackie]
		template <class templateType>
		inline void WriteChangedValue(const templateType &latestVal,
			const templateType &lastVal)
		{
			if (latestVal == lastVal)
			{
				Write(false);
			}
			else
			{
				Write(true);
				Write(latestVal);
			}
		}



		/// @func WriteChanged 
		/// @brief write a bool delta. Same thing as just calling Write
		/// @access  public  
		/// @param [in] const bool & currentValue  
		/// @param [in] const bool & lastValue  
		/// @return void 
		/// @author mengdi[Jackie]
		template <>
		inline void WriteChangedValue(const bool &currentValue,
			const bool &lastValue)
		{
			(void)lastValue;
			Write(currentValue);
		}

		/// @brief WriteDelta when you don't know what the last value is, or there is no last value.
		/// @param[in] currentValue The current value to write
		/// @func WriteChanged 
		/// @brief 
		/// writeDelta when you don't know what the last value is, or there is no last value.
		/// @access  public  
		/// @param [in] const templateType & currentValue  
		/// @return void  
		/// @author mengdi[Jackie]
		template <class templateType>
		inline void WriteChangedValue(const templateType &currentValue)
		{
			Write(true);
			Write(currentValue);
		}


		/// @func WriteMiniChanged 
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
		template <class templateType>
		inline void WriteMiniChanged(const templateType&currVal,
			const templateType &lastValue)
		{
			if (currVal == lastValue)
			{
				Write(false);
			}
			else
			{
				Write(true);
				WriteMini(currVal);
			}
		}

		/// @brief Write a bool delta.  Same thing as just calling Write
		/// @param[in] currentValue The current value to write
		/// @param[in] lastValue The last value to compare against
		template <>
		inline void WriteMiniChanged(const bool &currentValue, const bool&
			lastValue)
		{
			(void)lastValue;
			Write(currentValue);
		}

		/// @brief Same as WriteMiniChanged() 
		/// when we have an unknown second parameter
		template <class templateType>
		inline void WriteMiniChanged(const templateType &currentValue)
		{
			Write(true);
			WriteMini(currentValue);
		}


		/// @func WriteMini 
		/// @access  public  
		/// @param [in] const UInt8 * src  
		/// @param [in] const BitSize bits2Write  write size in bits
		/// @param [in] const bool isUnsigned  
		/// @return void 
		/// @notice 
		/// this function assumes that @src points to a native type,
		/// compress and write it.
		/// @Remarks
		/// assume we have src with value of FourOnes-FourOnes-FourOnes-11110001
		///++++++++++++++> High Memory Address (hma)
		///++++++++++++++++++++++++++++++
		/// | FourOnes | FourOnes | FourOnes | 11110001 |  Big Endian 
		///++++++++++++++++++++++++++++++
		///++++++++++++++++++++++++++++++
		/// |11110001 | FourOnes | FourOnes | FourOnes |  Little Endian 
		///++++++++++++++++++++++++++++++
		/// for little endian, the high bytes are located in hma and so @currByte should 
		/// increment from value of highest index ((bits2Write >> 3) - 1)
		/// for big endian, the high bytes are located in lma and so @currByte should 
		/// increment from value of lowest index (0)
		/// 在字节内部，一个字节的二进制排序，不存在大小端问题。
		/// 就和平常书写的一样，先写高位，即低地址存储高位。
		/// 如char a=0x12.存储从低位到高位就为0001 0010
		/// @author mengdi[Jackie]
		void WriteMini(const UInt8* src, const BitSize bits2Write,
			const bool isUnsigned);


		/// @func WriteMini 
		/// @brief Write any integral type to a bitstream.  
		/// @access  public  
		/// @param [in] const IntergralType & src  
		/// @return void 
		/// @notice
		/// For floating point, this is lossy, using 2 bytes for a float and 4 for 
		/// a double.  The range must be between -1 and +1.
		/// For non-floating point, this is lossless, but only has benefit 
		/// if you use less than half the bits of the type
		/// we write low bits and reassenble the value in receiver endpoint
		/// based on its endian, so no need to do endian swap here
		/// @author mengdi[Jackie]
		template <class IntergralType>
		inline void WriteMini(const IntergralType &src)
		{
			WriteMini((UInt8*)&src, sizeof(IntergralType) << 3, true);
			//if (sizeof(src) == 1)
			//{
			//	WriteMini((UInt8*)& src, sizeof(IntergralType) << 3, true);
			//	return;
			//}
			//#ifndef DO_NOT_SWAP_ENDIAN
			//			if (DoEndianSwap())
			//			{
			//				JINFO << "DoEndianSwap";
			//				UInt8 output[sizeof(IntergralType)];
			//				ReverseBytes((UInt8*)&src, output, sizeof(IntergralType));
			//				WriteMini(output, sizeof(IntergralType) << 3, true);
			//			}
			//			else
			//			{
			//				WriteMini((UInt8*)&src, sizeof(IntergralType) << 3, true);
			//			}
			//#else
			//			WriteMini((UInt8*)&src, sizeof(IntergralType) << 3, true);
			//#endif
		}

		template <> inline void WriteMini(const JackieAddress &src)
		{
			//Write(src);
			UInt8 version = src.GetIPVersion();
			WriteMini(version);

			if (version == 4)
			{
				/// Hide the address so routers don't modify it
				JackieAddress addr = src;
				UInt32 binaryAddress = ~src.address.addr4.sin_addr.s_addr;
				UInt16 p = addr.GetPortNetworkOrder();
				WriteMini(binaryAddress);
				WriteMini(p);
			}
			else
			{
#if NET_SUPPORT_IPV6 == 1
				UInt32 binaryAddress = src.address.addr6;
				WriteMini(binaryAddress);
#endif
			}
		}
		template <> inline void WriteMini(const JackieGUID &src)
		{
			WriteMini(src.g);
		}
		template <> inline void WriteMini(const UInt24 &var)
		{
			WriteMini(var.val);
			//Write(var);
		}
		template <> inline void WriteMini(const bool &src)
		{
			Write(src);
		}
		template <> ///@notice only For values between -1 and 1
		inline void WriteMini(const float &src)
		{
			DCHECK(src > -1.01f && src < 1.01f);
			float varCopy = src;
			if (varCopy < -1.0f) varCopy = -1.0f;
			if (varCopy > 1.0f) varCopy = 1.0f;
			WriteMini((UInt16)((varCopy + 1.0f)*32767.5f));
		}
		template <> ///@notice For values between -1 and 1
		inline void WriteMini(const double &src)
		{
			DCHECK(src > -1.01f && src < 1.01f);
			double varCopy = src;
			if (varCopy < -1.0f) varCopy = -1.0f;
			if (varCopy > 1.0f) varCopy = 1.0f;
			WriteMini((UInt32)((varCopy + 1.0)*2147483648.0));
		}

		/// @access public 
		/// @returns void
		template <class destType, class srcType >
		void WriteCasted(const srcType &value)
		{
			destType val = (destType)value;
			Write(val);
		}

		/// @method WriteBitsIntegerRange
		/// @access public 
		/// @returns void
		/// @param [in] const templateType value 
		/// value Integer value to write, which should be
		/// between @param mini and @param max
		/// @param [in] const templateType mini
		/// @param [in] const templateType max
		/// @param [in] bool allowOutsideRange
		/// If true, all sends will take an extra bit,
		/// however value can deviate from outside @a minimum and @a maximum.
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
		template <class IntegerType> void WriteIntegerRange(
			const IntegerType value,
			const IntegerType mini,
			const IntegerType max,
			bool allowOutsideRange = false)
		{
			static int requiredBits = BYTES_TO_BITS(sizeof(IntegerType)) -
				GetLeadingZeroSize(IntegerType(max - mini));
			WriteIntegerRange(value, mini, max, requiredBits, allowOutsideRange);
		}

		/// @Brief
		/// Assume@valueBeyondMini's value is 0x000012
		///------------------> Memory Address
		///+++++++++++
		///| 00 | 00 | 00 | 12 |  Big Endian
		///+++++++++++
		///+++++++++++
		///| 12 | 00 | 00 | 00 |  Little Endian 
		///+++++++++++
		/// so for big endian, we need to reverse byte so that
		/// the high byte of 0x00 that was put in low address can be written correctly
		/// for little endian, we do nothing.
		template <class IntegerType>
		void WriteIntegerRange(
			const IntegerType value,
			const IntegerType minimum,
			const IntegerType maximum,
			const int requiredBits,
			bool allowOutsideRange = false)
		{
			DCHECK(max >= mini);
			DCHECK(allowOutsideRange == true || (value >= minimum && value <= maximum));

			if (allowOutsideRange)
			{
				if (value <mini || value>max)  ///< out of range
				{
					Write(true);
					Write(value);
					return;
				}
				Write(false); ///< inside range
			}

			IntegerType valueBeyondMini = value - mini;
			if (IsBigEndian())
			{
				UInt8 output[sizeof(IntegerType)];
				ReverseBytes((UInt8*)&valueBeyondMini, output, sizeof(IntegerType));
				WriteBits(output, requiredBits);
			}
			else
			{
				WriteBits((UInt8*)&valueBeyondMini, requiredBits);
			}
		}


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
		template <class templateType> void WriteNormVector(
			templateType x,
			templateType y,
			templateType z)
		{
			DCHECK(x <= 1.01 &&y <= 1.01 &&z <= 1.01 &&x >= -1.01 &&y >= -1.01 &&z >= -1.01);
			WriteFloatRange((float)x, -1.0f, 1.0f);
			WriteFloatRange((float)y, -1.0f, 1.0f);
			WriteFloatRange((float)z, -1.0f, 1.0f);
		}

		/// @method WriteVector
		/// @access public 
		/// @returns void
		/// @brief Write a vector, using 10 bytes instead of 12.
		/// @notice
		/// Loses accuracy to about 3/10ths and only saves 2 bytes, 
		/// so only use if accuracy is not important
		/// templateType for this function must be a float or double
		/// @see
		template <class templateType> void WriteVector(
			templateType x,
			templateType y,
			templateType z)
		{
			templateType magnitude = sqrt(x * x + y * y + z * z);
			Write((float)magnitude);
			if (magnitude > 0.00001f)
			{
				WriteMini((float)(x / magnitude));
				WriteMini((float)(y / magnitude));
				WriteMini((float)(z / magnitude));
			}
		}

		/// @method WriteNormQuat
		/// @access public 
		/// @returns void
		/// @brief 
		/// Write a normalized quaternion in (18 bits[best case] to 6 bytes[worest case]) + 4 bits instead of 16 bytes.  
		/// Slightly lossy.
		/// @notice
		/// templateType for this function must be a float or double
		/// @see
		template <class templateType> void WriteNormQuat(
			templateType w,
			templateType x,
			templateType y,
			templateType z)
		{
			Write((bool)(w < 0.0));
			Write((bool)(x < 0.0));
			Write((bool)(y < 0.0));
			Write((bool)(z < 0.0));
			WriteMini((UInt16)(fabs(x)*65535.0));
			WriteMini((UInt16)(fabs(y)*65535.0));
			WriteMini((UInt16)(fabs(z)*65535.0));
			// Leave out w and calculate it on the target
		}

		/// @method WriteOrthMatrix
		/// @access public 
		/// @returns void
		/// @brief
		/// Write an orthogonal matrix by creating a quaternion, 
		/// and writing 3 components of the quaternion in 2 bytes each.
		/// @notice
		/// Lossy, although the result is renormalized
		/// Use (18 bits to 6 bytes) +4 bits instead of 36
		/// templateType for this function must be a float or double
		/// @see WriteNormQuat()
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
			// http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixQuaternion/
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

		/// @brief  Write zeros until the bitstream is filled up to @param bytes
		/// @notice will internally align write pos and then reallocate if necessary
		///  the @mWritePosBits will be byte aligned
		void PadZeroAfterAlignedWRPos(UInt32 bytes);

		/// @brief swao bytes starting from @data with offset given
		inline void EndianSwapBytes(UInt32 byteOffset, UInt32 length)
		{
			if (DoEndianSwap()) ReverseBytes(data + byteOffset, length);
		}

		/// @brief Makes a copy of the internal data for you @param _data 
		/// will point to the stream. Partial bytes are left aligned
		/// @param[out] _data The allocated copy of GetData()
		/// @return The length in bits of the stream.
		/// @notice
		/// all bytes are copied besides the bytes in GetPayLoadBits()
		BitSize Copy(UInt8** _data) const
		{
			DCHECK(mWritingPosBits > 0);
			*_data = (UInt8*)jackieMalloc_Ex(BITS_TO_BYTES(mWritingPosBits),
				TRACE_FILE_AND_LINE_);
			memcpy(*_data, data, sizeof(UInt8) * BITS_TO_BYTES(mWritingPosBits));
			return mWritingPosBits;
		}

		///@brief Ignore data we don't intend to read
		void ReadSkipBits(const BitSize numberOfBits)
		{
			mReadingPosBits += numberOfBits;
		}
		void ReadSkipBytes(const ByteSize numberOfBytes)
		{
			ReadSkipBits(BYTES_TO_BITS(numberOfBytes));
		}

		void WriteOneAlignedBytes(const char *inByteArray)
		{
			DCHECK((mWritingPosBits & 7) == 0);
			AppendBitsCouldRealloc(8);
			data[mWritingPosBits >> 3] = inByteArray[0];
			mWritingPosBits += 8;
		}
		void ReadOneAlignedBytes(char *inOutByteArray)
		{
			DCHECK((mReadingPosBits & 7) == 0);
			DCHECK(GetPayLoadBits() >= 8);
			// if (mReadPosBits + 1 * 8 > mWritePosBits) return;

			inOutByteArray[0] = data[(mReadingPosBits >> 3)];
			mReadingPosBits += 8;
		}

		void WriteTwoAlignedBytes(const char *inByteArray)
		{
			DCHECK((mWritingPosBits & 7) == 0);
			AppendBitsCouldRealloc(16);
#ifndef DO_NOT_SWAP_ENDIAN
			if (DoEndianSwap())
			{
				data[(mWritingPosBits >> 3) + 0] = inByteArray[1];
				data[(mWritingPosBits >> 3) + 1] = inByteArray[0];
			}
			else
#endif
			{
				data[(mWritingPosBits >> 3) + 0] = inByteArray[0];
				data[(mWritingPosBits >> 3) + 1] = inByteArray[1];
			}

			mWritingPosBits += 16;
		}
		void ReadTwoAlignedBytes(char *inOutByteArray)
		{
			DCHECK((mReadingPosBits & 7) == 0);
			DCHECK(GetPayLoadBits() >= 16);
			//if (mReadPosBits + 16 > mWritePosBits) return ;
#ifndef DO_NOT_SWAP_ENDIAN
			if (DoEndianSwap())
			{
				inOutByteArray[0] = data[(mReadingPosBits >> 3) + 1];
				inOutByteArray[1] = data[(mReadingPosBits >> 3) + 0];
			}
			else
#endif
			{
				inOutByteArray[0] = data[(mReadingPosBits >> 3) + 0];
				inOutByteArray[1] = data[(mReadingPosBits >> 3) + 1];
			}

			mReadingPosBits += 16;
		}

		void WriteFourAlignedBytes(const char *inByteArray)
		{
			DCHECK((mWritingPosBits & 7) == 0);
			AppendBitsCouldRealloc(32);
#ifndef DO_NOT_SWAP_ENDIAN
			if (DoEndianSwap())
			{
				data[(mWritingPosBits >> 3) + 0] = inByteArray[3];
				data[(mWritingPosBits >> 3) + 1] = inByteArray[2];
				data[(mWritingPosBits >> 3) + 2] = inByteArray[1];
				data[(mWritingPosBits >> 3) + 3] = inByteArray[0];
			}
			else
#endif
			{
				data[(mWritingPosBits >> 3) + 0] = inByteArray[0];
				data[(mWritingPosBits >> 3) + 1] = inByteArray[1];
				data[(mWritingPosBits >> 3) + 2] = inByteArray[2];
				data[(mWritingPosBits >> 3) + 3] = inByteArray[3];
			}

			mWritingPosBits += 32;
		}
		void ReadFourAlignedBytes(char *inOutByteArray)
		{
			DCHECK((mReadingPosBits & 7) == 0);
			DCHECK(GetPayLoadBits() >= 32);
			//if (mReadPosBits + 4 * 8 > mWritePosBits) return;
#ifndef DO_NOT_SWAP_ENDIAN
			if (DoEndianSwap())
			{
				inOutByteArray[0] = data[(mReadingPosBits >> 3) + 3];
				inOutByteArray[1] = data[(mReadingPosBits >> 3) + 2];
				inOutByteArray[2] = data[(mReadingPosBits >> 3) + 1];
				inOutByteArray[3] = data[(mReadingPosBits >> 3) + 0];
			}
			else
#endif
			{
				inOutByteArray[0] = data[(mReadingPosBits >> 3) + 0];
				inOutByteArray[1] = data[(mReadingPosBits >> 3) + 1];
				inOutByteArray[2] = data[(mReadingPosBits >> 3) + 2];
				inOutByteArray[3] = data[(mReadingPosBits >> 3) + 3];
			}

			mReadingPosBits += 32;
		}

		///@brief text-print bits starting from @data to @mWritingPosBits
		void PrintBit(void);
		void PrintHex(void);

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
			static bool isNetworkOrder = *((char*)& a) != 1;
			return isNetworkOrder;
		}
		inline static bool IsBigEndian(void) { return IsNetworkOrder(); }

		/// @Brief faster than ReverseBytes() if you want to reverse byte
		/// for a variable teself internnaly like uint64 will loop 12 times 
		/// compared to 8 times using ReverseBytes()
		inline static void ReverseBytes(UInt8 *src, const UInt32 length)
		{
			UInt8 temp;
			for (UInt32 i = 0; i < (length >> 1); i++)
			{
				temp = src[i];
				src[i] = src[length - i - 1];
				src[length - i - 1] = temp;
			}
		}
		inline static void ReverseBytes(UInt8 *src, UInt8 *dest, const UInt32 length)
		{
			for (UInt32 i = 0; i < length; i++)
			{
				dest[i] = src[length - i - 1];
			}
		}

		/// Can only print 4096 size of UInt8 no materr is is bit or byte
		/// mainly used for dump binary data
		static void PrintBit(char* outstr, BitSize bitsPrint, UInt8* src);
		static void PrintHex(char* outstr, BitSize bitsPrint, UInt8* src);
	};

}
#endif  //__BITSTREAM_H__
