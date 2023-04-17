/* Copyright 2023 Francis Guindon <fbadilla10@gmail.com> */

#ifndef ARTIFACT_DETECTOR_INCLUDE_VIDEOFILESINK_HPP_
#define ARTIFACT_DETECTOR_INCLUDE_VIDEOFILESINK_HPP_

#include <string>

#include "include/types.hpp"

/**
 * @brief The VideofileSink class allows to save a video file frame by frame
 *
 * The VideofileSource class uses gstreamer as the media processing backend. In
 * order use the class simply pushFrame to add a frame to the video file.
 */
class VideofileSink {
 public:
  /**
   * @brief Construct a VideofileSink
   *
   * @param filename  : Name of video file to save to
   */
  explicit VideofileSink(std::string filename);
  /**
   * @brief Add a new frame to th video file
   *
   * @param frame       : Buffer to add to the video file
   * @return ReturnType : Operation status
   */
  ReturnValue pushFrame(Buffer<uint8_t>* frame);
};

#endif  // ARTIFACT_DETECTOR_INCLUDE_VIDEOFILESINK_HPP_
