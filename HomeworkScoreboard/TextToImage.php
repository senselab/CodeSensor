#!/usr/bin/php
<?php

date_default_timezone_set('Asia/Taipei');

$HW_NAME = "<!CODESENSOR_HW_NAME!>";
$HOMEWORK_DIR = "/var/homeworks";
$WEB_ROOT = "/var/www/html";
$FORCE_GEN = true;
$OUTPUT_HTML_FILENAME = "code.html";

$use_base_user = false;

$powerI = 1;
$powerM = 1;
$priority_tone = 0.5;

$score_P_ratio = 0.3;
$TOTAL_P_SCORE = 80*60;

$score_board = array();

$VERBOSE = False;
$global_tb = NULL;


function CreateFullPath($dir, $file)
{
	$n = strlen($dir);
	
	
	
	if ( $n == 0 || $dir[$n-1]=='/') {
		return $dir . $file;

	}
	else {
		return $dir . "/"  . $file;		
	}
	
	
}


 


# Snippet from PHP Share: http://www.phpshare.org

/**
 * Converts tabs to the appropriate amount of spaces while preserving formatting
 *
 * @author      Aidan Lister <aidan@php.net>
 * @version     1.2.0
 * @link        http://aidanlister.com/repos/v/function.tab2space.php
 * @param       string    $text     The text to convert
 * @param       int       $spaces   Number of spaces per tab column
 * @return      string    The text with tabs replaced
 */
function tab2space($text, $spaces = 4)
{
    // Explode the text into an array of single lines
    $lines = explode("\n", $text);
    
    // Loop through each line
    foreach ($lines as $line)
    {
        // Break out of the loop when there are no more tabs to replace
        while (false !== $tab_pos = strpos($line, "\t"))
        {
            // Break the string apart, insert spaces then concatenate
            $start = substr($line, 0, $tab_pos);
            $tab = str_repeat(' ', $spaces - $tab_pos % $spaces);
            $end = substr($line, $tab_pos + 1);
            $line = $start . $tab . $end;
        }
        
        $result[] = $line;
    }
    
    return implode("\n", $result);
}


function TextToImage($single_line, $font_size,  $out_file, $extra_line_space = 0, $linenum = -1)
{
	global $global_tb;
  // show the correct header for the image type
  //header("Content-type: image/jpg");

		
	$single_line = tab2space($single_line);
	
	if ($linenum !=-1)
		$string = sprintf("%6d:  %s", $linenum,  $single_line);
	else
		$string = $single_line;
		
/*
	if ( mb_strlen($single_line, "Big5") != strlen($single_line) )
  		$font = 'msjh.ttf';
	else
	  $font = 'consola.ttf';
*/

	  $font = getcwd() . '/WenQuanYiMicroHeiMono.ttf';

  // some variables to set

  if ($global_tb==NULL) {
  	$global_tb = imagettfbbox($font_size, 0, $font, "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWZYX");
  }
  
  $tb = imagettfbbox($font_size, 0, $font, $string);
  
  
  $width = abs($tb[2] - $tb[0])+1;
  //$height = abs($tb[1] - $tb[7])+1;
  $height = abs($global_tb[1] - $global_tb[7])+1;

  //echo $tb[1] + "\n";
  
  //$width = 400;
  //$height = 200;
  //$width  = imagefontwidth($font) * strlen($string);
  //$height = imagefontheight($font);

  // lets begin by creating an image
  $im = @imagecreatetruecolor ($width,$height + $extra_line_space);

  //white background
  $background_color = imagecolorallocate ($im, 255, 255, 255);

  imagefill($im, 0, 0, $background_color); 
  
  //black text
  $text_color = imagecolorallocate ($im, 0, 0, 0);

  // put it all together
  imagettftext ($im, $font_size, 0, 0,$height - $global_tb[1] -1, $text_color, $font, $string);

  
  
  // and display
  imagepng ($im, $out_file, 9);
}


function IsCodeCompilable($student_dir)
{
	return file_exists(CreateFullPath($student_dir, "/current/check_pattern_concise_memcheck.report.final"));
/*
	$retV = false;
	$gccf = CreateFullPath($student_dir, "/current/gcc_exit_code.report.final");
	
	$fp = fopen($gccf, "rt");

	if (!$fp)
		return false;

	while(!feof($fp)) {
		$buf = fgets($fp, 4096);
		$status = explode(" \t\n", $buf);


		if ( count($status) > 1) 
			break;
		if ( count($status)==1 && !strcmp(trim($status[0]), "0") ) {
			$retV = true;
			break;
		}

	}

	fclose($fp);
		
	return $retV;*/
}


