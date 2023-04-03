/*
 * Copyright (C) 2022 RidgeRun, LLC (http://www.ridgerun.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "buffer_utils.h"
#include "pipeline.h"

#include <cstdio>
#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <chrono>

constexpr uint32_t kMicrosPerSecond = 1000000;
using MicroSeconds = std::chrono::microseconds;

uint64_t GetCurrentTimeSinceEpochUs() {
  auto now = std::chrono::system_clock::now();
  auto now_us = std::chrono::time_point_cast<MicroSeconds>(now);
  auto epoch = now_us.time_since_epoch();
  auto value = std::chrono::duration_cast<MicroSeconds>(epoch);
  return value.count();
}

constexpr size_t kBlockDimension = 16;
constexpr size_t kRowsPerBlocklength = kBlockDimension;
constexpr size_t kPixelsPerBlocklength = kBlockDimension;
constexpr size_t kMetricsPerBorder = 8;

/**
 * @brief Returns formated string with  input arguments
 * 
 * This function is similar to sprintf, except using string return value
 * 
 * @tparam Args...           : Used for variable number and types of arguments
 * @param format             : Format string using format descriptors
 * @param args               : Arguments to format into string
 * @return const std::string : Formatted string
 */
template <typename... Args>
const std::string format(const std::string &format, Args... args) {
  int size_s = std::snprintf(nullptr, 0, format.c_str(), args...) + 1;
  auto size = static_cast<size_t>(size_s);
  std::unique_ptr<char[]> buf(new char[size]);
  std::snprintf(buf.get(), size, format.c_str(), args...);
  return std::string(buf.get(), buf.get() + size - 1);
}

#define ABS_DIFF(in1, in2) in1 > in2 ? in1 - in2 : in2 - in1

#define PRINT_2_BLOCK_VALS(src, step, n0, n1) ( \
  printf("%d %d\n",*(src + n0*step), *(src + n1*step)) \
)

#define PRINT_8_BLOCK_VALS(src, step, n0, n1, n2, n3, n4, n5, n6, n7) (          \
  printf("%d %d %d %d %d %d %d %d\n",                                            \
         *(src + n0*step), *(src + n1*step), *(src + n2*step), *(src + n3*step), \
         *(src + n4*step), *(src + n5*step), *(src + n6*step), *(src + n7*step)) \
)

#define SUM_2_BLOCK_VALS(dst, src, step, n0, n1) ( \
  (dst) += *(src + n0*step) + *(src + n1*step)     \
)

#define SUM_4_BLOCK_VALS(dst, src, step, n0, n1, n2, n3) ( \
  (dst) += *(src + n0*step) + *(src + n1*step)             \
         + *(src + n2*step) + *(src + n3*step)             \
)

#define SUM_8_BLOCK_VALS(dst, src, step, n0, n1, n2, n3, n4, n5, n6, n7) ( \
  (dst) += *(src + n0*step) + *(src + n1*step)                             \
         + *(src + n2*step) + *(src + n3*step)                             \
         + *(src + n4*step) + *(src + n5*step)                             \
         + *(src + n6*step) + *(src + n7*step)                             \
)

#define SQ_SUM_2_BLOCK_VALS(dst, src, step, n0, n1) ( \
  (dst) += (*(src + n0*step))*(*(src + n0*step))      \
         + (*(src + n1*step))*(*(src + n1*step))      \
)

#define SQ_SUM_4_BLOCK_VALS(dst, src, step, n0, n1, n2, n3) ( \
      (dst) += (*(src + n0*step))*(*(src + n0*step))          \
             + (*(src + n1*step))*(*(src + n1*step))          \
             + (*(src + n2*step))*(*(src + n2*step))          \
             + (*(src + n3*step))*(*(src + n3*step))          \
)

#define SQ_SUM_8_BLOCK_VALS(dst, src, step, n0, n1, n2, n3, n4, n5, n6, n7) ( \
    (dst) += (*(src + n0*step))*(*(src + n0*step))                            \
           + (*(src + n1*step))*(*(src + n1*step))                            \
           + (*(src + n2*step))*(*(src + n2*step))                            \
           + (*(src + n3*step))*(*(src + n3*step))                            \
           + (*(src + n4*step))*(*(src + n4*step))                            \
           + (*(src + n5*step))*(*(src + n5*step))                            \
           + (*(src + n6*step))*(*(src + n6*step))                            \
           + (*(src + n7*step))*(*(src + n7*step))                            \
)

