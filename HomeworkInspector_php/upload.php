<!DOCTYPE html>
<html>
<head>
<title>CODESENSOR submission</title>
</head>
<body>
<h1 style="text-align: center; font-family:Verdana; color: #174B6E"><!CODESENSOR_HW_TITLE!></h1>

<?php


session_start();

include_once ("config.php");
include_once ("account.php");


//error_reporting(E_ALL);
//ini_set('display_errors','On');

$szDeadline = GetConfigValueList("deadline");
echo '<h3 style="text-align: center"><div style="display:inline-block; font-family:Tahoma; color: #6E1717">Deadline: </div><div style="display:inline-block;font-family: Arial; color:#900B54">'.$szDeadline[0]."</div></h3><br/>";

echo '<hr/>';

$deadline = date_parse_from_format("Y-n-j H:i:s", $szDeadline[0]);

if ( $deadline==false)
	exit(-1);
//echo var_dump($deadline)."<br/>";

$tmDeadline = mktime($deadline["hour"], $deadline["minute"], $deadline["second"], 
                     $deadline["month"], $deadline["day"], $deadline["year"]);


if ( time()> $tmDeadline ) 
	$past_deadline_confirmed = true;
else
	$past_deadline_confirmed = false;

$_SESSION['past_deadline_confirmed'] = $past_deadline_confirmed;	
	
if ( $past_deadline_confirmed) {
	echo '<h1 style="font-size:10; color:red">';
	echo "Warning : The Deadline Has Passed ! <br/>&nbsp; &nbsp; Submission at this time will get a fixed 0 priority with penalty !<br/><br/>";
	echo "</h1>";
}	



if ( CookieAuthenticate($id, $name )) {
	
	
	echo '<FORM action="upload_core.php" method="POST" enctype="multipart/form-data" onSubmit="document.getElementById(\'Submit\').disabled=true;">';
	
//	echo '<input type="hidden" name="MAX_FILE_SIZE" value="2048000" style="font-size: 16px;">';
	/*
	echo '<font style="font-size:16px;">Student ID : </font>' ;
	echo '<input type="text" name="userid" style="font-size: 16px;"><br/>';
	echo '<font style="font-size:16px;">Password : </font>' ;
	echo '<input type="password" name="userpwd" style="font-size: 16px;"><br/>';
	*/

echo '<script>';
echo 'function DetectFileType()';
echo '{';
echo '  var h = document.getElementById("language_type");';
echo '		var fname = document.getElementById("userfile").value;';
echo '	if ( fname.indexOf(".py") != -1) {';
echo '			h.value = "python";';
echo '  }';
#echo '	else if ( fname.indexOf(".java") != -1) {';
#echo '			h.value = "java";';
#echo '	}';
#echo '	else if ( fname.indexOf(".zip") != -1) {';
#echo '			h.value = "customized";';
#echo '	}';
echo '  OnLanguageTypeSelect();';
echo '}';
echo '</script>';

echo '<script>';
echo 'function OnLanguageTypeSelect()';
echo '{';
echo '  var h = document.getElementById("language_type");';
echo '  var hJVMXMX = document.getElementById("jvm_xmx");';
echo '  var hGCCOptimal = document.getElementById("gcc_optimization");';
echo '  if ( h.value=="cpp" || h.value=="cpp_shm"  ) {';
echo '      hJVMXMX.style.display = "none";';
echo '		hGCCOptimal.style.display = "block";';
echo '  }';
echo '	else if ( h.value=="python") {';
echo '      hJVMXMX.style.display = "none";';
echo '		hGCCOptimal.style.display = "none";';
echo '  }';
echo '	else if ( h.value=="java") {';
echo '  		hGCCOptimal.style.display = "none";';
echo '          hJVMXMX.style.display = "block";';
echo '	}';
echo '}';
echo '</script>';

	
	echo '<font style="font-size: 16px">Language type : </font>' ;
	echo '<select name="language" id="language_type" onchange="OnLanguageTypeSelect()">';
	echo '<option value="cpp" selected>C++ (file)</option>';	
	echo '<option value="cpp_shm">C++ (function)</option>';
	echo '<option value="python">Python</option>';
//    echo '<option value="clj">Clojure</option>';
//	echo '<option value="js">Javascript</option>';
//	echo '<option value="customized">customized</option>';
	echo '</select><br/><br/>';

	echo '<font style="font-size: 16px">File to upload : </font>' ;
	echo '<input type="file" name="userfile" id="userfile" style="font-size: 16px;" onchange="DetectFileType()"/><br/><br/>';
 	echo '<br/>';

	echo '<div id="gcc_optimization" style="font-size:16px; display:none">'; 	
		echo '<input type ="checkbox" name="check_disable_optimization" /><font style="font-size:14px;">Disable GCC Optimization</font><br/>';
	echo '</div>';

	echo '<input type ="checkbox" name="check_kill_job" /><font style="font-size:14px;">Kill Existing Job</font><br/>';
	echo '<br/>';

//	echo '<div id="jvm_xmx" style="font-size:16px; display:none">JVM maximum heap size(-Xmx) (MB) : ';
//	echo '<input type="text" name="memory_size_cap" value="-1" size="5"/ style="text-align: right"></div><br/><br/>';
?>


	
	
	<img src="cool-php-captcha/captcha.php" id="captcha" /><br/>


	<!-- CHANGE TEXT LINK -->
	<a href="#" onclick="
	    document.getElementById('captcha').src='cool-php-captcha/captcha.php?'+Math.random();
	    document.getElementById('captcha-form').focus();"
	    id="change-image">Not readable? Change text.</a><br/><br/>

	Key in the captcha word : &nbsp; 
	<input type="text" name="captcha" id="captcha-form" autocomplete="off" /><br/><br/>
	


<?php	
	if ( $past_deadline_confirmed ) {
		echo '<INPUT TYPE="SUBMIT"  value="Yes, I want to submit and get a 0 priority !"  style="color: #FFFFFF; background-color:  #FF0000; font-size: 14px"  id="Submit" />';
	}
	else {
		echo '<INPUT TYPE="SUBMIT"  value="Submit" style="font-size: 16px;"   id="Submit" />';
	}
	
	echo '&nbsp';
	
	echo '<INPUT TYPE="RESET"  value="Reset" style="font-size: 16px;" />';

	if ( $id == 'baseline' ) {
		echo '<br><br><br><INPUT TYPE="SUBMIT" value="re-evaluate" formaction="re-eval.php" />';
		echo '<INPUT TYPE="SUBMIT" value="move results" formaction="move.php" /><br>';

		if (file_exists("/var/homeworks/h_".$HW_NAME."/progress")) {
			show_text_file("/var/homeworks/h_".$HW_NAME."/progress");
		}
	}

	echo '</FORM>';
}
else {
	
	echo '<h2 style="font-size:5; color=red">You need to login first !</h2>';
}

?>
</body>

</html>

