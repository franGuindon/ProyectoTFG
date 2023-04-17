/* Copyright 2023 Francis Guindon <fbadilla10@gmail.com> */

#ifndef ARTIFACT_DETECTOR_INCLUDE_TRAINER_HPP_
#define ARTIFACT_DETECTOR_INCLUDE_TRAINER_HPP_

#include "include/types.hpp"

class Trainer {
 public:
  Trainer();
  ReturnValue train();
};

#endif  // ARTIFACT_DETECTOR_INCLUDE_TRAINER_HPP_
