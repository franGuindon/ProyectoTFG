# Dataset


## How to build the dataset

There are four main tools used to build the dataset.
* `ssyoutube`
* `gst-launch-1.0`
* `generate_ground_truth.sh`
* `feature_generator`

#### `ssyoutube`
* Used to download youtube videos
* [Webpage found here](https://ssyoutube.com/en73/youtube-video-downloader)

#### `gst-launch-1.0`
* Used to generate lossy video
* The actual pipeline may vary depending on the source video
* Use the `mediainfo` command to find tecnical info on the source video
* Perhaps use this pipeline to see how gstreamer naturally decides to decode a given video:
  ```
  VIDEOFILE=/path/to/videofile
  gst-launch-1.0 -ve filesrc location=$VIDEOFILE ! decodebin ! fakesink
  ```
* Example of pipeline used on [walking_tour.mp4](https://drive.google.com/file/d/1YwzqGhnweLDrkpeVsIdnBzKoj3-baY-a/view?usp=share_link) file:
  ```
  gst-launch-1.0 -ve filesrc location=walking_tour.mp4 ! qtdemux ! "video/x-h264, format=I420" ! identity drop-probability=0.01 ! h264parse ! qtmux ! filesink location=lossy_walking_tour_720p.mov
  ```
* The `identity` element simulates packet loss

#### `generate_ground_truth.sh`
* [Script found here](/tools/bash_tools/generate_ground_truth.sh)
* Arguments:
  * The original video
  * The lossy video
  * The snip spec
  * The output directory
* Behavior:
  * Creates subdirectories in output directory, one for each snip specified in the snip spec.
  * Each snip directory is named with the snip id and contains:
    * The video created from snipping 200 frames of the original video
    * The video created from snipping 200 frames of the lossy video
    * The video created from subtracting each snip
    * The video created from discretizing subtraction into macroblocks
    * The discretization data in binary format (to later be used as label data)

#### `feature_generator`
* [Source found here](/tools/cpp_tools/feature_generator/feature_generator.cpp)

## Snip specs

The dataset does not consist of complete videos. Instead the dataset generation
tools extract snips from each video to create ground truth files, feature files, and label files. The snip spec is simply a markdown style table specifying the locations where the tools can extract snips.
* The first column of the table specifies the snip id in '##' format.
* The second column of the table specifies the snip location in 'hh:mm:ss' format.
