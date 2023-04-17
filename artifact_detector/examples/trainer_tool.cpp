/* Copyright 2023 Francis Guindon <fbadilla10@gmail.com> */

#include <memory>

#include "include/trainer.hpp"
#include "include/types.hpp"

int main(int argc, char** argv) {
  ReturnValue ret;

  auto trainer = std::make_unique<Trainer>();

  ret = trainer->train();

  return 0;
}