function TextFileToImage($infile, $font_size,  $out_dir, $extra_line_space = 0)
{
	global $OUTPUT_HTML_FILENAME;
	global $VERBOSE;
	global $FORCE_GEN;

	$utf_file = NULL;

	if (!file_exists($out_dir)) {
		mkdir($out_dir);
	}

	///// check time
	if ( $FORCE_GEN==false && file_exists($infile) ) {
        $infile_time = filemtime($infile);
        $output_html_path = CreateFullPath($out_dir, $OUTPUT_HTML_FILENAME);
		if ( file_exists($output_html_path) ){
	        $html_gen_time = filemtime($output_html_path);
			if ( $html_gen_time > $infile_time ) {
				if ( $VERBOSE)
					printf("skipping old file %s\n", $infile);
				return;
			}
		}

		//big5 to unicode
		/*
		$utf_file = tempnam("/tmp", "texttoimageutf_");
		system("iconv -sc -f BIG5 -t UTF8 < ".$infile." > ".$utf_file);
		$infile = $utf_file;
		*/
	}
/*	
	{
 		$outfile = CreateFullPath($out_dir, "code.html");
 		$tmLCDst = filectime($outfile);
 		$tmLCSrc = filectime($infile);

 		if ( $tmLCDst && $tmLCSrc && $tmLCSrc < $tmLCDst) {
 			printf("skip $infile...\n");
 			return;
 		}
	
	}
*/

	{ // clear output dir
		$output_dir_files = CreateFullPath($out_dir, "*");
		system("\\rm -rf $output_dir_files");
	}


	$sn = 0;
	$linenum = 1;

	
	if ( file_exists($infile) &&   ($fp = fopen($infile, "rt")) ) {

		while (!feof($fp))
		{
			$buf = fgets($fp, 4096);
		
			$outfile = sprintf( "img_%d.png", $sn);
		
			$outfile = CreateFullPath($out_dir, $outfile);
		
			TextToImage($buf, $font_size, $outfile, $extra_line_space, $linenum);
			$sn++;
			$linenum++;	 	
		}	
		
		fclose($fp);
	}
	
	/////// link by html ///////
	
	$buf = CreateFullPath($out_dir, $OUTPUT_HTML_FILENAME);
	
	//$buf = sprintf("%s/code.html", $out_dir);
	
	$fp = fopen($buf , "wt");
	
	if ($fp) {
		fprintf($fp, "<html>\n");
		
		//fprintf($fp, "File uploaded at %s<br/><br/>", date ("Y/n/j H:i:s.", filemtime($infile)));
		fprintf($fp, "Conversion time: %s<br/><br/>", date ("Y/n/j H:i:s.", time()));
		
		for ( $k=0; $k < $sn; $k++) {
			fprintf($fp, "<img src=\"img_%d.png\"/>\n", $k );
			fprintf($fp, "<br/>");
		}
		
		fprintf($fp, "</html>\n");
		fclose($fp);
	}
	else {
		printf("cannot open %s\n", $buf);
	}
	
	if ($utf_file != NULL)
		unlink($utf_file);	
}

function Process($srcdir, $dstdir_root)
{
	global $VERBOSE;

	printf("Generating code image %s => %s\n", $srcdir, $dstdir_root);

	$h_dir = opendir($srcdir);
	
	if ( $h_dir== FALSE) {
		exit (-1);
	}
	
	if ( !file_exists($dstdir_root))
		mkdir($dstdir_root);
	
	while (($file = readdir($h_dir)) !== false) {
		if ( $file=="." || $file==".." || $file=="queue" || $file=="config") {
			continue;
		}
	
		$dir = CreateFullPath($srcdir,  $file);
		$dstdir = CreateFullPath($dstdir_root, $file); 
		$dstdir_code = CreateFullPath($dstdir, "code"); 
		$dstdir_callgrind = CreateFullPath($dstdir, "callgrind"); 
		$dstdir_massif = CreateFullPath($dstdir, "massif"); 

		
		
		if ( filetype($dir)!="dir") {
			continue;
		}

		
			

		if ( !file_exists($dstdir)) 
			mkdir($dstdir);
				
		if ( !file_exists($dstdir_code))
			mkdir($dstdir_code);
//				if ( !file_exists($dstdir_massif))
//					mkdir($dstdir_massif);				
		if ( !file_exists($dstdir_callgrind))
			mkdir($dstdir_callgrind);					
	

		if ( ($compilable = IsCodeCompilable($dir))==false) {
            printf("[$dir] is not compilable !\n");
        }


		if ( $compilable)	
			$code_filename = "current/code";
		else
			$code_filename = "notexistabcde";
        $code_file = CreateFullPath($dir, $code_filename);
		TextFileToImage($code_file, 14, $dstdir_code , 2);
	
		$valgrind_massif =  CreateFullPath($dir, "current/valgrind.massif_out_ms_print.report.final");
	
		if ( $compilable && file_exists($valgrind_massif)) {
			$dstfile_valgrind_massif =  CreateFullPath($dstdir, "valgrind.massif_out_ms_print.report.final");
			if ( filemtime($valgrind_massif) > filemtime( $dstfile_valgrind_massif) ) { 	
				printf("copying %s => %s\n", $valgrind_massif, $dstdir);
				copy($valgrind_massif, $dstfile_valgrind_massif);
			}
		}
		
		if ( $compilable)
			$valgrind_callgrind = CreateFullPath($dir, "current/valgrind.callgrind_detail.report.final");
		else
			$valgrind_callgrind = CreateFullPath($dir, "notlikelytoexist");
		TextFileToImage($valgrind_callgrind, 14, $dstdir_callgrind , 2);
			
		//echo "filename: $file : filetype: " . filetype($HOMEWORK_DIR ."/" . $file) . "<br/>";
	}
	
	closedir($h_dir);

}


#Process($argv[1], $argv[2]);
Process( CreateFullPath($HOMEWORK_DIR,$HW_NAME), CreateFullPath($WEB_ROOT, $HW_NAME));
///////////////////////////////////////////////////////

///TextToImage("email@example.com 你好 ggG", 24,"/tmp/a.png");
//printf("%s\n", CreateFullPath("/tmp", "a.txt"));
//printf("%s\n",  CreateFullPath("/tmp/", "b.txt") );


//TextFileToImage($argv[1], 14, "/tmp",2);



?>
