/*
 * Copyright (C) 2022 RidgeRun, LLC (http://www.ridgerun.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef BUFFER_UTILS_H
#define BUFFER_UTILS_H

#include "pipeline.h"

#include <algorithm>
#include <fstream>
#include <vector>

/**
 * @brief Get the histogram data
 *
 * @param value : Each pixel value to add to dataset.
 */
void get_histogram(int value);

/**
 * @brief Save pixels difference and its frequencies.
 *
 * @param values : Difference value from original and damaged frame.
 * @param frequencies : Data repetition rates.
 */
void save_data(std::vector<int> values, std::vector<int> frequencies);

/**
 * @brief Performs the macroblocks visualization of input difference map.
 *
 * @param map : map data to with input frame info.
 * @param height : height of the given frame.
 * @param width : width of the given frame.
 * @param threshold : int value to define what section is or not macroblock.
 */
void pix_2_macroblocks(GstMapInfo map, const int height, const int width,
                       const int threshold);

/**
 * @brief Get difference between two given buffers.
 *
 * @param refmap : Map info from the reference buffer.
 * @param plmap : Map info from the buffer to compare.
 * @param timestamp : Timestamp to attach in output buffer.
 * @return GstBuffer* : Buffer with substraction of the given inputs.
 */
GstBuffer* subtract_frames(GstMapInfo refmap, GstMapInfo plmap,
                           GstClockTime timestamp);

/**
 * @brief Get the sample width.
 *
 * @param sample : GstSample to get width from.
 * @return int : width of the given sample.
 */
int get_sample_width(GstSample* sample);

/**
 * @brief Get the sample height.
 *
 * @param sample : GstSample to get height from.
 * @return int : height of the given sample.
 */
int get_sample_height(GstSample* sample);

#endif
