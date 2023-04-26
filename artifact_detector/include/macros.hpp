/* Copyright 2023 Francis Guindon <fbadilla10@gmail.com> */

#ifndef ARTIFACT_DETECTOR_INCLUDE_MACROS_HPP_
#define ARTIFACT_DETECTOR_INCLUDE_MACROS_HPP_

#include "include/logging.hpp"

/**
 * @brief HANDLE_WITH is a quick repetitive error handler code
 *
 * - Requires to be used within object with ReturnValue ret_ member.
 *   Throws compile error if not
 *
 */
#define HANDLE_WITH(cmd, ret)               \
  do {                                      \
    ret_ = (cmd);                           \
    if (ReturnCode::Success != ret_.code) { \
      ERROR("%s", ret_.str().c_str());      \
      return ret;                           \
    } else {                                \
      STATUS;                               \
    }                                       \
  } while (0)

#define HANDLE_WITH_THROW(cmd, ret)         \
  do {                                      \
    ret_ = (cmd);                           \
    if (ReturnCode::Success != ret_.code) { \
      ERROR("%s", ret_.str().c_str());      \
      throw ret;                            \
    } else {                                \
      STATUS;                               \
    }                                       \
  } while (0)

#endif  // ARTIFACT_DETECTOR_INCLUDE_MACROS_HPP_
