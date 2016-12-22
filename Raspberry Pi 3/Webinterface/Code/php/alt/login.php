<?php
// define variables and set to empty values
$usernameErr = $pwErr = "";
$username = $pw = "";
$username_saved = "CNC";
$hashed_password = '$2y$10$JgVlLOl76rgIX/5d9BO5nuKl/gWXu4Y/XLqnrjZpBYuiOP5zJ4Ddy'; //PW=12345 for testing

if ($_SERVER["REQUEST_METHOD"] == "POST") {
  if (empty($_POST["username"])) {
    $usernameErr = "Username is required";
  } else {
	$username = test_input($_POST["username"]);
	if ( $username == $username_saved ) {
	  // User war richtig.
	} else {
	  // User war falsch.
	}
  }
  
  if (empty($_POST["pw"])) {
    $pwErr = "Password is required";
  } else {
	$pw = test_input($_POST["pw"]);
	if ( password_verify($pw, $hashed_password) ) {
	  // Passwort war richtig.
	} else {
	  // Passwort war falsch.
	}
  }
}

function test_input($data) {
  $data = trim($data);
  $data = stripslashes($data);
  $data = htmlspecialchars($data);
  return $data;
}
?>