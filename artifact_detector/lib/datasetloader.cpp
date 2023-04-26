/* Copyright 2023 Francis Guindon <fbadilla10@gmail.com> */

#include "include/datasetloader.hpp"

#include <cstdio>

#include "include/utility.hpp"

DatasetLoader::DatasetLoader(std::string dataset_path)
    : file_{filename},
      ret_{ReturnCode::Success, "Videofilesrc initialized correctly"} {}
