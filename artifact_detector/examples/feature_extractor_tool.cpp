/* Copyright 2023 Francis Guindon <fbadilla10@gmail.com> */

#include <memory>

#include "include/feature_extractor.hpp"
#include "include/types.hpp"

int main(int argc, char **argv) {
  auto extractor = std::make_unique<FeatureExtractor>();

  Buffer<uint8_t> frame = {nullptr, 0, 0, 0};
  Buffer<float> features = {nullptr, 0, 0, 0};

  ReturnValue error = extractor->extractFeatures(&frame, &features);

  return 0;
}
