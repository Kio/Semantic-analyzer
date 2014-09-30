# encoding: utf8
from __future__ import print_function
import sys

sentence = str(sys.argv[1])
#sentence = "Эти типы стали есть на складе ."
#sentence = "Мама Александра Овечкина двукратная олимпийская чемпионка и великий баскетбольный тренер Татьяна Овечкина ."
#sentence = "В связи с последними событиями на Украине российские казаки решили помочь украинским казачьим организациям ."
#sentence = "Мама делала раму."
f = open('testSentence.xml', 'w')

print ('<?xml version="1.0" encoding="utf-8" standalone="yes"?>', file=f)
print ('<sentences><p>', file=f)
print ('<se>', file=f)
id = 0
for word in sentence.split():
    id += 1
    print ('<w id="%d">' % id + word + '<rel id_head="" type=""/></w>', file=f)

print ('</se>', file=f)
print ('</p>', file=f)
print ('</sentences>', file=f)
