/**
 * Copyright (c) 2016-present, Facebook, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// This script converts an image dataset to a database.


#include <opencv2/opencv.hpp>

#include <algorithm>
#include <fstream>
#include <queue>
#include <random>
#include <string>
#include <thread>

#include "caffe2/core/common.h"
#include "caffe2/core/db.h"
#include "caffe2/core/init.h"
#include "caffe2/proto/caffe2_pb.h"
#include "caffe2/core/logging.h"

#include <compsky/mysql/query>


C10_DEFINE_bool(
    shuffle,
    false,
    "Randomly shuffle the order of images and their labels");
C10_DEFINE_string(output_db_name, "", "The output training leveldb name.");
C10_DEFINE_string(db, "leveldb", "The db type.");
C10_DEFINE_bool(
    raw,
    false,
    "If set, we pre-read the images and store the raw buffer.");
C10_DEFINE_bool(color, true, "If set, load images in color.");
C10_DEFINE_int(
    scale,
    256,
    "If FLAGS_raw is set, scale the shorter edge to the given value.");
C10_DEFINE_bool(warp, false, "If warp is set, warp the images to square.");
C10_DEFINE_int(
    num_threads,
    -1,
    "Number of image parsing and conversion threads.");


MYSQL_RES* RES;

struct Jomsborg {
    const char* fp;
    const uint64_t tag_id;
    const double x;
    const double y;
    const double w;
    const double h;
};


namespace caffe2 {

class Converter {
 public:
  explicit Converter() {
    data_ = protos_.add_protos();
    label_ = protos_.add_protos();
    if (FLAGS_raw) {
      data_->set_data_type(TensorProto::BYTE);
      data_->add_dims(0);
      data_->add_dims(0);
      if (FLAGS_color) {
        data_->add_dims(3);
      }
    } else {
      data_->set_data_type(TensorProto::STRING);
      data_->add_dims(1);
      data_->add_string_data("");
    }
    label_->set_data_type(TensorProto::INT32);
    label_->add_dims(1);
    label_->add_int32_data(0);
  }

  ~Converter() {
    if (thread_.joinable()) {
      thread_.join();
    }
  }

  void queue(const Jomsborg& jom) {
    in_.push(jom);
  }

  void start() {
    thread_ = std::thread(&Converter::run, this);
  }

  std::string get() {
    std::unique_lock<std::mutex> lock(mutex_);
    while (out_.empty()) {
      cv_.wait(lock);
    }

    auto value = out_.front();
    out_.pop();
    cv_.notify_one();
    return value;
  }

  void run() {
    std::unique_lock<std::mutex> lock(mutex_);
    std::string value;
    while (!in_.empty()) {
      Jomsborg jom = in_.front();
      in_.pop();
      lock.unlock();

      label_->set_int32_data(0, jom.tag_id);

      cv::Mat orig_img = cv::imread(jom.fp,  FLAGS_color ? cv::IMREAD_COLOR : cv::IMREAD_GRAYSCALE);
      cv::Rect rect(jom.x, jom.y, jom.w, jom.h);
      cv::Mat img = orig_img(rect);

      // Add raw file contents to DB if !raw
      if (!FLAGS_raw) {
        data_->set_dims(0, img.rows);
        data_->set_dims(1, img.cols);
      } else {
        // Resize image
        orig_img = img;
        int scaled_width, scaled_height;
        if (FLAGS_warp) {
          scaled_width = FLAGS_scale;
          scaled_height = FLAGS_scale;
        } else if (orig_img.rows > orig_img.cols) {
          scaled_width = FLAGS_scale;
          scaled_height = static_cast<float>(orig_img.rows) * FLAGS_scale / orig_img.cols;
        } else {
          scaled_height = FLAGS_scale;
          scaled_width = static_cast<float>(orig_img.cols) * FLAGS_scale / orig_img.rows;
        }
        cv::resize(
            orig_img,
            img,
            cv::Size(scaled_width, scaled_height),
            0,
            0,
            cv::INTER_LINEAR);
        data_->set_dims(0, scaled_height);
        data_->set_dims(1, scaled_width);
      }

      // Assert we don't have to deal with alignment
      DCHECK(img.isContinuous());
      auto nbytes = img.total() * img.elemSize();
      data_->set_byte_data(img.ptr(), nbytes);

      protos_.SerializeToString(&value);

      // Add serialized proto to out queue or wait if it is not empty
      lock.lock();
      while (!out_.empty()) {
        cv_.wait(lock);
      }
      out_.push(value);
      cv_.notify_one();
    }
  }

 protected:
  TensorProtos protos_;
  TensorProto* data_;
  TensorProto* label_;
  std::queue<Jomsborg> in_;
  std::queue<std::string> out_;

  std::mutex mutex_;
  std::condition_variable cv_;
  std::thread thread_;
};

void ConvertImageDataset(
    const string& output_db_name,
    const bool /*shuffle*/) {
  std::vector<Jomsborg> lines;
  std::string filename;
  int file_label;
  
  {
  char* fp;
  uint64_t tag_id;
  double x, y, w, h;
  auto f = compsky::asciify::flag::guarantee::between_zero_and_one_inclusive;
  MYSQL_ROW row;
  while(compsky::mysql::assign_next_result(RES, &ROW, &fp, &tag_id, f, &x, f, &y, f, &w, f, &h){
    lines.emplace_back(fp, tag_id, x, y, w, h);
  }
  }

  if (FLAGS_shuffle) {
    LOG(INFO) << "Shuffling data";
    std::shuffle(lines.begin(), lines.end(), std::default_random_engine(1701));
  }

  auto num_threads = FLAGS_num_threads;
  if (num_threads < 1) {
    num_threads = std::thread::hardware_concurrency();
  }

  LOG(INFO) << "Processing " << lines.size() << " images...";
  LOG(INFO) << "Opening DB " << output_db_name;

  auto db = db::CreateDB(FLAGS_db, output_db_name, db::NEW);
  auto transaction = db->NewTransaction();

  LOG(INFO) << "Using " << num_threads << " processing threads...";
  std::vector<Converter> converters(num_threads);

  // Queue entries across converters
  for (auto i = 0; i < lines.size(); i++) {
    converters[i % converters.size()].queue(lines[i]);
  }

  // Start all converters
  for (auto& converter : converters) {
    converter.start();
  }

  constexpr auto key_max_length = 256;
  char key_cstr[key_max_length];
  string value;
  int count = 0;
  for (auto i = 0; i < lines.size(); i++) {
    // Get serialized proto for this entry
    auto value = converters[i % converters.size()].get();

    // Synthesize key for this entry
    auto key_len = snprintf(key_cstr, sizeof(key_cstr), "%08d_%s", i, lines[i].fp);
    DCHECK_LE(key_len, sizeof(key_cstr));

    // Put in db
    transaction->Put(string(key_cstr), value);

    if (++count % 1000 == 0) {
      // Commit the current writes.
      transaction->Commit();
      LOG(INFO) << "Processed " << count << " files.";
    }
  }

  // Commit final transaction
  transaction->Commit();
  LOG(INFO) << "Processed " << count << " files.";
}

}  // namespace caffe2


