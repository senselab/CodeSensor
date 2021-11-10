/*jslint white: true, browser: true, undef: true, nomen: true, eqeqeq: true, plusplus: false, bitwise: true, regexp: true, strict: true, newcap: true, immed: true, maxerr: 14 */
/*global window: false, ActiveXObject: false*/

/*
The onreadystatechange property is a function that receives the feedback. It is important to note that the feedback
function must be assigned before each send, because upon request completion the onreadystatechange property is reset.
This is evident in the Mozilla and Firefox source.
*/

/* enable strict mode */
"use strict";

// global variables
var progress,			// progress element reference
	request,			// request object
	request_get_result,			// request object	
	intervalID = false,	// interval ID
	number_max = 60,	// limit of how many times to request the server (this limit is needed only for this demo)
	number,				// current number of requests
	// method definition
	initXMLHttpClient,	// create XMLHttp request object in a cross-browser manner
	send_request,		// send request to the server
	request_handler,	// request handler (started from send_request)
	polling_start,		// button start action
	polling_stop,		// button start action
	progress_value,
	norm_progress_value = 0,
	load_state;
	
/*
function set_progress_bar_max_value(max_value)
{
	progress_value_max = max_value;
	alert(progress_value_max);
}
*/

function IsNoPending()
{
	for ( var key in load_state) {
		if (load_state[key] ==1 ) {
//			alert(key + " is pending");
			return false;
		}
	}
	
	return true;
}


function ShowDivByID(szID, bShow)
{
	
	
	var h = window.top.frames["bodyframe"]
	
	var currSelected  = h.document.getElementById(szID);
	var filename = null;
	
	switch(szID) {
	case "setting":
		filename = "setting";
		break;
	case "code":
		filename = "code";
		break;
	case "memcheck":
		filename = "valgrind.memcheck.report.final";
		break;
	case "callgrind":
		filename = "valgrind.callgrind.report.final";
		break;
	case "callgrind_annotate":
		filename = "valgrind.callgrind_detail.report.final";
		break;		
	case "massif":
		filename = "valgrind.massif_out_ms_print.report.final";
		break;
	case "callcheck":
		filename = "syscall_usage.report.final";
		break;
	case "callgrind_check":
		filename = "check_msg_callgrind.report.final";
		break;
	case "memcheck_check":
		filename = "check_msg_memcheck.report.final";
		break;
	case "gcc":
		filename = "gcc_internal.report.final";
		break;
	case "script":
		filename = "script_runtime_log.report.final";
		break;
	case "illegal_headers":
		filename = "illegal_header.report.final";
		break;
	case "analysis_result":
		filename = "analysis_result.final";
		progress_value = progress_value_max;
		break;
	case "check_pattern_concise":
		filename = "check_pattern_concise.report.final";
		break;
	default:
		filename = "invalidxxx";
		break;
	}
	
	if(currSelected != null ) {

//		alert(szID);
		
					
			if ( /*currSelected.style.display =="none" || */load_state[szID]<2 ) {
				var timestamp = new Date().getTime();
				request_get_result.open('GET', '../view_result.php?file='+filename+"."+timestamp, false);
				//request_get_result.open('GET', '../view_result.php?file='+filename, false);
				request_get_result.send(null);									// send request
				var result = request_get_result.responseText;

	
				if ( result.search("result not ready") <0 ) {

					currSelected.innerHTML = result;

					load_state[szID] = 2;	// successfully loaded
				}
				else {
					//alert (szID + " = " + load_state[szID]);
				}

			}
/*			
		if ( bShow) {
	
			currSelected.style.display = "block";
		}
		else
			currSelected.style.display = "none";*/
	}
};




// define reference to the progress bar and create XMLHttp request object
window.onload = function () {

	load_state = { "memcheck":0, "callgrind":0, "callgrind_annotate":0 , "massif":0, "callcheck":0, "callgrind_check":0, 
					"memcheck_check":0, "gcc":0, "illegal_headers":0, "analysis_result":0,
					"check_pattern_concise":0,  "setting": 0, "code": 0};



	progress = document.getElementById('progress');
	request = initXMLHttpClient();
	request_get_result = initXMLHttpClient(); 
	progress_value = 0;
	polling_start();
	
	
};


