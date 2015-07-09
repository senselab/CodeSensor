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

	printf("$file1 : $lc1 lines\n");
	printf("$file2 : $lc2 lines\n");

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


	if ($argc != 3) {
		printf("\n\tE.g. CalcTextFileSim file1 file2\n\n");
		exit(-1);
	}

	$dvalue = CalcTextFileDissim($argv[1], $argv[2]);

	printf("dissimilarity = $dvalue\n");

?>

