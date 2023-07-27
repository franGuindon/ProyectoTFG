import argparse
from argparse import RawTextHelpFormatter
import glob
import os
from os.path import join as joinpath
from datetime import datetime

import cv2 as cv
import numpy as np
from scipy.signal import convolve2d
from ground_truth import bgr2yuv, yuv2bgr, make_yuv_black_and_white

def get_frame(dataset_path, vid_id, snip_id, frame_id):
    vid_name = joinpath(dataset_path, vid_id, snip_id, "pl.mp4")
    video = cv.VideoCapture(vid_name)

    video.set(cv.CAP_PROP_POS_FRAMES, frame_id)

    ret, frame = video.read()
    if not ret: raise RuntimeError("Could not return frame")

    video.release()

    return frame

class Dataset:

    NUM_FRAMES_PER_SNIP = 200
    NUM_BLOCK_COLS = 1280//16
    NUM_BLOCK_ROWS = 720//16

    def __init__(self):
        self.frames = []
        self.labels = []

    def load_video(self, filename):
        cap = cv.VideoCapture(filename)
        frames = []
        while cap.isOpened():
            ret, frame = cap.read()
            if not ret and len(frames) == self.NUM_FRAMES_PER_SNIP: break
            if not ret and len(frames) != self.NUM_FRAMES_PER_SNIP:
                raise RuntimeError("Could not load frame", len(frames))
            frames.append(frame)
        cap.release()
        self.frames = frames

    def load_labels(self, filename):
        with open(filename, 'rb') as f: data = f.read()
        labels = np.frombuffer(data, dtype=np.uint8)
        labels = labels.reshape(self.NUM_FRAMES_PER_SNIP, self.NUM_BLOCK_ROWS, self.NUM_BLOCK_COLS)
        print(labels.shape)
        self.labels = labels

