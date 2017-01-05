<?php
	//Logged in?
	session_start();
	if(!$_SESSION['logged_in']) header("Location: /login.html");
	
	include 'verify_cnc_code.php';
	
	if ($_SERVER["REQUEST_METHOD"] == "POST") {
		/*
		foreach($_POST as $name => $value) { // Most people refer to $key => $value
			echo "HTML name: $name <br />";
			echo "value of it: $value <br />";
		}
		*/
		//$cnc_code = $_POST['CncCodeTxt'];
		$cnc_code = test_input($_POST['CncCodeTxt']);
		$cnc_code_array = preg_split("/\\r\\n|\\r|\\n/", $cnc_code);
		
	}
	
	//debug
	//var_dump($cnc_code_array);
	
	verify_cnc_code($cnc_code_array) or die("File not updated because of CNC-Code-Error!");
   
	$target_file = "/var/www/html/uploads/cnc_code.txt";
	$myfile = fopen($target_file, "w") or die("Unable to open file!");
	fwrite($myfile, $cnc_code);
	fclose($myfile);
	echo "CAM-File has been updated.";
?>