/*
 * Copyright 2017-present Facebook, Inc.
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

/*
 * WARNING
 * 
 * This file has been modified from its original state. (4.b)
 * 
 * WARNING
 */

#include "FrameDecoder.h"

namespace wangle {

using folly::IOBuf;
using folly::IOBufQueue;

FrameDecoder::FrameDecoder(){}

bool FrameDecoder::decode(Context* ctx,
                                   IOBufQueue& buf,
                                   std::unique_ptr<IOBuf>& result,
                                   size_t&) {
      std::unique_ptr<folly::IOBuf> frame = buf.splitAtMost(1024 * 1024); // Arbitrary limit
      result = std::move(frame);
      return true;
}

} // namespace wangle
