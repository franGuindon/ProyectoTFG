/* Copyright 2023 Francis Guindon <fbadilla10@gmail.com> */

#include "include/datasetloader.hpp"

#include <cstdio>

#include "include/utility.hpp"

DatasetLoader::DatasetLoader(std::string dataset_path)
    : dataset_path_{dataset_path},
      ret_{ReturnCode::Success, "Videofilesrc initialized correctly"},
      consts_{},
      raw_feature_mem_{},
      raw_label_mem_{} {
  HANDLE_WITH_THROW(allocate(),
                    ReturnValue(ReturnCode::MemoryError,
                                "Could not allocate dataset memory"));

  HANDLE_WITH_THROW(loadRawDataset(), ReturnValue(ReturnCode::MemoryError,
                                                  "Could not load dataset"));

  HANDLE_WITH_THROW(
      generateRandomizedBalancedDataset(),
      ReturnValue(ReturnCode::MemoryError,
                  "Could not generate randomized balanced dataset"));
}

ReturnValue DatasetLoader::allocate() {
  consts_.setNumFiles(dataset_path_.feature_files().size());

  raw_feature_mem_.size = consts_.total_feature_mem_size;
  HANDLE_WITH(raw_feature_mem_.allocate(),
              ReturnValue(ReturnCode::MemoryError,
                          "Could not allocate raw feature mem"));

  raw_label_mem_.size = consts_.total_label_mem_size;
  HANDLE_WITH(
      raw_label_mem_.allocate(),
      ReturnValue(ReturnCode::MemoryError, "Could not allocate raw label mem"));

  aligned_label_mem_.size = consts_.total_aligned_label_mem_size;

  HANDLE_WITH(aligned_label_mem_.allocate(),
              ReturnValue(ReturnCode::MemoryError,
                          "Could not allocate aligned label mem"));

  return ret_;
}

ReturnValue DatasetLoader::loadRawDataset() {
  for (size_t i = 0; i < consts_.num_files; ++i) {
    std::string feature_path = dataset_path_.feature_files()[i];
    std::string label_path = dataset_path_.label_files()[i];
    float *feature_ptr = raw_feature_mem_.data + i * consts_.feature_file_size;
    uint8_t *label_ptr = raw_label_mem_.data + i * consts_.label_file_size;
    float *aligned_label_ptr =
        aligned_label_mem_.data + i * consts_.num_examples_per_file;
    printf("Loading feature file: %s\n", feature_path.c_str());
    load_frame(feature_path, feature_ptr, consts_.feature_file_size);
    printf("Loading label file: %s\n", label_path.c_str());
    load_frame(label_path, label_ptr, consts_.label_file_size);

    HANDLE_WITH(
        alignLabels(label_ptr, aligned_label_ptr),
        ReturnValue(
            ReturnCode::MemoryError,
            "Load raw dataset failed while aligning labels to feature order"));
  }

  return ret_;
}

ReturnValue DatasetLoader::alignLabels(const uint8_t *raw,
                                       float *const aligned) {
  float *out_ptr = aligned;
  for (size_t i = 0; i < consts_.num_frames_per_snip; ++i) {
    for (size_t j = 1; j < consts_.blocks_per_col - 1; ++j) {
      for (size_t k = 1; k < consts_.blocks_per_row - 1; ++k) {
        size_t offset =
            i * consts_.blocks_per_frame + j * consts_.blocks_per_row + k;
        *out_ptr = static_cast<float>(raw[offset]);
        out_ptr++;
      }
    }
  }
  if (aligned + consts_.num_examples_per_file != out_ptr) {
    return {ReturnCode::MemoryError,
            "Iteration count and Aligned feature size mismatch"};
  }

  return ret_;
}

ReturnValue DatasetLoader::generateRandomizedBalancedDataset() {
  HANDLE_WITH(
      generateBalancedDataset(),
      ReturnValue(ReturnCode::MemoryError, "Failed to balance dataset"));

  HANDLE_WITH(randomizeBalancedDataset(),
              ReturnValue(ReturnCode::MemoryError,
                          "Failed to randomize balanced dataset"));

  return ret_;
}

ReturnValue DatasetLoader::generateBalancedDataset() { return ret_; }

ReturnValue DatasetLoader::randomizeBalancedDataset() { return ret_; }
