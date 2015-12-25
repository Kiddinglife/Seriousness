/// \file
/// \brief \b [Internal] Random number generator
///

#ifndef RandomSeedCreator_H_
#define RandomSeedCreator_H_

#include "DLLExport.h"

namespace Ultils
{
	/// Initialise seed for Random Generator
	/// \note not threadSafe, use an instance of RandomSeedCreator if necessary per thread
	/// \param[in] seed The seed value for the random number generator.
	extern void JACKIE_EXPORT seedMT(unsigned int seed);

	/// \internal
	/// \note not threadSafe, use an instance of RandomSeedCreator if necessary per thread
	extern unsigned int JACKIE_EXPORT reloadMT(void);

	/// Gets a random unsigned int
	/// \note not threadSafe, use an instance of RandomSeedCreator if necessary per thread
	/// \return an integer random value.
	extern unsigned int JACKIE_EXPORT randomMT(void);

	/// Gets a random float
	/// \note not threadSafe, use an instance of RandomSeedCreator if necessary per thread
	/// \return 0 to 1.0f, inclusive
	extern float JACKIE_EXPORT frandomMT(void);

	/// Randomizes a buffer
	/// \note not threadSafe, use an instance of RandomSeedCreator if necessary per thread
	extern void JACKIE_EXPORT fillBufferMT(void *buffer, unsigned int bytes);

	class RandomSeedCreator
	{
		protected:
		unsigned int state[624 + 1];
		unsigned int *next;
		int left;

		public:
		RandomSeedCreator();
		virtual ~RandomSeedCreator();
		void SeedMT(unsigned int seed);
		unsigned int ReloadMT(void);
		unsigned int RandomMT(void);
		float FrandomMT(void);
		void FillBufferMT(void *buffer, unsigned int bytes);
	};
}
#endif

