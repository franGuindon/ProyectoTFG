/* Copyright 2023 Francis Guindon <fbadilla10@gmail.com> */

#ifndef ARTIFACT_DETECTOR_INCLUDE_LOGGING_HPP_
#define ARTIFACT_DETECTOR_INCLUDE_LOGGING_HPP_

#define K_RST "\x1B[0m"
#define K_RED "\x1B[31m"
#define K_GREEN "\x1B[32m"

#define RED(fmt, ...) K_RED fmt K_RST, ##__VA_ARGS__
#define GREEN(fmt, ...) K_GREEN fmt K_RST, ##__VA_ARGS__

#define ERROR(fmt, ...)                                                  \
  do {                                                                   \
    printf(RED("ERROR:") " %s:%s:%d: " fmt "\n", __FILE__, __FUNCTION__, \
           __LINE__, ##__VA_ARGS__);                                     \
  } while (0)

#ifdef DEBUG_LEVEL

#define K_WARNING 1
#define K_INFO 2
#define K_DEBUG 3
#define K_TRACE 4

#define K_STATUS K_INFO

#define WARNING(fmt, ...)                                                      \
  do {                                                                         \
    if (DEBUG_LEVEL >= K_WARNING) {                                            \
      printf(RED("WARNING:") " %s:%s:%d: ", __FILE__, __FUNCTION__, __LINE__); \
      printf(fmt, ##__VA_ARGS__);                                              \
    }                                                                          \
  } while (0)
#define INFO(fmt, ...)                                                        \
  do {                                                                        \
    if (DEBUG_LEVEL >= K_INFO) {                                              \
      printf(GREEN("INFO:") " %s:%s:%d: ", __FILE__, __FUNCTION__, __LINE__); \
      printf(fmt, ##__VA_ARGS__);                                             \
    }                                                                         \
  } while (0)
#define DEBUG(fmt, ...)                                                  \
  do {                                                                   \
    if (DEBUG_LEVEL >= K_DEBUG) {                                        \
      printf("DEBUG:", " %s:%s:%d: ", __FILE__, __FUNCTION__, __LINE__); \
      printf(fmt, ##__VA_ARGS__);                                        \
    }                                                                    \
  } while (0)
#define TRACE(fmt, ...)                                                  \
  do {                                                                   \
    if (DEBUG_LEVEL >= K_TRACE) {                                        \
      printf("TRACE:", " %s:%s:%d: ", __FILE__, __FUNCTION__, __LINE__); \
      printf(fmt, ##__VA_ARGS__);                                        \
    }                                                                    \
  } while (0)

/**  */
#define STATUS                     \
  do {                             \
    if (DEBUG_LEVEL >= K_STATUS) { \
      ret_.print();                \
    }                              \
  } while (0)

#else

#define WARNING(...)
#define INFO(...)
#define DEBUG(...)
#define TRACE(...)

#define STATUS

#endif

#endif  // ARTIFACT_DETECTOR_INCLUDE_LOGGING_HPP_
