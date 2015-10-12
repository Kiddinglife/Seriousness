/*
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#ifndef NET_TIME_H
#define NET_TIME_H

#include "BasicTypes.h"
#include "DefaultNetDefines.h"

namespace JACKIE_INET
{
	// Define USE_TIME_MS_64BITS if you want to use large types for GetTime
	/// (takes more bandwidth when you transmit time though!)
	// You would want to do this if your system is going to run long enough 
	/// to overflow the millisecond counter (over a month)
#if USE_TIME_MS_64BITS==1
	typedef UInt64 Time;
	typedef UInt32 TimeMS;
	typedef UInt64 TimeUS;
#else
	typedef UInt32 Time;
	typedef UInt32 TimeMS;
	typedef UInt64 TimeUS;
#endif

} // namespace RakNet

#endif
