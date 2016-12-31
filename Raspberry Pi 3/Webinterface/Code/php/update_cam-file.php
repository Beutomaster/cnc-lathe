<?php
	//Logged in?
	session_start();
	if(!$_SESSION['logged_in']) header("Location: /login.html");
	
	function test_input($data) {
		$data = trim($data);
		$data = stripslashes($data);
		$data = htmlspecialchars($data);
		return $data;
	}
	
	if ($_SERVER["REQUEST_METHOD"] == "POST") {
		/*
		foreach($_POST as $name => $value) { // Most people refer to $key => $value
			echo "HTML name: $name <br />";
			echo "value of it: $value <br />";
		}
		*/
		$txt = test_input($_POST['CncCodeTxt']);
	}
   
	$target_file = "/var/www/html/uploads/cnc_code.txt";
	$myfile = fopen($target_file, "w") or die("Unable to open file!");
	fwrite($myfile, $txt);
	fclose($myfile);
	echo "CAM-File has been updated.";
?>