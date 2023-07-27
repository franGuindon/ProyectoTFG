import cv2 as cv
import numpy as np
import sys
import argparse

NUM_MSEC_PER_SEC = 1000
MIN_INT8 = 0
MAX_INT8 = 255
BLOCK_SIZE = 16

def int_round(value): return int(value + 0.5)

def sum_threshold_macroblock_ground_truth(frame, threshold):
    """ sum_threshold_macroblock_ground_truth

    For each 16x16 macroblock in image, calculate luma component sum.
    If sum is larger than threshold, mark as ground truth.
    Return ground truth as a frame and as an array ready to be turned to bytes

    parameters:
      frame (2D I420 np.ndarray)  : Input frame, expected to be a difference
        between unaltered frames and frames with simulated loss
      threshold (numeric) : A macroblock sum above this result is considered
        an artefact
    returns tuple(np.ndarray, np.ndarray) : Pair of a frame with ground truth blocks
        and the same data as an array that can easily be turned to bytes
    """

    luma_and_chroma_height, width = frame.shape
    assert luma_and_chroma_height % 3 == 0, \
           "Error: Input must be in I420 format and height must be multiple of 3"
    height = (luma_and_chroma_height*2)//3

    num_block_rows = height//BLOCK_SIZE
    num_block_cols = width//BLOCK_SIZE
    height = num_block_rows*BLOCK_SIZE
    width = num_block_cols*BLOCK_SIZE
    
    output_frame = frame.copy()
    output_array = np.empty((num_block_rows*num_block_cols), dtype=np.uint8)

    block_locations = [
        (xmin, ymin)
        for ymin in range(0, height, BLOCK_SIZE)
        for xmin in range(0, width, BLOCK_SIZE)
    ]
    for i, (xmin, ymin) in enumerate(block_locations):
        xmax, ymax = xmin+BLOCK_SIZE, ymin+BLOCK_SIZE
        block = frame[ymin:ymax,xmin:xmax]
        out_block = output_frame[ymin:ymax,xmin:xmax]
        is_artifact = np.sum(block) >= threshold*BLOCK_SIZE*BLOCK_SIZE
        block_val = MAX_INT8 if is_artifact else MIN_INT8
        out_block.fill(block_val)
        output_array[i] = block_val
    return output_frame, output_array

def img_abs_diff(img1, img2):
    """ imgs_abs_diff

    Take the absolute difference of two numpy arrays.
    Return the absolute difference in np.uint8 type.
    Both inputs should have the same shape.
    In principle it should work for any shape.
    It has only been tested on 2D frames representing cv I420 format
                            and 3D frames representing cv BGR  format

    parameters:
        img1 (np.ndarray)  : First input image
        img2 (np.ndarray)  : Second input image
    returns (np.ndarray) : Absolute difference result
    """
    img1_signed = img1.astype(int)
    img2_signed = img2.astype(int)
    abs_diff = np.absolute(img1_signed - img2_signed)
    return abs_diff.astype(np.uint8)

def bgr2yuv(src):
    return cv.cvtColor(src, cv.COLOR_BGR2YUV_I420)

def yuv2bgr(src):
    return cv.cvtColor(src, cv.COLOR_YUV2BGR_I420)

def make_yuv_black_and_white(src):
    height = src.shape[0]/1.5
    assert(height == int(height))
    height = int(height)
    src[height:,:] = 127

def get_next_pts_aligned_frame_pair(frame_id, src1_video, src2_video):
    """get_next_pts_aligned_frame_pair

    Find and return next pair of timestamp aligned frames from the pair of 
    input videos.

    parameters:
        frame_id (int) : Frame snippet id (used for logging)
        src1_video (cv.VideoCapture) : First input video
        src2_video (cv.VideoCapture) : Second input video
    returns tuple(np.ndarray, np.ndarray) : Next available pair of frames with aligned timestamps
    """
    ret, src1_frame = src1_video.read()
    if not ret: raise RuntimeError("Could not capture from src1")

    ret, src2_frame = src2_video.read()
    if not ret: raise RuntimeError("Could not capture from src2")

    src1_pts = int_round(src1_video.get(cv.CAP_PROP_POS_MSEC))
    src2_pts = int_round(src2_video.get(cv.CAP_PROP_POS_MSEC))
    
    while src1_pts > src2_pts:
        ret, src2_frame = src2_video.read()
        if not ret: raise RuntimeError("Could not capture from src2")
        src2_pts = int_round(src2_video.get(cv.CAP_PROP_POS_MSEC))

    while src2_pts > src1_pts:
        ret, src1_frame = src1_video.read()
        if not ret: raise RuntimeError("Could not capture from src1")
        src1_pts = int_round(src1_video.get(cv.CAP_PROP_POS_MSEC))
    
    print(f"id: {frame_id} src1: {src1_pts} src2: {src2_pts}")
    if src1_pts != src2_pts:
        raise RuntimeError(f"Unalined frames at src1: {src1_pts} src2: {src2_pts}")

    return src1_frame, src2_frame

