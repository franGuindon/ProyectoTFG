/* Copyright 2023 Francis Guindon <fbadilla10@gmail.com> */

#include <cstdio>
#include <string>

#include "include/types.hpp"

int main(int argc, char** argv) {
  if (3 != argc) {
    printf(
        "Usage: [MODEL_FILE] [FEATURE_FILE] [OUTPUT_PREFIX]\n"
        "  MODEL_FILE    : This file contains model to run\n"
        "  FEATURE_FILE  : This file contains data to predict\n"
        "  OUTPUT_PREFIX : Several files will be saved with this prefix:\n"
        "       .confusion  : Confusion matrix\n"
        "                     (currently not implemented)\n"
        "       .prediction : Binary prediction data\n"
        "                     (currently not implemented)\n"
        "       .map        : Binary id map to match up the prediction to\n"
        "                     the corresponding original macroblock id\n"
        "                     (currently not implemented)\n");
  }

  return 0;
}
