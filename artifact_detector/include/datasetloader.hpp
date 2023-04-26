/* Copyright 2023 Francis Guindon <fbadilla10@gmail.com> */

#ifndef ARTIFACT_DETECTOR_INCLUDE_DATASETLOADER_HPP_
#define ARTIFACT_DETECTOR_INCLUDE_DATASETLOADER_HPP_

#include <string>

#include "include/datasetpath.hpp"
#include "include/types.hpp"

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
   */
  explicit DatasetLoader(std::string dataset_path);
  /**
   * @brief Destroy the DatasetLoader instance
   */
  ~DatasetLoader();
  /**
   * @brief Opens the file such that frames can be pulled from it
   *
   * @return ReturnValue : Operation status
   */
  ReturnValue parseDatasetPath() {
    return {ReturnCode::NotImplementedError, ""};
  }
  ReturnValue loadVideofiles() { return {ReturnCode::NotImplementedError, ""}; }
  ReturnValue loadRawDataset();
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

  /* Dataset path */
  DatasetPath dataset_path_;
  /* For internal error management */
  ReturnValue ret_;
};

#endif  // ARTIFACT_DETECTOR_INCLUDE_DATASETLOADER_HPP_
