<HTML>
<HEAD>
<title>Re-evaluation</title>
</HEAD>
<BODY>
<h2> </h2><br/>
<?php
include_once("config.php");
include_once ("account.php");

if (!CookieAuthenticate($id, $name)) {
        exit(-1);
} 

if ($id != "baseline") {
        exit(-1);
}

if(!isset($_POST["re-eval"])) {
	echo "post error";
	exit(-1);
}


function eval_id($student_id) {
	global $HW_NAME;
	global $root_dir;
	global $filename_on_server;
	global $homework_inspector_executable;

	$h_root_dir = "/var/homeworks/h_".$HW_NAME."/";
	$h_userdir = $h_root_dir.$student_id."/";
	$dir = $root_dir.$student_id."/current/";

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
	symlink($h_target_file, $h_queue_dir.$student_id);

	proc_close(proc_open($homework_inspector_executable." ".$student_id." h_".$HW_NAME, Array(), $foo));
}

session_destroy();
$progress_file = "/var/homeworks/h_".$HW_NAME."/progress";

if ($_POST["re-eval"] == "all") {
	$folder_cnt = 0;
	$folder_done = 0;

	foreach(glob($root_dir."*", GLOB_ONLYDIR) as $sub) {

		$h_id =  basename($sub);
		if ($h_id != "queue" && $h_id != "config" && $h_id != "customized_templates") {
			$folder_cnt++;
		}
	}

	file_put_contents($progress_file, "working... (0/".$folder_cnt.")\n");

	foreach(glob($root_dir."*", GLOB_ONLYDIR) as $sub) {
		$h_id =  basename($sub);
		if ($h_id != "queue" && $h_id != "config" && $h_id != "customized_templates") {
			eval_id($h_id);
			$folder_done++;
			file_put_contents($progress_file, "working... (".$folder_done."/".$folder_cnt.")\n");
		}
	}
} else {
	file_put_contents($progress_file, "working... (0/1)\n");
	eval_id($_POST["re-eval"]);
}

echo '<a href="hidden.php">Done</a>';
file_put_contents($progress_file, "all done\n");
?>
</BODY>
</HTML>
