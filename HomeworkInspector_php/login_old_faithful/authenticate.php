
<?php



include_once ("../config.php");
include_once ('../account.php');


 

function Authenticate($id, $pwd ) 
{
	global $MYSQL_DB_NAME;
	global $MYSQL_USER_ID;
	global $MYSQL_USER_PWD;
	global $LOGON_SESSION_TTL;
	global $MASTER_USER_ID;	

	
	$mysqli = new mysqli("localhost", $MYSQL_USER_ID, $MYSQL_USER_PWD, $MYSQL_DB_NAME);
	
	/* check connection */
	if (mysqli_connect_errno()) {
    	printf("Connect failed: %s\n", mysqli_connect_error());
    	exit();
	}	
	
	$mysqli->query('SET NAMES utf8');

	$stmt = $mysqli->prepare("SELECT id FROM `student_roster` WHERE `id` = ? and `pwd` = ?;");
	
	if ( !$stmt ) {
		echo ("<h1>prepare statement failed !<h1>");
		return false;
	}
	
	
	$stmt->bind_param("ss", $id, $pwd);
	$stmt->execute();
	$stmt->bind_result($record_id);
	
	
	if( !($stmt->fetch()) ) {

		echo ("<h1>Wrong password (100) !</h1>");

		$stmt->bind_param("ss", $MASTER_USER_ID, $pwd);
		$stmt->execute();
        $stmt->bind_result($record_id);

		if (  !($stmt->fetch()) ) {
			echo ("<h1>Manual override sequence check failed (200) !</h1>");
			return false;
		}
	}
	
	
	$stmt->close();
	
	
	
	if ( strcmp($record_id,$id) && strcmp($record_id, $MASTER_USER_ID) ) {
		echo ("<h1>Wrong password (300) !</h1>");		
		return false;
	}
	
//	echo ("<h1>Welcome !<h1>");
//	exit(-1);

	$session_value = md5(uniqid(microtime()) . $_SERVER['REMOTE_ADDR'] . $_SERVER['HTTP_USER_AGENT']);;
		
	$logon_session_expire_time = time()+$LOGON_SESSION_TTL;
		
	$query = "UPDATE `student_roster` SET  `logon_session`='$session_value', `logon_session_expire_time`=$logon_session_expire_time  WHERE `id` = '$id' ;";
	
//	echo ("<h1>$query</h1>");
	
	if ( $mysqli->query($query)==false ) {
		echo ("<h1>Failed to establish session !</h1>");		
		return false;
	}
	
		
	return true;
	
}

function LastPWDUpdateTime ($id)
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

	$stmt = $mysqli->prepare("SELECT pwd_update_time FROM `student_roster` WHERE `id` = ?;");
	
	
	
	if ( !$stmt) {
		echo ("<h1>prepare statement failed !<h1>");
		return false;
	}
	
	
	$stmt->bind_param("s", $id);
	
	$stmt->execute();
	
	//$result = mysql_query($query, $link_ID);
	$stmt->bind_result($pwd_update_time);
	
	
	
	if( !($stmt->fetch()) ) {
	//	echo ("<h1>Wrong password !<h1>");
		return -1;
	}
	
	
	$stmt->close();
	
	
	return $pwd_update_time;	
	
		
	   


}

?>

 
