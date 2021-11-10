#!/usr/bin/php
<?php

$HW_NAME = "<!CODESENSOR_HW_NAME!>";

include ("/var/www/html/".$HW_NAME."/account.php");
//include ("/var/www/html/config.php");

//$HOMEWORK_DIR = "/var/homeworks";
$HOMEWORK_DIR = $root_dir;

$ANALYSIS_FILE = "analysis_result.final";
$USER_DEFINED_METRIC_FILE = "user_defined_metric.report.final";
$SCOREBOARD_OUTPUT = "/var/www/html/".$HW_NAME."/scoreboard.php";

$MASSIF_USE_PAGES_AS_HEAP=true;

$LATEST_ANALYSIS_RESULT_MTIME = 0;
$SCOREBOARD_MTIME = filemtime($SCOREBOARD_OUTPUT);	// CHANGE THIS TO 0 to force generation of scoreboard

$use_base_user = true;
$baseM = 31122354	; 
$baseI = 1210892124;
$baseCPU_TIME  = 1000;
$baseFLT  = 1000;
$baseMAXRSS = 1000;

				// 9223372036854775807
const INFINITY = 9223372036854775100; // PHP_INT_MAX-100

const CPU_TIME_UNIT = 10000;	//getrusage cputime granularity threshold (us)

$score_P_ratio = 0.4;
$TOTAL_P_SCORE = 6000;
$DIVERSITY_MULTIPLIER = 1;
$LATE_SUBMISSION_PENALTY = 5;

$USE_SCALABLE_TOTAL_P_SCORE = false;

$NUM_OF_SUBMISSION = 0;
$NUM_OF_EFFECTIVE_SUBMISSION = 0;

$REMAINING_P_SCORE = $TOTAL_P_SCORE;

$SHOW_CODE = true;
$CODE_BLANK_FILE = "/usr/bin/HomeworkInspector/$HW_NAME/code_blank.h";

$score_board = array();


$szDeadline = GetConfigValueList("deadline");

if ( count($szDeadline) !=1 )
	exit(-1);

//echo $szDeadline[0]. "<br/>";

$deadline = date_parse_from_format("Y-n-j H:i:s", $szDeadline[0]);

if ( $deadline==false)
	exit(-1);
//echo var_dump($deadline)."<br/>";

$tmDeadline = mktime($deadline["hour"], $deadline["minute"], $deadline["second"], 
                     $deadline["month"], $deadline["day"], $deadline["year"]);


function bytesToSize($bytes, $precision = 2)
{  
    $kilobyte = 1024;
    $megabyte = $kilobyte * 1024;
    $gigabyte = $megabyte * 1024;
    $terabyte = $gigabyte * 1024;

    if ($bytes < 0)
    	return $bytes;

   	if ($bytes>= INFINITY)
   		return "&infin;";    
   
    if (($bytes >= 0) && ($bytes < $kilobyte)) {
        return $bytes . ' B';
 
    } elseif (($bytes >= $kilobyte) && ($bytes < $megabyte)) {
        return round($bytes / $kilobyte, $precision) . ' KB';
 
    } elseif (($bytes >= $megabyte) && ($bytes < $gigabyte)) {
        return round($bytes / $megabyte, $precision) . ' MB';
 
    } elseif (($bytes >= $gigabyte) && ($bytes < $terabyte)) {
        return round($bytes / $gigabyte, $precision) . ' GB';
 
    } elseif ($bytes >= $terabyte) {
        return round($bytes / $terabyte, $precision) . ' TB';
    } else {
        return $bytes . ' B';
    }
}

function TimeAbbrev($t, $precision = 0)
{  
	$ms = 1000;
	$s = $ms * 1000;
	$min = $s * 60;
	$hour = $min * 60;
    
    if ( $t < 0)
    	return $t;

   	if ($t>= INFINITY)
   		return "&infin;";

    if ( ($t >= 0) && ($t < 1000)) {
        return $t . ' us';
 
    } elseif (($t >= $ms) && ($t < $s)) {
        return round($t / $ms, $precision) . ' ms';
 
    } elseif (($t >= $s) && ($t < $min)) {
        return round($t / $s, $precision) . ' s';
 
    } elseif (($t >= $min) && ($t < $hour)) {
        return round($t / $min, $precision) . ' m';
 
    } elseif ($t >= $hour) {
        return round($t / $hour, $precision) . ' h';
    } else {
        return $bytes . '  ';
    }
}

function IrCntAbbrev($bytes, $precision = 2)
{  
    $kilobyte = 1024;
    $megabyte = $kilobyte * 1024;
    $gigabyte = $megabyte * 1024;
    $terabyte = $gigabyte * 1024;
   
   	if ( $bytes<0)
   		return $bytes;

   	if ($bytes>= INFINITY)
   		return "&infin;";

    if (($bytes >= 0) && ($bytes < $kilobyte)) {
        return $bytes . '  ';
 
    } elseif (($bytes >= $kilobyte) && ($bytes < $megabyte)) {
        return round($bytes / $kilobyte, $precision) . ' K';
 
    } elseif (($bytes >= $megabyte) && ($bytes < $gigabyte)) {
        return round($bytes / $megabyte, $precision) . ' M';
 
    } elseif (($bytes >= $gigabyte) && ($bytes < $terabyte)) {
        return round($bytes / $gigabyte, $precision) . ' G';
 
    } elseif ($bytes >= $terabyte) {
        return round($bytes / $terabyte, $precision) . ' T';
    } else {
        return $bytes . '  ';
    }
}

