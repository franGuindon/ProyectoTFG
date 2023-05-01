/* Copyright 2023 Francis Guindon <fbadilla10@gmail.com> */

#include <cstdio>
#include <cstring>
#include <memory>
#include <string>

#include "include/utility.hpp"

int main(int argc, char **argv) {
  if (3 != argc) {
    printf("Usage: test_data_generation_tool [LABEL_FILE] [OUTPUT_FILE]\n");
    return 1;
  }

  std::string label_file = argv[1];
  std::string output_file = argv[2];

  size_t block_dim = 16;
  size_t width = 1280;
  size_t height = 720;
  size_t num_frames = 200;
  size_t num_blocks_per_row = width / block_dim;
  size_t num_blocks_per_col = height / block_dim;
  size_t num_blocks_per_frame = num_blocks_per_col * num_blocks_per_row;
  size_t labels_size = num_blocks_per_col * num_blocks_per_row * num_frames;
  size_t vals_per_feature = 132;
  size_t features_size = vals_per_feature * (num_blocks_per_col - 2) *
                         (num_blocks_per_row - 2) * num_frames;
  auto labels = std::unique_ptr<uint8_t[]>(new uint8_t[labels_size]);
  auto features = std::unique_ptr<float[]>(new float[features_size]);

  load_frame(label_file, labels.get(), labels_size);

  size_t feature_cnt = 0;
  for (size_t i = 0; i < num_frames; ++i) {
    for (size_t j = 1; j < num_blocks_per_row - 1; ++j) {
      for (size_t k = 1; k < num_blocks_per_row - 1; ++k) {
        uint8_t label_val =
            labels[i * num_blocks_per_frame + j * num_blocks_per_row + k];

        if (0 != label_val && 255 != label_val) {
          printf("Error, invalid value labels[%ld,%ld,%ld] = %d\n", i, j, k,
                 label_val);
          return 1;
        }

        for (size_t l = 0; l < vals_per_feature; ++l) {
          features[vals_per_feature * feature_cnt + l] =
              static_cast<float>(label_val);
        }

        ++feature_cnt;
      }
    }
  }
  printf("\n");

  save_frame(output_file, features.get(), features_size);

  return 0;
}
