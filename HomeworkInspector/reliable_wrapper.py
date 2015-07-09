#!/usr/bin/python
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


"""
cmd = ""
for i in range(1, len(sys.argv)):
	cmd = cmd + sys.argv[i] + " "
	"""
	
if (len(sys.argv) != 3):
	Log("reliable_wrapper runtime_result cmd")
	sys.exit(-1)	
	
	
runtime_result = sys.argv[1]	
cmd = sys.argv[2]	
Log ("command = [" + cmd + "]")
Log ("runtime_result = [" + runtime_result + "]")

for cnt in range(1,33):
	Log("(cnt:"+str(cnt)+")\tEXecuting: " + cmd)
	os.system(cmd)
	if (DetectRuntimeError(runtime_result) ==False):
		break
		
	
