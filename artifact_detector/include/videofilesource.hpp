/* Copyright 2023 Francis Guindon <fbadilla10@gmail.com> */

#ifndef ARTIFACT_DETECTOR_INCLUDE_VIDEOFILESOURCE_HPP_
#define ARTIFACT_DETECTOR_INCLUDE_VIDEOFILESOURCE_HPP_

#include <gst/app/gstappsink.h>
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
   * @brief Destroy the VideofileSource instance
   */
  ~VideofileSource();
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
  ReturnValue pullFrame(Buffer<uint8_t> *frame);
  /**
   * @brief Deallocate frame memory after consuption
   *
   * @param frame       : Buffer to deallocate
   * @return ReturnType : Operation status
   */
  ReturnValue pushFrame(Buffer<uint8_t> *frame);

 private:
  /**
   * @brief Deallocates all gstreamer related memory
   */
  void freeResources();

  /* Video filename */
  std::string file_;
  /* For internal error management */
  ReturnValue ret_;
  /* Video pipeline */
  GstElement *gst_pipeline_;
  /* Frame buffer */
  GstBuffer *gst_buffer_;
  /* For gstreamer error management */
  GError *gst_error_;
  /* Gstreamer error description */
  gchar *gst_error_description_;
  /* For gstreamer returns */
  GstStateChangeReturn gst_ret_;
  /* Gstreamer element */
  GstElement *gst_elem_;
  /* Gstreamer sink */
  GstAppSink *gst_sink_;
  /* Gstreamer sample */
  GstSample *gst_sample_;
  /* Gstreamer sink name */
  const gchar *gst_sink_name_;
  /* Gstreamer map */
  GstMapInfo gst_map_;
  /* Gstreamer timestamp */
  GstClockTime gst_pts_;
  /* Gstreamer bus */
  GstBus *gst_bus_;
  /* Gstreamer message */
  GstMessage *gst_msg_;
};

#endif  // ARTIFACT_DETECTOR_INCLUDE_VIDEOFILESOURCE_HPP_
