/* Copyright 2023 Francis Guindon <fbadilla10@gmail.com> */

#include "include/videofilesource.hpp"

#include <cstdio>

#include "include/utility.hpp"

constexpr gchar kDefaultGstSinkName[] = "sink";

int get_sample_width(GstSample* sample) {
  int width = 0;

  GstCaps* caps = gst_sample_get_caps(sample);
  GstStructure* caps_struct = gst_caps_get_structure(caps, 0);
  gst_structure_get_int(caps_struct, "width", &width);
  return width;
}

int get_sample_height(GstSample* sample) {
  int height = 0;

  GstCaps* caps = gst_sample_get_caps(sample);
  GstStructure* caps_struct = gst_caps_get_structure(caps, 0);
  gst_structure_get_int(caps_struct, "height", &height);
  return height;
}

VideofileSource::VideofileSource(std::string filename, float loss)
    : file_{filename},
      loss_{loss},
      ret_{ReturnCode::Success, "Videofilesrc initialized correctly"},
      gst_pipeline_{nullptr},
      gst_buffer_{nullptr},
      gst_error_{nullptr},
      gst_error_description_{nullptr},
      gst_ret_{GST_STATE_CHANGE_SUCCESS},
      gst_elem_{nullptr},
      gst_sink_{nullptr},
      gst_sample_{nullptr},
      gst_sink_name_{kDefaultGstSinkName},
      gst_map_{0},
      gst_pts_{0},
      gst_bus_{nullptr},
      gst_msg_{nullptr} {
  ret_ = openFile();
  if (ReturnCode::Success != ret_.code) {
    throw ret_;
  }
}

VideofileSource::~VideofileSource() { freeResources(); }

ReturnValue VideofileSource::openFile() {
  gst_init(0, 0);

  std::string pipeline_str = format(
      "filesrc location=%s ! decodebin ! video/x-raw, format=I420 ! "
      "appsink sync=false max-buffers=5 name=%s",
      file_.c_str(), gst_sink_name_);

  if (0 < loss_ && loss_ <= 1) {
    pipeline_str = format(
        "filesrc location=%s ! decodebin ! video/x-raw, format=I420 ! "
        "x264enc ! h264parse ! identity drop_probability=%f ! avdec_h264 ! "
        "video/x-raw, format=I420 ! appsink sync=false max-buffers=5 name=%s",
        file_.c_str(), loss_, gst_sink_name_);
  }

  gst_pipeline_ = gst_parse_launch(pipeline_str.c_str(), &gst_error_);

  if (!gst_pipeline_) {
    ret_.code = ReturnCode::FileError;
    ret_.description =
        format("Gst parse launch returned error: %s", gst_error_->message);
    freeResources();
    return ret_;
  }

  gst_elem_ = gst_bin_get_by_name(GST_BIN(gst_pipeline_), gst_sink_name_);
  gst_sink_ = GST_APP_SINK(gst_elem_);

  gst_ret_ = gst_element_set_state(gst_pipeline_, GST_STATE_PLAYING);
  if (GST_STATE_CHANGE_FAILURE == gst_ret_) {
    ret_.code = ReturnCode::FileError;
    ret_.description = "Could not set pipeline to playing";
    freeResources();
    return ret_;
  }

  gst_bus_ = gst_element_get_bus(gst_pipeline_);
  gst_msg_ = gst_bus_pop_filtered(
      gst_bus_,
      static_cast<GstMessageType>(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

  if (gst_msg_ != nullptr) {
    ret_.code = ReturnCode::FileError;
    switch (GST_MESSAGE_TYPE(gst_msg_)) {
      case GST_MESSAGE_ERROR:
        gst_message_parse_error(gst_msg_, &gst_error_, &gst_error_description_);
        ret_.description =
            format("Element: %s\nError: %s\nDebug info:%s",
                   GST_OBJECT_NAME(gst_msg_->src), gst_error_->message,
                   gst_error_description_ ? gst_error_description_ : "None");
      case GST_MESSAGE_EOS:
        ret_.description = "Reached end of stream";
      default:
        ret_.description = "Unexpected message received on bus";
    }
    freeResources();
    return ret_;
  }

  ret_.code = ReturnCode::Success;
  ret_.description = "File opened successfully";
  return ret_;
}

void VideofileSource::freeResources() {
  if (gst_msg_) {
    gst_message_unref(gst_msg_);
    gst_msg_ = nullptr;
  }
  if (gst_bus_) {
    gst_object_unref(gst_bus_);
    gst_bus_ = nullptr;
  }
  if (gst_pipeline_) {
    gst_element_set_state(gst_pipeline_, GST_STATE_NULL);
    gst_object_unref(gst_pipeline_);
    gst_pipeline_ = nullptr;
  }
}

ReturnValue VideofileSource::pullFrame(Buffer<uint8_t>* frame) {
  gst_sample_ = gst_app_sink_pull_sample(gst_sink_);

  if (!gst_sample_) {
    ret_.code = ReturnCode::FileError;
    ret_.description = "Could not pull sample from pipeline";
    freeResources();
    return ret_;
  }

  frame->width = get_sample_width(gst_sample_);
  frame->height = get_sample_height(gst_sample_);

  gst_buffer_ = gst_sample_get_buffer(gst_sample_);
  gst_buffer_map(gst_buffer_, &gst_map_, GST_MAP_READ);
  gst_pts_ = GST_BUFFER_PTS(gst_buffer_);
  frame->pts = gst_pts_;

  frame->data = static_cast<uint8_t*>(gst_map_.data);

  return ReturnValue{ReturnCode::Success, "Operation returned successfully"};
}

ReturnValue VideofileSource::pushFrame(Buffer<uint8_t>* frame) {
  frame->data = nullptr;
  gst_buffer_unmap(gst_buffer_, &gst_map_);
  gst_sample_unref(gst_sample_);

  return ReturnValue{ReturnCode::Success, "Operation returned successfully"};
}
