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
//#include

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

/**
 * @brief Converts string to a GstElement with pipeline.
 *
 * @tparam Args.
 * @param format: string with the pipeline to be build.
 * @param args: arguments to format the string with.
 * @return GstElement*: pipeline element.
 */
template <typename... Args>
GstElement *build_pipeline(const std::string &format, Args... args) {
  int size_s = std::snprintf(nullptr, 0, format.c_str(), args...) + 1;
  auto size = static_cast<size_t>(size_s);
  std::unique_ptr<char[]> buf(new char[size]);
  std::snprintf(buf.get(), size, format.c_str(), args...);
  std::string tmp_str = std::string(buf.get(), buf.get() + size - 1);
  const char *pipeline_char = tmp_str.c_str();
  printf("%s\n", pipeline_char);
  GstElement *pipeline = NULL;
  pipeline = gst_parse_launch(pipeline_char, NULL);

  return pipeline;
}

#define ABS_DIFF(in1, in2) in1 > in2 ? in1 - in2 : in2 - in1

#define SUM_2_BLOCK_ROWS(dst, src, step, n0, n1) ( \
  (dst) += *(src + n0*step) + *(src + n1*step)     \
)

#define PRINT_2_BLOCK_ROWS(src, step, n0, n1) ( \
  printf("%d %d\n",*(src + n0*step), *(src + n1*step)) \
)

#define PRINT_8_BLOCK_ROWS(src, step, n0, n1, n2, n3, n4, n5, n6, n7) (          \
  printf("%d %d %d %d %d %d %d %d\n",                                            \
         *(src + n0*step), *(src + n1*step), *(src + n2*step), *(src + n3*step), \
         *(src + n4*step), *(src + n5*step), *(src + n6*step), *(src + n7*step)) \
)

#define SUM_4_BLOCK_ROWS(dst, src, step, n0, n1, n2, n3) ( \
  (dst) += *(src + n0*step) + *(src + n1*step)             \
         + *(src + n2*step) + *(src + n3*step)             \
)

#define SUM_8_BLOCK_ROWS(dst, src, step, n0, n1, n2, n3, n4, n5, n6, n7) ( \
  (dst) += *(src + n0*step) + *(src + n1*step)                             \
         + *(src + n2*step) + *(src + n3*step)                             \
         + *(src + n4*step) + *(src + n5*step)                             \
         + *(src + n6*step) + *(src + n7*step)                             \
)

#define SQ_SUM_2_BLOCK_ROWS(dst, src, step, n0, n1) ( \
  (dst) += (*(src + n0*step))*(*(src + n0*step))      \
         + (*(src + n1*step))*(*(src + n1*step))      \
)

#define SQ_SUM_4_BLOCK_ROWS(dst, src, step, n0, n1, n2, n3) ( \
      (dst) += (*(src + n0*step))*(*(src + n0*step))          \
             + (*(src + n1*step))*(*(src + n1*step))          \
             + (*(src + n2*step))*(*(src + n2*step))          \
             + (*(src + n3*step))*(*(src + n3*step))          \
)

#define SQ_SUM_8_BLOCK_ROWS(dst, src, step, n0, n1, n2, n3, n4, n5, n6, n7) ( \
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
                  size_t width, size_t height) {
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
    *ver_elem_ptr = 0;
    *hor_elem_ptr = ABS_DIFF(*in_elem_ptr, *(in_elem_ptr + 1));
  }

  return true;
}

/**
 * @brief Generate feature for single frame
 * 
 * @param data           : Frame data
 * @param width          : Frame width
 * @param height         : Frame height
 * @param feature_vector : 
 * @param size           : Feature vector size
 * @returns bool         : Operation status
 */
