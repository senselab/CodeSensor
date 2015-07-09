<HTML>
<HEAD>
<title>Password Retrieval</title>
</HEAD>
<BODY>
<h2> </h2><br/>
<?php

include_once ("config.php");

?>
<hr>
<?php

include_once ("account.php");

$id = $_POST["userid"];
$name = $_POST["username"];

if (array_key_exists($id, $users)==false) {
	echo "<font size=5 color=red>Account does not exist !</font></h1>";
	exit(-1);
}

if ( strncmp($users[$id]["name"], $name, 2)!=0) {
	echo "<font size=5 color=red>Account does not exist !</font></h1>";
	exit(-1);
}

//////////////send e-mail
 $to = $users[$id]["email"];
 $subject = "2011 Introduction to Algorithms password retrieval";
 $body = "Your password is ". $users[$id]["pwd"];
 $headers = "From: hankwu@g2.nctu.edu.tw\r\n" .
     "X-Mailer: php";
  
  if (mail($to, $subject, $body, $headers)) {
   	echo("<p>Message successfully sent to $to !</p>");
  } else {
   	echo("<p>Message delivery failed...</p>");
  }

 

?>
</BODY>
</HTML>
