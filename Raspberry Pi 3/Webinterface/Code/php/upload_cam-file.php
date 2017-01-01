<?php
	//Logged in?
	session_start();
	if(!$_SESSION['logged_in']) header("Location: /login.html");

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

	// if everything is ok, try to upload file
	if (move_uploaded_file($_FILES["file-1"]["tmp_name"], $target_file)) {
		echo basename($_FILES["file-1"]["name"]). " has been uploaded.";
	} else {
		die ("Error uploading the file.");
	}
?>