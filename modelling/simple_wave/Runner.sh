#!/bin/bash

make clean
make
if [ ! -f simple_wave ]
then
   exit -1
fi
   
rm -rf images
mkdir images
./simple_wave

rm -rf jpgs
mkdir jpgs
for file in images/iter_*.ppm
do
  base=`basename "$file" .ppm`
  echo Converting $base
  magick "$file" jpgs/"$base".jpg
  magick jpgs/"$base".jpg -pointsize 24 -fill red -annotate +20+20 "$base" jpgs/"$base".jpg
done

rm -f iter.gif
magick jpgs/iter*.jpg -loop 0 -delay 3 iter.gif




exit 0

