/* Copyright 2023 Francis Guindon <fbadilla10@gmail.com> */

#include "include/datasetloader.hpp"

#include <algorithm>
#include <cstdio>
#include <cstring>

#include "include/utility.hpp"

DatasetLoader::DatasetLoader(std::string dataset_path)
    : dataset_path_{dataset_path},
      ret_{ReturnCode::Success, "Videofilesrc initialized correctly"},
      consts_{},
      raw_feature_mem_{},
      raw_label_mem_{},
      aligned_label_mem_{},
      balanced_feature_mem_{},
      balanced_label_mem_{} {
  HANDLE_WITH_THROW(allocate(),
                    ReturnValue(ReturnCode::MemoryError,
                                "Could not allocate dataset memory"));

  HANDLE_WITH_THROW(loadRawDataset(), ReturnValue(ReturnCode::MemoryError,
                                                  "Could not load dataset"));

  HANDLE_WITH_THROW(
      generateRandomizedBalancedDataset(),
      ReturnValue(ReturnCode::MemoryError,
                  "Could not generate randomized balanced dataset"));
}

ReturnValue DatasetLoader::allocate() {
  consts_.setNumFiles(dataset_path_.feature_files().size());

  raw_feature_mem_.size = consts_.total_feature_mem_size;
  HANDLE_WITH(raw_feature_mem_.allocate(),
              ReturnValue(ReturnCode::MemoryError,
                          "Could not allocate raw feature mem"));

  raw_label_mem_.size = consts_.total_label_mem_size;
  HANDLE_WITH(
      raw_label_mem_.allocate(),
      ReturnValue(ReturnCode::MemoryError, "Could not allocate raw label mem"));

  aligned_label_mem_.size = consts_.total_aligned_label_mem_size;
  HANDLE_WITH(aligned_label_mem_.allocate(),
              ReturnValue(ReturnCode::MemoryError,
                          "Could not allocate aligned label mem"));

  return ret_;
}

ReturnValue DatasetLoader::loadRawDataset() {
  for (size_t i = 0; i < consts_.num_files; ++i) {
    std::string feature_path = dataset_path_.feature_files()[i];
    std::string label_path = dataset_path_.label_files()[i];
    float *feature_ptr = raw_feature_mem_.data + i * consts_.feature_file_size;
    uint8_t *label_ptr = raw_label_mem_.data + i * consts_.label_file_size;
    float *aligned_label_ptr =
        aligned_label_mem_.data + i * consts_.num_examples_per_file;
    printf("Loading feature file: %s\n", feature_path.c_str());
    load_frame(feature_path, feature_ptr, consts_.feature_file_size);
    printf("Loading label file: %s\n", label_path.c_str());
    load_frame(label_path, label_ptr, consts_.label_file_size);

    HANDLE_WITH(
        alignLabels(label_ptr, aligned_label_ptr),
        ReturnValue(
            ReturnCode::MemoryError,
            "Load raw dataset failed while aligning labels to feature order"));
  }

  return ret_;
}

ReturnValue check_valid(const uint8_t *label) {
  if (0x00 != *label && 0xFF != *label) {
    return ReturnValue(ReturnCode::MemoryError,
                       "Label: '" + std::to_string(*label) + "' invalid");
  }

  return ReturnValue(ReturnCode::Success,
                     "Label: '" + std::to_string(*label) + "' is valid");
}

ReturnValue DatasetLoader::alignLabels(const uint8_t *raw,
                                       float *const aligned) {
  float *out_ptr = aligned;
  for (size_t i = 0; i < consts_.num_frames_per_snip; ++i) {
    // int ii = i;

    for (size_t j = 1; j < consts_.blocks_per_col - 1; ++j) {
      for (size_t k = 1; k < consts_.blocks_per_row - 1; ++k) {
        size_t offset =
            i * consts_.blocks_per_frame + j * consts_.blocks_per_row + k;
        const uint8_t *label = raw + offset;
        HANDLE_WITH(check_valid(label),
                    ReturnValue(ReturnCode::MemoryError, "Label is not valid"));
        *out_ptr = static_cast<float>(*label);
        out_ptr++;
      }
    }
  }
  if (aligned + consts_.num_examples_per_file != out_ptr) {
    return {ReturnCode::MemoryError,
            "Iteration count and Aligned feature size mismatch"};
  }

  return ret_;
}

