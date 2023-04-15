#include <memory>
#include <ForestClassification.h>
#include <stdio.h>

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

int main(int argc, char** argv) {
  if (argc != 4) {
    printf("Usage: analyzer [MODEL_FILE] [TREE_ID] [MAXDEPTH]\n");
    return 1;
  }

  auto forest = std::make_unique<ForestRangerx>();

  Args arg_handler{};
  arg_handler.outprefix = "rangerx";
  arg_handler.verbose = true;
  arg_handler.file = "test_data.dat";
  arg_handler.depvarname = "y";
  arg_handler.ntree = 10;
  arg_handler.nthreads = 12;
  arg_handler.predict = argv[1];

  const size_t features_per_block = 132;
  const size_t num_rows = 1;
  const size_t num_cols = features_per_block;
  const size_t mem_size = num_rows*num_cols;
  auto x_mem = std::unique_ptr<float[]>(new float[mem_size]);
  auto y_mem = std::unique_ptr<float[]>(new float[num_rows]);
  
  printf("Initializing Rangerx\n");
  forest->initCppFromMem(arg_handler.depvarname, arg_handler.memmode,
      x_mem.get(), y_mem.get(), num_rows, num_cols, arg_handler.mtry,
      arg_handler.outprefix, arg_handler.ntree, &std::cout, arg_handler.seed,
      arg_handler.nthreads, arg_handler.predict, arg_handler.impmeasure,
      arg_handler.targetpartitionsize, arg_handler.splitweights,
      arg_handler.alwayssplitvars, arg_handler.statusvarname,
      arg_handler.replace, arg_handler.catvars, arg_handler.savemem,
      arg_handler.splitrule, arg_handler.caseweights, arg_handler.predall,
      arg_handler.fraction, arg_handler.alpha, arg_handler.minprop,
      arg_handler.holdout, arg_handler.predictiontype, arg_handler.randomsplits,
      arg_handler.maxdepth, arg_handler.regcoef, arg_handler.usedepth);
  
  forest->print(std::stoi(argv[2]), std::stoi(argv[3]));

  return 0;
}
