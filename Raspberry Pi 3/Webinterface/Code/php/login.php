<?php
// define variables and set to empty values
$usernameErr = $pwErr = "";
$username = $pw = "";

if ($_SERVER["REQUEST_METHOD"] == "POST") {
  if (empty($_POST["username"])) {
    $usernameErr = "Username is required";
  } else {
	$username = test_input($_POST["username"]);
  }
  
  if (empty($_POST["pw"])) {
    $pwErr = "Password is required";
  } else {
	$pw = test_input($_POST["pw"]);
  }
}

function test_input($data) {
  $data = trim($data);
  $data = stripslashes($data);
  $data = htmlspecialchars($data);
  return $data;
}
?>