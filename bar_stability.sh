#!/bin/bash

#clean
rm output/* caliblist.txt stability.pdf fitinfo*

#enable python libraries
#source ~/env/bin/activate

#Enable ! in ls
shopt -s extglob

#Not sure why this works
NFILES=`ls data!(*out*) | grep wunderbar | wc -l`
echo "Found $NFILES files."

echo "Getting calibration data..."
./calib_match.py
echo "Done."

#Execute root and Python scripts
root -l -b -q "bar_stability.C+"
root -l -b -q "bar_stability.C+(\"sub\")"

echo "Generating stability.pdf..."
./bar_stability.py
echo "Done."

#Show newly create PDF
#evince stability.pdf