function rgb2html($r, $g=-1, $b=-1)
{
    if (is_array($r) && sizeof($r) == 3)
        list($r, $g, $b) = $r;

    $r = intval($r); $g = intval($g);
    $b = intval($b);

    $r = dechex($r<0?0:($r>255?255:$r));
    $g = dechex($g<0?0:($g>255?255:$g));
    $b = dechex($b<0?0:($b>255?255:$b));

    $color = (strlen($r) < 2?'0':'').$r;
    $color .= (strlen($g) < 2?'0':'').$g;
    $color .= (strlen($b) < 2?'0':'').$b;
    return '#'.$color;
}                     
                     
function shuffle_assoc($list) {
  if (!is_array($list)) return $list;

  $keys = array_keys($list);
  shuffle($keys);
  $random = array();
  foreach ($keys as $key)
    $random[$key] = $list[$key];

  return $random;
} 



function fill_missing_entry()
{
	global $score_board;

	$all_keys = array();	

	for ( $k = 0; $k < count($score_board); $k++) {
		foreach($score_board[$k] as $key => $value) {
			$all_keys[ $key ] = true;
		}

		$score_board[$k]["missing"] = 0;
	}

	//// handle run.sh.settings.report

	for ( $k = 0; $k < count($score_board); $k++) {
		if ( !array_key_exists( "m_error",$score_board[$k]) && array_key_exists("@memcheck", $score_board[$k]) && $score_board[$k]["@memcheck"]!="true" ) {
			$score_board[$k]["heap_lost"] = 0;
			$score_board[$k]["m_error"] = 0;
		}

		if ( !array_key_exists("I_refs",$score_board[$k])  && array_key_exists("@callgrind", $score_board[$k]) && $score_board[$k]["@callgrind"]!="true" ) {
			$score_board[$k]["I_refs"] = INFINITY;
		}		

		if ( !array_key_exists("m_peak",$score_board[$k]) && array_key_exists("@massif", $score_board[$k]) && $score_board[$k]["@massif"]!="true" ) {
			$score_board[$k]["m_peak"] = INFINITY;
		}			
	}	

	////// fill missing
	for ( $k = 0; $k < count($score_board); $k++) {
		foreach ($all_keys as $key => $value) {
			if ( $key == 'id')
				continue;
			if (  dump_scoreboard_hide_key($key) )
				continue;

			if ( $key[0]=="@")		//ignore values from run.sh.settings.report
				continue;

			if ( $key == 'lang' && array_key_exists($key, $score_board[$k])==true)
				continue;
			/*
			if( ($key=='maxrss' || $key=='flt' || $key=='cpu_time' || key=='I_refs' || key=='m_peak' || key=='heap_lost') && array_key_exists($key, $score_board[$k])==false ) {
				$score_board[$k][$key] = INFINITY;
				continue;
			}

			if( ($key=='check' ) && array_key_exists($key, $score_board[$k])==false ) {
				$score_board[$k][$key] = 0;
				continue;
			}
			*/

			if ( array_key_exists($key, $score_board[$k])==false || !is_numeric($score_board[$k][$key])) {
				$score_board[$k][$key] = -1;
				$score_board[$k]["missing"]++;
			}
		}
	}
}

function GetTextFileLineCount($file)
{
	if ( !file_exists($file) )
		return 0;

	$fp = fopen($file, "rt");

	if ( !$fp)
		return -1;

	$line_cnt = 0;

    while (($buffer = fgets($fp, 4096)) !== false) {
		$line_cnt++;
    }	

	fclose($fp);

	return $line_cnt;
}

function CalcTextFileDissim($file1, $file2)
{

	if ( !file_exists($file1) || !file_exists($file2))
		return -1;

	$diff = tempnam("/tmp", "cal_sim");

	system("diff -bB $file1 $file2 > $diff"	);

	$lc1 = GetTextFileLineCount($file1);
	$lc2 = GetTextFileLineCount($file2);

	//printf("$file1 : $lc1 lines\n");
	//printf("$file2 : $lc2 lines\n");

	if ( $lc1 < 0 || $lc2 <0) {
		unlink($diff);
		return -1;
	}

	$fp = fopen($diff, "rt");

	if ( $fp == false)	 {
		unlink($diff);
		return -1;
	}
	
	$n_diff = 0;

    while (($buf = fgets($fp, 4096)) !== false) {
		if ( $buf[0]=='<' || $buf[0] =='>' || $buf[0] =='-')
			continue;

//		printf("$buf");


		if ( preg_match('/([0-9]+),([0-9]+)[acd]/', $buf, $matches, PREG_OFFSET_CAPTURE)>0) {
			$ls = (int)($matches[1][0]);
			$le = (int)($matches[2][0]);
		}
		else if ( preg_match('/([0-9]+)[acd]/', $buf, $matches, PREG_OFFSET_CAPTURE) >0 ) {
			$ls = $le = (int)($matches[1][0]);
		}

		if ( preg_match('/[acd]([0-9]+),([0-9]+)/', $buf, $matches, PREG_OFFSET_CAPTURE)>0) {
			$rs = (int)($matches[1][0]);
			$re = (int)($matches[2][0]);
		}
		else if ( preg_match('/[acd]([0-9]+)/', $buf, $matches, PREG_OFFSET_CAPTURE) >0 ) {
			$rs = $re = (int)($matches[1][0]);
		}


	//	printf("Found: $ls-$le    $rs-$re\n");

		//printf("\n");
		


		$n_diff += $le-$ls + 1 + $re - $rs;
    }	

	$dvalue = $n_diff / ($lc1 + $lc2);


	fclose($fp);	

	unlink($diff);

	return $dvalue;
}

