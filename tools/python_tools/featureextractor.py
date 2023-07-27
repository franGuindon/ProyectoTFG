# Copyright (c) 2023
# Author: Francis Guindon <fbadilla10@gmail.com>

class FeatureExtractor:
  def __init__(self):
    
    pass

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

  def calculate_statistics(self, vsat, vssat, hsat, hssat):


  def extract_features(self, raw_frame):
    hor_frame, ver_frame = self.filter_frame(raw_frame)
    ver_sat, ver_ssat = self.get_sat(ver_frame)
    hor_sat, hor_ssat = self.get_sat(hor_frame)

    return feature_matrix
