# encoding: utf8
from __future__ import print_function
import sys

cmd = str(sys.argv[1])
f = open('testSentence.xml', 'w')

if (cmd == 'start'):
	print ('<?xml version="1.0" encoding="utf-8" standalone="yes"?>', file=f)
	print ('<sentences><p>', file=f)
elif (cmd == 'add'):
	sentence = str(sys.argv[2])
	print ('<se>', file=f)
	id = 0
	for word in sentence.split():
		id += 1
		print ('<w id="%d">' % id + word + '<rel id_head="" type=""/></w>', file=f)
	print ('</se>', file=f)
elif (cmd == 'end'):
	print ('</p></sentences>', file=f)
