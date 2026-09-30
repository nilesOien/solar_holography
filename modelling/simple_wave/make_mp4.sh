#!/bin/bash

rm -f iter.mp4
ffmpeg -pattern_type glob -i "jpgs/*.jpg" \
      -filter:v "setpts=2.0*PTS" iter.mp4

exit 0