function dump_scoreboard_hide_key($key)
{
	global $MASSIF_USE_PAGES_AS_HEAP;

//	printf("[KEY : %s]\n", $key);

	if ( gettype($key) != "string")  {
		printf( "key should be string in dump_score_hide_key");
		var_dump(debug_backtrace());
		assert(false);
		exit(-1);	
	}

	if ( $MASSIF_USE_PAGES_AS_HEAP==true) {
		switch ($key) {
		case "m_heap":
		case "m_heap_extra":
		case "m_stack":
//			printf("Hit !");
			return true;
		default:
			
		}	
	}

	switch ($key) {
	case "m_text":
	case "m_bss":
	case "m_data":
//	case "maxrss":	
//			printf("Hit !");
		return true;
	default:
			
	}	

	if ( $key[0] == '@')
		return true;

	return false;
}

function dump_scoreboard($filename)
{
	global $score_board;
	global $SHOW_CODE;
	global $tmDeadline;

	$fp = fopen($filename, "w");

	if ( $fp == false)
		return;	

	fprintf($fp, "<html>\n");
	
	
	fprintf($fp, "<HEAD><title>Scoreboard View Result</title>\n");
	fprintf($fp, '<meta http-equiv="Content-Type" content="text/html; charset=utf-8">');
	fprintf($fp, '<STYLE TYPE="text/css">');
  	fprintf($fp, '.normal { background-color: rgb(255,255,255); color: rgb(60,60,60); } ');
  	fprintf($fp, '.highlight { background-color: rgb(255,246,0);  color: rgb(0,0,0); }');

  	fprintf($fp, '.cart { width: 100%; }');
	fprintf($fp, '.hasTooltip span {display: none;  color: #000;   text-decoration: none;   padding: 3px;}');
	fprintf($fp, '.hasTooltip:hover span {display: block;  position: absolute; background-color: #FF99CC; opacity:0.85; border: 0px;   margin-left: 80px; margin-top:15px;}');

	fprintf($fp, "</STYLE>\n\n");

	
	fprintf($fp, "</HEAD>\n");
	
	fprintf($fp, "<BODY><h2> </h2>\n");

	fprintf($fp, '<div style="text-align:center; font-size:24; font-family: tahoma; color: #003366"><!CODESENSOR_HW_NUM!>. <!CODESENSOR_HW_TITLE!></div>');		
	fprintf($fp, '<div style="text-align:center; font-size:16; font-family: sans-serif; color: #FF0066"><br/>Deadline: %s</div>',  date('Y-n-j H:i:s', $tmDeadline) );
	fprintf($fp, '<div  style="font-family:arial; font-size:16; text-align:right; font-style:italic; color: gray" >generated at : %s</div>',  date('Y-n-j H:i:s') );
	


	fprintf($fp, '<table border=5 width=100%%  style="font-family: sans-serif" >' );

	if ( count($score_board) >0) {

		$key_order = array();
/*		
		for ( $k = 0; $k < count($score_board[0]); $k++)
			$key_order[$k] = null;
*/		
		$key_order[0] = "id";
		$key_order[1] = "check";		
		$key_order[2] = "I_refs";
		$key_order[3] = "cpu_time";
		$key_order[4] = "m_peak";		
		$key_order[5] = "heap_lost";		
		$key_order[6] = "m_error";		
		$key_order[7] = "gcc";
		$key_order[8] = "illegal_func";
		$key_order[9] = "illegal_header";
		$key_order[10] = "lang";

		
		/// fill the unspecified ones 
		foreach($score_board[0] as $key => $value) {
						
			if ( in_array($key, $key_order)==false) {
				$key_order[ count($key_order) ] = $key;
			}
		}

//		var_dump($key_order);

		/// printe header
		fprintf($fp, "<tr>  ");		

		foreach($key_order as $key => $value) {
	
			if ( dump_scoreboard_hide_key($value) )
				goto skip;

			switch($value) {
				case "illegal_header":
					$value = "i.h.";
					break;
				case "illegal_func":
					$value = "i.f.";
					break;
				case "m_error":
					$value = "m.e.";
					break;
				case "m_heap_extra":
					$value = "m_heap_e";
					break;	
				case "heap_lost":
					$value = "lost";
					break;		
				case "missing":
					$value = "?";
					break;		
				case "m_total";
					goto skip;				
				default:
					break;
			}
				
			fprintf($fp, "<th>%s</th>", $value);	
skip:
		}
		fprintf($fp, "</tr>\n");

		///// print entries
		for ( $k = 0; $k < count($score_board); $k++) {

			if ( GetSubmissionTime($score_board[$k]['id']) > $tmDeadline ) 
				fprintf($fp, "<tr style=\"background-color: #66FF66\">");
			else
				fprintf($fp, "<tr onMouseOver=\"this.className='highlight'\" onMouseOut=\"this.className='normal'\">");

			$id = $score_board[$k]['id'];

			foreach($key_order as $tmp => $key) {

				if ( dump_scoreboard_hide_key($key) )
					goto skip_data_col;

				if ( array_key_exists($key, $score_board[$k])==false) {
					printf("Error! missing entry ".$key." in scoreboard at id=%s\n", $score_board[$k]["id"]);
					exit(-1);
				}
				
				$value = $score_board[$k][$key];
				

				if ( $key=="id" && $SHOW_CODE) {
					fprintf($fp, "<td align=center><a href=\"%s/code/code.html\">%s</a></td>", $id, $value);
					
				}
				else if ($key=="m_total") {
					goto skip_data_col;
/*
					settype($value,"float");
					
					if ( $SHOW_CODE) {
						fprintf($fp, "<td align=center><a href=\"%s/valgrind.massif_out_ms_print.report.final\">%.3e</a></td>", $id, $value);
					}
					else {
						fprintf($fp, "<td align=center>%.3e</td>", $value);
					}
*/
                }
				else if ( $key=="score") {
					$v =  is_float($value) ? round($value) : $value;
					fprintf($fp, '<td align=center style="color:#FF0000;">%d</td>', $v );
				}
				else if ( $key=="p_score") {
					$v =  is_float($value) ? round($value) : $value;					
					fprintf($fp, '<td align=center class="hasTooltip"><span>p_score</span>  %d  </td>', $v );
				}
				else if ( $key=="priority") {
					if (is_float($value) )
						fprintf($fp, '<td align=center class="hasTooltip"><span>priority</span>%.3f</td>', round($value,3) );
					else
						fprintf($fp, '<td align=center class="hasTooltip"><span>priority</span>%d</td>', $value );
				}
				else if ($key=="I_refs") {
					if ( $SHOW_CODE) {
						fprintf($fp, '<td align=center class="hasTooltip"><span>I_refs</span><a href="%s/callgrind/code.html">%s</a></td>', $id, IrCntAbbrev($value));
						//fprintf($fp, "<td align=center><a href=\"%s/callgrind/code.html\">%s</a></td>", $id, number_format($value));
					}
					else { 
						fprintf($fp, '<td align=center class="hasTooltip"><span>I_refs</span>%s</td>', IrCntAbbrev($value));
						//fprintf($fp, "<td align=center>%s</td>", number_format($value));
					}
				}
				else if ( $key =="cpu_time") {
					fprintf($fp, '<td align=center class="hasTooltip"><span>cpu_time</span>%s</td>', TimeAbbrev($value));
				}
				else if ( $key=="m_peak") {
					fprintf($fp, '<td align=center class="hasTooltip"><span>m_peak</span>%s</td>', bytesToSize($value));
				}
				else if ( $key=="maxrss") {
					fprintf($fp, '<td align=center class="hasTooltip"><span>maximum resident set size</span>%s</td>', bytesToSize($value));
				}
				else if ( $key=="flt")  {
					fprintf($fp, '<td align=center class="hasTooltip"><span>flt (# of page faults)</span>%s</td>', IrCntAbbrev($value));	
				}
				else if ( $key=="diversity")  {
					fprintf($fp, '<td align=center class="hasTooltip"><span>diversity</span>%.2f</td>', round($value,2));	
				}
				else if ( $key=="func")  {
					fprintf($fp, '<td align=center class="hasTooltip"><span>functionality</span>%.3f</td>', round($value,3));	
				}
				else if ( $key=="missing")  {
					fprintf($fp, '<td align=center class="hasTooltip"><span># of missing entries</span>%d</td>', $value);	
				}
				else if ( $key=="lang")  {
					fprintf($fp, '<td align=center class="hasTooltip"><span>language</span>%s</td>', $value);	
				}
				else if ( $key=="illegal_header")  {
					fprintf($fp, '<td align=center class="hasTooltip"><span># of illegal header inclusions</span>%d</td>', $value);	
				}				
				else if ( $key=="illegal_func")  {
					fprintf($fp, '<td align=center class="hasTooltip"><span># of illegal function usages</span>%d</td>', $value);	
				}				
				else if ( $key=="gcc")  {
					fprintf($fp, '<td align=center class="hasTooltip"><span>gcc compilation exit code</span>%d</td>', $value);	
				}	
				else if ( $key=="m_error")  {
					fprintf($fp, '<td align=center class="hasTooltip"><span># of memory errors</span>%d</td>', $value);	
				}			
				else if ( $key=="heap_lost")  {
					fprintf($fp, '<td align=center class="hasTooltip"><span>heap lost</span>%s</td>', bytesToSize($value));	
				}
				else if ( $key=="check")  {
					if ( is_numeric($value) && $value >=0 )
						fprintf($fp, '<td align=center class="hasTooltip"><span>check</span>%.2f</td>', round($value,2));	
					else
						fprintf($fp, '<td align=center class="hasTooltip"><span>check</span>-1</td>' );	
				}
				else {
					if ( is_float($value))
						fprintf($fp, "<td align=center>%.2f</td>", round($value,2));
					else
						fprintf($fp, "<td align=center>%s</td>", $value);
				}	
skip_data_col:	
			}
			fprintf($fp , "</tr>\n");
		}

	}

	fprintf($fp, "</table>\n");
	///////////////////////////////////
	
	//fprintf($fp, '<iframe src="https://sensecloud.cs.nctu.edu.tw/smarttext/" width="100%%" height="60" frameBorder="0"  align="left" SCROLLING=NO></iframe>');
	
	
	
	///////////////////////////////////////
	global $baseI;
	global $baseCPU_TIME;
	global $baseM;
	global $baseFLT;
	global $baseMAXRSS;
	global $TOTAL_P_SCORE;
	global $score_P_ratio;
	global $REMAINING_P_SCORE;	
	global $NUM_OF_SUBMISSION;
	global $NUM_OF_EFFECTIVE_SUBMISSION;
	global $USE_SCALABLE_TOTAL_P_SCORE;
	global $DIVERSITY_MULTIPLIER;

	fprintf($fp, "<br/>number of submission = %d<br/>", $NUM_OF_SUBMISSION);	
	fprintf($fp, "<br/>number of effective submission (priority>0) = %d<br/>", $NUM_OF_EFFECTIVE_SUBMISSION);
	
	if ( $USE_SCALABLE_TOTAL_P_SCORE)
		fprintf($fp, "<br/>total p_score = (%d * 80) = %d<br/>", $NUM_OF_EFFECTIVE_SUBMISSION, $TOTAL_P_SCORE);

	fprintf($fp, "<br/>remaining p_score = %.2f<br/><br/>", $REMAINING_P_SCORE);
	fprintf($fp, "<br/><a href=\"code_sensor_fields_meanining.htm\">Field Definitions</a><br/><br/>");
	
	//fprintf($fp, "<br/><br/>total_score = %d<br/>\n", $TOTAL_P_SCORE);
	fprintf($fp, "<br/><br/><br/>if (gcc | illegal_func | illegal_header | missing),<br/>");
	fprintf($fp, " &nbsp;  &nbsp; priority = 0<br/>");
	fprintf($fp, " &nbsp;  &nbsp; func = 0<br/>");
	fprintf($fp, "else,<br/>");
	fprintf($fp, " &nbsp;  &nbsp; func = check * (0.95^m_error)<br/>");
	fprintf($fp, " &nbsp;  &nbsp; if ( check==1 ),<br/>");

	fprintf($fp, " &nbsp;  &nbsp; &nbsp; &nbsp; perf_I1 = lg(2+%d)/lg(2+I_refs) <br/>",  $baseI);
	fprintf($fp, " &nbsp;  &nbsp; &nbsp; &nbsp; perf_I2 = lg(2+%d)/lg(2+cpu_time) )<br/>",  $baseCPU_TIME);
	fprintf($fp, " &nbsp;  &nbsp; &nbsp; &nbsp; perf_I = (perf_I1>0) ? perf_I1 :  perf_I2<br/>",  $baseCPU_TIME);
	fprintf($fp, " &nbsp;  &nbsp; &nbsp; &nbsp; priority = perf_I * max( lg(2+%d)/lg(2+m_peak) , lg(2+%d*%d)/lg(2+flt*maxrss) )<br/>",  $baseM, $baseFLT, $baseMAXRSS );
	

//	fprintf($fp, " &nbsp;  &nbsp; &nbsp; &nbsp; priority = lg( _pri+2 ) <br/>",  ($baseI), ($baseCPU_TIME));
	fprintf($fp, " &nbsp;  &nbsp; else,<br/>");
	fprintf($fp, " &nbsp;  &nbsp; &nbsp; &nbsp; priority = 0<br/>",  $baseI, $baseM);
	
	fprintf($fp, "<br/>p_score = <a href=\"CreateScoreboard_release.html\">schedule</a>(%d, priority)<br/><br/>", $TOTAL_P_SCORE);

	fprintf($fp, "if (m_peak > 0),<br/>");
	fprintf($fp, " &nbsp;  &nbsp; func *=  (m_peak-heap_lost)/m_peak<br/><br/>");
	fprintf($fp, "diversity = -ln(  [number of effective submissions using the same language] / [number of effective submissions] ) * %.2f<br/><br/>", $DIVERSITY_MULTIPLIER);
	fprintf($fp, "score = min( func*(%.2f*100 + %.2f*p_score + diversity), 100)<br/>", 1-$score_P_ratio, $score_P_ratio );
	///////////////////////////////////
	
	
	fprintf($fp, "</body></html>\n");
	

	fclose($fp);
}

