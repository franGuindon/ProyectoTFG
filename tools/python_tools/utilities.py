import sys
import numpy as np

def read_uint8_frame(filename, width, height):
  """ Parse raw binary frame file into a uint8 numpy array """
  with open(filename, "rb") as f: data = f.read()
  return np.frombuffer(data, dtype=np.uint8).reshape((height, width))

def read_uint32_frame(filename, width, height):
  """ Parse raw binary frame file into a uint32 numpy array """
  with open(filename, "rb") as f: data = f.read()
  return np.frombuffer(data, dtype=np.uint32).reshape((height, width))

def read_float32_frame(filename, width, height):
  """ Parse raw binary frame file into a float32 numpy array """
  with open(filename, "rb") as f: data = f.read()
  return np.frombuffer(data, dtype=np.float32).reshape((height, width))

if __name__ == "__main__":
  frame = read_uint8_frame(sys.argv[1], int(sys.argv[2]), int(sys.argv[3]))
  print(frame.shape)
