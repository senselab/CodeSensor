<HTML>
<HEAD>
<title>Re-evaluation</title>
</HEAD>
<BODY>
<h2> </h2><br/>
<?php
set_time_limit(0);
ignore_user_abort(true);

include_once("config.php");

$h_root_dir = "/var/homeworks/h_".$HW_NAME."/";

foreach(glob($root_dir."*", GLOB_ONLYDIR) as $sub) {
	$h_id =  basename($sub);
	if ($h_id != "queue" && $h_id != "config" && $h_id != "customized_templates") {
		$h_userdir = $h_root_dir.$h_id."/";
		$dir = $sub."/current/";

		if (!file_exists($h_userdir)) {
			mkdir($h_userdir);
			chmod($h_userdir, 0700);
		}

		$h_dir = $h_userdir."current/";

		if (!file_exists($h_dir)) {
			mkdir($h_dir);
			chmod($h_dir, 0700);
		}

		foreach(glob($h_dir."*") as $file) {
			unlink($file);
		}

		$h_target_file = $h_dir.$filename_on_server;
		copy($dir.$filename_on_server, $h_target_file);
		touch($h_target_file, filemtime($dir.$filename_on_server));

		$h_zip_file = $h_target_file.".zip";
		copy($dir."setting", $h_dir."setting");

		// language=xxx
		$lang = file_get_contents($h_dir."setting", FALSE, NULL, 9);
		$lang = trim($lang, "\r\n");
		switch ($lang) {
			case "cpp":
			$h_customized_template = $h_root_dir."customized_templates/cpp/";
			break;
			case "cpp_shm":
			$h_customized_template = $h_root_dir."customized_templates/cpp_shm/";
			break;
			case "python":
			$h_customized_template = $h_root_dir."customized_templates/python/";
			break;
		}

		proc_close(proc_open("cd ".$h_dir."; zip ".$h_zip_file."  ".$filename_on_server, Array(), $foo));
		proc_close(proc_open("cd ".$h_customized_template."; zip -r ".$h_zip_file."  *", Array(), $foo));

		$h_queue_dir = $h_root_dir."queue/";
		symlink($h_target_file, $h_queue_dir.$h_id);

		proc_close(proc_open($homework_inspector_executable." ".$h_id." h_".$HW_NAME, Array(), $foo));
	}
}

echo "all done<br>";
?>
</BODY>
</HTML>
