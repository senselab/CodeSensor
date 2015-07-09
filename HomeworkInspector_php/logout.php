<HTML>

<HEAD>
<title>logout</title>
</HEAD>
<BODY>

<?php 

	include_once ("config.php");

	
	global $MYSQL_USER_ID;
	global $MYSQL_USER_PWD;
	

	
	$id = "not set";

	if ( isset($_GET["id"]) ) {
		$id = $_GET["id"];
	}
	
	

	echo '<script language="javascript">'."\n";
	echo ' window.open("'.$code_sensor_homepage.'", "_top");'."\n";
	echo '</script>'."\n"; 

	$session_value = "INVALID";
	setcookie("logon_session", $session_value, time()+86400*7, "/" );

	
?>


</BODY>
</HTML>
