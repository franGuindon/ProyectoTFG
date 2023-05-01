/* Copyright 2023 Francis Guindon <fbadilla10@gmail.com> */

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <memory>
#include <numeric>

#define DEBUG_LEVEL 3
#include "ForestClassification.h"
#include "artifact_detector/include/datasetloader.hpp"
#include "artifact_detector/include/logging.hpp"
#include "artifact_detector/include/utility.hpp"

using ranger::DEFAULT_ALPHA;
using ranger::DEFAULT_IMPORTANCE_MODE;
using ranger::DEFAULT_MAXDEPTH;
using ranger::DEFAULT_MINPROP;
using ranger::DEFAULT_NUM_RANDOM_SPLITS;
using ranger::DEFAULT_NUM_THREADS;
using ranger::DEFAULT_NUM_TREE;
using ranger::DEFAULT_PREDICTIONTYPE;
using ranger::DEFAULT_SPLITRULE;
using ranger::EXTRATREES;
using ranger::ForestClassification;
using ranger::ImportanceMode;
using ranger::MEM_DOUBLE;
using ranger::MemoryMode;
using ranger::PredictionType;
using ranger::SplitRule;

typedef ForestClassification ForestRangerx;

struct Args {
  bool verbose = true;
  std::string outprefix = "";
  std::string depvarname = "";
  MemoryMode memmode = MEM_DOUBLE;
  std::string file = "test_data.dat";
  uint mtry = 0;
  uint ntree = DEFAULT_NUM_TREE;
  uint seed = 0;
  uint nthreads = DEFAULT_NUM_THREADS;
  std::string predict = "";
  ImportanceMode impmeasure = DEFAULT_IMPORTANCE_MODE;
  uint targetpartitionsize = 0;
  std::string splitweights = "";
  std::vector<std::string> alwayssplitvars = {};
  std::string statusvarname = "";
  bool replace = true;
  std::vector<std::string> catvars = {};
  bool savemem = false;
  SplitRule splitrule = DEFAULT_SPLITRULE;
  std::string caseweights = "";
  bool predall = false;
  double fraction = 0;
  double alpha = DEFAULT_ALPHA;
  double minprop = DEFAULT_MINPROP;
  bool holdout = false;
  PredictionType predictiontype = DEFAULT_PREDICTIONTYPE;
  uint randomsplits = DEFAULT_NUM_RANDOM_SPLITS;
  uint maxdepth = DEFAULT_MAXDEPTH;
  std::vector<double> regcoef = {};
  bool usedepth = false;
  bool skipoob = false;
  bool write = false;
};

