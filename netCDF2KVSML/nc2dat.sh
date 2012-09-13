#!/bin/sh

filename=$1
parameter=$2
pattern=" $parameter ="

# Dump the specified data region to a temporary file named *.tmp1.
ncdump -v $parameter $filename > $filename.$parameter.tmp1

# Remove a character ";" from the *.tmp1 and output the result to a temporary file *.tmp2.
sed 's/;/ /g' $filename.$parameter.tmp1 > $filename.$parameter.tmp2

# Remove the *.tmp1.
rm $filename.$parameter.tmp1

# Remove a character "}" from the *.tmp2 and output the result to a temporary file *.tmp3.
sed 's/}/ /g' $filename.$parameter.tmp2 > $filename.$parameter.tmp3

# Remove the *.tmp2.
rm $filename.$parameter.tmp2

# Check a position of EOH (End Of Header).
eoh=`awk -v pattern="$pattern" '{ while( getline >= 0 ){ if ( index($0,pattern) != 0 ){print NR + 1; exit} } }' $filename.$parameter.tmp3`

# Remove the header information from the *.tmp3 and output the result to a data file *.dat.
tail +$eoh $filename.$parameter.tmp3 > $filename.$parameter.dat
echo "$filename.$parameter.dat"hb

# Remove the *.tmp3.
rm $filename.$parameter.tmp3