/**
 * @brief FIXME: Missing docs
 * 
 * @param filename 
 * @param data 
 * @param size 
 * @return true 
 * @return false 
 */
bool save_frame(const std::string filename, const uint8_t *data,
                const size_t size) {
  if (!data) {
    printf("Save frame error: Data pointer is null\n");
    return false;
  }

  std::ofstream file(filename, std::ios::out | std::ios::binary | std::ios::ate);

  if (!file.is_open()) {
    printf("Save frame error: File did not open\n");
    return false;
  }

  file.write(reinterpret_cast<const char *>(data), size);

  file.close();

  return true;
}

/**
 * @brief Applies [-1, 1] filter horizontally and vertically on input frame
 * 
 * Note: Filtered results must have the same dimensions as input data
 * 
 * @param data : Input frame of dimensions 'width' and 'height'
 * @param vfiltered : where vertically filtered result will be stored.
 * @param hfiltered : where horizontally filtered result will be stored.
 * @param width     : Width for data, vfiltered, and hfiltered buffers.
 * @param height    : Height for data, vfiltered, and hfiltered buffers.
 * @return true     : In case of success
 * @return false    : In case of failure
 */
bool filter_frame(const uint8_t *data, uint8_t *vfiltered, uint8_t *hfiltered,
                  const size_t width, const size_t height) {
  if (!data || !vfiltered || !hfiltered) {
    printf("Filter frame error: null parameters");
    return false;
  }

  const uint8_t* in_row_ptr = data;
  uint8_t* ver_row_ptr = vfiltered;
  uint8_t* hor_row_ptr = hfiltered;

  const uint8_t* in_elem_ptr = nullptr;
  uint8_t* ver_elem_ptr = nullptr;
  uint8_t* hor_elem_ptr = nullptr;

  const uint8_t* in_row_ptr_end = in_row_ptr + width*height;

  /* Loop excludes last row by subtracting width from end */
  for (; in_row_ptr != in_row_ptr_end - width; in_row_ptr += width,
                                               ver_row_ptr += width,
                                               hor_row_ptr += width) {
    in_elem_ptr = in_row_ptr;
    ver_elem_ptr = ver_row_ptr;
    hor_elem_ptr = hor_row_ptr;

    const uint8_t* in_elem_ptr_end = in_elem_ptr + width;
    
    /* Loop excludes last element in row by subtracting 1 from end */
    for (; in_elem_ptr != in_elem_ptr_end - 1; ++in_elem_ptr) {
      *ver_elem_ptr++ = ABS_DIFF(*in_elem_ptr, *(in_elem_ptr + width));
      *hor_elem_ptr++ = ABS_DIFF(*in_elem_ptr, *(in_elem_ptr + 1));
    }

    /* Last element of row (not considered in loop) */
    *ver_elem_ptr = ABS_DIFF(*in_elem_ptr, *(in_elem_ptr + width));
    *hor_elem_ptr = 0;
  }

  /* Last row in image (not considered in loop) */
  for (; in_elem_ptr != in_row_ptr_end - 1; ++in_elem_ptr) {
    *ver_elem_ptr++ = 0;
    *hor_elem_ptr++ = ABS_DIFF(*in_elem_ptr, *(in_elem_ptr + 1));
  }
  
  *ver_elem_ptr = 0;
  *hor_elem_ptr = 0;

  return true;
}

