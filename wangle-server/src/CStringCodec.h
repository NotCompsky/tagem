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

#include <wangle/channel/Handler.h>
#include <string_view>


namespace wangle {

/*
 * CStringCodec converts a pipeline from IOBufs to const std::string_view*.
 */
class CStringCodec : public Handler<std::unique_ptr<folly::IOBuf>, const char*,
                                   const std::string_view, std::unique_ptr<folly::IOBuf>> {
 public:
  typedef typename Handler<
   std::unique_ptr<folly::IOBuf>, const char*,
   const std::string_view, std::unique_ptr<folly::IOBuf>>::Context Context;

  void read(Context* ctx, std::unique_ptr<folly::IOBuf> buf) override {
    if (buf) {
      buf->coalesce();
      ctx->fireRead((const char*)buf->data());
    }
  }

  folly::Future<folly::Unit> write(Context* ctx,  const std::string_view msg) override {
		constexpr size_t min_chunk_sz = 100;
		size_t chunk_sz = 10 * 1024;
		/*
		 * The chunk size needs to become smaller for larger files.
		 * Sending files in one go seems to be fine for files below a megabyte.
		 * But for sending a 14MiB file, even a (constant) chunk_sz as low as 1000 regularly fails to deliver the file without truncation.
		 * I have no idea why this is the case.
		 * TODO: Fix this
		 */
		size_t sz = msg.size();
		auto data = msg.data();
		while(sz > chunk_sz){
			auto buf = folly::IOBuf::copyBuffer(data, chunk_sz);
			data += chunk_sz;
			sz -= chunk_sz;
			chunk_sz /= 2;
			if (chunk_sz < min_chunk_sz)
				chunk_sz = min_chunk_sz;
			ctx->fireWrite(std::move(buf));
		}
		auto buf = folly::IOBuf::copyBuffer(data, sz);
		return ctx->fireWrite(std::move(buf));
  }
};

} // namespace wangle
