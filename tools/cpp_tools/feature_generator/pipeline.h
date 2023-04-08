/*
 * Copyright (C) 2022 RidgeRun, LLC (http://www.ridgerun.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef PIPELINE_H
#define PIPELINE_H

#include <gst/app/gstappsink.h>
#include <gst/app/gstappsrc.h>
#include <gst/gst.h>
/**
 * @brief Class Pipeline for construct and setup Gstreamer pipelines.
 *
 * This class is intended to ease the handling of gstreamer pipeline
 * elements, events and functions.
 *
 */
class Pipeline {
 public:
  GstAppSink *appsink = NULL;
  GstAppSrc *appsrc = NULL;
  GstBuffer *buffer = NULL;
  GstMapInfo map;
  GstElement *pipeline = NULL;
  GstClockTime timestamp = 0;
  /**
   * @brief Construct a new Pipeline object.
   *
   * @param gst_pipeline : GstElement to setup the given pipeline.
   */
  Pipeline(GstElement *gst_pipeline);
  /**
   * @brief Set pipeline to playing state.
   *
   */
  void start_pipeline(const gchar *name);
  /**
   * @brief Set pipeline to NULL state.
   *
   */
  void stop_pipeline();
  /**
   * @brief
   *
   * @param width : frame width.
   * @param height : frame height.
   * @return GstBuffer* new buffer from the pipeline.
   */
  GstBuffer *pull_buffer(int &width, int &height);
  /**
   * @brief Destroy the Pipeline object.
   *
   */
  ~Pipeline();

 private:
  GstStateChangeReturn ret;
  GstElement *io_element = NULL;
  GstSample *sample = NULL;
};
#endif  // PIPELINE_H
