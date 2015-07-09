#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
import os
import datetime
import re


if ( len(sys.argv) ==3 ):
	setting_file = 'setting'
	key = sys.argv[1]
	value = sys.argv[2]
elif (len(sys.argv)==4):
	setting_file = sys.argv[1]
	key = sys.argv[2]
	value = sys.argv[3]
else:
	print("if_setting_has setting_file key value")
	sys.exit(-1)	
	
	
pattern = r'^\s*' + key + r'\s*=\s*' + value + r'\s*$'

#print('['+pattern+']')

"""Log ("command = [" + cmd + "]")
Log ("runtime_result = [" + runtime_result + "]")
"""
prog = re.compile(pattern );


f = open(setting_file, 'r')
for line in f: 
#	print(line)
	result = prog.match(line)
	
	if ( result != None) :
		print("Found !")
		sys.exit(1)
	
print("Not found !");
sys.exit(0)
		
	
