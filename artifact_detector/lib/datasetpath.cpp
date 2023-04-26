/* Copyright 2023 Francis Guindon <fbadilla10@gmail.com> */

#include "include/datasetpath.hpp"

#include <cstdio>
#include <filesystem>

#include "include/utility.hpp"

DatasetPath::DatasetPath(std::string dataset_path)
    : dataset_path_{dataset_path},
      ret_{ReturnCode::Success, "Videofilesrc initialized correctly"} {}

ReturnValue DatasetPath::getPathType() {
  // if dataset_path_

  return {ReturnCode::Success, "Successfully got path type"};
}