// create XMLHttp request object in a cross-browser manner
initXMLHttpClient = function () {
	var XMLHTTP_IDS,
		xmlhttp,
		success = false,
		i;
	// Mozilla/Chrome/Safari/IE7+ (normal browsers)
	try {
		xmlhttp = new XMLHttpRequest(); 
	}
	// IE(?!)
	catch (e1) {

	//	alert("XMLHttpReuquest not available");
		XMLHTTP_IDS = [ 'MSXML2.XMLHTTP.5.0', 'MSXML2.XMLHTTP.4.0',
						'MSXML2.XMLHTTP.3.0', 'MSXML2.XMLHTTP', 'Microsoft.XMLHTTP' ];
		for (i = 0; i < XMLHTTP_IDS.length && !success; i++) {
			try {
				success = true;
				xmlhttp = new ActiveXObject(XMLHTTP_IDS[i]);
			}
			catch (e2) {}
		}
		if (!success) {
			throw new Error('Unable to create XMLHttpRequest!');
		}
	}
	return xmlhttp;
};

function getCookie(CookieName)
{

	var CookieVal = null;

	if(document.cookie)	   //only if exists
	{

		var arr = document.cookie.split((escape(CookieName) + '=')); 

		if(arr.length >= 2)
		{
			var arr2 = arr[1].split(';');
			CookieVal  = unescape(arr2[0]); //unescape() : Decodes the String
		}

    }

    return CookieVal;
}

// send request to the server
send_request = function () {
//	if (number < number_max) {
	if( progress_value < progress_value_max  /*|| IsNoPending()==false*/) {
  		var timestamp = new Date().getTime();

		request.open('GET', 'ajax-progress-bar.php?sn='+timestamp, true);	// open asynchronus request
		request.onreadystatechange = request_handler;		// set request handler	
		request.send(null);									// send request
		number++;											// increase counter
	}
	else {
	
		polling_stop();
	}
};


// request handler (started from send_request)
request_handler = function () {
	var level;
	var new_v;
	
	
	if (request.readyState === 4) { // if state = 4 (operation is completed)
		if (request.status === 200) { // and the HTTP status is OK
			// get progress from the XML node and set progress bar width and innerHTML
			var x0  = request.responseXML.getElementsByTagName('PROGRESS');
			var x1 = x0[0];

			if ( x1 != null) {
				level = x1.firstChild;
				new_v = level.nodeValue;


				if ( new_v > progress_value) {
					progress_value = new_v;
				
					if (progress_value > progress_value_max)
						progress_value = progress_value_max;
				}
			
				if ( progress_value == progress_value_max) {
					var h = window.top.frames["bodyframe"]
	
					var btnExpire  = h.document.getElementById("ExpireItNowButton");
					
					if (btnExpire!= null)
						btnExpire.style.display = "none"	;
					
				}

			
				var milestones = request.responseXML.getElementsByTagName('MILESTONE');
				var flag = true;

				for (var i = 0; i < milestones.length; i++) {
					var ms = milestones[i].firstChild.nodeValue;
					if (ms == "done") flag = false;
//					alert(ms + ' ' + load_state[ms]);
					
					if ( load_state[ms] < 2) {
						load_state[ms] = 1;

						if ( ms=="code" )
							ShowDivByID(ms, false);
						else
							ShowDivByID(ms,true);
					}
				}

				if (flag) {
					norm_progress_value = Math.round(progress_value*100 / progress_value_max);
					progress.style.width = progress.innerHTML = norm_progress_value + '%';
				} else {
					progress.style.width = '100%';
					progress.innerHTML = 'analysis completed';
				}

			}
		}
		else { // if request status is not OK
			progress.style.width = '100%';
			progress.innerHTML = 'Error:[' + request.status + ']' + request.statusText;
		}
	}
};


// button start
polling_start = function () {

//	logon_session =	getCookie("logon_session");

	if (!intervalID) {
		// set initial value for current number of requests
		number = 0;
//		 request.onreadystatechange = request_handler;       // set request handler
		// start polling
		intervalID = window.setInterval('send_request()', 15*1000);
		send_request();
	}

//	ShowDivByID("code",false);
//	ShowDivByID("setting",true);	
};


// button stop
polling_stop = function () {
	// abort current request if status is 1, 2, 3
	// 0: request not initialized 
	// 1: server connection established
	// 2: request received 
	// 3: processing request 
	// 4: request finished and response is ready
	if (0 < request.readyState && request.readyState < 4) {
		request.abort();
	}
	window.clearInterval(intervalID);
	intervalID = false;
	// display 'Demo stopped'
	progress.style.width = '100%';
	progress.innerHTML = 'analysis completed';
	
	ShowDivByID("msg_still_running", false);
};
