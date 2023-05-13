/* Copyright 2023 Francis Guindon <fbadilla10@gmail.com> */

#define DEBUG_LEVEL 3
#include <stdio.h>

#include <memory>

#include "ForestClassification.h"
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
using ranger::ForestClassification;
using ranger::ImportanceMode;
using ranger::MEM_DOUBLE;
using ranger::MemoryMode;
using ranger::PredictionType;
using ranger::SplitRule;
using ranger::Tree;
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

bool check_args(int argc, char** argv) {
  if (argc < 3) {
    return false;
  }

  int option = std::stoi(argv[2]);

  if (option == 2) {
    if (argc != 5) {
      return false;
    }
  } else if (option == 4) {
    if (argc < 5) {
      return false;
    }
    if (argc % 2 != 1) {
      return false;
    }
  } else {
    if (argc != 3) {
      return false;
    }
  }

  return true;
}

int main(int argc, char** argv) {
  if (!check_args(argc, argv)) {
    printf("Usage: analyzer [MODEL_FILE] [PRINT_TYPE] \n");
    printf("  PRINT_TYPE:\n");
    printf("           0: Feature count\n");
    printf("           1: Complete\n");
    printf("           2: Single tree up to maxdepth\n");
    printf("              (Requires additional args: [TREE_ID] [MAXDEPTH])\n");
    printf("           3: Depth (for each leaf node)\n");
    printf("           4: Predict\n");
    printf(
        "              (Requires additional args: [FEATURE_FILE] [LABEL_FILE] "
        "...)\n");
    return 1;
  }

  int option = std::stoi(argv[2]);

  auto forest = std::make_unique<ForestRangerx>();

  Args arg_handler{};
  arg_handler.outprefix = "rangerx";
  arg_handler.verbose = true;
  arg_handler.file = "test_data.dat";
  arg_handler.depvarname = "y";
  arg_handler.ntree = 10;
  arg_handler.nthreads = 12;
  arg_handler.predict = argv[1];
  arg_handler.maxdepth = 10;

  //----------------------------------------------------------------------------

  size_t num_rows = 0;
  size_t num_cols = 0;
  std::unique_ptr<float[]> x_mem = nullptr;
  std::unique_ptr<float[]> y_mem = nullptr;

  size_t total_label_mem_size = 0;

  if (option == 4) {
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
    const size_t num_file_rows =
        (blocks_per_row - 2) * (blocks_per_col - 2) * num_frames_per_snip;
    const size_t num_file_cols = features_per_block;
    const size_t feature_file_size = num_file_rows * num_file_cols;
    const size_t num_file_pairs = (argc - 3) / 2;
    const size_t total_feature_mem_size = feature_file_size * num_file_pairs;
    const size_t total_raw_label_mem_size = label_file_size * num_file_pairs;
    total_label_mem_size = num_file_rows * num_file_pairs;

    num_rows = total_label_mem_size;
    num_cols = num_file_cols;

    auto x_mem_raw =
        std::unique_ptr<float[]>(new float[total_feature_mem_size]);
    auto y_mem_raw = std::unique_ptr<unsigned char[]>(
        new unsigned char[total_raw_label_mem_size]);

    x_mem = std::unique_ptr<float[]>(new float[total_feature_mem_size]);
    y_mem = std::unique_ptr<float[]>(new float[total_label_mem_size]);

    size_t y_offset = 0;
    for (size_t i = 0; i < num_file_pairs; ++i) {
      printf("Loading feature file: %s\n", argv[3 + 2 * i]);
      load_frame(argv[3 + 2 * i], x_mem_raw.get() + i * feature_file_size,
                 feature_file_size);
      printf("Loading label file: %s\n", argv[4 + 2 * i]);
      load_frame(argv[4 + 2 * i], y_mem_raw.get() + i * label_file_size,
                 label_file_size);

      printf("Converting feature file from row major to column major\n");
      for (size_t j = 0; j < num_file_rows; ++j) {
        for (size_t k = 0; k < num_file_cols; ++k) {
          float feature =
              x_mem_raw[i * feature_file_size + j * num_file_cols + k];
          x_mem[k * total_label_mem_size + (i * num_file_rows + j)] = feature;
        }
      }

      printf("Building label memory\n");
      for (size_t j = 0; j < num_frames_per_snip; ++j) {
        for (size_t k = 1; k < blocks_per_col - 1; ++k) {
          for (size_t l = 1; l < blocks_per_row - 1; ++l) {
            size_t offset = i * label_file_size + j * blocks_per_frame +
                            k * blocks_per_row + l;
            y_mem[y_offset] = static_cast<float>(y_mem_raw[offset]);
            ++y_offset;
          }
        }
      }
    }
    if (y_offset != total_label_mem_size) {
      printf("Mismatch between processed label size and total label size\n");
      return -1;
    }

    printf("Dataset loading duration: %ld\n",
           GetCurrentTimeSinceEpochUs() - now);
  } else {
    num_rows = 1;
    num_cols = 132;
    x_mem = std::unique_ptr<float[]>(new float[num_rows * num_cols]);
    y_mem = std::unique_ptr<float[]>(new float[num_rows]);
  }
  //----------------------------------------------------------------------------

  printf("Initializing Rangerx\n");
  forest->initCppFromMem(
      arg_handler.depvarname, arg_handler.memmode, x_mem.get(), y_mem.get(),
      num_rows, num_cols, arg_handler.mtry, arg_handler.outprefix,
      arg_handler.ntree, &std::cout, arg_handler.seed, arg_handler.nthreads,
      arg_handler.predict, arg_handler.impmeasure,
      arg_handler.targetpartitionsize, arg_handler.splitweights,
      arg_handler.alwayssplitvars, arg_handler.statusvarname,
      arg_handler.replace, arg_handler.catvars, arg_handler.savemem,
      arg_handler.splitrule, arg_handler.caseweights, arg_handler.predall,
      arg_handler.fraction, arg_handler.alpha, arg_handler.minprop,
      arg_handler.holdout, arg_handler.predictiontype, arg_handler.randomsplits,
      arg_handler.maxdepth, arg_handler.regcoef, arg_handler.usedepth);

  if (0 == option || 1 == option || 3 == option) {
    forest->print(static_cast<Tree::PrintType>(std::stoi(argv[2])));
  } else if (2 == option) {
    forest->print(static_cast<Tree::PrintType>(std::stoi(argv[2])),
                  std::stoi(argv[3]), std::stoi(argv[4]));
  } else if (4 == option) {
    forest->run(true, true);
    auto pred_mem = std::unique_ptr<float[]>(new float[total_label_mem_size]);
    forest->get_predictions(pred_mem.get(), total_label_mem_size);

    size_t true_positives = 0;
    size_t true_negatives = 0;
    size_t false_positives = 0;
    size_t false_negatives = 0;

    for (size_t i = 0; i < total_label_mem_size; ++i) {
      if (255.0F == y_mem[i] && 255.0F == pred_mem[i]) {
        ++true_positives;
      } else if (0.0F == y_mem[i] && 0.0F == pred_mem[i]) {
        ++true_negatives;
      } else if (0.0F == y_mem[i] && 255.0F == pred_mem[i]) {
        ++false_positives;
      } else if (255.0F == y_mem[i] && 0.0F == pred_mem[i]) {
        ++false_negatives;
      } else {
        printf("Found invalid values, id: %ld, label: %f, pred: %f\n", i,
               y_mem[i], pred_mem[i]);
      }
    }
    printf("Confusion matrix:\n");
    printf("              0   255\n");
    printf("predicted 0   %ld %ld\n", true_negatives, false_negatives);
    printf("predicted 255 %ld %ld\n", false_positives, true_positives);
    printf("\nPrecision: %f\n",
           static_cast<float>(true_positives) /
               static_cast<float>(true_positives + false_positives));
    printf("\nRecall:    %f\n",
           static_cast<float>(true_positives) /
               static_cast<float>(true_positives + false_negatives));
  }

  return 0;
}
