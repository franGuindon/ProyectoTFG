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

/**
 * @brief Calculate absolute diference of two values using ternary operator
 * 
 * @param in1       : First input
 * @param in2       : Second input
 * @returns uint8_t : Absolute difference
 */
inline uint8_t absolute_difference(const uint8_t in1, const uint8_t in2) {
  return in1 > in2 ? in1 - in2 : in2 - in1;
}

/**
 * @brief Generate feature for single frame
 * 
 * @param map            : Frame data
 * @param width          : Frame width
 * @param height         : Frame height
 * @param feature_vector : 
 * @param size           : Feature vector size
 * @returns bool         : Operation status
 */
/* FIXME: Change name to generate_frame_features */
/* FIXME: Remove gstreamer from function */
/* FIXME: Abstract whole procedure to class perhaps*/
bool generate_feature(const GstMapInfo map, const size_t width,
                      const size_t height, std::vector<float> features) {
  uint8_t* data = static_cast<uint8_t*>(map.data);

  /* Allocate vertical filter and horizontal filter */
  auto vertical_filtered = std::unique_ptr<int32_t>(new int[width*height]);
  auto horizontal_filtered = std::unique_ptr<int32_t>(new int[width*height]);

  /* Calculate filters*/
  uint8_t* in_row_ptr = data;
  int32_t* ver_row_ptr = vertical_filtered.get();
  int32_t* hor_row_ptr = horizontal_filtered.get();

  uint8_t* in_elem_ptr = nullptr;
  int32_t* ver_elem_ptr = nullptr;
  int32_t* hor_elem_ptr = nullptr;

  
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
      *ver_elem_ptr++ = absolute_difference(*in_elem_ptr, *(in_elem_ptr + width));
      *hor_elem_ptr++ = absolute_difference(*in_elem_ptr, *(in_elem_ptr + 1));
    }

    /* Last element of row (not considered in loop) */
    *ver_elem_ptr = absolute_difference(*in_elem_ptr, *(in_elem_ptr + width));
    *hor_elem_ptr = 0;
  }

  /* Last row in image (not considered in loop) */
  for (; in_elem_ptr != in_row_ptr_end - 1; ++in_elem_ptr) {
    *ver_elem_ptr = 0;
    *hor_elem_ptr = absolute_difference(*in_elem_ptr, *(in_elem_ptr + 1));
  }

  /* Filter test */
  /* FIXME: refactor test into unit */
  // uint8_t* test_in_ptr = data + 100*width + 100;
  // int32_t* test_out_ptr = horizontal_filtered.get() + 100*width + 100;
  // printf("|%d-%d|=%d\n", *test_in_ptr, *(test_in_ptr+1), *test_out_ptr);
  // test_out_ptr = vertical_filtered.get() + 100*width + 100;
  // printf("|%d-%d|=%d\n", *test_in_ptr, *(test_in_ptr+width), *test_out_ptr);

  /* Log */
  /* FIXME: refactor log into unit*/
  /* I trust these filtered results
   * I must now calculate features.
   * Rectanges at 1,2,4,8 pixels away from each border
   * 
   */

  /* Allocate feature memory ... not necessary, memory is parameter */
  /* which feature first? */
  /* how about top border of first macroblock at 1 pixel width */
  /* how to calculate offset and size? */
  /* pixel(i,j) = i*width + j goes pixel by pixel row major */
  /* pixel(i,j) = j*height + i goes pixel by pixel col major ... */
  /* macroblock_topleftcorner(bi,bj) = 
            bi*block_width*num_blocks_in_row + bj*block_height */
  // constexpr size_t kRowsPerBlocklength = 16;
  // constexpr size_t kPixelsPerBlocklength = 16;
  // const size_t kPixelsPerRow = width;
  // const size_t kPixelsPerCol = height;
  
  // size_t block_i = 1; /* Measured in blocklengths */
  // size_t block_j = 1; /* Measured in blocklengths */
  // size_t block_offset = block_i*kRowsPerBlocklength*kPixelsPerRow +
  //                       block_j*kPixelsPerBlocklength;
  // size_t sum = 0;
  // size_t sum_sq = 0;

  // /* Calculate sum for block row of block_offset */
  // for (size_t i = 0; i < kPixelsPerBlocklength; ++i) {
  //   sum += vertical_filtered.get()[i];
  // }
  
  // features.push_back();

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
  int total_frames = 200;

  /* Loop to process frames */
  printf("Starting processing loop\n");
  while (processed_frames < total_frames) {
    printf("Processing frame: %d\n", processed_frames);

    int width = 0;
    int height = 0;

    printf("Pulling buffer\n");

    input_buffer = InputPipeline.pull_buffer(width, height);

    printf("Unmapping buffer\n");

    /* Use InputPipeline.map to process frame info */
    printf("Generating feature\n");
    if (!generate_feature(InputPipeline.map, width, height, features)) {
      /** FIXME: Perhaps ensure no memory leaks before throwing error */
      throw std::runtime_error("Feature generation failed");
    }

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