def generate_ground_truth(videos, config):
    """ generate_ground_truth

    Given some input videos, output videos, a start time, and a number of frames,
    this function will capture a snippet from the input videos, process them
    to generate ground truths, then write the outputs to video files.

    parameters:
        videos.ref_video (cv.VideoCapture) : Base or reference video
        videos.pl_video (cv.VideoCapture)  : Packet loss or lossy video
        videos.ref_writer (cv.VideoWriter) : Output video for reference video snippet
        videos.pl_writer (cv.VideoWriter)  : Output video for lossy video snippet
        videos.sub_writer (cv.VideoWriter) : Output video for video differnece snippet
        config.start_time_sec (numeric)    : Video timestamp from where to start snippet
        config.num_frames (int)            : How many frames to capture in snippet
        config.write (bool)                : Whether to record the snippets to the output videos
        config.show (bool)                 : Whether to use imshow and waitKey to display snippet
    """
    
    start_time_msec = config.start_time_sec * NUM_MSEC_PER_SEC
    videos.ref_video.set(cv.CAP_PROP_POS_MSEC, start_time_msec)

    for frame_id in range(config.num_frames):
        if not videos.ref_video.isOpened() or not videos.pl_video.isOpened():
            raise RuntimeError("Videos not open")
        
        ref_frame, pl_frame = get_next_pts_aligned_frame_pair(frame_id, videos.ref_video, videos.pl_video)
        
        ref_frame = bgr2yuv(ref_frame)
        pl_frame = bgr2yuv(pl_frame)

        sub_frame = img_abs_diff(ref_frame, pl_frame)
        make_yuv_black_and_white(sub_frame)

        block_frame, block_array = sum_threshold_macroblock_ground_truth(sub_frame, 30)

        ref_frame = yuv2bgr(ref_frame)
        pl_frame = yuv2bgr(pl_frame)
        sub_frame = yuv2bgr(sub_frame)
        block_frame = yuv2bgr(block_frame)

        if config.write:
            videos.ref_writer.write(ref_frame)
            videos.pl_writer.write(pl_frame)
            videos.sub_writer.write(sub_frame)
            videos.block_writer.write(block_frame)
            with open(videos.block_bytes, 'ab') as f: f.write(block_array.tobytes())

        if config.show:
            cv.imshow("ref", ref_frame)
            cv.imshow("pl", pl_frame)
            cv.imshow("sub", sub_frame)
            cv.imshow("blocks", block_frame)
            cv.waitKey(1)

class Namespace: pass

def main(parser):
    parser.add_argument('ref_vid', help="")
    parser.add_argument('lossy_vid', help="")
    parser.add_argument('out_dir', help="")
    parser.add_argument('start_pts', help="")
    args = parser.parse_args()

    start_time_sec = float(args.start_pts)
    num_frames = 200
    write = True
    show = False

    ref_video = args.ref_vid
    pl_video = args.lossy_vid
    output_dir = args.out_dir

    ref_video = cv.VideoCapture(ref_video)
    pl_video = cv.VideoCapture(pl_video)

    if not ref_video.isOpened() or not pl_video.isOpened():
        print("Error, could not open videos")
        exit(1)

    width = ref_video.get(cv.CAP_PROP_FRAME_WIDTH)
    height = ref_video.get(cv.CAP_PROP_FRAME_HEIGHT)
    assert(width == pl_video.get(cv.CAP_PROP_FRAME_WIDTH))
    assert(height == pl_video.get(cv.CAP_PROP_FRAME_HEIGHT))
    width = int(width)
    height = int(height)

    fourcc = cv.VideoWriter_fourcc(*'xvid')
    ref_writer = cv.VideoWriter(f"{output_dir}/ref.mp4", fourcc, 30.0, (width, height))
    pl_writer = cv.VideoWriter(f"{output_dir}/pl.mp4", fourcc, 30.0, (width, height))
    sub_writer = cv.VideoWriter(f"{output_dir}/sub.mp4", fourcc, 30.0, (width, height)) 
    block_writer = cv.VideoWriter(f"{output_dir}/block.mp4", fourcc, 30.0, (width, height))

    videos = Namespace()
    videos.ref_video = ref_video
    videos.pl_video = pl_video
    videos.ref_writer = ref_writer
    videos.pl_writer = pl_writer
    videos.sub_writer = sub_writer
    videos.block_writer = block_writer
    videos.block_bytes = f"{output_dir}/block.bytes"

    config = Namespace()
    config.start_time_sec = start_time_sec
    config.num_frames = num_frames
    config.write = write
    config.show = show

    generate_ground_truth(videos, config)

    ref_video.release()
    pl_video.release()

    ref_writer.release()
    pl_writer.release()
    sub_writer.release()
    block_writer.release()

    cv.destroyAllWindows()

if __name__ == "__main__":
    
    parser = argparse.ArgumentParser(
        prog = "ground_truth",
        description = "Generates ground truth data for TFG")
    
    main(parser)