/* FIXME: Abstract whole procedure to class perhaps*/
bool generate_frame_features(const uint8_t *data, const size_t width,
                      const size_t height, std::vector<float> features) {

  /* Allocate vertical filter and horizontal filter */
  auto vertical_filtered = std::unique_ptr<uint8_t>(new uint8_t[width*height]);
  auto horizontal_filtered = std::unique_ptr<uint8_t>(new uint8_t[width*height]);

  filter_frame(data, vertical_filtered.get(), horizontal_filtered.get(), width, height);

  /* Allocate feature memory ... not necessary, memory is parameter */
  /* which feature first? */
  /* how about top border of first macroblock at 1 pixel width */
  /* how to calculate offset and size? */
  /* pixel(i,j) = i*width + j goes pixel by pixel row major */
  /* pixel(i,j) = j*height + i goes pixel by pixel col major ... */
  /* macroblock_topleftcorner(bi,bj) = 
            bi*block_width*num_blocks_in_row + bj*block_height */
  constexpr size_t kRowsPerBlocklength = 16;
  constexpr size_t kPixelsPerBlocklength = 16;
  const size_t kPixelsPerRow = width;
  // const size_t kPixelsPerCol = height;
  
  size_t block_i = 1; /* Measured in blocklengths */
  size_t block_j = 1; /* Measured in blocklengths */
  size_t block_offset = block_i*kRowsPerBlocklength*kPixelsPerRow +
                        block_j*kPixelsPerBlocklength;

  constexpr size_t kMetricsPerBorder = 8;
  size_t* sums = new size_t[kMetricsPerBorder];
  size_t* sq_sums = new size_t[kMetricsPerBorder];

  memset(sums, 0, sizeof(size_t)*kMetricsPerBorder);
  memset(sq_sums, 0, sizeof(size_t)*kMetricsPerBorder);

  uint8_t *vptr = vertical_filtered.get() + block_offset;
  uint8_t *hptr = horizontal_filtered.get() + block_offset;
  const uint8_t *vptr_end = vptr + kPixelsPerBlocklength;

  printf("Original image range\n");
  const uint8_t *in_elem_ptr = data + block_offset;
  const uint8_t *in_elem_ptr_end = in_elem_ptr + kPixelsPerBlocklength;
  for (; in_elem_ptr != in_elem_ptr_end; ++in_elem_ptr) {
    PRINT_8_BLOCK_ROWS(in_elem_ptr, width, -4, -3, -2, -1, 0, 1, 2, 3);
  }

  printf("Horizontal range\n");
  for (; vptr != vptr_end; ++vptr, ++hptr) {
    PRINT_8_BLOCK_ROWS(hptr, width, -4, -3, -2, -1, 0, 1, 2, 3);
  }
  vptr = vertical_filtered.get() + block_offset;
  hptr = horizontal_filtered.get() + block_offset;

  printf("Vertical range\n");
  for (; vptr != vptr_end; ++vptr, ++hptr) {
    PRINT_8_BLOCK_ROWS(vptr, width, -4, -3, -2, -1, 0, 1, 2, 3);
  }
  vptr = vertical_filtered.get() + block_offset;
  hptr = horizontal_filtered.get() + block_offset;
  
  /* Calculate sums for top border of block at block_offset */
  for (; vptr != vptr_end; ++vptr, ++hptr) {
    SUM_2_BLOCK_ROWS(sums[0], vptr, width, 0, -1);
    SUM_2_BLOCK_ROWS(sums[1], vptr, width, 1, -2);
    SUM_4_BLOCK_ROWS(sums[2], vptr, width, 2, 3, -3, -4);
    SUM_8_BLOCK_ROWS(sums[3], vptr, width, 4, 5, 6, 7, -5, -6, -7, -8);
    SUM_2_BLOCK_ROWS(sums[4], hptr, width, 0, -1);
    SUM_2_BLOCK_ROWS(sums[5], hptr, width, 1, -2);
    SUM_4_BLOCK_ROWS(sums[6], hptr, width, 2, 3, -3, -4);
    SUM_8_BLOCK_ROWS(sums[7], hptr, width, 4, 5, 6, 7, -5, -6, -7, -8);

    SQ_SUM_2_BLOCK_ROWS(sq_sums[0], vptr, width, 0, -1);
    SQ_SUM_2_BLOCK_ROWS(sq_sums[1], vptr, width, 1, -2);
    SQ_SUM_4_BLOCK_ROWS(sq_sums[2], vptr, width, 2, 3, -3, -4);
    SQ_SUM_8_BLOCK_ROWS(sq_sums[3], vptr, width, 4, 5, 6, 7, -5, -6, -7, -8);
    SQ_SUM_2_BLOCK_ROWS(sq_sums[4], hptr, width, 0, -1);
    SQ_SUM_2_BLOCK_ROWS(sq_sums[5], hptr, width, 1, -2);
    SQ_SUM_4_BLOCK_ROWS(sq_sums[6], hptr, width, 2, 3, -3, -4);
    SQ_SUM_8_BLOCK_ROWS(sq_sums[7], hptr, width, 4, 5, 6, 7, -5, -6, -7, -8);
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
  float* means = new float[kMetricsPerBorder];
  float* vars = new float[kMetricsPerBorder];
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

  /* Test if values are correct */
  printf("means = [%f, %f, %f, %f, %f, %f, %f, %f]\n",
         means[0], means[1], means[2], means[3],
         means[1], means[2], means[3], means[4]);
  printf("vars = [%f, %f, %f, %f, %f, %f, %f, %f]\n",
         vars[0], vars[1], vars[2], vars[3],
         vars[4], vars[5], vars[6], vars[7]);


  // features.push_back();
  delete[] means;
  delete[] vars;
  delete[] sums;
  delete[] sq_sums;

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
                      std::vector<float> features, std::vector<float> labels) {
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
