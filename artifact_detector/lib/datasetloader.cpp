/* Copyright 2023 Francis Guindon <fbadilla10@gmail.com> */

#include "include/datasetloader.hpp"

#include <cstdio>

#include "include/utility.hpp"

DatasetLoader::DatasetLoader(std::string dataset_path)
    : dataset_path_{dataset_path},
      ret_{ReturnCode::Success, "Videofilesrc initialized correctly"} {}
