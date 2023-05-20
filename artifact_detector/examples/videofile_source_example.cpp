/* Copyright 2023 Francis Guindon <fbadilla10@gmail.com> */

#include <cstdio>
#include <memory>
#include <string>

#include "include/utility.hpp"
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
  ReturnValue ret;

  /* Object construction */
  try {
    input_video = std::make_unique<VideofileSource>(input_videofile, 0.1);
  } catch (ReturnValue& ret) {
    printf("(%d): %s\n", static_cast<int>(ret.code), ret.description.c_str());
  }

  for (size_t i = 0; i < 200; ++i) {
    ret = input_video->pullFrame(&frame);
    if (ReturnCode::Success != ret.code) {
      printf("(%d): %s\n", static_cast<int>(ret.code), ret.description.c_str());
    }

    printf("Pulled frame: %p %ldx%ld (%ld)\n", frame.data, frame.width,
           frame.height, frame.pts);
    save_frame("test_frame." + std::to_string(frame.width) + "." +
                   std::to_string(frame.height) + ".i420",
               frame.data, frame.height * frame.width);

    ret = input_video->pushFrame(&frame);
    if (ReturnCode::Success != ret.code) {
      printf("(%d): %s\n", static_cast<int>(ret.code), ret.description.c_str());
    }
  }

  return 0;
}
