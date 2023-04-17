/* Copyright 2023 Francis Guindon <fbadilla10@gmail.com> */

#include "include/videofilesink.hpp"

VideofileSink::VideofileSink(std::string filename) {}

ReturnValue VideofileSink::pushFrame(Buffer<uint8_t>* frame) {
  return ReturnValue{ReturnCode::Success, "Operation returned successfully"};
}
