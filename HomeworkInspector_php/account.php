<?php

include_once ("config.php");

//$ACCOUNT_FILE = $root_dir."config/student_roster.txt";

$users = Array();

function GetSubmissionTime($id)
{
	global $HW_NAME;
	
	$filename = "/var/homeworks/".$HW_NAME."/". $id . "/current/code";
	
	if (file_exists($filename)) {
    //	echo "Analysis result generated at " . date ("Y/n/j H:i:s.", filemtime($filename)) ."<br/><br/>";
    	return filemtime($filename);
	}
	
	return NULL;
}

function generatePassword($length=8,$level=2){

   list($usec, $sec) = explode(' ', microtime());
   srand((float) $sec + ((float) $usec * 100000));

   $validchars[1] = "0123456789abcdfghjkmnpqrstvwxyz";
   $validchars[2] = "0123456789abcdfghjkmnpqrstvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
   $validchars[3] = "0123456789_!@#$%&*()-=+/abcdfghjkmnpqrstvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_!@#$%&*()-=+/";

   $password  = "";
   $counter   = 0;

   while ($counter < $length) {
     $actChar = substr($validchars[$level], rand(0, strlen($validchars[$level])-1), 1);

     // All character must be different
     if (!strstr($password, $actChar)) {
        $password .= $actChar;
        $counter++;
     }
   }

   return $password;

}
/*
function ReadAccountFile()
{
	global $ACCOUNT_FILE;
	global $users;

	$users = Array();
	
	$fp = fopen($ACCOUNT_FILE, "rt");
	
	if ( $fp == false) {
		echo "error reading account file";
		exit(-1);
	}


		
	while (!feof($fp))   
  	{  
  		$delim = " \t\n\r";
   		$buf = fgets($fp, 1024);  
  
   		
   		$id = strtok($buf, $delim);
   		if ( $id == false)
   			continue;
   		
   		$name = strtok( $delim);
   		if ( $name == false)
   			continue;
   			
   		$dept = strtok( $delim);
   		if ( $dept == false)
   			continue;
   			
   		$email = strtok( $delim);
   		if ( $email == false)
   			continue;

   		$pwd = strtok( $delim);
   		if ( $pwd == false)
   			$pwd = generatePassword();
   			
   		$users[$id]['name'] = $name;
   		$users[$id]['dept'] = $dept;
   		$users[$id]['email'] = $email;
   		$users[$id]['pwd'] = $pwd;

  	}  
		
		
	fclose($fp);

//	print_r($users);
}

function WriteAccountFile()
{

	global $ACCOUNT_FILE;
	global $users;

	
	$fp = fopen($ACCOUNT_FILE, "wt");
	
	if ( $fp == false) {
		echo "error writing account file";
		exit(-1);
	}

	foreach ($users as $id => $r) {
		fprintf($fp, "%s\t%s\t%s\t%s\t%s\n", $id, $users[$id]["name"], $users[$id]["dept"], $users[$id]["email"], $users[$id]["pwd"]);	
	}	
	
		
	fclose($fp);
}
*/

