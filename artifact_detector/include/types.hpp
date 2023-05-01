/* Copyright 2023 Francis Guindon <fbadilla10@gmail.com> */

#ifndef ARTIFACT_DETECTOR_INCLUDE_TYPES_HPP_
#define ARTIFACT_DETECTOR_INCLUDE_TYPES_HPP_

#include <cstdio>
#include <memory>
#include <string>

#include "include/logging.hpp"
#include "include/utility.hpp"

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
  FileError = 3,
  /** Indicates that a method can't be used since it is not implemented */
  NotImplementedError = 4,
  /** Indicates that a method has been deprecated */
  DeprecatedError = 5
};

/** This table translates between ReturnCode and its string representation */
static const char *ReturnCodeTable[] = {
    "Success",   "MemoryError",         "ParameterError",
    "FileError", "NotImplementedError", "DeprecatedError"};

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
  /** Print */
  std::string str() {
    size_t code_id = static_cast<size_t>(this->code);
    const char *code_str = ReturnCodeTable[code_id];
    return format("Code %s: %s", code_str, description.c_str());
  }
  ReturnValue(ReturnCode code, std::string description) {
    this->code = code;
    this->description = description;
  }
  ReturnValue() {
    this->code = ReturnCode::DeprecatedError;
    this->description =
        "Creating method local Return Values is deprecated over adding "
        "ReturnValue ret_ member to class";
    printf("%s\n", this->str().c_str());
  }
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
  dtype *data = nullptr;
  /** Size of the data memory block in dtype units (NOT bytes) */
  size_t size = 0;
  /** Width of the data memory block */
  size_t width = 0;
  /** Height of the data memory block */
  size_t height = 0;
  /** Buffer timestamp */
  uint64_t pts = 0;
  /** Unique pointer to manage deallocation of buffer memory */
  std::unique_ptr<dtype[]> data_uptr = nullptr;
  /** Allocate using width and height */
  ReturnValue allocate(size_t width, size_t height) {
    this->width = width;
    this->height = height;
    this->size = width * height;
    return this->allocate();
  }
  /** Allocate using size */
  ReturnValue allocate(size_t size) {
    this->size = size;
    return this->allocate();
  }
  /** Allocate */
  ReturnValue allocate() {
    if (!this->size) {
      return {ReturnCode::MemoryError, "Size not set"};
    }
    this->data_uptr = std::unique_ptr<dtype[]>(new dtype[this->size]);
    this->data = data_uptr.get();
    return {ReturnCode::Success, "Allocated buffer"};
  }
};

#endif  // ARTIFACT_DETECTOR_INCLUDE_TYPES_HPP_
