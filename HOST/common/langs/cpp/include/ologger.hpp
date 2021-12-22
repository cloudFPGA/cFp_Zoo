/*******************************************************************************
 * Copyright 2016 -- 2022 IBM Corporation
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
*******************************************************************************/

/*
 * Copyright 2020, arcomber, https://codereview.stackexchange.com/questions/254134/cout-style-logger-in-c
 */

/*
ologger or ostream logger, a logger as convenient as using std::cout
Features:
1. logging as per cout/cerr/clog ie logger.error << "i=" << 3 << std::endl;
2. logging obeys log level set. If you set log level error, then logging to info or debug is a no-op.
3. log rotation - specify a maximum file size in bytes and maximum number of files
4. Configure via path, file name prefix and file name extension.

End a line with std::endl

Example usage:
    ologger logger(".", "projectz", "log", 10000, 5, ologger::log_level::LOG_INFO);
   // nothing should be logged because we have set logger at ERROR level only
   logger.debug << "low level developer data, X level exceeded, x=" << 9.75 << std::endl;
   // nothing should be logged because we have set logger at ERROR level only
   logger.info << "informational stuff about sending x protocol data to server, bytes transferred: " << 1250 << std::endl;
   // this should be logged
   logger.error << "!!! Invalid Data Received !!!" << std::endl;
*/

#ifndef OLOGGER_HPP_
#define OLOGGER_HPP_

#include <string>
#include <iostream>
#include <fstream>

class ologger {
public:
   using endl_type = std::ostream& (std::ostream&);

   enum class log_level {
      LOG_NONE,
      LOG_ERROR,
      LOG_INFO,
      LOG_DEBUG
   };

   /* Construct with log files path, file name prefix and suffix, max_file_size in bytes, max_files before rotation and logging level */
   ologger(const std::string& path,
      const std::string& file_prefix,
      const std::string& file_suffix,
      size_t max_file_size,
      size_t max_files,
      log_level level = log_level::LOG_NONE);

   // prevent copying object
   ologger(const ologger&) = delete;
   ologger(ologger&&) = delete;
   ologger& operator=(const ologger&) = delete;
   ologger& operator=(ologger&&) = delete;

   // debug level logging
   class Debug {
   public:
      Debug(ologger& parent);

      template<typename T>
      Debug& operator<< (const T& data)
      {
         if (parent_.level_ >= log_level::LOG_DEBUG) {
            if (start_of_line_) {
               parent_.prefix_message();
                    start_of_line_ = false;
                }

            parent_.log_stream_ << data;
         }

         return *this;
      }

      Debug& operator<<(endl_type endl);

   private:
      ologger& parent_;
      bool start_of_line_;
   };

   // info level logging
   class Info {
   public:
      Info(ologger& parent);

      template<typename T>
      Info& operator<< (const T& data)
      {
         if (parent_.level_ >= log_level::LOG_INFO) {
            if (start_of_line_) {
               parent_.prefix_message();
               start_of_line_ = false;
            }
            parent_.log_stream_ << data;
         }
         return *this;
      }

      Info& operator<<(endl_type endl);

   private:
      ologger& parent_;
      bool start_of_line_;
   };

   // error level logging
   class Error {
   public:
      Error(ologger& parent);

      template<typename T>
      Error& operator<< (const T& data)
      {
         if (parent_.level_ >= log_level::LOG_ERROR) {
            if (start_of_line_) {
               parent_.prefix_message();
               start_of_line_ = false;
            }
            parent_.log_stream_ << data;
         }
         return *this;
      }

      Error& operator<<(endl_type endl);

   private:
      ologger& parent_;
      bool start_of_line_;
   };

private:
   size_t changeover_if_required();
   const std::string to_string(ologger::log_level level);
   void prefix_message();
   void make_logger(const std::string& path, const std::string& file_prefix, const std::string& file_suffix);

   const std::string path_;
   const std::string file_prefix_;
   const std::string file_suffix_;
   size_t max_file_size_;
   size_t max_files_;
   log_level level_;

   std::fstream log_stream_;

public:
   Debug debug;
   Info info;
   Error error;
};

#endif // OLOGGER_HPP_
