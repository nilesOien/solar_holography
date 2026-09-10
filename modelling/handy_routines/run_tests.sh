#!/bin/bash

# Small script to build and run the test program.

# Build the program. 
make clean
make

if [ ! -f testprog ]
then
 echo Failed to build C program
 exit -1
fi

# Delete any prior output (although make clean should have done this)
rm -f *.ppm *.jpg

# Run the test program.
echo Running C program
./testprog

# Convert the PPM format images to jpg
for file in *.ppm
do
 echo Converting $file to jpg format
 base=`basename "$file" .ppm`
 magick "$base".ppm "$base".jpg
done

echo
echo Finished.
echo Look at quick_look.html with a browser.

exit 0

