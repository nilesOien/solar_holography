#!/bin/bash

# Delete prior output.
rm -f interpolated_atmosphere_250

# Interpolate data onto a regular grid.
./interp.R 0.0 350000.0 250 interpolated_atmosphere_250

# Run the C reading routines on the interpolated data, just to test.
./read_atmosphere_test interpolated_atmosphere_250

exit 0