bool get_frame_sat(const uint8_t *data, uint8_t *sat, uint8_t *sq_sat,
                   const size_t width, const size_t height) {
  if (!data || !sat || !sq_sat) {
    printf("Get frame sat error: null parameters");
    return false;
  }

  const uint8_t* in_row_ptr = data;
  uint8_t* sat_row_ptr = sat;
  uint8_t* sq_sat_row_ptr = sq_sat;

  const uint8_t* in_elem_ptr = in_row_ptr;
  uint8_t* sat_elem_ptr = sat_row_ptr;
  uint8_t* sq_sat_elem_ptr = sq_sat_row_ptr;

  /* First element in image */
  *sat_elem_ptr++ = *in_elem_ptr;
  *sq_sat_elem_ptr++ = (*in_elem_ptr)*(*in_elem_ptr);

  /* First row in image (not considered in loop) */
  ++in_elem_ptr;
  const uint8_t* in_elem_ptr_end = in_row_ptr + width;
  for (; in_elem_ptr != in_elem_ptr_end; ++in_elem_ptr,
                                         ++sat_elem_ptr,
                                         ++sq_sat_elem_ptr) {
    *sat_elem_ptr = *in_elem_ptr + *(sat_elem_ptr-1);
    *sq_sat_elem_ptr = (*in_elem_ptr)*(*in_elem_ptr) + *(sq_sat_elem_ptr-1);
  }

  /* Loop excludes first row */
  in_row_ptr += width;
  const uint8_t* in_row_ptr_end = in_row_ptr + width*height;
  for (; in_row_ptr != in_row_ptr_end; in_row_ptr += width,
                                       sat_row_ptr += width,
                                       sq_sat_row_ptr += width) {
    in_elem_ptr = in_row_ptr;
    sat_elem_ptr = sat_row_ptr;
    sq_sat_elem_ptr = sq_sat_row_ptr;

    /* First element in row */
    *sat_elem_ptr = *in_elem_ptr + *(sat_elem_ptr-width);
    *sq_sat_elem_ptr = (*in_elem_ptr)*(*in_elem_ptr) + *(sat_elem_ptr-width);
    
    /* Loop excludes first element in row */
    ++sat_elem_ptr;
    ++sq_sat_elem_ptr;
    ++in_elem_ptr;
    const uint8_t* in_elem_ptr_end = in_row_ptr + width;
    for (; in_elem_ptr != in_elem_ptr_end; ++in_elem_ptr,
                                           ++sat_elem_ptr,
                                           ++sq_sat_elem_ptr) {
      *sat_elem_ptr = *in_elem_ptr + *(sat_elem_ptr-1) + *(sat_elem_ptr-width)
                    - *(sat_elem_ptr-width-1);
      *sq_sat_elem_ptr = (*in_elem_ptr)*(*in_elem_ptr)
                       + *(sq_sat_elem_ptr-1) + *(sq_sat_elem_ptr-width)
                       - *(sq_sat_elem_ptr-width-1);
    }
  }

  return true;
}

/**
 * @brief Generate features for a border of a block
 * 
 * Note: The order of features that are pushed back into the features vector:
 * - First, the mean of a size 2 block border of the vertical filtered frame
 * - Then, the mean of a size 4 border, then 8, then 16
 * - Then, the means of the horizontal filtered frame block border
 *   (following the same size order)
 * - Then the variances for the vertical filtered frame
 * - Finally, the variances for the horizontal filtered frame
 * 
 * Note: Value and group terminology:
 * The algorithm involves a loop that iterates over groups of values within the
 * block.
 * For horizontal borders, each border column is a group and values are
 * arranged vertically. So vstep = width and gstep = 1
 * For vertical borders, each border row is a group and
 * values are arranges horizontally. So vstep = 1 and gstep = width
 * 
 * Note: Border offset:
 * Each border offset should always correspond to a block top left corner (not
 * necesarily for the block that the border is being calculated for).
 * For horizontal borders, use the leftmost block top left corner within border
 * For vertical borders, use the topmost block top left corner within border
 * 
 * @param vfiltered     : Vertical filtered frame data
 * @param hfiltered     : Horizontal filtered frame data
 * @param border_offset : Border location within frame data
 * @param vstep         : Value step used to jump between values within group
 * @param gstep         : Group step used to jump between groups within border
 * @param features      : Feature memory
 * @return true         : In case of success
 * @return false        : In case of failure
 */
