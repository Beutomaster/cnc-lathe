<?php
	//Logged in?
	session_start();
	if(!$_SESSION['logged_in']) header("Location: /login.html");
	
	include 'verify_cnc_code.php';

	/*
   if(is_writable(".") && isset($_FILES['file-1'])) {
      move_uploaded_file($_FILES['file-1']['name'], ".");
   }
   */
   
   /*
   //debug
	if (!empty($_FILES)) {
		echo "<pre>\r\n";
		echo htmlspecialchars(print_r($_FILES,1));
		echo "</pre>\r\n";
	}
	*/
   
	//$target_dir = "/var/www/html/uploads/";
	//$target_file = $target_dir . basename($_FILES["file-1"]["name"]);
	$target_file = "/var/www/html/uploads/cnc_code.txt";
	
	// Check file size
	if ($_FILES["file-1"]["size"] > 500000) die ("File is too large!");

	//echo "Checking Filetype of: " . basename($_FILES["file-1"]["name"]) . "<br />";
	// Allow certain file formats
	$FileType = pathinfo(basename($_FILES["file-1"]["name"]),PATHINFO_EXTENSION);
	//echo "Filetype is: " . $FileType . "<br />";
	if($FileType != "txt" && $FileType != "TXT") die ("Only txt-files are allowed!");
	if($_FILES["file-1"]["type"] != "text/plain") die ("Only txt-files are allowed!");
	//echo mime_content_type('php.gif') . "\n";
	
	$cnc_code = file($_FILES["file-1"]["tmp_name"]);
	
	//Security (needs more attention in written file!)
	foreach ($cnc_code_reference as $line => $linevalue) {
		$cnc_code_reference[$line] = test_input($linevalue);
	}
	
	//debug
	//var_dump($cnc_code);
	
	verify_cnc_code($cnc_code, false, 0) or die("File not uploaded because of CNC-Code-Error!");
	
	/*
	$tmpfile = fopen($_FILES["file-1"]["tmp_name"], "r") or die("Unable to open tmp file!");
	// Output one line until end-of-file
	while(!feof($tmpfile)) {
	  $line = fgets($tmpfile);
	}
	fclose($tmpfile);
	*/

	// if everything is ok, try to upload file
	if (move_uploaded_file($_FILES["file-1"]["tmp_name"], $target_file)) {
		echo basename($_FILES["file-1"]["name"]). " has been uploaded.";
	} else {
		die ("Error uploading the file.");
	}
?>