#!/usr/bin/php
<?php

function GetTextFileLineCount($file)
{
	$fp = fopen($file, "rt");

	if ( !$fp)
		return -1;

	$line_cnt = 0;

    while (($buffer = fgets($fp, 4096)) !== false) {
		$line_cnt++;
    }	

	fclose($fp);

	return $line_cnt;
}

function CalcTextFileDissim($file1, $file2)
{
	$diff = tempnam("/tmp", "cal_sim");
	system("diff -bB $file1 $file2 > $diff"	);

	$lc1 = GetTextFileLineCount($file1);
	$lc2 = GetTextFileLineCount($file2);

//	printf("$file1 : $lc1 lines\n");
	//printf("$file2 : $lc2 lines\n");

	if ( $lc1 < 0 || $lc2 <0) {
		unlink($diff);
		return -1;
	}

	$fp = fopen($diff, "rt");

	if ( $fp == false)	 {
		unlink($diff);
		return -1;
	}
	
	$n_diff = 0;

    while (($buf = fgets($fp, 4096)) !== false) {
		if ( $buf[0]=='<' || $buf[0] =='>' || $buf[0] =='-')
			continue;

//		printf("$buf");

		$ls = $le = $rs =$re = 0;	

		if ( preg_match('/([0-9]+),([0-9]+)[acd]/', $buf, $matches, PREG_OFFSET_CAPTURE)>0) {
			$ls = (int)($matches[1][0]);
			$le = (int)($matches[2][0]);
		}
		else if ( preg_match('/([0-9]+)[acd]/', $buf, $matches, PREG_OFFSET_CAPTURE) >0 ) {
			$ls = $le = (int)($matches[1][0]);
		}

		if ( preg_match('/[acd]([0-9]+),([0-9]+)/', $buf, $matches, PREG_OFFSET_CAPTURE)>0) {
			$rs = (int)($matches[1][0]);
			$re = (int)($matches[2][0]);
		}
		else if ( preg_match('/[acd]([0-9]+)/', $buf, $matches, PREG_OFFSET_CAPTURE) >0 ) {
			$rs = $re = (int)($matches[1][0]);
		}


	//	printf("Found: $ls-$le    $rs-$re\n");

		//printf("\n");
		


		$n_diff += $le-$ls + 1 + $re - $rs;
    }	

	$dvalue = $n_diff / ($lc1 + $lc2);


	fclose($fp);	

	unlink($diff);

	return $dvalue;
}

if ($argc < 2) {
	printf("\n\tE.g. ManualExec hw4 (1000001) (avoid_file)\n\n");
	exit(-1);
}

$HW_NAME = $argv[1];

if ( $argc > 2) {
	$ID_FILTER = $argv[2];
}
else {
	$ID_FILTER = 'all';
}

	

include_once ("/var/www/html/".$HW_NAME."/account.php");
//include ("/var/www/html/config.php");

//$HOMEWORK_DIR = "/var/homeworks";
$HOMEWORK_DIR = $root_dir;

$HOMEWORK_INSPECTOR = "/usr/bin/HomeworkInspector/$HW_NAME/HomeworkInspector";



$h_dir = opendir($HOMEWORK_DIR);

if ( $h_dir== FALSE) {
	exit (-1);
}

$NUM_OF_SUBMISSION = 0;

while (($file = readdir($h_dir)) !== false) {
	if ( $file=="." || $file==".." || $file=="queue" || $file=="config") {
		continue;
	}

	
	$dir = CreateFullPath($HOMEWORK_DIR, $file);
	 
	if ( filetype($dir)!="dir") {
		continue;
	}


	if ( $ID_FILTER != 'all' && $ID_FILTER != $file)
		continue;

 

	$code_file = "/tmp/dummy_shouild_not_exist";
	$ds = 0;

	if ( $argc >=4) {
		$code_file = CreateFullPath($dir, "code");
		$k = 1;		
		
		do {
			if( !file_exists($code_file) )
				break;
			
			$ds =  CalcTextFileDissim($code_file, $argv[3]);

			if ( $ds < 0 || $ds > 0.3)
				break;

			$code_file = CreateFullPath($dir, "code.$k");
			$k++;
		}while (1);
	}	

	if ( !file_exists($code_file) )
		$code_file = CreateFullPath($dir, "code");


	if ( !file_exists($code_file) )
		continue;

	$queue_file = CreateFullPath( CreateFullPath($HOMEWORK_DIR, "queue"), $file);

	printf("Processing %s (ds=%f)\n", $code_file, $ds);
	
	system("sudo -u apache ln -s $code_file $queue_file");
		 
	proc_close(proc_open("sudo -u apache $HOMEWORK_INSPECTOR $file  $HW_NAME"  . " > /dev/null 2>&1 &", Array(), $foo));

//	sleep(40);	
	//echo "filename: $file : filetype: " . filetype($HOMEWORK_DIR ."/" . $file) . "<br/>";
}

closedir($h_dir);

?>
