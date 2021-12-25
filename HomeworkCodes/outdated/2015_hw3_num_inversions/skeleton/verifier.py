#!/usr/bin/python3
# -*- coding: utf-8 -*-

import sys
import os
import datetime
import msgpack


def DetectRuntimeError(filename):
	f = open(filename, 'r')
	for line in f:
		if ( line.find('run_innerloop detected host state invariant failure') >=0 ):
			return True
	return False

def Log(msg, bStdout = True):
	log_file = open('reliable_wrapper_log.report','a')
	szLine = '[' + str(datetime.datetime.now()) + ']\t' + msg 
	log_file.write(szLine + '\n')
	if (bStdout):
		print(szLine)
	log_file.close()

def Equal(out_array, gold_array):
	return out_array == gold_array
	
if (len(sys.argv) != 2):
	Log("Syntax: verifier.py clean|checkname")
	sys.exit(-1)	
	
if (sys.argv[1]=='clean'):
	os.remove('output.txt')
	sys.exit(0)

fCP = open('check_pattern_concise_{0}.report.final'.format(sys.argv[1]),'w')

outputs = []
with open('output.txt.{0}'.format(sys.argv[1]), 'rb') as f:
	unpacker = msgpack.Unpacker(f)
	for o in unpacker:
		outputs.append(o)
	print("output.txt.{0} has {1} entries\n".format(sys.argv[1], len(outputs)))

outputs_gold = []
with open('output.txt.gold', 'rb') as f:
	unpacker = msgpack.Unpacker(f)
	for o in unpacker:
		outputs_gold.append(o)
	print("output.txt.gold has {0} entries\n".format(len(outputs_gold)))

nTest = len(outputs_gold)

for k in range(0, min(len(outputs),len(outputs_gold))):
	if ( Equal(outputs[k], outputs_gold[k])):
		strCheck = 'check {0}: PASSED (1/{1})\n'.format(k+1,nTest)
	else:
		strCheck = 'check {0}:        (0/{1})\n'.format(k+1,nTest)	
	fCP.write(strCheck)

fCP.close()	
