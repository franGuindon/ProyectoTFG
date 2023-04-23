/* Copyright 2023 Francis Guindon <fbadilla10@gmail.com> */

#include <cstdio>
#include <memory>
#include <string>

#include "include/videofilesource.hpp"

int main(int argc, char** argv) {
  /* Argparse */
  if (2 != argc) {
    printf("Usage: videfile_source_example [INPUT_VIDEOFILE]\n");
    printf(
        "  INPUT_VIDEOFILE: The videofile should use qt format and h264 "
        "codec\n");
    return 1;
  }

  /* Variable declarations */
  std::string input_videofile = argv[1];
  std::unique_ptr<VideofileSource> input_video;
  Buffer<uint8_t> frame = {0};

  /* Object construction */
  try {
    input_video = std::make_unique<VideofileSource>(input_videofile);
  } catch (ReturnValue& ret) {
    printf("(%d): %s\n", static_cast<int>(ret.code), ret.description.c_str());
  }

  input_video->pullFrame(&frame);

  return 0;
}
