import numpy as np
import sys

from utilities import read_uint8_frame, read_uint32_frame, read_float32_frame

# Constants
BLOCK_DIMENSION = 16

def filter_frame(frame):
  frame = frame.astype(int)
  height, width = frame.shape
  xframe = np.absolute(frame[:,:-1] - frame[:,1:])
  yframe = np.absolute(frame[:-1,:] - frame[1:,:])
  zero_col = np.zeros((height,1))
  zero_row = np.zeros((1,width))
  xframe = np.concatenate((xframe, zero_col), axis=1)
  yframe = np.concatenate((yframe, zero_row), axis=0)
  return xframe.astype(np.uint8), yframe.astype(np.uint8)

def get_sat(frame):
  frame = frame.astype(int)
  return frame.cumsum(axis=0).cumsum(axis=1), (frame*frame).cumsum(axis=0).cumsum(axis=1)

def generate_block_features(vertical_filtered, horizontal_filtered, block_i,
                            block_j, debug, vsat, vsat2, hsat, hsat2):
  i1 = block_i*BLOCK_DIMENSION
  i2 = (block_i+1)*BLOCK_DIMENSION
  j1 = block_j*BLOCK_DIMENSION
  j2 = (block_j+1)*BLOCK_DIMENSION

  vblock = vertical_filtered[i1:i2,j1:j2]
  hblock = horizontal_filtered[i1:i2,j1:j2]
  rois = [
    # left border
    vertical_filtered[i1:i2,j1-1:j1+1],
    vertical_filtered[i1:i2,j1-2:j1+2],
    vertical_filtered[i1:i2,j1-4:j1+4],
    vertical_filtered[i1:i2,j1-8:j1+8],
    # top border
    vertical_filtered[i1-1:i1+1,j1:j2],
    vertical_filtered[i1-2:i1+2,j1:j2],
    vertical_filtered[i1-4:i1+4,j1:j2],
    vertical_filtered[i1-8:i1+8,j1:j2],
    # right border
    vertical_filtered[i1:i2,j2-1:j2+1],
    vertical_filtered[i1:i2,j2-2:j2+2],
    vertical_filtered[i1:i2,j2-4:j2+4],
    vertical_filtered[i1:i2,j2-8:j2+8],
    # bottom border
    vertical_filtered[i2-1:i2+1,j1:j2],
    vertical_filtered[i2-2:i2+2,j1:j2],
    vertical_filtered[i2-4:i2+4,j1:j2],
    vertical_filtered[i2-8:i2+8,j1:j2],
    # left border
    horizontal_filtered[i1:i2,j1-1:j1+1],
    horizontal_filtered[i1:i2,j1-2:j1+2],
    horizontal_filtered[i1:i2,j1-4:j1+4],
    horizontal_filtered[i1:i2,j1-8:j1+8],
    # top border
    horizontal_filtered[i1-1:i1+1,j1:j2],
    horizontal_filtered[i1-2:i1+2,j1:j2],
    horizontal_filtered[i1-4:i1+4,j1:j2],
    horizontal_filtered[i1-8:i1+8,j1:j2],
    # right border
    horizontal_filtered[i1:i2,j2-1:j2+1],
    horizontal_filtered[i1:i2,j2-2:j2+2],
    horizontal_filtered[i1:i2,j2-4:j2+4],
    horizontal_filtered[i1:i2,j2-8:j2+8],
    # bottom border
    horizontal_filtered[i2-1:i2+1,j1:j2],
    horizontal_filtered[i2-2:i2+2,j1:j2],
    horizontal_filtered[i2-4:i2+4,j1:j2],
    horizontal_filtered[i2-8:i2+8,j1:j2],
  ]

  vblock_avg = vblock.sum()/vblock.size
  vblock_var = (vblock*vblock).sum()/vblock.size-np.mean(vblock)*np.mean(vblock)
  hblock_avg = hblock.sum()/hblock.size
  hblock_var = (hblock*hblock).sum()/hblock.size-np.mean(hblock)*np.mean(hblock)

  if debug:
    vsat = vsat.astype(int)
    vsat2 = vsat2.astype(int)
    hsat = hsat.astype(int)
    hsat2 = hsat2.astype(int)
    hblock = hblock.astype(np.uint32)
    print((hblock*hblock).sum())
    print(hsat2[i2-1,j2-1], hsat2[i1-1,j2-1], hsat2[i2-1,j1-1], hsat2[i1-1,j1-1])
    print(hsat2[i2-1,j2-1]-hsat2[i1-1,j2-1]-hsat2[i2-1,j1-1]+hsat2[i1-1,j1-1])

  avgs_ = [roi.sum()/roi.size for roi in rois]
  vars_ = [(roi*roi).sum()/roi.size - np.mean(roi)*np.mean(roi) for roi in rois]
  
  pyfeatures = [vblock_avg, vblock_var, hblock_avg, hblock_var]

  for i in range(0, len(avgs_), 4):
    pyfeatures.append(avgs_[i])
    pyfeatures.append(avgs_[i+1])
    pyfeatures.append(avgs_[i+2])
    pyfeatures.append(avgs_[i+3])
    pyfeatures.append(vars_[i])
    pyfeatures.append(vars_[i+1])
    pyfeatures.append(vars_[i+2])
    pyfeatures.append(vars_[i+3])

    pyfeatures.append(avgs_[i] - (vblock_avg if i < len(avgs_)/2 else hblock_avg))
    pyfeatures.append(avgs_[i+1] - (vblock_avg if i < len(avgs_)/2 else hblock_avg))
    pyfeatures.append(avgs_[i+2] - (vblock_avg if i < len(avgs_)/2 else hblock_avg))
    pyfeatures.append(avgs_[i+3] - (vblock_avg if i < len(avgs_)/2 else hblock_avg))
    pyfeatures.append(vars_[i] - (vblock_var if i < len(avgs_)/2 else hblock_var))
    pyfeatures.append(vars_[i+1] - (vblock_var if i < len(avgs_)/2 else hblock_var))
    pyfeatures.append(vars_[i+2] - (vblock_var if i < len(avgs_)/2 else hblock_var))
    pyfeatures.append(vars_[i+3] - (vblock_var if i < len(avgs_)/2 else hblock_var))
  
  return pyfeatures

