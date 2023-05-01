/*
 * Copyright (C) 2022 RidgeRun, LLC (http://www.ridgerun.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2 as
 * published by the Free Software Foundation.
 */

// #define DEBUG
// #define PROF_BY_FILTER_SAT_STAT
#define PROF_BY_FRAME
// #define SAVE_FRAME_FILTER_SAT_STAT

#include <cstdio>
#include <iostream>
#include <memory>
#include <string>

#include "artifact_detector/include/utility.hpp"
#include "artifact_detector/include/videofilesource.hpp"
#include "tools/cpp_tools/feature_generator/buffer_utils.h"
#include "tools/cpp_tools/feature_generator/pipeline.h"

constexpr size_t kBlockDimension = 16;
constexpr size_t kRowsPerBlocklength = kBlockDimension;
constexpr size_t kPixelsPerBlocklength = kBlockDimension;
constexpr size_t kMetricsPerBorder = 8;

#define ABS_DIFF(in1, in2) in1 > in2 ? in1 - in2 : in2 - in1

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

  const uint8_t *in_row_ptr = data;
  uint8_t *ver_row_ptr = vfiltered;
  uint8_t *hor_row_ptr = hfiltered;

  const uint8_t *in_elem_ptr = in_row_ptr;
  uint8_t *ver_elem_ptr = ver_row_ptr;
  uint8_t *hor_elem_ptr = hor_row_ptr;

  const uint8_t *in_row_ptr_end = in_row_ptr + width * height;

  /* Loop excludes last row by subtracting width from end */
  for (; in_row_ptr != in_row_ptr_end - width;) {
    const uint8_t *in_elem_ptr_end = in_elem_ptr + width;

    /* Loop excludes last element in row by subtracting 1 from end */
    for (; in_elem_ptr != in_elem_ptr_end - 1;) {
      *ver_elem_ptr = ABS_DIFF(*in_elem_ptr, *(in_elem_ptr + width));
      *hor_elem_ptr = ABS_DIFF(*in_elem_ptr, *(in_elem_ptr + 1));

      ++in_elem_ptr;
      ++ver_elem_ptr;
      ++hor_elem_ptr;
    }

    /* Last element of row (not considered in loop) */
    *ver_elem_ptr = ABS_DIFF(*in_elem_ptr, *(in_elem_ptr + width));
    *hor_elem_ptr = 0;

    in_row_ptr += width;
    ver_row_ptr += width;
    hor_row_ptr += width;

    in_elem_ptr = in_row_ptr;
    ver_elem_ptr = ver_row_ptr;
    hor_elem_ptr = hor_row_ptr;
  }

  /* Last row in image (not considered in loop) */
  for (; in_elem_ptr != in_row_ptr_end - 1;) {
    *ver_elem_ptr = 0;
    *hor_elem_ptr = ABS_DIFF(*in_elem_ptr, *(in_elem_ptr + 1));

    ++in_elem_ptr;
    ++ver_elem_ptr;
    ++hor_elem_ptr;
  }

  /* Last element in last row (not considered in loop) */
  *ver_elem_ptr = 0;
  *hor_elem_ptr = 0;

  return true;
}

/**
 * @brief Get the frame sat object
 *
 * @param data
 * @param sat
 * @param sq_sat
 * @param width
 * @param height
 * @return true
 * @return false
 */
