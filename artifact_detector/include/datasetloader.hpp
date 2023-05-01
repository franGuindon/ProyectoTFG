/* Copyright 2023 Francis Guindon <fbadilla10@gmail.com> */

#ifndef ARTIFACT_DETECTOR_INCLUDE_DATASETLOADER_HPP_
#define ARTIFACT_DETECTOR_INCLUDE_DATASETLOADER_HPP_

#include <string>

#include "include/datasetpath.hpp"
#include "include/types.hpp"

struct DatasetConstants {
  const size_t block_dimension = 16;
  const size_t frame_width = 1280;
  const size_t frame_height = 720;
  const size_t num_frames_per_snip = 200;
  const size_t feature_vector_size = 132;

  const size_t blocks_per_row = frame_width / block_dimension;
  const size_t blocks_per_col = frame_height / block_dimension;
  const size_t blocks_per_frame = blocks_per_row * blocks_per_col;

  const size_t num_examples_per_file =
      (blocks_per_row - 2) * (blocks_per_col - 2) * num_frames_per_snip;

  const size_t feature_file_size = num_examples_per_file * feature_vector_size;
  const size_t label_file_size =
      blocks_per_row * blocks_per_col * num_frames_per_snip;

  size_t num_files = 0;
  size_t total_feature_mem_size = 0;
  size_t total_label_mem_size = 0;
  size_t total_aligned_label_mem_size = 0;

  void setNumFiles(const size_t num_files) {
    this->num_files = num_files;
    this->total_feature_mem_size = this->feature_file_size * num_files;
    this->total_label_mem_size = this->label_file_size * num_files;
    this->total_aligned_label_mem_size =
        this->num_examples_per_file * num_files;
  }
};

/**
 * @brief The DatasetLoader class allows to load and process dataset info
 *
 * The DatasetLoader class is capable of loading and processing dataset info
 */
class DatasetLoader {
 public:
  /**
   * @brief Construct a DatasetLoader
   *
   * @param filename  : Name of dataset dir to use
   * @param type      : Name of dataset type to use
   */
  explicit DatasetLoader(std::string dataset_path, DatasetType type);
  /**
   * @brief Destroy the DatasetLoader instance
   */
  ~DatasetLoader() {}
  /**
   * @brief Opens the file such that frames can be pulled from it
   *
   * @return ReturnValue : Operation status
   */
  ReturnValue loadVideofiles() { return {ReturnCode::NotImplementedError, ""}; }
  ReturnValue loadRawDataset();
  ReturnValue alignLabels(const uint8_t *raw, float *const aligned);
  ReturnValue loadBalancedDataset() {
    return {ReturnCode::NotImplementedError, ""};
  }
  ReturnValue loadRandomizedBalancedDataset() {
    return {ReturnCode::NotImplementedError, ""};
  }
  /**
   * @brief Load a frame from the file
   *
   * @param frame       : Buffer to hold frame information
   * @return ReturnType : Operation status
   */
  ReturnValue generateRawDataset() {
    return {ReturnCode::NotImplementedError, ""};
  }
  ReturnValue generateBalancedDataset();
  ReturnValue randomizeBalancedDataset();
  ReturnValue generateRandomizedBalancedDataset();

  ReturnValue saveRawDataset() { return {ReturnCode::NotImplementedError, ""}; }
  ReturnValue saveBalancedDataset() {
    return {ReturnCode::NotImplementedError, ""};
  }
  ReturnValue saveRandomizedBalancedDataset();

 private:
  /**
   * @brief Allocate all dataset related memory
   *
   * @return ReturnValue
   */
  ReturnValue allocate();
  /**
   * @brief Deallocate all dataset related memory
   */
  void freeResources();

  /** Dataset path */
  DatasetPath dataset_path_;
  /** For internal error management */
  ReturnValue ret_;
  /** Dataset Constants */
  DatasetConstants consts_;
  /** Raw feature memory */
  Buffer<float> raw_feature_mem_;
  /** Raw label memory */
  Buffer<uint8_t> raw_label_mem_;
  /** Aligned label memory */
  Buffer<float> aligned_label_mem_;
  /** Balanced feature memory */
  Buffer<float> balanced_feature_mem_;
  /** Balanced label memory */
  Buffer<float> balanced_label_mem_;
};

#endif  // ARTIFACT_DETECTOR_INCLUDE_DATASETLOADER_HPP_