ReturnValue DatasetLoader::generateRandomizedBalancedDataset() {
  HANDLE_WITH(
      generateBalancedDataset(),
      ReturnValue(ReturnCode::MemoryError, "Failed to balance dataset"));

  HANDLE_WITH(randomizeBalancedDataset(),
              ReturnValue(ReturnCode::MemoryError,
                          "Failed to randomize balanced dataset"));

  return ret_;
}

ReturnValue DatasetLoader::generateBalancedDataset() {
  INFO("Measuring raw dataset balance");
  size_t cnt_positives = 0;
  size_t cnt_negatives = 0;
  auto neg_then_pos_ids =
      std::unique_ptr<size_t[]>(new size_t[consts_.total_label_mem_size]);
  for (size_t i = 0; i < consts_.total_label_mem_size; ++i) {
    float label = aligned_label_mem_.data[i];
    if (0.0F == label) {
      neg_then_pos_ids[cnt_negatives] = i;  // Fill neg ids from beginning
      ++cnt_negatives;
    } else if (255.0F == label) {
      neg_then_pos_ids[consts_.total_label_mem_size - 1 - cnt_positives] =
          i;  // Fill pos ids from end
      ++cnt_positives;
    } else {
      /** Perhaps bug is in alignmend uint8_t to float conversion */
      ERROR("Found invalid label value (%d) at index: %ld\n",
            static_cast<int>(aligned_label_mem_.data[i]), i);
      return {ReturnCode::MemoryError, "Class separation failed"};
    }
  }
  float negative_percent = static_cast<float>(cnt_negatives) /
                           static_cast<float>(consts_.total_label_mem_size);
  float positive_percent = static_cast<float>(cnt_positives) /
                           static_cast<float>(consts_.total_label_mem_size);
  printf("Negatives: %ld, Positives: %ld, Total: %ld, Balance: %3.2f : %3.2f\n",
         cnt_negatives, cnt_positives, consts_.total_label_mem_size,
         negative_percent, positive_percent);
  printf("Randomizing negative and positive ids separately\n");
  std::random_shuffle(neg_then_pos_ids.get(),
                      neg_then_pos_ids.get() + cnt_negatives);
  std::random_shuffle(neg_then_pos_ids.get() + cnt_negatives,
                      neg_then_pos_ids.get() + consts_.total_label_mem_size);

  float balance = 0.5F;
  printf("Adjusting dataset balance to %3.2f%% pos\n", balance);
  float pos_to_neg_factor = (1 - balance) / balance;
  size_t neg_dataset_size = (balance > positive_percent)
                                ? cnt_positives * pos_to_neg_factor
                                : cnt_negatives;
  size_t pos_dataset_size = (balance > positive_percent)
                                ? cnt_positives
                                : cnt_negatives / pos_to_neg_factor;
  printf("Balanced neg cnt: %ld, pos cnt: %ld\n", neg_dataset_size,
         pos_dataset_size);
  size_t total_dataset_size = neg_dataset_size + pos_dataset_size;

  auto x_mem_balanced = std::unique_ptr<float[]>(
      new float[total_dataset_size * consts_.feature_vector_size]);
  auto y_mem_balanced = std::unique_ptr<float[]>(new float[total_dataset_size]);

  for (size_t i = 0; i < neg_dataset_size; ++i) {
    void *raw_feature = reinterpret_cast<void *>(
        raw_feature_mem_.data +
        neg_then_pos_ids[i] * consts_.feature_vector_size);
    void *balanced_feature = reinterpret_cast<void *>(
        balanced_feature_mem_.data + i * consts_.feature_vector_size);
    memcpy(balanced_feature, raw_feature,
           sizeof(float) * consts_.feature_vector_size);
    balanced_label_mem_.data[i] = aligned_label_mem_.data[neg_then_pos_ids[i]];
  }

  for (size_t i = 0; i < pos_dataset_size; ++i) {
    void *raw_feature = reinterpret_cast<void *>(
        raw_feature_mem_.data +
        neg_then_pos_ids[i + cnt_negatives] * consts_.feature_vector_size);
    void *balanced_feature = reinterpret_cast<void *>(
        balanced_feature_mem_.data +
        (i + neg_dataset_size) * consts_.feature_vector_size);
    memcpy(balanced_feature, raw_feature,
           sizeof(float) * consts_.feature_vector_size);
    balanced_label_mem_.data[i + neg_dataset_size] =
        aligned_label_mem_.data[neg_then_pos_ids[i + cnt_negatives]];
  }

  return ret_;
}

ReturnValue DatasetLoader::randomizeBalancedDataset() { return ret_; }