bool get_frame_sat(const uint8_t *data, uint32_t *sat, uint32_t *sq_sat,
                   const size_t width, const size_t height) {
  if (!data || !sat || !sq_sat) {
    printf("Get frame sat error: null parameters");
    return false;
  }

  const uint8_t *in_row_ptr = data;
  uint32_t *sat_row_ptr = sat;
  uint32_t *sq_sat_row_ptr = sq_sat;

  const uint8_t *in_elem_ptr = in_row_ptr;
  uint32_t *sat_elem_ptr = sat_row_ptr;
  uint32_t *sq_sat_elem_ptr = sq_sat_row_ptr;

  /* First element in image */
  *sat_elem_ptr = *in_elem_ptr;
  *sq_sat_elem_ptr = (*in_elem_ptr) * (*in_elem_ptr);

  /* First row in image (not considered in loop) */
  ++in_elem_ptr;
  ++sat_elem_ptr;
  ++sq_sat_elem_ptr;
  const uint8_t *in_elem_ptr_end = in_row_ptr + width;
  for (; in_elem_ptr != in_elem_ptr_end;
       ++in_elem_ptr, ++sat_elem_ptr, ++sq_sat_elem_ptr) {
    *sat_elem_ptr = *in_elem_ptr + *(sat_elem_ptr - 1);
    *sq_sat_elem_ptr = (*in_elem_ptr) * (*in_elem_ptr) + *(sq_sat_elem_ptr - 1);
  }

  /* Loop excludes first row */
  in_row_ptr += width;
  sat_row_ptr += width;
  sq_sat_row_ptr += width;
  const uint8_t *in_row_ptr_end = data + width * height;
  for (; in_row_ptr != in_row_ptr_end;
       in_row_ptr += width, sat_row_ptr += width, sq_sat_row_ptr += width) {
    in_elem_ptr = in_row_ptr;
    sat_elem_ptr = sat_row_ptr;
    sq_sat_elem_ptr = sq_sat_row_ptr;

    /* First element in row */
    *sat_elem_ptr = *in_elem_ptr + *(sat_elem_ptr - width);
    *sq_sat_elem_ptr =
        (*in_elem_ptr) * (*in_elem_ptr) + *(sq_sat_elem_ptr - width);

    /* Loop excludes first element in row */
    ++sat_elem_ptr;
    ++sq_sat_elem_ptr;
    ++in_elem_ptr;
    const uint8_t *in_elem_ptr_end = in_row_ptr + width;
    for (; in_elem_ptr != in_elem_ptr_end;
         ++in_elem_ptr, ++sat_elem_ptr, ++sq_sat_elem_ptr) {
      *sat_elem_ptr = *in_elem_ptr + *(sat_elem_ptr - 1) +
                      *(sat_elem_ptr - width) - *(sat_elem_ptr - width - 1);
      *sq_sat_elem_ptr = (*in_elem_ptr) * (*in_elem_ptr) +
                         *(sq_sat_elem_ptr - 1) + *(sq_sat_elem_ptr - width) -
                         *(sq_sat_elem_ptr - width - 1);
    }
  }

  return true;
}

inline bool get_vertical_border_sums(const uint32_t *sat, const size_t offset,
                                     const size_t stride, uint32_t *sum_ptr) {
  sum_ptr[0] = sat[offset + (kBlockDimension - 1) * stride] -
               sat[offset + (kBlockDimension - 1) * stride - 2] -
               sat[offset - stride] + sat[offset - stride - 2];
  sum_ptr[1] = sat[offset + (kBlockDimension - 1) * stride + 1] -
               sat[offset + (kBlockDimension - 1) * stride - 3] -
               sat[offset - stride + 1] + sat[offset - stride - 3];
  sum_ptr[2] = sat[offset + (kBlockDimension - 1) * stride + 3] -
               sat[offset + (kBlockDimension - 1) * stride - 5] -
               sat[offset - stride + 3] + sat[offset - stride - 5];
  sum_ptr[3] = sat[offset + (kBlockDimension - 1) * stride + 7] -
               sat[offset + (kBlockDimension - 1) * stride - 9] -
               sat[offset - stride + 7] + sat[offset - stride - 9];
  return true;
}

