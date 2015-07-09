<?php


include_once '../account.php';
include_once '../config.php';

//$progress = 0;

if ( CookieAuthenticate($id, $name ) ) {
	$dir = $root_dir . $id . "/current/";
	$filename = $dir . "analysis_progress.report.final";

	
	if (file_exists($filename) && ($file = fopen($filename, "r")) ) {
		$buf = fread($file, filesize($filename));
		fclose($file);
		
		// no cache
		header('Pragma: no-cache');
		// HTTP/1.1
		header('Cache-Control: no-cache, must-revalidate');
		// date in the past
		header('Expires: Mon, 26 Jul 1997 05:00:00 GMT');
		
		header('Accept-Ranges: bytes');
		header('Content-Length: '.strlen( $buf )); // How many bytes we're going to send
		
		// define XML content type
		header('Content-type: text/xml; charset=utf-8');
		// print XML header
		
		echo $buf;
		/*
		while (!feof($file))   
		{  
			$value = trim(fgets($file));  
		  	
			if ( strlen($value) >0 ) {
		  		$progress = intval($value);
		  		break;
			} 	
		}  

		fclose($file);  		
	*/
	}
	
}
else {
        header('Pragma: no-cache');
        // HTTP/1.1
        header('Cache-Control: no-cache, must-revalidate');
        // date in the past
        header('Expires: Mon, 26 Jul 1997 05:00:00 GMT');

        header('Accept-Ranges: bytes');
        header('Content-Length: '.strlen( $buf )); // How many bytes we're going to send

        // define XML content type
        header('Content-type: text/xml; charset=utf-8');
		
		echo '<?xml version="1.0" ?><PROGRESS_STATUS><PROGRESS>0</PROGRESS></PROGRESS_STATUS>';
}

//$progress = round($progress*100 / $MAX_VIEW_PROGRESS_VALUE);

?>