function IsValidScoreboardEntry($k)
{
	global $score_board;
	
	$check_list = array("gcc","illegal_func","missing","illegal_header");
	
	for($i=0; $i < count($check_list); $i++) {
		if ( array_key_exists($check_list[$i], $score_board[$k])==false )
			return false;

		if ( $score_board[$k][$check_list[$i]]!=0 )
			return false;
	}
	
	return true;
}

function calculate_diversity()
{
	global $score_board;
	global $baseI;
	global $baseM;
	global $use_base_user;
	global $NUM_OF_EFFECTIVE_SUBMISSION ;	
	global $TOTAL_P_SCORE;
	global $USE_SCALABLE_TOTAL_P_SCORE;
	global $tmDeadline;
	global $DIVERSITY_MULTIPLIER;


	
//	$NUM_OF_EFFECTIVE_SUBMISSION = 0;
	$LANG_CNT = array();
	
	for ( $k = 0; $k < count($score_board); ++$k) {
		if ( IsValidScoreboardEntry($k) && $score_board[$k]["check"] == 1 && 
			GetSubmissionTime($score_board[$k]["id"])<= $tmDeadline  &&
			$score_board[$k]["priority"]>0 ) {
//			$NUM_OF_EFFECTIVE_SUBMISSION++;
			$lang = $score_board[$k]["lang"];
			if ( array_key_exists($lang, $LANG_CNT) ) {
				if ($lang == "cpp") {
					$LANG_CNT["cpp_shm"]++;
				} elseif ($lang == "cpp_shm") {
					$LANG_CNT["cpp"]++;
				}

				$LANG_CNT[$lang]++;
			}
			else {
				if ($lang == "cpp") {
					$LANG_CNT["cpp_shm"] = 1;
				} elseif ($lang == "cpp_shm") {
					$LANG_CNT["cpp"] = 1;
				}

				$LANG_CNT[$lang]=1;
			}
		}
	}

		
	for ( $k = 0; $k < count($score_board); ++$k) {
		if ( IsValidScoreboardEntry($k) && $score_board[$k]["check"] == 1 && GetSubmissionTime($score_board[$k]["id"])<= $tmDeadline  ) {
			
			$diversity = -log( $LANG_CNT[$score_board[$k]["lang"]] / $NUM_OF_EFFECTIVE_SUBMISSION) * $DIVERSITY_MULTIPLIER;

		}
		else {
			$diversity = 0;
		}	
	
		$score_board[$k]["diversity"] = $diversity;


	}


}

