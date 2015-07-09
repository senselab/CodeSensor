<?php


header("Expires: Sat, 1 Jan 2005 00:00:00 GMT");
header("Last-Modified: ".gmdate( "D, d M Y H:i:s")."GMT");
header("Cache-Control: no-cache, must-revalidate");
header("Pragma: no-cache");
	

	  // 	no-cache headers - complete set
	  // 	these copied from [php.net/header][1], tested myself - works
	  	header("Expires: Sat, 26 Jul 1997 05:00:00 GMT"); // Some time in the past
  		header("Last-Modified: " . gmdate("D, d M Y H:i:s") . " GMT");
  		header("Cache-Control: no-store, no-cache, must-revalidate");
  		header("Cache-Control: post-check=0, pre-check=0", false);
  		header("Pragma: no-cache");

	
		include_once ("account.php");


		if ( !CookieAuthenticate($userid , $user_name)) {
			echo "<font size=5 color=red>You need to login first !</font></h1>";
			mylog($_POST["userid"], "view result login faiulre");
			exit(-1);	
		}

		$dir = $root_dir . $userid . "/current";
		$real_dir = readlink($dir);
		
		$file = $_GET["file"];
		
		$file = substr($file, 0, strrpos($file, "."));
		
	//	error_log("show_text_file ".$dir.$file."  ==>  ".$real_dir.$file);
		
		if ( $file=="code")
			show_text_file($real_dir.$file, true);
		else		
			show_text_file($real_dir.$file);
	
	/*
		$userid = $_GET["id"];
		echo "<h1>Hi</h1>" ;
		echo "<h1>$userid</h1>" ;*/
	?>
	
	
