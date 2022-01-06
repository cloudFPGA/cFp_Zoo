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

#ifdef _WIN32 
#define _CRT_SECURE_NO_WARNINGS
#define SEPARATOR ('\\')
#else
#define SEPARATOR ('/')
#endif

#include "ologger.hpp"

#include <cstring>
#include <ctime>
#include <chrono>
#include <stdexcept>  // domain_error for bad path

static size_t get_next_file_suffix(const std::string& path) {
   size_t next { 0 };
   std::fstream fstrm(path + SEPARATOR + "next", std::ofstream::in | std::ofstream::out);
   if (fstrm) {
      fstrm >> next;
   }
   return next;
}

// convert blank string to . and remove trailing path separator
static const std::string make_path(const std::string& original_path) {
   std::string massaged_path{ original_path };
   if (massaged_path.empty()) {
      massaged_path = ".";
   }

   if (massaged_path[massaged_path.size() - 1] == SEPARATOR) {
      massaged_path = massaged_path.substr(0, massaged_path.size() - 1);
   }
   return massaged_path;
}

const std::string ologger::to_string(ologger::log_level level) {
   switch (level) {
   case ologger::log_level::LOG_NONE: return "";
   case ologger::log_level::LOG_ERROR: return "error";
   case ologger::log_level::LOG_INFO: return "info";
   case ologger::log_level::LOG_DEBUG: return "debug";
   default:
      return "";
   }
}

void ologger::make_logger(const std::string& path, const std::string& file_prefix, const std::string& file_suffix) {

   size_t next_id = get_next_file_suffix(path);
   log_stream_.open(path + SEPARATOR + file_prefix + std::to_string(next_id) + "." + file_suffix, std::ofstream::out | std::ofstream::app);

   if (!log_stream_.good()) {
      throw std::domain_error("ologger error: unable to open log file: " + path + SEPARATOR + file_prefix + std::to_string(next_id) + "." + file_suffix);
   }
}

ologger::ologger(const std::string& path,
   const std::string& file_prefix,
   const std::string& file_suffix,
   size_t max_file_size,
   size_t max_files,
   log_level level)
   :
   path_(make_path(path)),
   file_prefix_(file_prefix),
   file_suffix_(file_suffix),
   max_file_size_(max_file_size),
   max_files_(max_files),
   level_(level),
   debug(*this), info(*this), error(*this) {

   make_logger(path_, file_prefix_, file_suffix);
}

size_t ologger::changeover_if_required() {
   size_t next_id{0};

   if (log_stream_) {
      const std::streampos pos = log_stream_.tellp();

      if (static_cast<size_t>(pos) > max_file_size_) {
         next_id = get_next_file_suffix(path_);
         next_id = (next_id + 1) % max_files_;

         std::fstream fstrm(path_ + SEPARATOR + "next", std::ofstream::out | std::ofstream::trunc);
         if (fstrm) {
            fstrm << next_id;
         }

         log_stream_.close();

         log_stream_.clear();  

         // if next file exists, delete so we start with empty file
         const std::string next_file{ path_ + SEPARATOR + file_prefix_ + std::to_string(next_id) + "." + file_suffix_ };

         std::remove(next_file.c_str());

         log_stream_.open(next_file, std::ofstream::out | std::ofstream::app);
      }
   }

   return next_id;
}


std::string get_time_stamp()
{
   const auto now = std::chrono::system_clock::now();
   const std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);

   char timestamp[50]{};
   std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d,%H:%M:%S", std::localtime(&now_time_t));

   const int millis = std::chrono::time_point_cast<std::chrono::milliseconds>(now).time_since_epoch().count() % 100;
    snprintf(timestamp + strlen(timestamp), sizeof(timestamp) - strlen(timestamp), ".%03d,", millis);
   return timestamp;
}

void ologger::prefix_message() {
   log_stream_ << get_time_stamp() << to_string(level_) << ',';
}

//inner logging level classes for ostream overloading
ologger::ologger::Debug::Debug(ologger& parent)
  : parent_(parent), start_of_line_(true)
{}

ologger::Debug& ologger::Debug::operator<<(endl_type endl)
{
   if (parent_.level_ >= log_level::LOG_INFO) {
      parent_.log_stream_ << endl;
   }

   parent_.changeover_if_required();
   start_of_line_ = true;
   return *this;
}

ologger::ologger::Info::Info(ologger& parent)
  : parent_(parent), start_of_line_(true)
{}

ologger::Info& ologger::Info::operator<<(endl_type endl)
{
   if (parent_.level_ >= log_level::LOG_INFO) {
      parent_.log_stream_ << endl;
   }

   parent_.changeover_if_required();
   start_of_line_ = true;
   return *this;
}

ologger::ologger::Error::Error(ologger& parent)
  : parent_(parent), start_of_line_(true)
{}

ologger::Error& ologger::Error::operator<<(endl_type endl)
{
   if (parent_.level_ >= log_level::LOG_ERROR) {
      parent_.log_stream_ << endl;
   }

   parent_.changeover_if_required();
   start_of_line_ = true;
   return *this;
}
