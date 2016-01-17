/*
 * Copyright 2015 Facebook, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FOLLY_DETAIL_RANGE_SSE42_H_
#define FOLLY_DETAIL_RANGE_SSE42_H_

#include <cstddef>
#include <folly/detail/RangeCommon.h>

namespace folly {

namespace detail {

size_t qfind_first_byte_of_sse42(const StringPieceLite haystack,
                                 const StringPieceLite needles);

}

}

#endif
