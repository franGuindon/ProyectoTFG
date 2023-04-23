/* Copyright 2023 Francis Guindon <fbadilla10@gmail.com> */

#include <cxxabi.h>

#include <cstdio>
#include <stdexcept>
#include <string>
#include <vector>

#include "include/types.hpp"

// Header
/* #include <string> */

class Argparse {
 public:
  Argparse(const int argc, const char** argv);

  ReturnValue usageString(ReturnCode code);

  template <typename T>
  std::string type_name() {
    int status;
    std::string tname = typeid(T).name();
    char* demangled_name =
        abi::__cxa_demangle(tname.c_str(), NULL, NULL, &status);
    if (status == 0) {
      tname = demangled_name;
      std::free(demangled_name);
    }
    return tname;
  }

  template <class T>
  void add(const std::string& arg_name, const std::string& arg_description) {
    printf("The type '%s' is unsupported.\n", type_name<T>().c_str());
    throw ReturnValue{ReturnCode::ParameterError, "Add argument failed"};
  }

  void parse();

 private:
  const int argc_;
  const char** argv_;
  int current_arg_count_;
  std::vector<std::string> arg_names_;
  std::vector<std::string> arg_descriptions_;
  std::vector<std::string> args_;
};

// Source
Argparse::Argparse(const int argc, const char** argv)
    : argc_{argc}, argv_{argv}, current_arg_count_{0} {}

ReturnValue Argparse::usageString(ReturnCode code) {
  ReturnValue ret;
  ret.code = code;
  ret.description = "Usage:";
  for (size_t i = 0; i < arg_names_.size(); ++i) {
    ret.description += " " + arg_names_[i];
  }
  ret.description += "\n";
  for (size_t i = 0; i < arg_names_.size(); ++i) {
    ret.description += "\t" + arg_names_[i] + ":\n";
    ret.description += "\t\t" + arg_descriptions_[i];
  }
  return ret;
}

template <>
void Argparse::add<std::string>(const std::string& arg_name,
                                const std::string& arg_description) {
  arg_names_.push_back(arg_name);
  arg_descriptions_.push_back(arg_description);
  args_.push_back(argv_[current_arg_count_]);
  ++current_arg_count_;
}

void Argparse::parse() {
  if (argc_ != current_arg_count_) {
    throw usageString(ReturnCode::ParameterError);
  }
}

int main(int argc, const char** argv) {
  Argparse parser{argc, argv};

  try {
    parser.add<std::string>("[MODEL_FILE]",
                            "This file contains model to run\n");
    parser.add<std::string>("[FEATURE_FILE]",
                            "This file contains data to predict\n");
    // parser.add<std::string>("[OUTPUT_PREFIX]",
    //                           "\tSeveral files will be saved with this
    //                           prefix:\n"
    //                           "\t- .confusion\n");
  } catch (ReturnValue& error) {
    printf("%d: %s\n", static_cast<int>(error.code), error.description.c_str());
    return 1;
  } catch (std::logic_error& error) {
    printf("%s\n", error.what());
  }

  parser.parse();

  return 0;
}
