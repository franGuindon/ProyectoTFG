## Tools usage
The ground truth tool is used to generate datasets with custom videos. The generated dataset includes:

- Folder with video corrupted images
- Folder with respective mask of the video corrupted images

To use the tool follow the next steps:

Create and setup the tool project.
```
mkdir build && meson build
```

Build the executable file to use the tool.
```
ninja -C build
```

Setup the directories to store the images.

```
mkdir video masks
```

Use the tool (the input path should be of a H264 video):

```
./build/getvideo path/to/h264video
```

After running the tool, the folders will contain the generated dataset of the given video.
