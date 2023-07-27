import cv2 as cv
import sys
import numpy as np

def main():
  if len(sys.argv) != 3:
    print("Usage ex:")
    print("  $ python3 parse_images_into_videofile.py %d.i420 test")
    print("  $ ls")
    print("  test.mp4")
    exit(1)

  input_files = sys.argv[1]
  output_prefix = sys.argv[2]

  fourcc = cv.VideoWriter_fourcc(*'xvid')
  writer = cv.VideoWriter(output_prefix, fourcc, 30, (1280, 720))

  for id in range(200):
    frame = 
    writer.write(frame)

if __name__ == "__main__":
  main()