bool generate_border_features(const uint8_t *vfiltered,
                              const uint8_t *hfiltered,
                              const size_t border_offset,
                              const size_t vstep, const size_t gstep,
                              std::vector<float> &features) {
  size_t* sums = new size_t[kMetricsPerBorder];
  size_t* sq_sums = new size_t[kMetricsPerBorder];
  float* means = new float[kMetricsPerBorder];
  float* vars = new float[kMetricsPerBorder];

  memset(sums, 0, sizeof(size_t)*kMetricsPerBorder);
  memset(sq_sums, 0, sizeof(size_t)*kMetricsPerBorder);

  const uint8_t *vptr = vfiltered + border_offset;
  const uint8_t *hptr = hfiltered + border_offset;
  const uint8_t *vptr_end = vptr + gstep*kPixelsPerBlocklength;

  for (; vptr != vptr_end; vptr += gstep, hptr += gstep) {
    SUM_2_BLOCK_VALS(sums[0], vptr, vstep, 0, -1);
    SUM_2_BLOCK_VALS(sums[1], vptr, vstep, 1, -2);
    SUM_4_BLOCK_VALS(sums[2], vptr, vstep, 2, 3, -3, -4);
    SUM_8_BLOCK_VALS(sums[3], vptr, vstep, 4, 5, 6, 7, -5, -6, -7, -8);
    SUM_2_BLOCK_VALS(sums[4], hptr, vstep, 0, -1);
    SUM_2_BLOCK_VALS(sums[5], hptr, vstep, 1, -2);
    SUM_4_BLOCK_VALS(sums[6], hptr, vstep, 2, 3, -3, -4);
    SUM_8_BLOCK_VALS(sums[7], hptr, vstep, 4, 5, 6, 7, -5, -6, -7, -8);

    SQ_SUM_2_BLOCK_VALS(sq_sums[0], vptr, vstep, 0, -1);
    SQ_SUM_2_BLOCK_VALS(sq_sums[1], vptr, vstep, 1, -2);
    SQ_SUM_4_BLOCK_VALS(sq_sums[2], vptr, vstep, 2, 3, -3, -4);
    SQ_SUM_8_BLOCK_VALS(sq_sums[3], vptr, vstep, 4, 5, 6, 7, -5, -6, -7, -8);
    SQ_SUM_2_BLOCK_VALS(sq_sums[4], hptr, vstep, 0, -1);
    SQ_SUM_2_BLOCK_VALS(sq_sums[5], hptr, vstep, 1, -2);
    SQ_SUM_4_BLOCK_VALS(sq_sums[6], hptr, vstep, 2, 3, -3, -4);
    SQ_SUM_8_BLOCK_VALS(sq_sums[7], hptr, vstep, 4, 5, 6, 7, -5, -6, -7, -8);
  }

  sums[1] += sums[0];
  sums[2] += sums[1];
  sums[3] += sums[2];
  
  sums[5] += sums[4];
  sums[6] += sums[5];
  sums[7] += sums[6];

  sq_sums[1] += sq_sums[0];
  sq_sums[2] += sq_sums[1];
  sq_sums[3] += sq_sums[2];
  
  sq_sums[5] += sq_sums[4];
  sq_sums[6] += sq_sums[5];
  sq_sums[7] += sq_sums[6];

  /* Calculate metrics for block row of block_offset */
  means[0] = static_cast<float>(sums[0]) / kPixelsPerBlocklength;
  means[1] = static_cast<float>(sums[1]) / kPixelsPerBlocklength;
  means[2] = static_cast<float>(sums[2]) / kPixelsPerBlocklength;
  means[3] = static_cast<float>(sums[3]) / kPixelsPerBlocklength;
  means[4] = static_cast<float>(sums[4]) / kPixelsPerBlocklength;
  means[5] = static_cast<float>(sums[5]) / kPixelsPerBlocklength;
  means[6] = static_cast<float>(sums[6]) / kPixelsPerBlocklength;
  means[7] = static_cast<float>(sums[7]) / kPixelsPerBlocklength;
  vars[0] = static_cast<float>(sq_sums[0]) / kPixelsPerBlocklength - means[0]*means[0];
  vars[1] = static_cast<float>(sq_sums[1]) / kPixelsPerBlocklength - means[1]*means[1];
  vars[2] = static_cast<float>(sq_sums[2]) / kPixelsPerBlocklength - means[2]*means[2];
  vars[3] = static_cast<float>(sq_sums[3]) / kPixelsPerBlocklength - means[3]*means[3];
  vars[4] = static_cast<float>(sq_sums[4]) / kPixelsPerBlocklength - means[4]*means[4];
  vars[5] = static_cast<float>(sq_sums[5]) / kPixelsPerBlocklength - means[5]*means[5];
  vars[6] = static_cast<float>(sq_sums[6]) / kPixelsPerBlocklength - means[6]*means[6];
  vars[7] = static_cast<float>(sq_sums[7]) / kPixelsPerBlocklength - means[7]*means[7];

  features.push_back(means[0]);
  features.push_back(means[1]);
  features.push_back(means[2]);
  features.push_back(means[3]);
  features.push_back(means[4]);
  features.push_back(means[5]);
  features.push_back(means[6]);
  features.push_back(means[7]);
  features.push_back(vars[0]);
  features.push_back(vars[1]);
  features.push_back(vars[2]);
  features.push_back(vars[3]);
  features.push_back(vars[4]);
  features.push_back(vars[5]);
  features.push_back(vars[6]);
  features.push_back(vars[7]);

  delete[] means;
  delete[] vars;
  delete[] sums;
  delete[] sq_sums;

  return true;
}

