#!/usr/bin/python3
# -*- coding: utf-8 -*-

import sys
import os
import datetime



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

def Equal(out_string, gold_string):
	out_words = out_string.split()
	gold_words = gold_string.split()
	return out_words == gold_words
	
if (len(sys.argv) != 2):
	Log("Syntax: verifier.py clean|checkname")
	sys.exit(-1)	
	
if (sys.argv[1]=='clean'):
	os.remove('output.txt')
	sys.exit(0)

fCP = open('check_pattern_concise_{0}.report.final'.format(sys.argv[1]),'w')

with open('output.txt.{0}'.format(sys.argv[1]), 'r') as f:
	outputs = [line.strip() for line in f if line.strip()]

with open('output.txt.gold', 'r') as f:
	outputs_gold = [line.strip() for line in f if line.strip()]

nTest = len(outputs_gold)

for k in range(0, min(len(outputs),len(outputs_gold))):
	if ( Equal(outputs[k], outputs_gold[k])):
		strCheck = 'check {0}: PASSED (1/{1})\n'.format(k+1,nTest)
	else:
		strCheck = 'check {0}:        (0/{1})\n'.format(k+1,nTest)	
	fCP.write(strCheck)

fCP.close()	
