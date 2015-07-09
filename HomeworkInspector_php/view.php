
<?php
  // no-cache headers - complete set
  // these copied from [php.net/header][1], tested myself - works
  header("Expires: Sat, 26 Jul 1997 05:00:00 GMT"); // Some time in the past
  header("Last-Modified: " . gmdate("D, d M Y H:i:s") . " GMT");
  header("Cache-Control: no-store, no-cache, must-revalidate");
  header("Cache-Control: post-check=0, pre-check=0", false);
  header("Pragma: no-cache");
?>

<HTML>



<HEAD>
	<title>View Result</title>
		<meta charset="utf-8">
	  	<link rel="stylesheet" href="https://code.jquery.com/ui/1.11.2/themes/smoothness/jquery-ui.css">
		<script src="https://code.jquery.com/jquery-1.10.2.js"></script>
		<script src="https://code.jquery.com/ui/1.11.2/jquery-ui.js"></script>
<!--		<link rel="stylesheet" href="style.css"> -->
		<script>
			$(function() {
				$( "#detailed_results" ).accordion({
					collapsible: true,
					heightStyle: "content"
				});
			});
		</script>
</HEAD>
<BODY>
<h2> </h2><br/>

<?php

//error_reporting(0);

//include ("config.php");

include_once ("account.php");

/*
if ( Authenticate($_POST["userid"], $_POST["userpwd"], $users)==FALSE && Authenticate("1000001", $_POST["userpwd"], $users)==FALSE) {
   echo "<font size=5 color=red>Account does not exist !</font></h1>";
   
   if (array_key_exists($_POST["userid"], $users) )
		mylog($_POST["userid"], "view result login faiulre");
   
	exit(-1);
}
*/

if ( !CookieAuthenticate($userid , $user_name)) {
	echo "<h1><font size=5 color=red>You need to login first !</font></h1>";
	mylog($userid, "view result login faiulre");
	exit(-1);	
}

$dir = $root_dir . $userid . "/current/";
//$user_name = $user;
echo "<font size=4 color=green>Submission Info for ".$userid. " $user_name<br/><br/></font></h1>";

{	// progress bar
	
	echo '<iframe src="view_progress_bar/pg.php?id='.$userid.'" width="100%" height=50 scrolling="no" frameborder="0" >		 </iframe>';	
}


{
	$queue_file =  $queue_dir . "/". $userid;
//	$timeout =  GetConfigValueList('child_process_timeout_seconds');
//	$timeout = (int)($timeout[0]);

	if ( file_exists($queue_file)) {
		
		echo '<div id="msg_still_running" style="display:block">';
		
		$st = filemtime($queue_file);
		$msg = sprintf( "Submission at %s is still running !",  date ("F j, Y H:i:s", $st) );
   		echo "<font size=4 color=blue>$msg</font></h1><br/><br/>";

   		$timeout_file = $dir . "timeout.final";

		$msg = sprintf( "Will expire at %s.",  file_get_contents($timeout_file) );
//		$msg = sprintf( "Will be expired at %s.",  date ("F j, Y H:i:s", $st + $timeout) );
   		echo "<font size=4 color=red>$msg</font></h1><br/><br/>";
   		
   		echo '<FORM action="view_expire_task.php" method="POST" enctype="multipart/form-data" onSubmit="document.getElementById(\'ExpireItNowButton\').disabled=true;">';
   		echo '<input type="hidden" name="userid" value="'.$userid.'">';
//   		echo '<input type="hidden" name="userpwd" value="'.$_POST["userpwd"].'">';
   		echo "<INPUT TYPE=\"SUBMIT\"  value=\"Expire It Now\"  id=\"ExpireItNowButton\">";
   		echo '</FORM>';
   		
   		echo '</div>';
//   		exit(-1);	
	}	
	else {
		$filename = $dir."analysis_result";
		
		if (file_exists($filename)) {
			echo "Analysis result generated at " . date ("Y/n/j H:i:s.", filemtime($filename)) ."<br/><br/>";
		}		
	}

}


{
	
	
	$filename = $dir."time.report.final";
	if ( file_exists($filename) ) {
		//$file = fopen($filename, "rt");
		$TR = LoadConfigFromFile($filename);
		
		
		$span = (int)($TR["time"][0]);
		$timeout = (int)($TR["timeout"][0]);
		
		$minute =(integer)( $span / 60);
		$second = $span % 60;
		
		if ( $timeout==1)
			echo"<font size=3 color=red>Analysis takes longer than expected. Process aborted !!!<br/><br/></font>";
		
		echo"<font size=3 color=\"#9900CC\">time spent in analysis: $minute minutes $second seconds<br/><br/></font>";

		
		//fclose($file);
	}
	
}

$filepath = $dir."sha1.report.final";

if (file_exists($filepath) ) {
	show_text_file($filepath);
}

echo "<br/>";

$filepath = $dir."gpg.sig.asc";

if (file_exists($filepath)) {
	show_text_file($filepath);
}

echo "<br/>";

echo "<font size=5 color=blue>Settings</br></font>";
echo '<div id="setting" >';
//show_text_file($dir. "analysis_result");
echo '</div>';

echo "<br/>";


echo '<script>';
echo 'function ToggleCodeDisplay()';
echo '{';
echo '	var h = document.getElementById("code");';
echo '  if (h.style.display=="block")   h.style.display="none";';
echo '  else h.style.display="block";';	
echo '}';
echo '</script>';

echo '<button type="button" onclick="ToggleCodeDisplay()">Toggle Code Display</button>';
echo '<div id="code" style="display:none">';
	//show_text_file($dir. "analysis_result");
echo '</div>';

echo "<br/><br/>";


echo "<font size=5 color=blue>Analysis result</br></font>";
echo '<div id="analysis_result" >';
//show_text_file($dir. "analysis_result");
echo '</div>';

echo "<br/>";

echo '<div id="check_pattern_concise"> </div>';
//show_text_file($dir. "check_pattern_concise.report");


echo "<br/>";



echo "<hr></br>";

echo '<div id="detailed_results">';

echo "<h3>Compilation status</h3>";
echo '<div id="gcc" > </div>';
//echo file_get_contents($dir."gcc.report");
//echo "g++ exit code : ".file_get_contents($dir."gcc_exit_code.report"). "<br/>";
//show_text_file($dir."gcc.report");


//echo '<div id="script" style="display:none">';
//echo '</div>';






//echo "<font size=5 color=blue>Runtime Output (from running memcheck)</br></font>";
//echo '<div id="memcheck_check" style="display:none">';
//show_text_file($dir."check_msg_memcheck.report");
//echo '</div>';
//echo "<hr></br>";

//echo "<font size=5 color=blue>Runtime Output (from running callgrind)</br></font>";
//echo '<div id="callgrind_check" style="display:none">';
//show_text_file($dir."check_msg_callgrind.report");
//echo '</div>';
//echo "<hr></br>";

echo "<h3>Valgrind memcheck result</h3>";
echo '<div id="memcheck" > </div>';


echo "<h3>Valgrind callgrind result</h3>";
echo '<div id="callgrind"  > </div>';

echo "<h3>Valgrind callgrind annotation result</h3>";
echo '<div id="callgrind_annotate"  > </div>';

echo "<h3>Valgrind massif result</h3>";
echo '<div id="massif"  > </div>';



echo "<h3>Illegal headers</h3>";
echo '<div id="illegal_headers"> </div>';

echo "<h3>Illegal library / system call usages</h3>";
echo '<div id="callcheck" > </div>';

echo '</div>';	// detailed_results

mylog($userid, "view result");


?>

</BODY>
</HTML>
