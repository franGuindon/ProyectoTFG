/* 
  Project: TFG
  Author: Francis Guindon < FIXME add email >
  Note: Borrowing and modifying file from ranger C++ core under MIT conditions
*/

/*-------------------------------------------------------------------------------
 This file is part of ranger.

 Copyright (c) [2014-2018] [Marvin N. Wright]

 This software may be modified and distributed under the terms of the MIT license.

 Please note that the C++ core of ranger is distributed under MIT license and the
 R package "ranger" under GPL3 license.
 #-------------------------------------------------------------------------------*/

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>
#include <memory>

#include "globals.h"
#include "ArgumentHandler.h"
#include "ForestClassification.h"
#include "ForestRegression.h"
#include "ForestSurvival.h"
#include "ForestProbability.h"
#include "utility.h"

using namespace ranger;
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

void run_ranger(const Args& arg_handler, std::ostream& verbose_out);

int main(int argc, char **argv) {

  try {
    // Handle command line arguments
    Args arg_handler{};
    arg_handler.verbose = true;
    arg_handler.file = "test_data.dat";
    arg_handler.depvarname = "y";
    arg_handler.ntree = 1000;
    arg_handler.nthreads = 12;
    arg_handler.write = true;

    if (arg_handler.verbose) {
      run_ranger(arg_handler, std::cout);
    } else {
      std::ofstream logfile { arg_handler.outprefix + ".log" };
      if (!logfile.good()) {
        throw std::runtime_error("Could not write to logfile.");
      }
      run_ranger(arg_handler, logfile);
    }
  } catch (std::exception& e) {
    std::cerr << "Error: " << e.what() << " Ranger will EXIT now." << std::endl;
    return -1;
  }

  return 0;
}

void run_ranger(const Args& arg_handler, std::ostream& verbose_out) {
  verbose_out << "Starting Ranger." << std::endl;

  // Create forest object
  std::unique_ptr<Forest> forest { };

  /* Goal: Construct subclass */
  forest = make_unique<ForestRangerx>();

  // Call Ranger
  float *mem = nullptr;
  int num_rows = 0;
  int num_cols = 0;

  forest->initCppFromMem(arg_handler.depvarname, arg_handler.memmode, mem, num_rows, num_cols, arg_handler.mtry,
      arg_handler.outprefix, arg_handler.ntree, &verbose_out, arg_handler.seed, arg_handler.nthreads,
      arg_handler.predict, arg_handler.impmeasure, arg_handler.targetpartitionsize, arg_handler.splitweights,
      arg_handler.alwayssplitvars, arg_handler.statusvarname, arg_handler.replace, arg_handler.catvars,
      arg_handler.savemem, arg_handler.splitrule, arg_handler.caseweights, arg_handler.predall, arg_handler.fraction,
      arg_handler.alpha, arg_handler.minprop, arg_handler.holdout, arg_handler.predictiontype,
      arg_handler.randomsplits, arg_handler.maxdepth, arg_handler.regcoef, arg_handler.usedepth);

  forest->run(true, !arg_handler.skipoob);
  if (arg_handler.write) {
    forest->saveToFile();
  }
  forest->writeOutput();
  verbose_out << "Finished Ranger." << std::endl;
}
