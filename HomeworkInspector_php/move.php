<HTML>
<HEAD>
<title>move results</title>
</HEAD>
<BODY>
<h2> </h2><br/>
<?php
include_once("config.php");

$h_root_dir = "/var/homeworks/h_".$HW_NAME."/";

foreach(glob($h_root_dir."*", GLOB_ONLYDIR) as $h_sub) {
	$h_id =  basename($h_sub);
	if ($h_id != "queue" && $h_id != "config" && $h_id != "customized_templates") {
		$h_dir = $h_root_dir.$h_id."/current/";
		$dir = $root_dir.$h_id."/final/";

		if ( !file_exists($dir) ) {
			mkdir($dir);
			chmod($dir, 0700);
		}

		foreach(glob($dir."*") as $file) {
			unlink($file);
		}

		foreach(glob($h_dir."*") as $file) {
			if(!is_dir($file) && is_readable($file)) {
				copy($file, $dir.basename($file));
			}
		}

		touch($dir."code", filemtime($h_dir."code"));
		unlink($root_dir.$h_id."/current");
		symlink($dir, $root_dir.$h_id."/current");
	}
}

echo "all done<br>";
?>
</BODY>
</HTML>
