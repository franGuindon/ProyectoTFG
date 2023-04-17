/* Copyright 2023 Francis Guindon <fbadilla10@gmail.com> */

#ifndef ARTIFACT_DETECTOR_INCLUDE_FEATURE_EXTRACTOR_HPP_
#define ARTIFACT_DETECTOR_INCLUDE_FEATURE_EXTRACTOR_HPP_

#include "include/types.hpp"

/**
 * @brief The FeatureExtractor class takes an image frame and extracts the
 * features used in the Random Decision Forest
 */
class FeatureExtractor {
 public:
  /**
   * @brief Construct a FeatureExtractor
   *
   */
  FeatureExtractor();

  /**
   * @brief Extracts features from an input image
   *
   * @param input_frame  : Image frame to extract features from
   * @param features     : Extracted features
   * @return ReturnValue : Operation status
   */
  ReturnValue extractFeatures(Buffer<uint8_t> *input_frame,
                              Buffer<float> *features);
};

#endif  // ARTIFACT_DETECTOR_INCLUDE_FEATURE_EXTRACTOR_HPP_
