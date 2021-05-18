#!/usr/bin/python3
# -*- coding: utf-8 -*-

import sys
import os
import os.path
import datetime
import pprint
import xml.dom.minidom
from xml.dom.minidom import Node
from xml.dom.minidom import Document


log_filename = "analysis_progress.report.final"



	
if (len(sys.argv) < 2):
	Log("progress_log log_msg (count)")
	sys.exit(-1)	

if (len(sys.argv)<3):
	progress_count = 0
else:
	progress_count = int(sys.argv[2]);
		
	
log_msg = sys.argv[1]	

if (os.path.exists(log_filename) ==False) :
	doc = Document()
	nodePS = doc.createElement("PROGRESS_STATUS")
	nodeP = doc.createElement("PROGRESS")
	txt = doc.createTextNode("0")
	nodeP.appendChild(txt)
	nodePS.appendChild(nodeP)
	doc.appendChild(nodePS)
	with open(log_filename, "w") as f:
		f.write( doc.toxml())

doc = xml.dom.minidom.parse(log_filename)

progress = doc.getElementsByTagName("PROGRESS")
txt = doc.createTextNode(str(int(progress[0].childNodes[0].data)  + progress_count ) )
progress[0].replaceChild(txt, progress[0].childNodes[0])

txt = doc.createTextNode(log_msg)
milestone = doc.createElement("MILESTONE")
milestone.appendChild(txt)

doc.childNodes[0].appendChild(milestone)

with open(log_filename, "w") as f:
    f.write( doc.toxml() )