function comp($v)
{
	
	return log($v+2, 2);
//	return $v;
}

function calculate_priority()
{
	global $score_board;
	global $baseI;
	global $baseM;
	global $baseCPU_TIME;
	global $baseFLT;
	global $baseMAXRSS;
	global $use_base_user;
	global $NUM_OF_EFFECTIVE_SUBMISSION ;	
	global $TOTAL_P_SCORE;
	global $USE_SCALABLE_TOTAL_P_SCORE;
	global $tmDeadline;
	global $CPU_TIME_UNIT;

	$mem_key = "m_peak";
	
	$NUM_OF_EFFECTIVE_SUBMISSION = 0;
	
	if ( $use_base_user ) {

		for ($k=0; $k < count($score_board); ++$k) {
			if ($score_board[$k]["id"] == "baseline") {
				break;
			}
		}
	
		if ($k < count($score_board)) {
			
			if ( array_key_exists("I_refs", $score_board[$k]) && array_key_exists($mem_key,$score_board[$k])) {
				if ( $score_board[$k]["I_refs"] != -1 && $score_board[$k][$mem_key] != -1) {
					$baseI = $score_board[$k]["I_refs"];
					$baseM = $score_board[$k][$mem_key];
					$baseCPU_TIME = $score_board[$k]["cpu_time"];
					$baseFLT = $score_board[$k]["flt"];
					$baseMAXRSS = $score_board[$k]["maxrss"];
				}
			}
		}
	}
		
	for ( $k = 0; $k < count($score_board); ++$k) {
		if ( IsValidScoreboardEntry($k) && $score_board[$k]["check"] == 1 && GetSubmissionTime($score_board[$k]["id"])<= $tmDeadline  ) {
			
			if ($score_board[$k]["I_refs"]>= INFINITY)
				$perf_I1 = 0;
			else	
				$perf_I1 = comp($baseI) / comp($score_board[$k]["I_refs"]);

			if ( $perf_I1 > 0) {
				$perf_I2 = 0;
			}
			else {
				if ($score_board[$k]["cpu_time"]>=INFINITY)
					$perf_I2 = 0;
				else
					$perf_I2 = comp($baseCPU_TIME) / comp($score_board[$k]["cpu_time"]);

				if ( $score_board[$k]["cpu_time"]<= CPU_TIME_UNIT && $perf_I1 < $perf_I2) {
					$perf_I2 += $perf_I1/100;
				}
			}

			$perf_I = max($perf_I1, $perf_I2);
			
			$perf_M1 = comp($baseM) / comp($score_board[$k][$mem_key]);
			
			if ( $score_board[$k]["flt"] >= INFINITY || $score_board[$k]["maxrss"]>=INFINITY)
				$perf_M2 = 0;
			else
				$perf_M2 = comp($baseFLT*$baseMAXRSS) / comp($score_board[$k]["flt"]*$score_board[$k]["maxrss"]);

			$perf_M = max ($perf_M1, $perf_M2);
//			$perf_M = $perf_M2;

			$priority = $perf_I * $perf_M;
			//$priority = pow($priority, 0.5);
//			$priority = log($priority + 2, 2) / log(3,2);

		}
		else {
			$priority = 0;
		}	
	
		$score_board[$k]["priority"] = $priority;

		if ( $priority>0)
			$NUM_OF_EFFECTIVE_SUBMISSION++;
	}


	if ($USE_SCALABLE_TOTAL_P_SCORE) {
		$TOTAL_P_SCORE = 80* $NUM_OF_EFFECTIVE_SUBMISSION;
		if ( $TOTAL_P_SCORE > 80*60)
			$TOTAL_P_SCORE = 80*60;
	}	
}


