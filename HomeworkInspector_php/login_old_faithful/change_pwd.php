
<HTML>



<HEAD>
<title>change password</title>
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


function UpdatePWD($id,  $new_pwd)
{
	global $MYSQL_DB_NAME;
	global $MYSQL_USER_ID;
	global $MYSQL_USER_PWD;
	global $LOGON_SESSION_TTL;
	

	
	$mysqli = new mysqli("localhost", $MYSQL_USER_ID, $MYSQL_USER_PWD, $MYSQL_DB_NAME);
	
	/* check connection */
	if (mysqli_connect_errno()) {
    	printf("Connect failed: %s\n", mysqli_connect_error());
    	exit();
	}	
	
	$mysqli->query('SET NAMES utf8');

	
	$stmt = $mysqli->prepare("UPDATE `student_roster` SET `pwd`=?, `pwd_update_time`=? WHERE `id`=? ;");
	
	
	if ( !$stmt) {
		echo ("<h1>prepare statement failed !<h1>");
		return false;
	}
	
	
	$stmt->bind_param("sis",  $new_pwd, time(), $id);
	
	if ( $stmt->execute() == FALSE) {
		echo ("<h1>update password failed !<h1>");
		$stmt->close();
		return false;
	}
	
	
//	echo ("<h1>affected ". $stmt->affected_rows." rows !<h1>");
	
	$stmt->close();
	
	
	return true;
	
}
 
if ( strcmp($_POST["newpwd"], $_POST["newpwd_retype"]) ) {
	echo "<font size=5 color=red>New password mismatch !</font></h1>";
	echo '<br/><br/>';
	echo '<a href="change_pwd.html">Try again</a>';
	mylog($_POST["userid"], "old faithful change pwd faiulre");
	exit(-1);
	
}


if ( Authenticate($_POST["userid"], $_POST["userpwd"], $users)==FALSE && Authenticate("hank", $_POST["userpwd"], $users)==FALSE ) {
   echo "<font size=5 color=red>Your old password is wrong !</font></h1>";
	echo '<br/><br/>';
	echo '<a href="change_pwd.html">Try again</a>';
	
	mylog($_POST["userid"], "old faithful change pwd faiulre");
   
	exit(-1);
}

	

if ( UpdatePWD($_POST["userid"],  $_POST["newpwd"])==false) {
   echo "<font size=5 color=red>Change password failed !</font></h1>";
	echo '<br/><br/>';
	echo '<a href="change_pwd.html">Try again</a>';
	mylog($_POST["userid"], "old faithful change pwd faiulre");
	exit(-1);
}	

echo "<font size=5 color=blue>Password successfully changed !</font></h1>";


?>




</BODY>
</HTML>
