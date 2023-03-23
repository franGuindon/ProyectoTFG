/*
 * Copyright (C) 2022 RidgeRun, LLC (http://www.ridgerun.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "buffer_utils.h"

void get_histogram(int value) {
  std::vector<int> values, frequencies;
  std::vector<int>::iterator it =
      std::find(values.begin(), values.end(), value);

  if (it != values.end()) {
    int index = std::distance(values.begin(), it);
    frequencies[index]++;
  } else {
    values.push_back(value);
    frequencies.push_back(1);
  }
}

void save_data(std::vector<int> values, std::vector<int> frequencies) {
  int histogram_data[255][2];
  std::ofstream file;

  for (int i = 0; i < 255; i++) {
    histogram_data[i][0] = values[i];
    histogram_data[i][1] = frequencies[i];
  }
  file.open("histogram.csv");
  for (auto& row : histogram_data) {
    for (auto col : row) file << col << ',';
    file << '\n';
  }
}

void pix_2_macroblocks(GstMapInfo map, const int height, const int width,
                       const int threshold = 256 * 64) {
  unsigned char* const kRefPixel = map.data;
  unsigned char* result = map.data;
  const int kMacroblockSize = 16;

  for (int my = 0; my < height; my += kMacroblockSize) {
    unsigned char* mrow_ptr = kRefPixel + my * width;
    for (int mx = 0; mx < width; mx += kMacroblockSize) {
      int sum = 0;
      result = mrow_ptr + mx;
      for (int row = 0; row < kMacroblockSize; ++row) {
        unsigned char* col_ptr = result + row * width;
        unsigned char* const endcol = col_ptr + kMacroblockSize;
        for (; col_ptr != endcol; ++col_ptr) {
          sum += *col_ptr;
        }
      }
      const char val = sum >= threshold ? 255u : 0u;

      for (int row = 0; row < kMacroblockSize; ++row) {
        unsigned char* col_ptr = result + row * width;
        unsigned char* const endcol = col_ptr + kMacroblockSize;
        for (; col_ptr != endcol; ++col_ptr) {
          *col_ptr = val;
        }
      }
    }
  }
}

GstBuffer* subtract_frames(GstMapInfo refmap, GstMapInfo plmap,
                           GstClockTime timestamp) {
  GstBuffer* buffer = NULL;
  GstMapInfo map;
  unsigned char* ref_data =
      static_cast<unsigned char*>(refmap.data);
  unsigned char* map_end = ref_data + (refmap.size);
  unsigned char* pl_data = static_cast<unsigned char*>(plmap.data);
  unsigned char* ref_ptr = ref_data;
  unsigned char* pl_ptr = pl_data;

  buffer = gst_buffer_new_and_alloc(plmap.size);
  gst_buffer_map(buffer, &map, GST_MAP_WRITE);
  unsigned char* result = map.data;
  map.size = plmap.size;
  for (ref_ptr = ref_data, pl_ptr = pl_data; ref_ptr != map_end;
       ++ref_ptr, ++pl_ptr, ++result) {
    *result = static_cast<unsigned char>(
        (*ref_ptr > *pl_ptr) ? *ref_ptr - *pl_ptr : *pl_ptr - *ref_ptr);
  }
  GST_BUFFER_PTS(buffer) = timestamp;
  gst_buffer_unmap(buffer, &map);
  return buffer;
}

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
