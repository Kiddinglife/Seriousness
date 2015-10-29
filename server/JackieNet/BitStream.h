#pragma once
namespace JACKIE_INET
{
	class BitStream
	{
		public:
		BitStream();
		/// \brief Create the bitstream, with some number of bytes to immediately allocate.
		/// \details There is no benefit to calling this, unless you know exactly how many bytes you need and it is greater than BITSTREAM_STACK_ALLOCATION_SIZE.
		/// In that case all it does is save you one or more realloc calls.
		/// \param[in] initialBytesToAllocate the number of bytes to pre-allocate.
		BitStream(const unsigned int initialBytesToAllocate);
		virtual ~BitStream();
	};
}
