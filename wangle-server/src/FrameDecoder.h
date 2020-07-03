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

#pragma once

#include <wangle/codec/ByteToMessageDecoder.h>

namespace wangle {

/**
 * A decoder that passes the entire received IOBufQueue.
 */
class FrameDecoder : public InboundHandler<folly::IOBufQueue&, std::unique_ptr<folly::IOBuf>> {
 public:
	explicit FrameDecoder();
	
	void transportActive(Context* ctx) override {
		transportActive_ = true;
		ctx->fireTransportActive();
	}

	void transportInactive(Context* ctx) override {
		transportActive_ = false;
		ctx->fireTransportInactive();
	}
	
	void read(Context* ctx, folly::IOBufQueue& q) override;
  
 private:
	bool transportActive_ = true;
	std::unique_ptr<folly::IOBuf> result;
};

} // namespace wangle
