#!/usr/bin/php
<?php

$ACCOUNT_FILE = "student_roster.txt";

$users = Array();


function generatePassword($length=8,$level=2){

   list($usec, $sec) = explode(' ', microtime());
   srand((float) $sec + ((float) $usec * 100000));

   $validchars[1] = "0123456789abcdfghjkmnpqrstvwxyz";
   $validchars[2] = "0123456789abcdfghjkmnpqrstvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
   $validchars[3] = "0123456789_!@#$%&*()-=+/abcdfghjkmnpqrstvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_!@#$%&*()-=+/";
   $validchars[4] = "0123456789abcdfghjkmnpqrstvwxyz";
   
   $password  = "";
   $counter   = 0;

   while ($counter < $length) {
     $actChar = substr($validchars[$level], rand(0, strlen($validchars[$level])-1), 1);

     // All character must be different
/*     if (!strstr($password, $actChar))*/ {
        $password .= $actChar;
        $counter++;
     }
   }

   return $password;

}

function ReadAccountFile()
{
	global $ACCOUNT_FILE;
	global $users;

	$users = Array();
	
	$fp = fopen($ACCOUNT_FILE, "rt");
	
	if ( $fp == false)
		exit(-1);


		
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

   		//if ( $pwd == false)
   			$pwd = generatePassword(13,4);
   			
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
	
	if ( $fp == false)
		exit(-1);

	foreach ($users as $id => $r) {
		fprintf($fp, "%s\t%s\t%s\t%s\t%s\n", $id, $users[$id]["name"], $users[$id]["dept"], $users[$id]["email"], $users[$id]["pwd"]);	
	}	
	
		
	fclose($fp);
}

function Authenticate($id, $pwd) 
{
	global $users;

	if (array_key_exists($id, $users)==false)
		return FALSE;
	
	if ( $users[$id]["pwd"]==$pwd)
		return true;
		
	return false;
}

ReadAccountFile();
WriteAccountFile();

/*
printf("%d\n", Authenticate("736","") );
printf("%d\n", Authenticate("","") );
printf("%d\n", Authenticate("9817016",""));
printf("%d\n", Authenticate("9817016","4r8NMP16"));
printf("%d\n", Authenticate("9817016","r8NMP16"));
printf("%d\n", Authenticate("4817016","4r8NMP16"));*/

?>
