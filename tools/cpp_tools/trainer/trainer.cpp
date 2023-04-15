#include <fstream>
#include <memory>
#include <stdio.h>
#include <ForestClassification.h>

using namespace ranger;
typedef ForestClassification ForestRangerx;

template <typename dtype>
bool load_frame(const std::string &filename, dtype *data, const size_t size) {
  std::streampos file_size;
  std::ifstream file(filename, std::ios::in | std::ios::binary | std::ios::ate);

  if (!data) {
    printf("Load frame error: Data pointer is null\n");
    return false;
  }

  if (!file.is_open()) {
    printf("Load frame error: File did not open\n");
    return false;
  }

  file_size = file.tellg();

  if (file_size != static_cast<long long>(sizeof(dtype)*size)) {
    printf("Load frame error: Expected size (%ld) and file size (%lld) differ\n",
           sizeof(dtype)*size, static_cast<long long>(file_size));
    return false;
  }

  file.seekg(0, std::ios::beg);
  auto data_char_ptr = reinterpret_cast<char *>(data);
  file.read(data_char_ptr, sizeof(dtype)*size);

  file.close();

  return true;
}

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
    printf("Usage: trainer [FEATURE_FILE] [LABEL_FILE] [OUTPUT_PREFIX]\n");
    return 1;
  }

  auto forest = std::make_unique<ForestRangerx>();

  Args arg_handler{};
  arg_handler.outprefix = argv[3];
  arg_handler.verbose = true;
  arg_handler.file = "test_data.dat";
  arg_handler.depvarname = "y";
  arg_handler.ntree = 20;
  arg_handler.nthreads = 12;
  arg_handler.write = true;
  arg_handler.maxdepth = 100;

  const size_t block_dimension = 16;
  const size_t frame_width = 1280; 
  const size_t frame_height = 720;
  const size_t blocks_per_row = frame_width/block_dimension;
  const size_t blocks_per_col = frame_height/block_dimension;
  const size_t blocks_per_frame = blocks_per_row*blocks_per_col;
  const size_t num_frames_per_snip = 200;
  const size_t features_per_block = 132;

  const size_t block_mem_size = blocks_per_row*blocks_per_col*num_frames_per_snip;
  const size_t num_rows = (blocks_per_row-2)*(blocks_per_col-2)*num_frames_per_snip;
  const size_t num_cols = features_per_block;
  const size_t mem_size = num_rows*num_cols;
  auto x_mem = std::unique_ptr<float[]>(new float[mem_size]);
  auto block_mem = std::unique_ptr<unsigned char[]>(new unsigned char[block_mem_size]);
  auto y_mem = std::unique_ptr<float[]>(new float[num_rows]);

  printf("Loading feature file\n");
  load_frame(argv[1], x_mem.get(), mem_size);
  printf("Loading label file\n");
  load_frame(argv[2], block_mem.get(), block_mem_size);

  printf("Building label memory\n");
  size_t y_offset = 0;
  for (size_t i = 0; i < num_frames_per_snip; ++i) {
    for (size_t j = 1; j < blocks_per_col-1; ++j) {
      for (size_t k = 1; k < blocks_per_row-1; ++k) {
        size_t offset = i*blocks_per_frame + j*blocks_per_row + k;
        y_mem[y_offset] = static_cast<float>(block_mem[offset]);
        ++y_offset;
      }
    }
  }

  printf("Measuring raw dataset balance\n");
  size_t sum_positives = 0;
  size_t sum_negatives = 0;
  for (size_t i = 0; i < num_rows; ++i) {
    if (0.0F == y_mem[i]) {
      ++sum_negatives;
    } else if (255.0F == y_mem[i]) {
      ++sum_positives;
    } else {
      printf("Error: Found invalid label value at index: %ld\n", i);
      return -1;
    }
  }
  float negative_percent = 100.0F*static_cast<float>(sum_negatives)/static_cast<float>(num_rows);
  float positive_percent = 100.0F*static_cast<float>(sum_positives)/static_cast<float>(num_rows);
  printf("Negatives: %ld, Positives: %ld, Total: %ld, Balance: %3.2f : %3.2f\n",
        sum_negatives, sum_positives, num_rows, negative_percent, positive_percent);

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
  
  printf("Running Rangerx\n");
  forest->run(true, !arg_handler.skipoob);
  if (arg_handler.write) {
    printf("Saving model to file\n");
    forest->saveToFile();
  }
  printf("Writting output\n");
  forest->writeOutput();

  return 0;
}
