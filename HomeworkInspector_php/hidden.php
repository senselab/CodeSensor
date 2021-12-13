<!DOCTYPE html>
<head>
<title>Re-evaluation</title>
</head>
<body>
	<?php
		include_once ("config.php");
		include_once ("account.php");

		if (!CookieAuthenticate($id, $name)) {
			exit(-1);
		}

		if ($id != "baseline") { 
			exit(-1); 
		}

		echo '<form action="re-eval.php" enctype=”multipart/form-data” method="POST">'."\n";
		echo '	<button type="submit" name="re-eval" value="all">Re-eval All</button>'."\n";
		echo '	<input type="submit" value="move results" formaction="move.php">'."\n";
		echo file_get_contents("/var/homeworks/h_" . $HW_NAME . "/progress");
		echo '<br><br><br>'."\n";

		foreach(glob($root_dir."*", GLOB_ONLYDIR) as $sub) {
			$foldername = basename($sub);
			if ($foldername != "queue" && $foldername != "config" && $foldername != "customized_templates") {
				echo '<button type="submit" name="re-eval" value="' . $foldername . '">Re-eval ' . $foldername . '</button>';

				$analysis_result= "/var/homeworks/h_".$HW_NAME."/".$foldername."/current/analysis_result.final";
				echo '<p>' . file_get_contents($analysis_result) . '</p>';

				echo '<br>'."\n";
			}
		}

		echo '</form>'."\n";
	?>
</body>
</html>

