# encoding: utf8
from __future__ import print_function
import sys

fin = open('output_upper.txt', 'r')
fout = open('output.txt', 'w')

lines = fin.readlines()
for line in lines:
	print(unicode(line, "utf-8").lower().encode("utf-8"), file=fout, end="")

fin.close()
fout.close()
