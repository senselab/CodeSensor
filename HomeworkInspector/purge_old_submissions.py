#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
import os
import datetime
import time
import shlex 
import subprocess
import shutil
import hashlib
import re
import syslog

target_dir= sys.argv[1]
max_cnt_submissions = 10 # keep only the 10 most recent submissions

debug_log=1
dry_run_mode=False

def ScanRoot(homework_root):
	for fname in os.listdir(homework_root):
		homework_user_root = os.path.join(homework_root, fname)
		PurgeOldSubmissions(homework_user_root)

def PurgeOldSubmissions(homework_user_root):
	folder_pattern = '^\d{4}-[\d]+-[\d]+\s+\d+:\d+:\d+$'
	folder_filter = re.compile(folder_pattern)
	submissions = {} 
	cnt_submissions = 0

	if not os.path.isdir(homework_user_root):
		return
	#print(' re pattern [%s]' % folder_pattern);

	for fname in os.listdir(homework_user_root):
		path = os.path.join(homework_user_root, fname)
		try:
			if not os.path.isdir(path):
				continue
			if folder_filter.match(fname)==None:
				continue

			code_path = os.path.join(path,"code")	## safty guard
			if not os.path.exists(code_path) :
				continue
#			if fname=="current" :
#				continue
			
			timestamp = os.path.getmtime( path )

			if submissions.has_key(timestamp):
				submissions[timestamp].append(  path)
			else:
				submissions[timestamp] = [ path]

			cnt_submissions = cnt_submissions+1
	#		print("[%s]  (%s)" % (fname, time.ctime(timestamp)));
						
		except OSError:
			print ("\ncannot access [%s] !\n" % path)

	cnt_purge = max(cnt_submissions - max_cnt_submissions, 0)

	if debug_log > 0:
		msg = "PurgeOldSubmissions: %s has %d submissions." % (homework_user_root, cnt_submissions)
		print(msg)		
		syslog.syslog(msg)
		msg = "\tPurging the %d oldest submissions." % cnt_purge
		print(msg)
		syslog.syslog(msg)	
	
	for timestamp in sorted(submissions.iterkeys()):
		for path in submissions[timestamp]:
			if cnt_purge > 0:
				if debug_log > 1:
					print("Removing [%s] (%s)" % (path, time.ctime(timestamp)));	
				cnt_purge = cnt_purge-1;

				if dry_run_mode==False:	
					cmd = '\\rm -rf "%s"' % path
#					print(cmd)
					os.system(cmd)
			else:
#				msg = "Purging of [%s] done !" % homework_user_root 
#				print(msg)
#				syslog.syslog(msg)
				return	
 
#PurgeOldSubmissions(target_dir)			

ScanRoot(target_dir)
 


