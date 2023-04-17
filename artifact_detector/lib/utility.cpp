/* Copyright 2023 Francis Guindon <fbadilla10@gmail.com> */

#include "include/utility.hpp"

uint64_t GetCurrentTimeSinceEpochUs() {
  auto now = std::chrono::system_clock::now();
  auto now_us = std::chrono::time_point_cast<MicroSeconds>(now);
  auto epoch = now_us.time_since_epoch();
  auto value = std::chrono::duration_cast<MicroSeconds>(epoch);
  return value.count();
}