class ScorePriorityQueue extends SplPriorityQueue
{
    public function compare($priority1, $priority2)
    {
        if ($priority1 === $priority2) return 0;
        return $priority1 > $priority2 ? 1 : -1;
    }
}




function calculate_score()
{
	global $score_board;
	global $TOTAL_P_SCORE;
	global $score_P_ratio;
	global $REMAINING_P_SCORE;
	global $LATE_SUBMISSION_PENALTY;
	global $tmDeadline;


	$qCur = new ScorePriorityQueue();
	$qExpire = new ScorePriorityQueue();

	$total_p_score = $TOTAL_P_SCORE;
	$multiplier = -1;
	
	
	$qCur->setExtractFlags(ScorePriorityQueue::EXTR_DATA);
	$qExpire->setExtractFlags(ScorePriorityQueue::EXTR_DATA);
	
	foreach($score_board as $k=>$r) {
	
		
		///// f_score
		if (IsValidScoreboardEntry($k)) {
	
			//var_dump($score_board[$k]);
					
			$score_board[$k]["func"] = $score_board[$k]["check"]*pow(0.95,$score_board[$k]["m_error"]);
						
			if ( $score_board[$k]["m_peak"] >0) {
				$score_board[$k]["func"] *= ($score_board[$k]["m_peak"] -$score_board[$k]["heap_lost"]) / ($score_board[$k]["m_peak"]);
			}

			if ( $r["priority"]>0)
				$qCur->insert($k, $r["priority"]);
		}
		else {
			$score_board[$k]["func"] = 0;
		}
		
		////// init p_score
		$score_board[$k]["p_score"] = 0;
		
		if ( $multiplier==-1 || $score_board[$k]["priority"]  > $multiplier) {
			$multiplier = $score_board[$k]["priority"];
		}
	}

	if ($multiplier <=0 )
		$multiplier = 1;

	print $multiplier."\n";

	
	while ($qCur->valid() && $total_p_score >0) {

		$bufExpire = array();

		$qCurSize = $qCur->count();
		
		while($qCur->valid() /*&& $total_p_score >0*/) {
			$idx = $qCur->extract();

	
			$s = $score_board[$idx]["priority"]/$multiplier;	// alloted score in the scheduling cycle

//			print  $score_board[$idx]["priority"]."\n";

			if ( $s < 0.00001) {
				$s = 0.00001;
			}			

			if ( $s > 100 - $score_board[$idx]["p_score"])
				$s =  100 - $score_board[$idx]["p_score"];

			//if ( $s > $total_p_score)
			//	$s = $total_p_score;			
	
			$score_board[$idx]["p_score"] += $s;

			$total_p_score -= $s;

			if ( $total_p_score < 0)	// Hank 2015.4.6  address uneven score distribution for students with same priority
				$total_p_score = 0;
					
			if ($score_board[$idx]["p_score"] < 100 && $score_board[$idx]["priority"]>0) {
				$bufExpire[$idx] = $score_board[$idx]["priority"];
			}

		}


		{
			$bufExpire = shuffle_assoc($bufExpire);
			foreach ($bufExpire as $idx => $priority) {
				$qExpire->insert($idx, $score_board[$idx]["priority"]);
			}		
		}


		if ( $qCurSize > $qExpire->count() && $qExpire->valid()) {	// need new  multiplier
			$multiplier = $score_board[ $qExpire->top() ]["priority"];
			print $multiplier."\n";
		}

		$qCur = $qExpire;
		$qExpire = new ScorePriorityQueue();
		$qExpire->setExtractFlags(ScorePriorityQueue::EXTR_DATA);

	}
	
	foreach($score_board as $k=>$r) {

		
		$score_board[$k]["score"] = min( $score_board[$k]["func"]*((1-$score_P_ratio)*100 + $score_board[$k]["p_score"]*$score_P_ratio + $score_board[$k]["diversity"]), 100);

		
        if (GetSubmissionTime($score_board[$k]['id']) > $tmDeadline) {
            print "user ".$score_board[$k]['id']." is late\n";
            $score_board[$k]["score"] -= $LATE_SUBMISSION_PENALTY;
        }

        if (  $score_board[$k]["score"]  < 0)
             $score_board[$k]["score"]  = 0;		
		
		/// submission time /////
		$score_board[$k]["submission_time"] = date ("n/j H:i:s", GetSubmissionTime($score_board[$k]["id"]) );
		////////////////////////
	}


	$REMAINING_P_SCORE = $total_p_score;	
	////////sort by score
	
	$tmp = Array();
	
	foreach($score_board as &$ma)
		$tmp[] = $ma["score"] + $ma["priority"]/1000;
	    	
	array_multisort($tmp,SORT_DESC, $score_board ); 	
}



