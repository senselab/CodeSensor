#!/usr/bin/python3
# -*- coding: utf-8 -*-

import tempfile
import os
import string

def KillValgrind(ps_line):
    #print(ps_line)
	tokens=ps_line.split( )
	#print tokens
	if (len( tokens) >  3):
		pid = tokens[1]
		ppid = tokens[2]
		#print("pid: %s  ppid: %s" % (pid, ppid))	
		if( ppid == '1'):
			os.system("kill -9 %s" % (pid) )

f=tempfile.NamedTemporaryFile(delete=False)
filename = f.name
f.close()

os.system('ps -ef|grep valgrind.safe > '+ filename)

f = open(filename, "r")

for line in f:
	if ( line.find(' sh -c ')>=0 or line.find('bin/valgrind.safe')==-1):
		continue
	KillValgrind(line)

f.close()

os.unlink(filename)

