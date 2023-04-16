/* Copyright 2023 Francis Guindon <fbadilla10@gmail.com> */

#ifndef ARTIFACT_DETECTOR_INCLUDE_TYPES_HPP_
#define ARTIFACT_DETECTOR_INCLUDE_TYPES_HPP_

#include <string>

/**
 * @brief The ReturnCode enumerator is used in ReturnValue structures to
 * indicate an operation's status
 */
enum class ReturnCode {
  /** Indicates an operation returned successfully */
  Success = 0,
  /** Indicates an error in memory management */
  MemoryError = 1,
  /** Indicates an error in the operation parameters */
  ParameterError = 2,
  /** Indicates an error in file management */
  FileError = 3
};

/**
 * @brief The ReturnValue structure is the main return value Artifact Detector
 * methods
 *
 * The ReturnValue structure uses a ReturnCode to specify the operation status
 * of library methods. Additionally, the description member helps specify
 * details on the operation status.
 */
struct ReturnValue {
  /** Return value code to identify operation status type */
  ReturnCode code;
  /** Return value description to specify operation status details */
  std::string description;
};

/**
 * @brief The Buffer structure holds the basic info for a memory block
 *
 * In the Artifact Detector library, the Buffer structure is the basic unit of
 * memory used as an argument in most methods
 *
 * @tparam dtype : The data type for the memory block
 */
template <typename dtype>
struct Buffer {
  /** Data pointer to the Buffer memory block */
  dtype *data;
  /** Size of the data memory block in dtype units (NOT bytes) */
  size_t size;
  /** Width of the data memory block */
  size_t width;
  /** Height of the data memory block */
  size_t height;
};

#endif  // ARTIFACT_DETECTOR_INCLUDE_TYPES_HPP_
