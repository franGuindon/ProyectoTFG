/* Copyright 2023 Francis Guindon <fbadilla10@gmail.com> */

#include "include/datasetpath.hpp"

#include <dirent.h>

#include <cstdio>
#include <cstring>

#include "include/utility.hpp"

DatasetPath::DatasetPath(std::string dataset_path)
    : dataset_path_{dataset_path},
      ret_{ReturnCode::Success, "Videofilesrc initialized correctly"},
      type_{DatasetType::kRawDataset},
      dataset_vid_dirs_{},
      dataset_snip_dirs_{},
      dataset_feature_files_{},
      dataset_label_files_{} {
  HANDLE_WITH_THROW(
      parseDatasetDir(),
      ReturnValue(ReturnCode::ParameterError, "Could not parse dataset dir"));

  HANDLE_WITH_THROW(parseVidDir(), ReturnValue(ReturnCode::ParameterError,
                                               "Could not parse vid dirs"));

  HANDLE_WITH_THROW(parseSnipDir(), ReturnValue(ReturnCode::ParameterError,
                                                "Could not parse snip dirs"));
}

ReturnValue DatasetPath::parseDatasetDir() {
  struct dirent *ent = nullptr;

  DIR *dataset_dir = nullptr;
  dataset_dir = opendir(dataset_path_.c_str());
  if (nullptr == dataset_dir) {
    return {ReturnCode::FileError,
            "Could not open dir '" + dataset_path_ + "'"};
  }

  while (true) {
    ent = readdir(dataset_dir);
    if (nullptr == ent) {
      break;
    }
    char digit0 = ent->d_name[0];
    char digit1 = ent->d_name[1];
    bool digit0_is_numeric = '0' <= digit0 && '9' >= digit0;
    bool digit1_is_eos = 0 == digit1;
    if (digit0_is_numeric && digit1_is_eos) {
      std::string vid_path = dataset_path_ + "/" + ent->d_name;
      dataset_vid_dirs_.push_back(vid_path);
    }
  }

  closedir(dataset_dir);

  return {ReturnCode::Success, "Successfully parsed dataset dir"};
}

ReturnValue DatasetPath::parseVidDir() {
  struct dirent *ent = nullptr;

  for (const std::string &vid_path : dataset_vid_dirs_) {
    DIR *vid_path_dir = opendir(vid_path.c_str());
    if (nullptr == vid_path_dir) {
      return {ReturnCode::FileError, "Could not open dir '" + vid_path + "'"};
    }

    while (true) {
      ent = readdir(vid_path_dir);
      if (nullptr == ent) {
        break;
      }
      char digit0 = ent->d_name[0];
      char digit1 = ent->d_name[1];
      char digit2 = ent->d_name[2];
      bool digit0_is_numeric = '0' <= digit0 && '9' >= digit0;
      bool digit1_is_numeric = '0' <= digit1 && '9' >= digit1;
      bool digit2_is_eos = 0 == digit2;
      if (digit0_is_numeric && digit1_is_numeric && digit2_is_eos) {
        std::string snip_path = vid_path + "/" + ent->d_name;
        dataset_snip_dirs_.push_back(snip_path);
      }
    }

    closedir(vid_path_dir);
  }

  return {ReturnCode::Success, "Successfully parsed vid dir"};
}

ReturnValue DatasetPath::parseSnipDir() {
  struct dirent *ent = nullptr;

  for (const std::string &snip_path : dataset_snip_dirs_) {
    DIR *snip_path_dir = opendir(snip_path.c_str());
    if (nullptr == snip_path_dir) {
      return {ReturnCode::FileError, "Could not open dir '" + snip_path + "'"};
    }

    while (true) {
      ent = readdir(snip_path_dir);
      if (nullptr == ent) {
        break;
      }

      bool is_feature_file = (0 == strcmp(ent->d_name, "features.bytes"));
      bool is_label_file = (0 == strcmp(ent->d_name, "block.bytes"));

      if (is_feature_file) {
        std::string feature_path = snip_path + "/" + ent->d_name;
        dataset_feature_files_.push_back(feature_path);
      }

      if (is_label_file) {
        std::string label_path = snip_path + "/" + ent->d_name;
        dataset_label_files_.push_back(label_path);
      }
    }

    closedir(snip_path_dir);
  }

  if (dataset_feature_files_.size() != dataset_label_files_.size()) {
    return {ReturnCode::MemoryError,
            "Did not find same number of feature and label files"};
  }

  return {ReturnCode::Success, "Successfully parsed snip dir"};
}