/**
 * @brief Generate mean and variance features for the block proper
 * 
 * Note: Feature order:
 * First, mean of vertical filtered block data,
 * Then, mean of horizontal filtered block data,
 * Then, variance of vertical filtered block data,
 * Then, variance of horizontal filtered block data.
 * 
 * @param vfiltered     : Vertical filtered frame data
 * @param hfiltered     : Horizontal filtered frame data
 * @param border_offset : Border location within frame data
 * @param step          : Step between rows of block
 * @param features      : Feature memory
 * @return true         : In case of success
 * @return false        : In case of failure
 */
bool generate_block_proper_features(const uint8_t *vfiltered,
                                    const uint8_t *hfiltered,
                                    const size_t border_offset,
                                    const size_t step,
                                    std::vector<float> &features) {
  constexpr size_t kMetricsPerBlockProper = 2;
  size_t* sums = new size_t[kMetricsPerBlockProper];
  size_t* sq_sums = new size_t[kMetricsPerBlockProper];
  float* means = new float[kMetricsPerBlockProper];
  float* vars = new float[kMetricsPerBlockProper];

  memset(sums, 0, sizeof(size_t)*kMetricsPerBlockProper);
  memset(sq_sums, 0, sizeof(size_t)*kMetricsPerBlockProper);

  const uint8_t *vptr = vfiltered + border_offset;
  const uint8_t *hptr = hfiltered + border_offset;
  const uint8_t *vptr_end = vptr + step*kPixelsPerBlocklength;

  for (; vptr != vptr_end; vptr += step, hptr += step) {
    SUM_8_BLOCK_VALS(sums[0], vptr, 1, 0, 1, 2, 3, 4, 5, 6, 7);
    SUM_8_BLOCK_VALS(sums[0], vptr, 1, 8, 9, 10, 11, 12, 13, 14, 15);
    SUM_8_BLOCK_VALS(sums[1], hptr, 1, 0, 1, 2, 3, 4, 5, 6, 7);
    SUM_8_BLOCK_VALS(sums[1], hptr, 1, 8, 9, 10, 11, 12, 13, 14, 15);

    SQ_SUM_8_BLOCK_VALS(sq_sums[0], vptr, 1, 0, 1, 2, 3, 4, 5, 6, 7);
    SQ_SUM_8_BLOCK_VALS(sq_sums[0], vptr, 1, 8, 9, 10, 11, 12, 13, 14, 15);
    SQ_SUM_8_BLOCK_VALS(sq_sums[1], hptr, 1, 0, 1, 2, 3, 4, 5, 6, 7);
    SQ_SUM_8_BLOCK_VALS(sq_sums[1], hptr, 1, 8, 9, 10, 11, 12, 13, 14, 15);
  }

  /* Calculate metrics for block row of block_offset */
  means[0] = static_cast<float>(sums[0]) / kPixelsPerBlocklength;
  means[1] = static_cast<float>(sums[1]) / kPixelsPerBlocklength;
  vars[0] = static_cast<float>(sq_sums[0]) / kPixelsPerBlocklength - means[0]*means[0];
  vars[1] = static_cast<float>(sq_sums[1]) / kPixelsPerBlocklength - means[1]*means[1];

  features.push_back(means[0]);
  features.push_back(means[1]);
  features.push_back(vars[0]);
  features.push_back(vars[1]);

  delete[] means;
  delete[] vars;
  delete[] sums;
  delete[] sq_sums;

  return true;
}