inline bool get_horizontal_border_sums(const uint32_t *sat, const size_t offset,
                                       const size_t stride, uint32_t *sum_ptr) {
  sum_ptr[0] = sat[offset + (kBlockDimension - 1)] -
               sat[offset + (kBlockDimension - 1) - stride * 2] -
               sat[offset - 1] + sat[offset - 1 - stride * 2];
  sum_ptr[1] = sat[offset + (kBlockDimension - 1) + stride] -
               sat[offset + (kBlockDimension - 1) - stride * 3] -
               sat[offset - 1 + stride] + sat[offset - 1 - stride * 3];
  sum_ptr[2] = sat[offset + (kBlockDimension - 1) + stride * 3] -
               sat[offset + (kBlockDimension - 1) - stride * 5] -
               sat[offset - 1 + stride * 3] + sat[offset - 1 - stride * 5];
  sum_ptr[3] = sat[offset + (kBlockDimension - 1) + stride * 7] -
               sat[offset + (kBlockDimension - 1) - stride * 9] -
               sat[offset - 1 + stride * 7] + sat[offset - 1 - stride * 9];
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

/**
 * @brief Generate features for a single block
 *
 * Some definitons:
 * - vblock : Vertically filtered (16 px)x(16 px) block region
 * - hblock : Horizontally filtered (16 px)x(16 px) block region
 * - vtop : Vertically filtered top border of block
 * - vleft : Vertically filtered left border of block
 * - vbot : Vertically filtered bottom border of block
 * - vright : Vertically filtered right border of block
 * - htop : Horizontally filtered top border of block
 * - nhleft : Horizontally filtered left border of block
 * - nhbot : Horizontally filtered bottom border of block
 * - hright : Horizontally filtered right border of block
 *
 * - Feature order: There are 132 features per block (indices range from 0-131):
 * - 0: vblock mean
 * - 1: vblock var
 * - 2: hblock mean
 * - 3: hblock var
 * - 4-19: vtop stats
 *   - 4-7: vtop means
 *     - 4:  2 px width
 *     - 5:  4 px width
 *     - 6:  8 px width
 *     - 7: 16 px width
 *   - 8-11: vtop vars
 *   - 12-15: differences between vtop means and vblock mean
 *   - 16-19: differences between vtop vars and vblock mean
 * - 20-35: vleft stats
 * - 36-51: vbot stats
 * - 52-67: vright stats
 * - 68-83: htop stats
 * - 84-99: hleft stats
 * - 100-115: hbot stats
 * - 116-131: hright stats
 *
 * @param vsat         : SAT for vertically filtered frame
 * @param vsat2        : Squared SAT for vertically filtered frame
 * @param hsat         : SAT for horizontally filtered frame
 * @param hsat2        : Squared SAT for horizontally filtered frame
 * @param block_offset : Block location within frame data (top left corner)
 * @param width        : Frame data width
 * @param height       : Frame data height
 * @param features     : Feature memory
 * @return true        : In case of success
 * @return false       : In case of failure
 */
#ifdef DEBUG
bool generate_block_features(const uint32_t *vsat, const uint32_t *vsat2,
                             const uint32_t *hsat, const uint32_t *hsat2,
                             const size_t block_offset, const size_t width,
                             const size_t height, std::vector<float> *features,
                             const bool debug) {
#else
bool generate_block_features(const uint32_t *vsat, const uint32_t *vsat2,
                             const uint32_t *hsat, const uint32_t *hsat2,
                             const size_t block_offset, const size_t width,
                             const size_t height,
                             std::vector<float> *features) {
#endif
  const size_t group_size = 8 * 4;
  auto sums = std::unique_ptr<uint32_t[]>(new uint32_t[group_size]);
  auto sums2 = std::unique_ptr<uint32_t[]>(new uint32_t[group_size]);
  auto means = std::unique_ptr<float[]>(new float[group_size]);
  auto vars = std::unique_ptr<float[]>(new float[group_size]);

  /* Block proper stats */
  uint32_t vblock_sum = vsat[block_offset + (kBlockDimension - 1) * width +
                             (kBlockDimension - 1)] -
                        vsat[block_offset + (kBlockDimension - 1) * width - 1] -
                        vsat[block_offset - width + (kBlockDimension - 1)] +
                        vsat[block_offset - width - 1];
  uint32_t vblock_sum2 =
      vsat2[block_offset + (kBlockDimension - 1) * width +
            (kBlockDimension - 1)] -
      vsat2[block_offset + (kBlockDimension - 1) * width - 1] -
      vsat2[block_offset - width + (kBlockDimension - 1)] +
      vsat2[block_offset - width - 1];
  uint32_t hblock_sum = hsat[block_offset + (kBlockDimension - 1) * width +
                             (kBlockDimension - 1)] -
                        hsat[block_offset + (kBlockDimension - 1) * width - 1] -
                        hsat[block_offset - width + (kBlockDimension - 1)] +
                        hsat[block_offset - width - 1];
  uint32_t hblock_sum2 =
      hsat2[block_offset + (kBlockDimension - 1) * width +
            (kBlockDimension - 1)] -
      hsat2[block_offset + (kBlockDimension - 1) * width - 1] -
      hsat2[block_offset - width + (kBlockDimension - 1)] +
      hsat2[block_offset - width - 1];
  float vblock_mean = static_cast<float>(vblock_sum) / (16 * 16);
  float vblock_var =
      static_cast<float>(vblock_sum2) / (16 * 16) - vblock_mean * vblock_mean;
  float hblock_mean = static_cast<float>(hblock_sum) / (16 * 16);
  float hblock_var =
      static_cast<float>(hblock_sum2) / (16 * 16) - hblock_mean * hblock_mean;

#ifdef DEBUG
  if (!debug) goto debug_end;
  printf("%u %u %u %u %u\n",
         hsat2[block_offset + (kBlockDimension - 1) * width +
               (kBlockDimension - 1)],
         hsat2[block_offset + (kBlockDimension - 1) * width - 1],
         hsat2[block_offset - width + (kBlockDimension - 1)],
         hsat2[block_offset - width - 1],
         hsat2[block_offset + (kBlockDimension - 1) * width +
               (kBlockDimension - 1)] -
             hsat2[block_offset + (kBlockDimension - 1) * width - 1] -
             hsat2[block_offset - width + (kBlockDimension - 1)] +
             hsat2[block_offset - width - 1]);
  printf("%u %u %u %u %f %f %f %f\n", vblock_sum, vblock_sum2, hblock_sum,
         hblock_sum2, vblock_mean, vblock_var, hblock_mean, hblock_var);

debug_end:
#endif

  features->push_back(vblock_mean);
  features->push_back(vblock_var);
  features->push_back(hblock_mean);
  features->push_back(hblock_var);

  /* Border stats */
  /* Left */
  get_vertical_border_sums(vsat, block_offset, width, sums.get());
  /* Top */
  get_horizontal_border_sums(vsat, block_offset, width, sums.get() + 4);
  /* Right */
  get_vertical_border_sums(vsat, block_offset + kBlockDimension, width,
                           sums.get() + 8);
  /* Bottom */
  get_horizontal_border_sums(vsat, block_offset + kBlockDimension * width,
                             width, sums.get() + 12);
  /* Left */
  get_vertical_border_sums(hsat, block_offset, width, sums.get() + 16);
  /* Top */
  get_horizontal_border_sums(hsat, block_offset, width, sums.get() + 20);
  /* Right */
  get_vertical_border_sums(hsat, block_offset + kBlockDimension, width,
                           sums.get() + 24);
  /* Bottom */
  get_horizontal_border_sums(hsat, block_offset + kBlockDimension * width,
                             width, sums.get() + 28);

  /* Left */
  get_vertical_border_sums(vsat2, block_offset, width, sums2.get());
  /* Top */
  get_horizontal_border_sums(vsat2, block_offset, width, sums2.get() + 4);
  /* Right */
  get_vertical_border_sums(vsat2, block_offset + kBlockDimension, width,
                           sums2.get() + 8);
  /* Bottom */
  get_horizontal_border_sums(vsat2, block_offset + kBlockDimension * width,
                             width, sums2.get() + 12);
  /* Left */
  get_vertical_border_sums(hsat2, block_offset, width, sums2.get() + 16);
  /* Top */
  get_horizontal_border_sums(hsat2, block_offset, width, sums2.get() + 20);
  /* Right */
  get_vertical_border_sums(hsat2, block_offset + kBlockDimension, width,
                           sums2.get() + 24);
  /* Bottom */
  get_horizontal_border_sums(hsat2, block_offset + kBlockDimension * width,
                             width, sums2.get() + 28);

  for (size_t i = 0; i < group_size; i += 4) {
    means[i] = static_cast<float>(sums[i]) / (16 * 2);
    means[i + 1] = static_cast<float>(sums[i + 1]) / (16 * 4);
    means[i + 2] = static_cast<float>(sums[i + 2]) / (16 * 8);
    means[i + 3] = static_cast<float>(sums[i + 3]) / (16 * 16);

    vars[i] = static_cast<float>(sums2[i]) / (16 * 2) - means[i] * means[i];
    vars[i + 1] = static_cast<float>(sums2[i + 1]) / (16 * 4) -
                  means[i + 1] * means[i + 1];
    vars[i + 2] = static_cast<float>(sums2[i + 2]) / (16 * 8) -
                  means[i + 2] * means[i + 2];
    vars[i + 3] = static_cast<float>(sums2[i + 3]) / (16 * 16) -
                  means[i + 3] * means[i + 3];

    features->push_back(means[i]);
    features->push_back(means[i + 1]);
    features->push_back(means[i + 2]);
    features->push_back(means[i + 3]);

    features->push_back(vars[i]);
    features->push_back(vars[i + 1]);
    features->push_back(vars[i + 2]);
    features->push_back(vars[i + 3]);

    features->push_back(means[i] -
                        (i < group_size / 2 ? vblock_mean : hblock_mean));
    features->push_back(means[i + 1] -
                        (i < group_size / 2 ? vblock_mean : hblock_mean));
    features->push_back(means[i + 2] -
                        (i < group_size / 2 ? vblock_mean : hblock_mean));
    features->push_back(means[i + 3] -
                        (i < group_size / 2 ? vblock_mean : hblock_mean));

    features->push_back(vars[i] -
                        (i < group_size / 2 ? vblock_var : hblock_var));
    features->push_back(vars[i + 1] -
                        (i < group_size / 2 ? vblock_var : hblock_var));
    features->push_back(vars[i + 2] -
                        (i < group_size / 2 ? vblock_var : hblock_var));
    features->push_back(vars[i + 3] -
                        (i < group_size / 2 ? vblock_var : hblock_var));
  }

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
                             const size_t height,
                             std::vector<float> *features) {
  /* Allocate vertical filter and horizontal filter */
  auto vertical_filtered =
      std::unique_ptr<uint8_t>(new uint8_t[width * height]);
  auto horizontal_filtered =
      std::unique_ptr<uint8_t>(new uint8_t[width * height]);

  auto vertical_sat = std::unique_ptr<uint32_t>(new uint32_t[width * height]);
  auto vertical_sq_sat =
      std::unique_ptr<uint32_t>(new uint32_t[width * height]);
  auto horizontal_sat = std::unique_ptr<uint32_t>(new uint32_t[width * height]);
  auto horizontal_sq_sat =
      std::unique_ptr<uint32_t>(new uint32_t[width * height]);

#ifdef PROF_BY_FILTER_SAT_STAT
  uint64_t now = 0;

  now = GetCurrentTimeSinceEpochUs();
#endif
  filter_frame(data, vertical_filtered.get(), horizontal_filtered.get(), width,
               height);
#ifdef PROF_BY_FILTER_SAT_STAT
  printf("Filtering duration: %ld\n", GetCurrentTimeSinceEpochUs() - now);

  now = GetCurrentTimeSinceEpochUs();
#endif
  get_frame_sat(vertical_filtered.get(), vertical_sat.get(),
                vertical_sq_sat.get(), width, height);
#ifdef PROF_BY_FILTER_SAT_STAT
  printf("Vertical SAT and SAT2 duration: %ld\n",
         GetCurrentTimeSinceEpochUs() - now);

  now = GetCurrentTimeSinceEpochUs();
#endif
  get_frame_sat(horizontal_filtered.get(), horizontal_sat.get(),
                horizontal_sq_sat.get(), width, height);
#ifdef PROF_BY_FILTER_SAT_STAT
  printf("Horizontal SAT and SAT2 duration: %ld\n",
         GetCurrentTimeSinceEpochUs() - now);
#endif
  const size_t kPixelsPerRow = width;
  const size_t kBlocksPerRow = width / kBlockDimension;
  const size_t kBlocksPerCol = height / kBlockDimension;

#ifdef PROF_BY_FILTER_SAT_STAT
  now = GetCurrentTimeSinceEpochUs();
#endif
  for (size_t block_i = 1; block_i < kBlocksPerCol - 1; ++block_i) {
    for (size_t block_j = 1; block_j < kBlocksPerRow - 1; ++block_j) {
      const size_t block_offset =
          block_i * kRowsPerBlocklength * kPixelsPerRow +
          block_j * kPixelsPerBlocklength;
#ifdef DEBUG
      bool debug = ((block_i == 1) && (block_j == 17)) ? true : false;

      generate_block_features(vertical_sat.get(), vertical_sq_sat.get(),
                              horizontal_sat.get(), horizontal_sq_sat.get(),
                              block_offset, width, height, features, debug);
#else
      generate_block_features(vertical_sat.get(), vertical_sq_sat.get(),
                              horizontal_sat.get(), horizontal_sq_sat.get(),
                              block_offset, width, height, features);
#endif
    }
  }
#ifdef PROF_BY_FILTER_SAT_STAT
  printf("Feature extraction duration: %ld\n",
         GetCurrentTimeSinceEpochUs() - now);

  now = GetCurrentTimeSinceEpochUs();
#endif

#ifdef SAVE_FRAME_FILTER_SAT_STAT
  save_frame("raw_frame", data, width * height);
  save_frame("vertical_filtered", vertical_filtered.get(), width * height);
  save_frame("horizontal_filtered", horizontal_filtered.get(), width * height);

  save_frame("vertical_sat", vertical_sat.get(), width * height);
  save_frame("vertical_sat2", vertical_sq_sat.get(), width * height);
  save_frame("horizontal_sat", horizontal_sat.get(), width * height);
  save_frame("horizontal_sat2", horizontal_sq_sat.get(), width * height);

  save_frame("features", features.data(), features.size());
#endif

#ifdef PROF_BY_FILTER_SAT_STAT
  printf("Saving data duration: %ld\n", GetCurrentTimeSinceEpochUs() - now);
#endif
  return true;
}

bool generate_dataset_(std::string video_path, std::string labels_path,
                       std::vector<float> *features,
                       std::vector<float> labels) {
  std::unique_ptr<VideofileSource> input_video;

  try {
    input_video = std::make_unique<VideofileSource>(video_path);
  } catch (ReturnValue &ret) {
    printf("(%d): %s\n", static_cast<int>(ret.code), ret.description.c_str());
  }

  int processed_frames = 0;
  const int total_frames = 200;
  Buffer<uint8_t> frame = {0};
  ReturnValue ret;

  /* Loop to process frames */
  printf("Starting processing loop\n");
  while (processed_frames < total_frames) {
    printf("Pulling frame: %d\n", processed_frames);

    ret = input_video->pullFrame(&frame);
    if (ReturnCode::Success != ret.code) {
      printf("(%d): %s\n", static_cast<int>(ret.code), ret.description.c_str());
    }

    printf("Generating feature\n");
#ifdef PROF_BY_FRAME
    uint64_t now = GetCurrentTimeSinceEpochUs();
#endif
    if (!generate_frame_features(frame.data, frame.width, frame.height,
                                 features)) {
      /** FIXME: Perhaps ensure no memory leaks before throwing error */
      throw std::runtime_error("Feature generation failed");
    }
#ifdef PROF_BY_FRAME
    printf("Frame feature extraction duration: %ld\n",
           GetCurrentTimeSinceEpochUs() - now);
#endif

    ret = input_video->pushFrame(&frame);
    if (ReturnCode::Success != ret.code) {
      printf("(%d): %s\n", static_cast<int>(ret.code), ret.description.c_str());
    }

    ++processed_frames;
  }

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
                      std::vector<float> *features, std::vector<float> labels) {
  GstElement *input_pipeline = NULL;
  int processed_frames = 0;
  GstBuffer *input_buffer = NULL;
  GError *gst_error = NULL;

  printf("Initializing gstreamer\n");
  gst_init(0, 0);

  /* Build the pipelines */
  printf("Building input pipeline from video: %s\n", video_path.c_str());
  input_pipeline = gst_parse_launch(
      format(
          "filesrc location=%s !"
          "decodebin ! queue ! appsink sync=false max-buffers=6 name=sink -e",
          video_path.c_str())
          .c_str(),
      &gst_error);
  if (!input_pipeline) {
    printf("Gst parse pipeline error: %s\n", gst_error->message);
    return false;
  }

  printf("Initializing pipeline wrapper for input pipeline\n");
  Pipeline InputPipeline(input_pipeline);

  printf("Starting pipeline wrapper for input pipeline\n");
  InputPipeline.start_pipeline("sink");
  int total_frames = 200;

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
    uint8_t *data = static_cast<uint8_t *>(InputPipeline.map.data);
#ifdef PROF_BY_FRAME
    uint64_t now = GetCurrentTimeSinceEpochUs();
#endif
    if (!generate_frame_features(data, width, height, features)) {
      /** FIXME: Perhaps ensure no memory leaks before throwing error */
      throw std::runtime_error("Feature generation failed");
    }
#ifdef PROF_BY_FRAME
    printf("Frame feature extraction duration: %ld\n",
           GetCurrentTimeSinceEpochUs() - now);
#endif

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
int run_generator(int argc, char **argv, std::ostream &verbose_out) {
  if (argc != 4) {
    printf(
        "Usage feature_generator [LOSSY_VID_FILE] [LABEL_FILE] "
        "[OUTPUT_FILE]\n");
    return 1;
  }

  std::vector<float> features;
  std::vector<float> labels;

  if (!generate_dataset_(argv[1], argv[2], &features, labels)) {
    printf("Data generation failed\n");
    return 1;
  }

  if (!save_frame(argv[3], features.data(), features.size())) {
    printf("Saving feature file failed\n");
    return 1;
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
  } catch (std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}