int main(int argc, char **argv) {
  // if (argc != 3) {
  //   printf("Usage: trainer [OUTPUT_PREFIX] [DATASET_PATH] \n");
  //   return 1;
  // }
  // std::string output_prefix = argv[1];
  // std::string dataset_path = argv[2];

  // INFO("Constructing dataset");
  // std::unique_ptr<DatasetLoader> dataset;
  // try {
  //   dataset = std::make_unique<DatasetLoader>(dataset_path);
  // } catch (ReturnValue &ret) {
  //   ERROR("%s", ret.str().c_str());
  // }

  // return 0;

  if (argc < 4 || argc % 2 != 0) {
    printf("Usage: trainer [OUTPUT_PREFIX] [FEATURE_FILE] [LABEL_FILE] ... \n");
    printf("  An arbitrary number of feature - label file pairs may be used\n");
    return 1;
  }

  auto forest = std::make_unique<ForestRangerx>();

  //----------------------------------------------------------------------------
  // Tree args

  Args arg_handler{};
  arg_handler.outprefix = argv[1];
  arg_handler.verbose = true;
  arg_handler.depvarname = "y";
  arg_handler.ntree = 32;
  arg_handler.nthreads = 32;
  arg_handler.write = true;
  arg_handler.maxdepth = 10;
  arg_handler.splitrule = EXTRATREES;
  arg_handler.randomsplits = 50;

  //----------------------------------------------------------------------------
  // Data load

  uint64_t now = GetCurrentTimeSinceEpochUs();

  const size_t block_dimension = 16;
  const size_t frame_width = 1280;
  const size_t frame_height = 720;
  const size_t blocks_per_row = frame_width / block_dimension;
  const size_t blocks_per_col = frame_height / block_dimension;
  const size_t blocks_per_frame = blocks_per_row * blocks_per_col;
  const size_t num_frames_per_snip = 200;
  const size_t features_per_block = 132;

  const size_t label_file_size =
      blocks_per_row * blocks_per_col * num_frames_per_snip;
  const size_t num_rows =
      (blocks_per_row - 2) * (blocks_per_col - 2) * num_frames_per_snip;
  const size_t num_cols = features_per_block;
  const size_t feature_file_size = num_rows * num_cols;
  const size_t num_file_pairs = (argc - 2) / 2;
  const size_t total_feature_mem_size = feature_file_size * num_file_pairs;
  const size_t total_raw_label_mem_size = label_file_size * num_file_pairs;
  const size_t total_label_mem_size = num_rows * num_file_pairs;

  auto x_mem = std::unique_ptr<float[]>(new float[total_feature_mem_size]);
  auto block_mem = std::unique_ptr<unsigned char[]>(
      new unsigned char[total_raw_label_mem_size]);
  auto y_mem = std::unique_ptr<float[]>(new float[total_label_mem_size]);

  size_t y_offset = 0;
  for (size_t i = 0; i < num_file_pairs; ++i) {
    printf("Loading feature file: %s\n", argv[2 + 2 * i]);
    load_frame(argv[2 + 2 * i], x_mem.get() + i * feature_file_size,
               feature_file_size);
    printf("Loading label file: %s\n", argv[3 + 2 * i]);
    load_frame(argv[3 + 2 * i], block_mem.get() + i * label_file_size,
               label_file_size);

    printf("Building label memory\n");
    for (size_t j = 0; j < num_frames_per_snip; ++j) {
      for (size_t k = 1; k < blocks_per_col - 1; ++k) {
        for (size_t l = 1; l < blocks_per_row - 1; ++l) {
          size_t offset = i * label_file_size + j * blocks_per_frame +
                          k * blocks_per_row + l;
          y_mem[y_offset] = static_cast<float>(block_mem[offset]);
          ++y_offset;
        }
      }
    }
  }
  if (y_offset != total_label_mem_size) {
    printf("Mismatch between processed label size and total label size\n");
    return -1;
  }

  printf("Dataset loading duration: %ld\n", GetCurrentTimeSinceEpochUs() - now);

  //----------------------------------------------------------------------------
  // Data balancing

  now = GetCurrentTimeSinceEpochUs();

  printf("Measuring raw dataset balance\n");
  size_t cnt_positives = 0;
  size_t cnt_negatives = 0;
  auto neg_then_pos_ids =
      std::unique_ptr<size_t[]>(new size_t[total_label_mem_size]);
  for (size_t i = 0; i < total_label_mem_size; ++i) {
    if (0.0F == y_mem[i]) {
      neg_then_pos_ids[cnt_negatives] = i;  // Fill neg ids from beginning
      ++cnt_negatives;
    } else if (255.0F == y_mem[i]) {
      neg_then_pos_ids[total_label_mem_size - 1 - cnt_positives] =
          i;  // Fill pos ids from end
      ++cnt_positives;
    } else {
      printf("Error: Found invalid label value at index: %ld\n", i);
      return -1;
    }
  }
  float negative_percent = static_cast<float>(cnt_negatives) /
                           static_cast<float>(total_label_mem_size);
  float positive_percent = static_cast<float>(cnt_positives) /
                           static_cast<float>(total_label_mem_size);
  printf("Negatives: %ld, Positives: %ld, Total: %ld, Balance: %3.2f : %3.2f\n",
         cnt_negatives, cnt_positives, num_rows, negative_percent,
         positive_percent);
  printf("Randomizing negative and positive ids separately\n");
  std::random_shuffle(neg_then_pos_ids.get(),
                      neg_then_pos_ids.get() + cnt_negatives);
  std::random_shuffle(neg_then_pos_ids.get() + cnt_negatives,
                      neg_then_pos_ids.get() + total_label_mem_size);

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
      new float[total_dataset_size * features_per_block]);
  auto y_mem_balanced = std::unique_ptr<float[]>(new float[total_dataset_size]);

  for (size_t i = 0; i < neg_dataset_size; ++i) {
    void *raw_feature = reinterpret_cast<void *>(
        x_mem.get() + neg_then_pos_ids[i] * features_per_block);
    void *balanced_feature =
        reinterpret_cast<void *>(x_mem_balanced.get() + i * features_per_block);
    memcpy(balanced_feature, raw_feature, sizeof(float) * features_per_block);
    y_mem_balanced[i] = y_mem[neg_then_pos_ids[i]];
  }

  for (size_t i = 0; i < pos_dataset_size; ++i) {
    void *raw_feature = reinterpret_cast<void *>(
        x_mem.get() + neg_then_pos_ids[i + cnt_negatives] * features_per_block);
    void *balanced_feature = reinterpret_cast<void *>(
        x_mem_balanced.get() + (i + neg_dataset_size) * features_per_block);
    mempcpy(balanced_feature, raw_feature, features_per_block);
    y_mem_balanced[(i + neg_dataset_size)] =
        y_mem[neg_then_pos_ids[i + cnt_negatives]];
  }

  auto x_mem_balanced_randomized = std::unique_ptr<float[]>(
      new float[total_dataset_size * features_per_block]);
  auto y_mem_balanced_randomized =
      std::unique_ptr<float[]>(new float[total_dataset_size]);
  auto ids_balanced_randomized =
      std::unique_ptr<size_t[]>(new size_t[total_dataset_size]);

  std::iota(ids_balanced_randomized.get(),
            ids_balanced_randomized.get() + total_dataset_size, 0);
  std::random_shuffle(ids_balanced_randomized.get(),
                      ids_balanced_randomized.get() + total_dataset_size);

  for (size_t i = 0; i < total_dataset_size; ++i) {
    size_t new_id = ids_balanced_randomized[i];
    float *dst_ptr = x_mem_balanced_randomized.get() + i * features_per_block;
    float *src_ptr = x_mem_balanced.get() + new_id * features_per_block;
    memcpy(reinterpret_cast<void *>(dst_ptr), reinterpret_cast<void *>(src_ptr),
           sizeof(float) * features_per_block);
    y_mem_balanced_randomized[i] = y_mem_balanced[new_id];
  }

  printf("Dataset balancing duration: %ld\n",
         GetCurrentTimeSinceEpochUs() - now);

  //----------------------------------------------------------------------------
  // Generate black and white dataset matching labels
  // printf("Generating black and white dataset from label memory\n");
  printf("Converting feature memory from row major to column major\n");

  auto x_mem_balanced_randomized_colmajor = std::unique_ptr<float[]>(
      new float[total_dataset_size * features_per_block]);

  float *labels = y_mem_balanced_randomized.get();
  float *features = x_mem_balanced_randomized.get();
  float *features_cm = x_mem_balanced_randomized_colmajor.get();

  for (size_t i = 0; i < total_dataset_size; ++i) {
    float label = labels[i];
    if (0.0F != label && 255.0F != label) {
      printf("Error: Invalid labels[%ld] = %f\n", i, label);
      return 1;
    }

    for (size_t j = 0; j < features_per_block; ++j) {
      float feature = features[i * features_per_block + j];

      /* Check if valid white and black dataset */
      // if (feature != label) {
      //   printf("Error: (Feature[%ld,%ld]=%f) != (Label[%ld]=%f)\n", i, j,
      //          feature, i, label);
      //   return 1;
      // }
      features_cm[j * total_dataset_size + i] = feature;
    }
  }

  //----------------------------------------------------------------------------
  // Train forest

  now = GetCurrentTimeSinceEpochUs();

  printf("Initializing Rangerx\n");
  forest->initCppFromMem(
      arg_handler.depvarname, arg_handler.memmode, features_cm, labels,
      total_dataset_size, num_cols, arg_handler.mtry, arg_handler.outprefix,
      arg_handler.ntree, &std::cout, arg_handler.seed, arg_handler.nthreads,
      arg_handler.predict, arg_handler.impmeasure,
      arg_handler.targetpartitionsize, arg_handler.splitweights,
      arg_handler.alwayssplitvars, arg_handler.statusvarname,
      arg_handler.replace, arg_handler.catvars, arg_handler.savemem,
      arg_handler.splitrule, arg_handler.caseweights, arg_handler.predall,
      arg_handler.fraction, arg_handler.alpha, arg_handler.minprop,
      arg_handler.holdout, arg_handler.predictiontype, arg_handler.randomsplits,
      arg_handler.maxdepth, arg_handler.regcoef, arg_handler.usedepth);

  printf("Rangerx init duration: %ld\n", GetCurrentTimeSinceEpochUs() - now);

  now = GetCurrentTimeSinceEpochUs();

  printf("Running Rangerx\n");
  forest->run(true, !arg_handler.skipoob);
  if (arg_handler.write) {
    printf("Saving model to file\n");
    forest->saveToFile();
  }

  printf("Rangerx training duration: %ld\n",
         GetCurrentTimeSinceEpochUs() - now);

  printf("Writting output\n");
  forest->writeOutput();

  return 0;
}
