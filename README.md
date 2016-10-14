# wunderbar_stability

Create a data/ folder and copy all the data files to it (wunderbar_1_400*.root)
Run calib_match.py to create a runlist
To analyze the runs, do root -l -b -q "bar_stability.C+"
Output data is in fitinfo.txt

To use the background subtraction method instead of the sipm cut  do root -l -b -q "bar_stability.C+(\"sub\")"
