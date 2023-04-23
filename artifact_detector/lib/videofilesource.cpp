/* Copyright 2023 Francis Guindon <fbadilla10@gmail.com> */

#include "include/videofilesource.hpp"

VideofileSource::VideofileSource(std::string filename) : file_{filename} {
  ReturnValue ret;
  ret = openFile();
  if (ReturnCode::Success != ret.code) {
    throw ret;
  }
}

ReturnValue VideofileSource::openFile() {
  return ReturnValue{ReturnCode::FileError, "File error"};
}

ReturnValue VideofileSource::pullFrame(Buffer<uint8_t>* frame) {
  return ReturnValue{ReturnCode::Success, "Operation returned successfully"};
}

ReturnValue VideofileSource::pushFrame(Buffer<uint8_t>* frame) {
  return ReturnValue{ReturnCode::Success, "Operation returned successfully"};
}
