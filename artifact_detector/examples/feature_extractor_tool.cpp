/* Copyright 2023 Francis Guindon <fbadilla10@gmail.com> */

#include <memory>

#include "include/feature_extractor.hpp"
#include "include/types.hpp"
#include "include/videofilesource.hpp"

int main(int argc, char **argv) {
  ReturnValue ret;

  auto file_src = std::make_unique<VideofileSource>("file.mp4");
  auto extractor = std::make_unique<FeatureExtractor>();

  Buffer<uint8_t> frame = {nullptr, 0, 0, 0};
  Buffer<float> features = {nullptr, 0, 0, 0};

  ret = file_src->pullFrame(&frame);
  ret = extractor->extractFeatures(&frame, &features);
  ret = file_src->pushFrame(&frame);

  return 0;
}
