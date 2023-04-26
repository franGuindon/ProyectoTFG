/* Copyright 2023 Francis Guindon <fbadilla10@gmail.com> */

#ifndef ARTIFACT_DETECTOR_INCLUDE_UTILITY_HPP_
#define ARTIFACT_DETECTOR_INCLUDE_UTILITY_HPP_

/** Include logging and error handling macros */
#include <stdio.h>

#include <chrono>  // NOLINT
#include <fstream>
#include <memory>
#include <string>

#include "include/macros.hpp"

constexpr uint32_t kMicrosPerSecond = 1000000;
using MicroSeconds = std::chrono::microseconds;

/**
 * @brief Get the current system clock time since epoch in microseconds
 *
 * @return uint64_t : Current system clock time since epoch
 */
uint64_t GetCurrentTimeSinceEpochUs();

/**
 * @brief
 *
 * @tparam dtype
 * @param filename
 * @param data
 * @param size
 * @return true
 * @return false
 */
template <typename dtype>
bool load_frame(const std::string &filename, dtype *data, const size_t size) {
  std::streampos file_size;
  std::ifstream file(filename, std::ios::in | std::ios::binary | std::ios::ate);

  if (!data) {
    printf("Load frame error: Data pointer is null\n");
    return false;
  }

  if (!file.is_open()) {
    printf("Load frame error: File did not open\n");
    return false;
  }

  file_size = file.tellg();

  if (static_cast<uint64_t>(sizeof(dtype) * size) !=
      static_cast<uint64_t>(file_size)) {
    printf("Load frame error: Expected size (%lu) and file size (%lu) differ\n",
           static_cast<uint64_t>(sizeof(dtype) * size),
           static_cast<uint64_t>(file_size));
    return false;
  }

  file.seekg(0, std::ios::beg);
  auto data_char_ptr = reinterpret_cast<char *>(data);
  file.read(data_char_ptr, sizeof(dtype) * size);

  file.close();

  return true;
}

/**
 * @brief Saves a buffer to a file
 *
 * @tparam dtype   : Data type for data to save
 * @param filename : Filename to save data
 * @param data     : Data pointer of dtype type
 * @param size     : In dtype units
 * @return true    : In case of success
 * @return false   : In case of failure
 */
template <typename dtype>
bool save_frame(const std::string &filename, const dtype *data,
                const size_t size) {
  if (!data) {
    printf("Save frame error: Data pointer is null\n");
    return false;
  }

  std::ofstream file(filename,
                     std::ios::out | std::ios::binary | std::ios::ate);

  if (!file.is_open()) {
    printf("Save frame error: File did not open\n");
    return false;
  }

  file.write(reinterpret_cast<const char *>(data), sizeof(dtype) * size);

  file.close();

  return true;
}

/**
 * @brief Returns formated string with  input arguments
 *
 * This function is similar to sprintf, except using string return value
 *
 * @tparam Args...           : Used for variable number and types of arguments
 * @param format             : Format string using format descriptors
 * @param args               : Arguments to format into string
 * @return const std::string : Formatted string
 */
template <typename... Args>
const std::string format(const std::string &format, Args... args) {
  int size_s = std::snprintf(nullptr, 0, format.c_str(), args...) + 1;
  auto size = static_cast<size_t>(size_s);
  std::unique_ptr<char[]> buf(new char[size]);
  std::snprintf(buf.get(), size, format.c_str(), args...);
  return std::string(buf.get(), buf.get() + size - 1);
}

#endif  // ARTIFACT_DETECTOR_INCLUDE_UTILITY_HPP_
