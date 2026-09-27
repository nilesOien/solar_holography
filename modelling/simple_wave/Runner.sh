#!/bin/bash


cleanup_and_exit() {
    echo -e "\n[!] CTRL+C detected. Cleaning up temporary files..."
    exit 130  # 130 is the standard exit code for a process terminated by CTRL+C
}
trap cleanup_and_exit INT

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
  anno=`echo "$base" | sed s/iter_//g`
  echo Converting "$base".ppm to "$base".jpg for annotation
  convert "$file" jpgs/"$base".jpg &> /dev/null # Send output to /dev/null due to warning about how convert is deprecated in ImageMagick 7
  convert jpgs/"$base".jpg -pointsize 16 -fill cyan -annotate +5+16 "$anno" jpgs/"$base".jpg &> /dev/null
done

echo Making animation iter.gif
rm -f iter.gif
convert jpgs/iter*.jpg -loop 0 -delay 3 iter.gif &> /dev/null




exit 0