if __name__ == "__main__":

  # Check args
  if len(sys.argv) != 11:
    print("Usage: python3 test_cpp_features.py [WIDTH] [HEIGHT] [RAW]"
          " [VFILTERED] [HVILTERED] [VSAT] [VSQSAT] [HSAT] [HSQSAT] [FEATURES]")
    exit(1)

  # Parse args
  width = int(sys.argv[1])
  height = int(sys.argv[2])
  blocks_per_row = width//BLOCK_DIMENSION
  blocks_per_col = height//BLOCK_DIMENSION

  raw_frame = read_uint8_frame(sys.argv[3], width, height)
  vertical_filtered = read_uint8_frame(sys.argv[4], width, height)
  horizontal_filtered = read_uint8_frame(sys.argv[5], width, height)
  vertical_sat = read_uint32_frame(sys.argv[6], width, height)
  vertical_sq_sat = read_uint32_frame(sys.argv[7], width, height)
  horizontal_sat = read_uint32_frame(sys.argv[8], width, height)
  horizontal_sq_sat = read_uint32_frame(sys.argv[9], width, height)
  features = read_float32_frame(sys.argv[10], 132, (blocks_per_row - 2)*(blocks_per_col - 2))

  vertical_filtered = vertical_filtered.astype(np.uint32)
  horizontal_filtered = horizontal_filtered.astype(np.uint32)

  # Create test vals
  ideal_hfiltered, ideal_vfiltered = filter_frame(raw_frame)
  ideal_vsat, ideal_vsat2 = get_sat(ideal_vfiltered)
  ideal_hsat, ideal_hsat2 = get_sat(ideal_hfiltered)

  # Run tests
  assert (vertical_filtered == ideal_vfiltered).all()
  assert (horizontal_filtered == ideal_hfiltered).all()
  assert (vertical_sat == ideal_vsat).all()
  assert (vertical_sq_sat == ideal_vsat2).all()
  assert (horizontal_sat == ideal_hsat).all()
  assert (horizontal_sq_sat == ideal_hsat2).all()

  feature_row = 0
  for block_i in range(1, 2): # for block_i in range(1, blocks_per_col-1):
    for block_j in range(1, blocks_per_row-1):
      #debug = True if ((block_i == 1) and (block_j == 17)) else False
      debug = False
      pyfeatures = generate_block_features(vertical_filtered, horizontal_filtered, block_i, block_j, debug, ideal_vsat, ideal_vsat2, ideal_hsat, ideal_hsat2)
      for i, (pyfeat, feat) in enumerate(zip(pyfeatures, features[feature_row,:])):
        try:
          assert(pyfeat == feat)
        except AssertionError as e:
          print(i, pyfeat, feat, block_i, block_j)
          raise e
      feature_row += 1
  
  print("All tests done")
