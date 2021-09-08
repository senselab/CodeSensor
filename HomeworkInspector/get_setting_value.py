#!/usr/bin/python3
# -*- coding: utf-8 -*-

import sys
import os
import datetime
import re


if ( len(sys.argv) ==2 ):
	setting_file = 'setting'
	key = sys.argv[1]
elif (len(sys.argv)==3):
	setting_file = sys.argv[1]
	key = sys.argv[2]
else:
	print("get_setting_value (setting_file) key ")
	sys.exit(-1)	
	


patternWithQuote = r'^\s*' + key + r'\s*=\s*\"(.+)\"\s*$'
pattern = r'^\s*' + key + r'\s*=\s*(\S+)\s*$'

#print('['+pattern+']')

"""Log ("command = [" + cmd + "]")
Log ("runtime_result = [" + runtime_result + "]")
"""
prog = re.compile(pattern )
progWithQuote = re.compile(patternWithQuote)

f = open(setting_file, 'r')
for line in f: 
#	print(line)
	result = progWithQuote.match(line)
	
	if ( result != None) :
		sys.stdout.write(result.group(1))
		sys.exit(1)
	else:
		result = prog.match(line)
		if (result != None):
			sys.stdout.write(result.group(1))
			sys.exit(1)	
	
sys.stdout.write("NOT_FOUND");
sys.exit(0)
		
	