function GetUserRecordByID($id)
{
	//global $users;
	global $MYSQL_USER_ID;
	global $MYSQL_USER_PWD;	
	global $MYSQL_DB_NAME;
 
	
	//$link_ID = mysql_connect("localhost",$MYSQL_USER_ID, $MYSQL_USER_PWD);
	$mysqli = new mysqli("localhost", $MYSQL_USER_ID, $MYSQL_USER_PWD, $MYSQL_DB_NAME);
	
	
	/* check connection */
	if (mysqli_connect_errno()) {
    	printf("Connect failed: %s\n", mysqli_connect_error());
    	exit();
	}	
	
	$mysqli->query('SET NAMES utf8');

	$stmt = $mysqli->prepare( "SELECT id,name,email,pwd,logon_session,logon_session_expire_time FROM `student_roster` WHERE `id` = ?;");
	//$stmt = $mysqli->prepare( "SELECT name FROM `student_roster` WHERE `id` = ?;");
	
	
		
	
	if ( !$stmt) {
		echo ("<h1>prepare statement failed !<h1>");
		return false;
	}
	
	
	if( $stmt->bind_param("s", $id) == false ) {
		echo "<h1>get record bind failed</h1>";
	}
	
	if( $stmt->execute()==false ) {
		echo "<h1>get record execute failed</h1>";
	}
	
	$record = null;
	
	//$result = mysql_query($query, $link_ID);
	//$stmt->bind_result($r_id, $record["name"],$record["email"], $record["pwd"], $record["logon_session"], $record["logon_session_expire_time"]);
//	if( $stmt->bind_result($r_id, $r_name, $r_email, $r_pwd, $r_logon_session, $r_logon_session_expire_time) ==false) {
//	if( $stmt->bind_result( $r_name) ==false) {

	if( $stmt->bind_result($record["id"], $record["name"],$record["email"], $record["pwd"], $record["logon_session"], $record["logon_session_expire_time"])==false) {
		echo "<h1>get record bind failed</h1>";
	}
	
	
	if( !($stmt->fetch()) ) {
		echo ("<h1>Wrong password !<h1>");
		return false;
	}
	
	
	$stmt->close();
		
	///////////////////////////////////////////////////////////	
	
/*		
	echo ("<pre>");
		var_dump($record);
	echo ("</pre>");
	*/
	
	if ( strcmp($record["id"],$id)  ) {
		return null;
	}

	return $record;
}


function CookieAuthenticate(&$id, &$name )
{
	//global $users;
	global $MYSQL_USER_ID;
	global $MYSQL_USER_PWD;	
	global $MYSQL_DB_NAME;
 
	
	if ( !isset($_COOKIE["logon_session"])) {
		return false;
	}

	$logon_session = $_COOKIE["logon_session"];

/*	
	$link_ID = mysql_connect("localhost",$MYSQL_USER_ID, $MYSQL_USER_PWD);
	
	if ( $link_ID==false) {
		echo ("<h1>Cannot connect to mysql !<h1>");
		return false;
	}
	
	mysql_query('SET NAMES utf8'); 
	
	if ( !mysql_select_db($MYSQL_DB_NAME, $link_ID) ) {
		echo ("<h1>Cannot open DB !<h1>");
		return false;
	}
	
		

	$query = "SELECT * FROM `student_roster` WHERE `logon_session` = '$logon_session';";
	
	*/
	
	$mysqli = new mysqli("localhost", $MYSQL_USER_ID, $MYSQL_USER_PWD, $MYSQL_DB_NAME);

	/* check connection */
	if (mysqli_connect_errno()) {
    	printf("Connect failed: %s\n", mysqli_connect_error());
    	exit();
	}	
	
	$mysqli->query('SET NAMES utf8');

	$stmt = $mysqli->prepare( "SELECT id,name,logon_session_expire_time FROM `student_roster` WHERE `logon_session` =  ?;");
		
	if ( !$stmt) {
		echo ("<h1>cookie auth prepare statement failed !<h1>");
		return false;
	}
	
	
	if( $stmt->bind_param("s", $logon_session ) == false ) {
		echo "<h1>cookie auth get record bind param failed</h1>";
	}
	
	if( $stmt->execute()==false ) {
		echo "<h1>cookie auth get record execute failed</h1>";
	}
	
	if( $stmt->bind_result($id, $name, $logon_session_expire_time)==false) {
		echo "<h1>cookie auth get record bind failed</h1>";
	}
	
	
	if( !($stmt->fetch()) ) {
//		echo ("<h1>cokkie auth failed !<h1>");
		return false;
	}
	
	
	$stmt->close();
	
	if ( time() > $logon_session_expire_time ) {
		return false;
	} 

	
	return true;
}

/*
function Authenticate($id, $pwd) 
{
	global $users;

	if (array_key_exists($id, $users)==false)
		return FALSE;
	
	if ( $users[$id]["pwd"]==$pwd)
		return true;
		
	return false;
}
*/


//ReadAccountFile();

?>