function parse_user_defined_metric( $id, $filename)
{
	global $NUM_OF_SUBMISSION;

	if (file_exists($filename))
	{
		global $score_board;

		$file = fopen($filename, "r");

		if ( $file == false) {
			return;
		}

		for( $idx = 0; $idx < count($score_board); $idx++) {
			if( $score_board[$idx]['id'] == $id )
				break;
		}
		
		if ( $idx == count($score_board) ) {
			echo "parse_user_defined_metric   ".$id." not exists\n";
			return;
		}
		

		while (!feof($file))
		{
			$buf = fgets($file, 1024);
			
			$property = strtok($buf, "=");
			
			if ($property==false) {
				continue;
			}
			
			$sw = array(" ", "\n", "\r", "\t");
			
			$property = str_replace($sw,"",$property);
			
			$value = strtok( "=");
			
			if ( $value==false) {
				continue;
			}
			
			$value = str_replace($sw, "", $value);
	
			 
			
			if ( is_numeric($value)) {
//				echo $value . "is numeric \n";
			
				if (strstr($value,"."))
					settype($value,"float");
				else
					settype($value,"integer");	
			}
			
			$score_board[$idx][$property] = $value;	
				
//			echo "\t[". $id ."]\t" . $property ." = ".$value."<br/>";
		}
		
		fclose($file);
	
	}
	else
	{
	//	echo $filename . " file not exists<br/>";
	}	
}