class ProcessDataset:

    XKERNEL = np.array([[-1, 1]])
    YKERNEL = np.array([[1], [-1]])

    def __init__(self, output_dir):
        self.output_dir = output_dir
        
        self.xfiltered = []
        self.yfiltered = []
        self.feats = []
        self.dataset = None

    def preprocess_frame(self, frame):
        frame = bgr2yuv(frame)
        make_yuv_black_and_white(frame)
        return frame.astype(np.int32)
    
    def postprocess_frame(self, frame):
        frame = np.abs(frame)
        assert(not np.any(frame > 255))
        assert(not np.any(frame < 0))
        frame = frame.astype(np.uint8)
        make_yuv_black_and_white(frame)
        return frame
    
    def filter_frame(self, frame):
        # Does not give same result as C++
        # xframe = convolve2d(frame, self.XKERNEL, mode='same')
        # yframe = convolve2d(frame, self.YKERNEL, mode='same')
        
        # Does give same result as C++
        right_shifted_frame = np.concatenate((frame[:,1:], np.zeros((1080,1))), axis=1)
        up_shifted_frame = np.concatenate((frame[1:,:], np.zeros((1,1280))), axis=0)
        xframe = frame - right_shifted_frame
        yframe = frame - up_shifted_frame
        return xframe, yframe

    def apply_filtering(self):
        xfiltered = []
        yfiltered = []
        for frame_id, frame in enumerate(self.dataset.frames):
            frame = self.preprocess_frame(frame)

            xframe, yframe = self.filter_frame(frame)

            xframe = self.postprocess_frame(xframe)
            yframe = self.postprocess_frame(yframe)
            
            # print("frame", frame_id)
            # print("raw frame:")
            # print(frame[12:20,16:32].transpose())
            # print("hor fil:")
            # print(xframe[12:20,16:32].transpose())
            # print("ver fil:")
            # print(yframe[12:20,16:32].transpose())
            # exit(1)
            xfiltered.append(xframe)
            yfiltered.append(yframe)

        self.xfiltered = xfiltered
        self.yfiltered = yfiltered

    def get_rect_stats(self, frame, rect):
        y,x,h,w = rect
        roi = frame[y:h,x:w]
        mean, var = np.mean(roi), np.var(roi)
        # print("Rect", y,h,x,w)
        # print("roi", roi)
        # print("mean, var", mean, var)
        return [mean, var]

    def get_rect_list(self, i, j):
        """
        i and j are expected to be the upper left most pixel of the block
        """
        izq = lambda dim: (i,j-dim,i+16,j+dim)
        der = lambda dim: (i,j+16-dim,i+16,j+16+dim)
        top = lambda dim: (i-dim,j, i+dim,j+16)
        bot = lambda dim: (i+16-dim,j, i+16+dim,j+16)
        get_borders = lambda dim: [izq(dim), der(dim), top(dim), bot(dim)]
        ret = []
        dims = [1,2,4,8]
        for dim in dims: ret += get_borders(dim)
        return ret
    
    def apply_rect_stats(self):
        feats = []
        times = []
        for xframe, yframe in zip(self.xfiltered, self.yfiltered):
            #for i in range(1,self.dataset.NUM_BLOCK_ROWS-1):
                # FIXME: Not finished ... right?
                # missing rest of frame
            now = datetime.now()
            rect_list = self.get_rect_list(16, 16)
            featv = []
            for rect in rect_list:
                featv += self.get_rect_stats(xframe, rect)
            for rect in rect_list:
                featv += self.get_rect_stats(yframe, rect)
            feats.append(featv)
            duration = (datetime.now() - now)
            times.append(duration.seconds*1000000 + duration.microseconds)
        self.feats = feats
        print(sum(times)/len(times))
    
    def save_feats(self):
        datastr = ""
        for frame_id, featv in enumerate(self.feats):
            for feat in featv: datastr += f"{feat:.3f} "
            # FIXME: from project root: python3 tools/python/features.py dataset/ other/tests/
            # Throws: IndexError: index 200 is out of bounds for axis 0 with size 200
            # on this line:
            datastr += "1" if self.dataset.labels[frame_id, 1, 1] > 1 else "0"
            datastr += "\n"

        with open(joinpath(self.output_dir, "dataset.dat"),'a') as f:
            f.write(datastr)

    def apply(self, dataset):
        self.dataset = dataset
        now = datetime.now()
        self.apply_filtering()
        print("Filtering duration:", datetime.now()-now)
        self.apply_rect_stats()
        self.save_feats()
    
    def show(self):
        for xframe, yframe in zip(self.xfiltered, self.yfiltered):
            cv.imshow('X', xframe)
            cv.imshow('Y', yframe)
            cv.waitKey(1)

def main(args):
    vid_list = glob.glob(joinpath(args.dataset_dir, "*/*/pl.mp4"))
    vid_list.sort()

    label_list = glob.glob(joinpath(args.dataset_dir, "*/*/block.bytes"))
    label_list.sort()

    dataset = Dataset()
    processer = ProcessDataset(args.output_dir)
    for i, (vid_name, label_name) in enumerate(zip(vid_list, label_list)):
        print(f"Processing: {vid_name:25} | {label_name:25}")
        dataset.load_video(vid_name)
        dataset.load_labels(label_name)
        processer.apply(dataset)
        if i == 1: break

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        prog = "features",
        description = "Generates a feature file from a dataset directory structure",
        formatter_class=RawTextHelpFormatter)
    parser.add_argument('dataset_dir',
        help = \
"""The dataset directory is expected to have a the following
structure:
$DATASET_DIR/$VID_ID/$SNIP_ID/$RESOURCE_NAME
Where $DATASET_DIR is the argument value
Where $VID_ID currently only has one option: 0
Where $SNIP_ID options range from 00 to 18
Where $RESOURCE_NAME can be one of:
- ref.mp4
- pl.mp4
- sub.mp4
- block.mp4
- block.bytes
e.g. $DATASET_DIR/1/01/block.bytes""")
    parser.add_argument('output_dir',
        help = "Where to save dataset.dat file")
    args = parser.parse_args()
    main(args)
