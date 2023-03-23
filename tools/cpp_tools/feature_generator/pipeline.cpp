/*
 * Copyright (C) 2022 RidgeRun, LLC (http://www.ridgerun.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "buffer_utils.h"
#include "pipeline.h"

#include <iostream>
#include <stdexcept>
#include <string>

Pipeline::Pipeline(GstElement *gst_pipeline) { pipeline = gst_pipeline; }

void Pipeline::start_pipeline(const gchar *name) {
  if (!pipeline) {
    throw std::invalid_argument("Cannot create pipeline");
  }
  io_element = gst_bin_get_by_name(GST_BIN(pipeline), name);
  if (strcmp(name, "sink") == 0) {
    appsink = GST_APP_SINK(io_element);
  } else if (strcmp(name, "src") == 0) {
    appsrc = GST_APP_SRC(io_element);
  } else {
    throw std::invalid_argument(
        "You should add appsink or appsrc elements with corresponding \"sink\" "
        "or \"src\" name");
  }
  gst_element_set_state(pipeline, GST_STATE_PLAYING);
}

void Pipeline::stop_pipeline() {
  gst_element_set_state(pipeline, GST_STATE_NULL);
}

GstBuffer *Pipeline::pull_buffer(int &width, int &height) {
  GstSample * sample = gst_app_sink_pull_sample(appsink);
  width = get_sample_width(sample);
  height = get_sample_height(sample);

  if (!sample) {
    throw std::invalid_argument("failed to get sample\n");
  }
  buffer = gst_sample_get_buffer(sample);
  gst_buffer_map(buffer, &map, GST_MAP_READ);
  timestamp = GST_BUFFER_PTS(buffer);

  gst_sample_unref(sample);

  return buffer;
}

Pipeline::~Pipeline() {
  stop_pipeline();
  gst_object_unref(pipeline);
};