function parse_analysis_result( $id, $filename)
{
	global $NUM_OF_SUBMISSION;
	global $LATEST_ANALYSIS_RESULT_MTIME;

	$fmtime = filemtime($filename);

	if ( $fmtime > $LATEST_ANALYSIS_RESULT_MTIME)
		$LATEST_ANALYSIS_RESULT_MTIME = $fmtime;

	if (file_exists($filename))
	{
		global $score_board;

		$file = fopen($filename, "r");

		if ( $file == false) {
			return;
		}

		$idx = count($score_board);
		$score_board[$idx]['id'] = $id;

		//printf("%s type:%s\n", $id, gettype($id));	
		//printf("scoreboard %d is %s\n", $idx, $score_board[$idx]['id']);		

		while (!feof($file))
		{
			$buf = fgets($file, 1024);
			
			$property = strtok($buf, "=");
			
			if ($property==false) {
				continue;
			}
			
			$sw = array(" ", "\n", "\r", "\t");
			
			$property = str_replace($sw,"",$property);
			
			$value = strtok( "=");
			
			if ( $value==false) {
				continue;
			}
			
			$value = str_replace($sw, "", $value);
	
			 
			
			if ( is_numeric($value)) {
//				echo $value . "is numeric \n";
			
				if (strstr($value,"."))
					settype($value,"float");
				else
					settype($value,"integer");	
			}
			else if ($property[0] != '@') {
				 echo $property."= [".$value."]" . " is not numeric \n";

			}

			if ( $property == "@lang")
				$property = "lang";
			
			if ( $property == "cpu_time" ) {
				$value = round($value / CPU_TIME_UNIT,0)*CPU_TIME_UNIT;

				if ( $value < CPU_TIME_UNIT)
					$value = CPU_TIME_UNIT;
			}

			$score_board[$idx][$property] = $value;	
				
//			echo "\t[". $id ."]\t" . $property ." = ".$value."<br/>";
		}
		
		fclose($file);

		$NUM_OF_SUBMISSION++;	
	}
	else
	{
		echo $filename . " file not exists.\n";
	}
}


srand(time());

$h_dir = opendir($HOMEWORK_DIR);

if ( $h_dir== FALSE) {
	exit (-1);
}

$NUM_OF_SUBMISSION = 0;

while (($file = readdir($h_dir)) !== false) {
	if ( $file=="." || $file==".." || $file=="queue" || $file=="config" || $file=="customized_templates") {
		continue;
	}

	$dir = rtrim($HOMEWORK_DIR,'/') ."/" . $file;
	 
	if ( filetype($dir)!="dir") {
		continue;
	}
	 
	parse_analysis_result($file, $dir."/current/".$ANALYSIS_FILE);
	parse_user_defined_metric($file, $dir."/current/".$USER_DEFINED_METRIC_FILE);
	 
	//echo "filename: $file : filetype: " . filetype($HOMEWORK_DIR ."/" . $file) . "<br/>";
	
 
}

closedir($h_dir);
fill_missing_entry();

printf("Latest analysis result modification time = %s\n",  date ("F d Y H:i:s.", $LATEST_ANALYSIS_RESULT_MTIME));
printf("scoreboard.php modification time = %s\n",  date ("F d Y H:i:s.", $SCOREBOARD_MTIME));

calculate_priority();
calculate_diversity();
calculate_score();

dump_scoreboard($SCOREBOARD_OUTPUT);

?>