int main(int argc, char** argv) {
  // USAGE: ./make_image_db TAG1 TAG2 ... TAGN - [original caffe args]
  char buf[4096];
  char* itr = buf;
  
  int myargs_n = 0;
  
  MYSQL* _mysql_conn;
  constexpr static const size_t _mysql_buf_sz = 512;
  char _mysql_buf[_mysql_buf_sz];
  char* _mysql_auth[6];
  compsky::mysql::init_auth(_mysql_buf, _mysql_buf_sz, _mysql_auth, getenv("TAGEM_MYSQL_CFG"));
  compsky::mysql::login_from_auth(_mysql_conn, _mysql_auth);
  
	compsky::asciify::asciify(
		itr,
		"SELECT "
			"name,"
			"tag,"
			"x,"
			"y,"
			"w,"
			"h "
		"FROM _file "
		"JOIN("
			"SELECT "
				"file,"
				"tag,"
				"x,"
				"y,"
				"w,"
				"h "
			"FROM instance "
			"JOIN("
				"SELECT "
					"instance,"
					"tag "
				"FROM instance2tag "
				"WHERE tag IN("
	);

  while (true){
    const char* arg = argv[++myargs_n];
    if (arg[0] == '-')
        break;
    compsky::asciify::asciify(itr, '"', arg, '"', ',');
  }
  --itr; // Remove trailing comma

	compsky::asciify::asciify(
		itr,
		")) A ON A.instance = id) B ON B.file=id"
	);

  compsky::mysql::query_buffer(buf,  (uintptr_t)itr - (uintptr_t)buf);

  argc += myargs_n;

  caffe2::GlobalInit(&argc, &argv);
  caffe2::ConvertImageDataset(FLAGS_output_db_name, FLAGS_shuffle);
  
  compsky::mysql::wipe_auth(_mysql_buf, _mysql_buf_sz);
  
  return 0;
}