/**
 * @brief Generate features for single block
 * 
 * @param vfiltered    : Vertical filtered frame data
 * @param hfiltered    : Horizontal filtered frame data
 * @param block_offset : Block location within frame data (top left corner)
 * @param width        : Frame data width
 * @param height       : Frame data height
 * @param features     : Feature memory
 * @return true        : In case of success
 * @return false       : In case of failure
 */
bool generate_block_features(const uint8_t *vfiltered, const uint8_t *hfiltered,
                             const size_t block_offset, const size_t width,
                             const size_t height, std::vector<float> &features) {
  constexpr size_t kFeaturesPerBorder = 2*2*4;

  /* Top border */
  generate_border_features(vfiltered, hfiltered, block_offset, width, 1, features);
  /* Bottom border */
  generate_border_features(vfiltered, hfiltered, block_offset + kRowsPerBlocklength*width, width, 1, features);
  /* Left border */
  generate_border_features(vfiltered, hfiltered, block_offset, 1, width, features);
  /* Right border */
  generate_border_features(vfiltered, hfiltered, block_offset + kPixelsPerBlocklength, 1, width, features);
  /* Block proper features */
  generate_block_proper_features(vfiltered, hfiltered, block_offset, width, features);


  for (size_t i = 0; i < 4*kFeaturesPerBorder + 4; ++i) {
    printf("%f ", features[i]);
  }
  printf("\n");

  return true;
}

/**
 * @brief Generate features for single frame
 * 
 * @param data           : Frame data
 * @param width          : Frame width
 * @param height         : Frame height
 * @param features       : Features vector
 * @returns true         : In case of success
 * @returns false        : In case of failure
 */
/* FIXME: Abstract whole procedure to class perhaps*/
bool generate_frame_features(const uint8_t *data, const size_t width,
                      const size_t height, std::vector<float> &features) {

  /* Allocate vertical filter and horizontal filter */
  auto vertical_filtered = std::unique_ptr<uint8_t>(new uint8_t[width*height]);
  auto horizontal_filtered = std::unique_ptr<uint8_t>(new uint8_t[width*height]);

  auto vertical_sat = std::unique_ptr<uint8_t>(new uint8_t[width*height]);
  auto vertical_sq_sat = std::unique_ptr<uint8_t>(new uint8_t[width*height]);
  auto horizontal_sat = std::unique_ptr<uint8_t>(new uint8_t[width*height]);
  auto horizontal_sq_sat = std::unique_ptr<uint8_t>(new uint8_t[width*height]);

  uint64_t now = 0;

  now = GetCurrentTimeSinceEpochUs();
  filter_frame(data, vertical_filtered.get(), horizontal_filtered.get(), width, height);
  printf("Filtering duration: %ld\n", GetCurrentTimeSinceEpochUs() - now);

  now = GetCurrentTimeSinceEpochUs();
  get_frame_sat(vertical_filtered.get(), vertical_sat.get(),
                vertical_sq_sat.get(), width, height);
  printf("Vertical SAT and SAT2 duration: %ld\n", GetCurrentTimeSinceEpochUs() - now);

  now = GetCurrentTimeSinceEpochUs();
  get_frame_sat(horizontal_filtered.get(), horizontal_sat.get(),
                horizontal_sq_sat.get(), width, height);
  printf("Horizontal SAT and SAT2 duration: %ld\n", GetCurrentTimeSinceEpochUs() - now);
  
  save_frame("raw_frame", data, width*height);
  save_frame("vertical_filtered", vertical_filtered.get(), width*height);
  save_frame("horizontal_filtered", horizontal_filtered.get(), width*height);

  save_frame("vertical_sat", vertical_sat.get(), width*height);
  save_frame("vertical_sat2", vertical_sq_sat.get(), width*height);
  save_frame("horizontal_sat", horizontal_sat.get(), width*height);
  save_frame("horizontal_sat2", horizontal_sq_sat.get(), width*height);

  const size_t kPixelsPerRow = width;
  
  size_t block_i = 1; /* Measured in blocklengths */
  size_t block_j = 1; /* Measured in blocklengths */
  size_t block_offset = block_i*kRowsPerBlocklength*kPixelsPerRow +
                        block_j*kPixelsPerBlocklength;

  now = GetCurrentTimeSinceEpochUs();
  generate_block_features(vertical_filtered.get(), horizontal_filtered.get(),
                          block_offset, width, height, features);
  printf("Feature extraction duration: %ld\n", GetCurrentTimeSinceEpochUs() - now);
  return true;
}

