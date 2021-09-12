<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
	<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
	<title>CODE SENSOR</title>
	<script src="SpryAssets/SpryMenuBar.js" type="text/javascript"></script>
	<link href="SpryAssets/SpryMenuBarHorizontal.css" rel="stylesheet" type="text/css" />

	<link href='https://fonts.googleapis.com/css?family=Ubuntu:500' rel='stylesheet' type='text/css'>
	<link href='https://fonts.googleapis.com/css?family=Open+Sans' rel='stylesheet' type='text/css'>	
<style type="text/css">
	#id_code_sensor {
		float:left;width:250px; 
	}

	#id_polynomial{
		float:left;width:150px; 
	}
	
	div#f1 { float: left; width: 500px; }
	div#f2 { float: left; width: auto; }
	div#f3 { float: right; 
			 width: auto; 
			 vertical-align: middle; 	
			 padding: 0.5em 0.75em; }
	
	div#username {float: left;
					width: 200px;}
	div.logout_link {float:left ;
						width: auto;}		 

	.CodeSensorFont {
		text-decoration: none;
		color: #8F2400;
		font-family: 'Ubuntu', sans-serif;
	}	

	.MenuItemText {
		font-family: 'Open Sans', sans-serif;
	}	

</style>	
</head>

<body>

<script language="JavaScript">
	function link(url) {
	//	var bodyframe = window.frames.getElementById("bodyframe");
    //	bodyframe.location.href = url;
		window.top.bodyframe.location.href = url;
//	    top.b.location.reload();
	}
</script>

<div>
	<div id="f1">
		<div id="id_code_sensor" class="TitleFont"  ><a href="https://codesensor.cs.nycu.edu.tw" class="CodeSensorFont" target="_top">Code Sensor</a></div>	 <div id="id_polynomial" class="TitleFontSmall">POLYNOMIAL TIME VERIFIABLE</div>
	</div>
	
	<div id="f2">
		<ul id="MenuBar1" class="MenuBarHorizontal">
	      <li><a href="javascript:link('view.php');" class="MenuItemText">View </a>        </li>
	      <li><a href="javascript:link('upload.php');" class="MenuItemText">Submit</a></li>
	      
	      <li><a href="javascript:link('scoreboard.php');" class="MenuItemText">Scoreboard</a>        </li>
	    </ul>
	</div>

	
	<div id="f3">
	
		<?php  	
			include_once ('account.php');	
			
			if ( CookieAuthenticate($id, $name )) {
				
		//		echo '<ul id="MenuBar2" class="MenuBarHorizontal">';
//				echo '<ul>';
				
				if ( !($record = GetUserRecordByID($id)) ) {
				//	echo "<li>Error</li>";	
				}


								
				$ttl =  $record["logon_session_expire_time"] - time();
				
				if ( $ttl < 0 )
					$ttl = 0;
				
				
					/*
				echo "<li>Welcome&nbsp";
				echo $name ;
				echo "</li>";
				*/
				echo '<div id="username">Welcome &nbsp';
				echo '<a href="login_old_faithful/change_pwd.html" target="_blank">'.$name.'</a>' ;
				echo '</div>';
				
				echo '<div class="logout_link">';
				echo '<a href="javascript:link(\'logout.php?id='.$id.'\');">Logout</a>';
				echo '</div>';
				
				echo '<div class="logout_link"> &nbsp in &nbsp </div> <div id="logout_timer" class="logout_link"></div >';
						
		//		echo '<li><a href="javascript:link(\'login.html\');">Logout</a> </li>';
			//	echo '</ul>';
				
				echo '<script type="text/javascript">';
					echo 'var ttl = '.$ttl.';';
					
					echo 'function reload_page()';
					echo '{';
						//echo 'alert('.$ttl.');';
						echo 'window.open("logout.php", "_top" );';
					echo '}';
					 
					echo 'function logout_timer()';
					echo '{';
					echo 	'var lt = document.getElementById("logout_timer");';					
					echo 	'lt.innerHTML = ttl + " seconds";';
					echo 	'ttl--;';
					//echo 'alert("Hi");';
					//echo 'divID.innerHTML = " X seconds";';
					echo '}';
					
					echo 'window.onload = function () {';
						echo 'setTimeout("reload_page()",'.$ttl.'*1000);';
						echo 'setInterval("logout_timer()", 1000);';
						
					echo '}'; 
				echo '</script>';
			}
			else {
			//	echo '<ul id="MenuBar2" class="MenuBarHorizontal">';
			//	echo '<li><a href="javascript:link(\'login.html\');">Login</a> </li>	</ul>';
				echo '<a href="javascript:link(\'login.html\');">Login</a> ';
			}
		?>		
	</div>	
</div>

<!-- 

<table width="100%" border="0">
  <tr>

	<div>
		<div id="id_code_sensor" class="TitleFont"  >Code Sensor</div>	 <div id="id_polynomial" class="TitleFontSmall">POLYNOMIAL TIME VERIFIABLE</div>
	</div>

  </tr>
  <tr>
    <td bgcolor="#ffcc00"><ul id="MenuBar1" class="MenuBarHorizontal">
      <li><a href="javascript:link('view.html');">View </a>        </li>
      <li><a href="javascript:link('upload.php');">Submit</a></li>
      <li><a href="javascript:link('scoreboard.php');">Scoreboard</a>        </li>
    </ul></td>
  </tr>
</table>

 -->
 
 
<p>&nbsp; </p>
<p>&nbsp;</p>


<!-- Start of StatCounter Code for Default Guide -->
<script type="text/javascript">
var sc_project=10225918; 
var sc_invisible=1; 
var sc_security="2781ecf1"; 
var sc_https=1; 
var scJsHost = (("https:" == document.location.protocol) ?
"https://secure." : "http://www.");
document.write("<sc"+"ript type='text/javascript' src='" +
scJsHost+
"statcounter.com/counter/counter.js'></"+"script>");
</script>
<noscript><div class="statcounter"><a title="web statistics"
href="http://statcounter.com/" target="_blank"><img
class="statcounter"
src="http://c.statcounter.com/10225918/0/2781ecf1/1/"
alt="web statistics"></a></div></noscript>
<!-- End of StatCounter Code for Default Guide -->

</body>
</html>

