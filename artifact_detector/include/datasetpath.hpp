/* Copyright 2023 Francis Guindon <fbadilla10@gmail.com> */

#ifndef ARTIFACT_DETECTOR_INCLUDE_DATASETPATH_HPP_
#define ARTIFACT_DETECTOR_INCLUDE_DATASETPATH_HPP_

#include <gst/app/gstappsink.h>
#include <gst/gst.h>

#include <string>

#include "include/types.hpp"

/** FIXME: Missing doc */
enum class DatasetType {
  /** FIXME: Missing doc */
  kRawDataset = 0
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
   * @param filename  : Name of dataset dir to use
   */
  explicit DatasetPath(std::string dataset_path);
  /**
   * @brief Destroy the DatasetPath instance
   */
  ~DatasetPath();
  ReturnValue getPathType();
  ReturnValue parseSnipDir();
  ReturnValue parseVidDir();
  ReturnValue parseDatasetDir();

  inline const DatasetType type() { return type_; }

 private:
  /* Video filename */
  std::string dataset_path_;
  /* For internal error management */
  ReturnValue ret_;
  /* */
  DatasetType type_;
};

#endif  // ARTIFACT_DETECTOR_INCLUDE_DATASETPATH_HPP_
