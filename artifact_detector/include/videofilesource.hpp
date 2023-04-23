/* Copyright 2023 Francis Guindon <fbadilla10@gmail.com> */

#ifndef ARTIFACT_DETECTOR_INCLUDE_VIDEOFILESOURCE_HPP_
#define ARTIFACT_DETECTOR_INCLUDE_VIDEOFILESOURCE_HPP_

#include <gst/gst.h>

#include <string>

#include "include/types.hpp"

/**
 * @brief The VideofileSource class allows to load a video file frame by frame
 *
 * The VideofileSource class uses gstreamer as the media processing backend. In
 * order use the class simply call pullFrame to load a frame, then pushFrame
 * when done with the frame processing.
 */
class VideofileSource {
 public:
  /**
   * @brief Construct a VideofileSource
   *
   * @param filename  : Name of video file to load
   */
  explicit VideofileSource(std::string filename);
  /**
   * @brief Opens the file such that frames can be pulled from it
   *
   * @return ReturnValue : Operation status
   */
  ReturnValue openFile();
  /**
   * @brief Load a frame from the file
   *
   * @param frame       : Buffer to hold frame information
   * @return ReturnType : Operation status
   */
  ReturnValue pullFrame(Buffer<uint8_t>* frame);
  /**
   * @brief Deallocate frame memory after consuption
   *
   * @param frame       : Buffer to deallocate
   * @return ReturnType : Operation status
   */
  ReturnValue pushFrame(Buffer<uint8_t>* frame);

 private:
  /* Video filename */
  std::string file_;
  /* Video pipeline */
  GstElement* pipeline_;
};

#endif  // ARTIFACT_DETECTOR_INCLUDE_VIDEOFILESOURCE_HPP_
