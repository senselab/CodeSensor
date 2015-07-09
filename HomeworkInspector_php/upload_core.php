<HTML>
<HEAD>
<title>上傳檔案</title>
</HEAD>
<BODY>
<h2> </h2><br/>
<?php



session_start();

include ("config.php");
include ("account.php");

///////// verify captcha /////////
if (empty($_SESSION['captcha']) || strtolower(trim($_REQUEST['captcha'])) != $_SESSION['captcha']) {
	echo "<br/><font color=red>Invalid captcha !</font><br/>";
	exit(-4);
}

/////////////////////

$szDeadline = GetConfigValueList("deadline");

$past_deadline_confirmed = $_SESSION['past_deadline_confirmed'];

if ( count($szDeadline) !=1 )
	exit(-1);

//echo $szDeadline[0]. "<br/>";

$deadline = date_parse_from_format("Y-n-j H:i:s", $szDeadline[0]);

if ( $deadline==false)
	exit(-1);
//echo var_dump($deadline)."<br/>";

$tmDeadline = mktime($deadline["hour"], $deadline["minute"], $deadline["second"], 
                     $deadline["month"], $deadline["day"], $deadline["year"]);

$tmNow = time();
if ( $tmNow > $tmDeadline && !$past_deadline_confirmed) {
	echo "Deadline is ".date('Y-n-j H:i:s', $tmDeadline). "<br/><br/>";
	echo "Current time is ".date('Y-n-j H:i:s', $tmNow). "<br/><br/>";
	echo "<br/><font color=red>Submission is now closed !</font><br/>";
	exit(-2);
}


//echo "檔案存放在Web伺服器上的位置 => ". $_FILES['userfile']['tmp_name'] ."<br/>";
echo "原始檔案名稱： ". $_FILES['userfile']['name'] ."<br/>";
echo "檔案大小 : ". $_FILES['userfile']['size'] ."<br/>";
echo "檔案型式 : ". $_FILES['userfile']['type'] ."<br/>";
echo "錯誤代碼 : ". $_FILES['userfile']['error'] ."<br/>";
?>
<hr>
<?php



include_once ("account.php");



//if ( Authenticate($_POST["userid"], $_POST["userpwd"], $users)==FALSE ) {
if ( !CookieAuthenticate($id, $name )) {
   echo "<font size=5 color=red>Account does not exist !</font></h1>";
   
	mylog($id, "upload login faiulre");
   
	exit(-1);
}


{	///abort upload if something is still in queue
	$queue_file =  $queue_dir . "/". $id;

	if ( isset($_POST["check_kill_job"]) ) {	// kill existing job
		
//		exec("$homework_inspector_executable  ". $id . "  ".$HW_NAME. "  kill", $dummy_output );

		proc_close(proc_open("$homework_inspector_executable  ". $id . "  ".$HW_NAME. "  kill", Array(), $foo));
//		sleep(3);	
		unlink($queue_file);
	}



	if ( file_exists($queue_file)) {
		$msg = sprintf( "Error: a previous submission at %s is still running !",  date ("F j, Y H:i:s", filemtime($queue_file)) );
   		echo "<font size=5 color=red>$msg</font></h1>";
   		exit(-1);	
	}

}



$userfilename = $_FILES['userfile']['name'];
$userdir = $root_dir . $id . "/";

if ( !file_exists($userdir) ) {
	mkdir($userdir);
	chmod($userdir, 0700);
}

$dir_with_date =  $userdir . date("Y-n-j H:i:s") . "/";
$dir =  $userdir . "current" ;



mkdir($dir_with_date);
chmod($dir_with_date, 0700);
unlink($dir);
$status = symlink($dir_with_date, $dir);
$dir = $dir . "/";

//error_log("[".$dir."]");
//error_log("symlink = ". $status);

//$dir = $root_dir;

//$target_file = $dir . $_FILES['userfile']['name']; //upload/
$target_file = $dir . $filename_on_server;


if ($_POST["language"]=="customized") {
	$zip_file = $target_file.".zip";
	$result = move_uploaded_file( $_FILES['userfile']['tmp_name'], $zip_file);
	proc_close(proc_open("unzip ".$zip_file."  code  -d ".$dir, Array(), $foo));
	
	if( file_exists($target_file)==false) {
		echo "<font size=5 color=red>Missing source code file</font></h1>";
		unlink($queue_file);
		exit(-1);
	}

	proc_close(proc_open("touch  ".$target_file, Array(), $foo));
	/////////////////////////////////////////////////

	
}
else {
	$zip_file = $target_file.".zip";

	$result = move_uploaded_file( $_FILES['userfile']['tmp_name'], $target_file);

	switch($_POST["language"]) {
		case "cpp":
			$customized_template = $root_dir . "customized_templates/cpp/"; 	// /var/homeworks/{hw_name}/customized_templates/cpp
			break;
		case "java":
			$customized_template = $root_dir . "customized_templates/java/"; 	
			break;			
		default:
			echo "<font size=5 color=red>Unknown language type</font></h1>";
			unlink($queue_file);
			exit(-1);
			break;	
	}

	proc_close(proc_open("cd ".$dir."; zip ".$zip_file."  ".$filename_on_server, Array(), $foo));
	proc_close(proc_open("cd ".$customized_template. "; zip -r ".$zip_file."  *", Array(), $foo));
}

/// write setting file
{
	if ( $fp = fopen( $dir . $setting_filename, 'at') ) {
		fprintf($fp, "language=%s\n", $_POST["language"]);
	/*	fprintf($fp, "memory_size_cap=%d\n", intval($_POST["memory_size_cap"]));
	
		if (  isset($_POST["check_disable_optimization"] ) ) 
			fprintf($fp, "disable_optimization=1\n");
		else
			fprintf($fp, "disable_optimization=0\n");*/

		fclose($fp);
	}
}


//echo "客戶端檔案 $userfilename 上傳</br>";

//echo "Try to put at $target_file</br>";


if ($result) {
   echo "<font size=5 color=red>成功</br></font></h1>";
}
else {
   echo "<font size=5 color=red>失敗</font></h1>";
   exit(-1);
}
/*
$target_file_v[0] = $target_file ;
$target_file_v[1] = $target_file . ".1";
$target_file_v[2] = $target_file . ".2";
$target_file_v[3] = $target_file . ".3";
$target_file_v[4] = $target_file . ".4";
$target_file_v[5] = $target_file . ".5";
$target_file_v[6] = $target_file . ".6";
$target_file_v[7] = $target_file . ".7";
$target_file_v[8] = $target_file . ".8";

for ( $i = 8; $i >=1 ;  $i--) {
	copy($target_file_v[$i-1], $target_file_v[$i]);
}
*/

symlink($target_file, $queue_dir . "/". $id);



$sha_value = sha1_file($target_file);
echo "sha1 : ". $sha_value ."</br>";


$OPTIONS = "";

if ( isset($_POST["check_disable_optimization"] ) )
	$OPTIONS = "DISABLE_OPTIMIZATION";


proc_close(proc_open($homework_inspector_executable." ". $id." " . $HW_NAME  . " $OPTIONS &", Array(), $foo));

mylog($id, "upload file (sha1:$sha_value)");

//chdir($dir);
//proc_close(proc_open("sh /var/www/html/build_script " . $target_file . " &", Array(), $foo));
//shell_exec("nohup sh /var/www/html/build_script " . $target_file . " &");
//exec("valgrind /tmp/a.out > /tmp/dog.txt 2>&1");
?>
</BODY>
</HTML>
