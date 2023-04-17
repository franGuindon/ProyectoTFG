/* Copyright 2023 Francis Guindon <fbadilla10@gmail.com> */

#ifndef ARTIFACT_DETECTOR_INCLUDE_UTILITY_HPP_
#define ARTIFACT_DETECTOR_INCLUDE_UTILITY_HPP_

#include <stdio.h>

#include <fstream>
#include <string>

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

#endif  // ARTIFACT_DETECTOR_INCLUDE_UTILITY_HPP_