/**
 * @brief Generate dataset memory for a given videopath and label_path
 * 
 * Currently, features and labels are expected to be vectors.
 * This way the function is not concerned about memory as much.
 * The hope is to preallocate all necesary memory.
 * This may speed up execution.
 * 
 * @param video_path  : Path to video with packet loss
 * @param labels_path : Path to labels generated by ground truth
 * @param features    : Feature memory
 * @param labels      : Label memory
 * @returns bool      : Operation status
 */
bool generate_dataset(std::string video_path, std::string labels_path,
                      std::vector<float> &features, std::vector<float> labels) {
  GstElement *input_pipeline = NULL;
  int processed_frames = 0;
  GstBuffer *input_buffer = NULL;
  GError *gst_error = NULL;

  printf("Initializing gstreamer\n");
  gst_init(0, 0);

  /* Build the pipelines */
  printf("Building input pipeline from video: %s\n", video_path.c_str());
  input_pipeline = gst_parse_launch(format("filesrc location=%s !"
      "decodebin ! queue ! appsink sync=false max-buffers=6 name=sink -e",
      video_path.c_str()).c_str(), &gst_error);
  if (!input_pipeline) {
    printf("Gst parse pipeline error: %s\n", gst_error->message);
    return false;
  }

  printf("Initializing pipeline wrapper for input pipeline\n");
  Pipeline InputPipeline(input_pipeline);

  printf("Starting pipeline wrapper for input pipeline\n");
  InputPipeline.start_pipeline("sink");
  int total_frames = 1;// 200;

  /* Loop to process frames */
  printf("Starting processing loop\n");
  while (processed_frames < total_frames) {
    printf("Processing frame: %d\n", processed_frames);

    int width = 0;
    int height = 0;

    printf("Pulling buffer\n");

    input_buffer = InputPipeline.pull_buffer(width, height);

    /* Use InputPipeline.map to process frame info */
    printf("Generating feature\n");
    uint8_t* data = static_cast<uint8_t*>(InputPipeline.map.data);
    if (!generate_frame_features(data, width, height, features)) {
      /** FIXME: Perhaps ensure no memory leaks before throwing error */
      throw std::runtime_error("Feature generation failed");
    }
    data = nullptr;

    printf("Unmapping buffer\n");
    gst_buffer_unmap(input_buffer, &InputPipeline.map);

    printf("Incrementing processed frame count\n");
    ++processed_frames;
  }

  return true;
}

/**
 * @brief Process a lossy video and generate detection features
 *
 * Run a gstreamer pipeline to load videos and process them frame by frame
 * Save features to file
 *
 * @param argc Number of command line arguments, expected to be 3
 * @param argv Command line argument array
 * @param verbose_out Where to log output.
 * @return int : 0 on success
 */
int run_generator(int argc, char **argv, std::ostream& verbose_out) {
  if (argc != 3) {
    printf("Usage feature_generator [LOSSY_VID_FILE] [LABEL_FILE]\n");
    return -1;
  }
  
  std::vector<float> features;
  std::vector<float> labels;

  if (!generate_dataset(argv[1], argv[2], features, labels)) {
    printf("Data generation failed\n");
    return -1;
  }

  return 0;
}

int main(int argc, char **argv) {
  bool verbose = true;
  try {
    if (verbose) {
      run_generator(argc, argv, std::cout);
    } else {
      std::ofstream logfile{"get_video_masks.log"};
      if (!logfile.good()) {
        throw std::runtime_error("Could not write to logfile.");
      }
      run_generator(argc, argv, logfile);
    }
  } catch (std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return -1;
  }
  return 0;
}
