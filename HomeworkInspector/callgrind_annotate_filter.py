#!/usr/bin/python3
# -*- coding: utf-8 -*-

import sys
import os
import datetime


"""
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

"""
cmd = ""
for i in range(1, len(sys.argv)):
	cmd = cmd + sys.argv[i] + " "
	"""
	
if (len(sys.argv) != 2):
	print("callgrind_annoate_filter report")
	sys.exit(-1)	
	
	
report_file = sys.argv[1]	

"""Log ("command = [" + cmd + "]")
Log ("runtime_result = [" + runtime_result + "]")
"""

line_hist = ["","",""]
show_line = True
f = open(report_file, 'r')
for line in f: 
	sys.stdout.write(line)
#	line_hist[0] = line_hist[1]
#	line_hist[1] = line_hist[2]
#	line_hist[2] = line
#  THIS IS NOT WORKING 2013.2.14
#	if ( line_hist[0].find("--------------------------------------------------------------------------------")>=0  and line_hist[2].find("--------------------------------------------------------------------------------")>=0):
#		show_line = True	
#		if (  line_hist[1].find("Auto-annotated source:")>=0 ):
#			show_line = False
#			if ( line_hist[1].find("HOMEWORK::")>=0):
#				show_line = True
#	if ( show_line):
#		sys.stdout.write(line_hist[0])

		
	
