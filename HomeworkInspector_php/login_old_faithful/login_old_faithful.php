<?php header("p3p: CP=\"ALL DSP COR PSAa PSDa OUR NOR ONL UNI COM NAV\"");?> 

<HTML>



<HEAD>
<title>Login old faithful</title>
</HEAD>
<BODY>
<h2> </h2><br/>

<?php


/*
error_reporting(E_ALL);
ini_set('error_reporting', E_ALL);
ini_set('display_startup_errors', 1);
ini_set("log_errors",1);
ini_set("display_errors",1);
ini_set("error_log", "/path/to/a/log/file/php_error.log"); 
*/
include_once ("../config.php");
include_once ('../account.php');
include_once ("authenticate.php");
 


if ( Authenticate($_POST["userid"], $_POST["userpwd"], $users)==FALSE ) {
   echo "<h1><font size=5 color=red>Login failed !</font></h1>";
   
//   if (array_key_exists($_POST["userid"], $users) )
	mylog($_POST["userid"], "old faithful login faiulre");
   
	exit(-1);
}

///////////////check password update date ////////////////

$pwd_update_time = LastPWDUpdateTime($_POST["userid"]);

if ( time() - $pwd_update_time > 86400 * 365) {

	echo "<h1><font size=5 color=red>Your last password change was at ".date("Y-m-d H:i:s", $pwd_update_time)."</font></h1>";
	echo "<h1><font size=5 color=green>You have to change your password first !</font></h1>";
	echo '<iframe src="change_pwd.html" frameborder=0 height="100%" width="100%">Please enable iframe on your browser !</iframe>';

	exit(-1);
}

///////////////////////////////////////////////////////

$record = GetUserRecordByID($_POST["userid"]);

$dir = $root_dir . $record["id"] . "/current/";
$user_name = $record["name"];

echo "<h1><font size=4 color=green>Submission Info for ".$record["id"]. " $user_name<br/><br/></font></h1>";


//header("Location: http://www.kimo.com");
//header("Location: http://192.168.0.122/algo_HW6_Graph_Coloring/");


echo '<script language="javascript">'."\n";
echo ' window.open("'.$code_sensor_homepage.'", "_top");'."\n";
echo '</script>'."\n"; 


setcookie("logon_session", $record["logon_session"], time()+86400*7, "/" );

?>




</BODY>
</HTML>
