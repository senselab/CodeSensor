<HTML>



<HEAD>
<title>Data Structures View Result</title>
</HEAD>
<BODY>
<h2> </h2><br/>

<?php

include ("config.php");

include_once ("account.php");


if ( !CookieAuthenticate($id , $name) ) { 
   echo "<font size=5 color=red>Please login first !</font></h1>";
   
	mylog($id, "view expire task login faiulre");
   
	exit(-1);
}



//proc_close(proc_open("$homework_inspector_executable  ". $id . "  ".$HW_NAME. "  kill", Array(), $foo));
exec("$homework_inspector_executable  ". $id . "  ".$HW_NAME. "  expire", $dummy_output );
$queue_file =  $queue_dir . "/". $id;
unlink($queue_file);

echo "<font size=4 color=green>Ok !<br/></font></h1>";


?>

</BODY>
</HTML>
