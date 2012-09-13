#!/usr/bin/python
import os
import sys
import re
import string


# Check specified arguments.
if len( sys.argv ) != 3:
    sys.exit( 0 )

filename  = sys.argv[1]
parameter = sys.argv[2]

pattern = " " + parameter + " ="
basename,ext = os.path.splitext( filename )

#print "filename  = %s" % filename
#print "parameter = %s" % parameter
#print "basename  = %s" % basename
#print "pattern   = %s" % pattern

# Dump the specified data region to a temporary file named *.tmp1.
tempfile1 = basename + ".tmp1"
#os.system("ncdump -v %s %s > %s" % (parameter, filename, tempfile1) )

file = open( tempfile1 )

dim1 = 0
dim2 = 0
dim3 = 0
while 1:
    line = file.readline()
    if not line: break
    if re.search( "\tlon =",   line ) > 0: dim1 = line.split( " " )[2]
    if re.search( "\tlat =",   line ) > 0: dim2 = line.split( " " )[2]
    if re.search( "\tlevel =", line ) > 0: dim3 = line.split( " " )[2]
    if re.search( pattern, line ) > 0: break

result = []
while 1:
    line = file.readline()
    if not line: break
    result += map(float, filter(None, map(string.strip, re.split('[ ;},\n\t]', line))))

#print line
#print result

print "dim1 = %s" % dim1
print "dim2 = %s" % dim2
print "dim3 = %s" % dim3

# Remove the header information from the *.tmp3 and output the result to a data file *.dat.
#os.system("tail +$eoh $filename.$parameter.tmp3 > $filename.$parameter.dat")
