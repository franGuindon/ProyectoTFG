/* Copyright 2023 Francis Guindon <fbadilla10@gmail.com> */

#include "include/feature_extractor.hpp"

FeatureExtractor::FeatureExtractor() {}

ReturnValue FeatureExtractor::extractFeatures(Buffer<uint8_t> *input_frame,
                                              Buffer<float> *features) {
  return ReturnValue{ReturnCode::Success, "Operation returned successfully"};
}
