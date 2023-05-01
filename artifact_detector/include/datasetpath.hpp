/* Copyright 2023 Francis Guindon <fbadilla10@gmail.com> */

#ifndef ARTIFACT_DETECTOR_INCLUDE_DATASETPATH_HPP_
#define ARTIFACT_DETECTOR_INCLUDE_DATASETPATH_HPP_

#include <string>
#include <vector>

#include "include/types.hpp"

/** FIXME: Missing doc */
enum class DatasetType {
  /** FIXME: Missing doc */
  kRawDataset = 0,
  /** FIXME: Missing doc */
  kBlackWhite = 1
};

/**
 * @brief The DatasetPath class allows to load and process dataset info
 *
 * The DatasetPath class is capable of loading and processing dataset info
 */
class DatasetPath {
 public:
  /**
   * @brief Construct a DatasetPath
   *
   * @param dataset_path : Name of dataset dir to use
   * @param type         : Type of dataset to load
   */
  DatasetPath(std::string dataset_path, DatasetType type);
  /**
   * @brief Destroy the DatasetPath instance
   */
  ~DatasetPath() {}
  ReturnValue parseDatasetDir();
  ReturnValue parseVidDir();
  ReturnValue parseSnipDir();

  inline const DatasetType type() { return type_; }
  inline const std::vector<std::string>& feature_files() {
    return dataset_feature_files_;
  }
  inline const std::vector<std::string>& label_files() {
    return dataset_label_files_;
  }

 private:
  const std::vector<std::string> filename_table_ = {"features.bytes", ""};
  /* Dataset path */
  std::string dataset_path_;
  /* For internal error management */
  ReturnValue ret_;
  /* Dataset path type */
  DatasetType type_;
  /* */
  std::vector<std::string> dataset_vid_dirs_;
  /* */
  std::vector<std::string> dataset_snip_dirs_;
  /* */
  std::vector<std::string> dataset_feature_files_;
  /* */
  std::vector<std::string> dataset_label_files_;
};

#endif  // ARTIFACT_DETECTOR_INCLUDE_DATASETPATH_HPP_
