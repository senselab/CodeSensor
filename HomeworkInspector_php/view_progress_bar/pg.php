
<?php
  // no-cache headers - complete set
  // these copied from [php.net/header][1], tested myself - works
  header("Expires: Sat, 26 Jul 1997 05:00:00 GMT"); // Some time in the past
  header("Last-Modified: " . gmdate("D, d M Y H:i:s") . " GMT");
  header("Cache-Control: no-store, no-cache, must-revalidate");
  header("Cache-Control: post-check=0, pre-check=0", false);
  header("Pragma: no-cache");
	
?>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html>
	<head>
		<link rel="stylesheet" href="style.css" type="text/css" media="screen" />
		
		<?php
			include_once ("../config.php");

			echo '<script type="text/javascript">';
			echo 'var progress_value_max='.$MAX_VIEW_PROGRESS_VALUE.";";
			echo '</script>';
			
			echo '<script type="text/javascript" src="script.js?x=192767770">';
//			echo 'set_progress_bar_max_value('.$MAX_VIEW_PROGRESS_VALUE.');'; 
			echo '</script>';
		?>
	</head>	

	<body>	
	
	<?php
	/*
		$userid = $_GET["id"];
		echo "<h1>Hi</h1>" ;
		echo "<h1>$userid</h1>" ;*/
/*
		if ( array_key_exists("logon_session", $_GET) ) {
			setcookie("logon_session", $_GET["logon_session"], time()+86400*7 );
		}*/
	?>
	
			<div id="progress_container">
				<div id="progress" style="width: 0%"></div>
			</div>
	
	
	
			<!-- buttons 
			<input type="button" value="Start" onclick="javascript:polling_start()"	class="green" />
			<input type="button" value="Stop" onclick="javascript:polling_stop()" class="blue" />
				-->
	</body>
</html>
