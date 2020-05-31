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
   const std::string_view*, std::unique_ptr<folly::IOBuf>>::Context Context;

  void read(Context* ctx, std::unique_ptr<folly::IOBuf> buf) override {
    if (buf) {
      buf->coalesce();
      ctx->fireRead((const char*)buf->data());
    }
  }

  folly::Future<folly::Unit> write(Context* ctx,  const std::string_view msg) override {
    auto buf = folly::IOBuf::copyBuffer(msg.data(), msg.size());
    return ctx->fireWrite(std::move(buf));
  }
};

} // namespace wangle
