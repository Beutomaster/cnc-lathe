<?php
/*** CONFIGURATION ***/

// Defines
define('MD5_ENCRYPT', true); // activate Encryption
define('SUCCESS_URL', '/index.php'); // URL for redirect after successful Login
define('LOGIN_FORM_URL', '/login.html'); // URL with Login-Form

// Array with Userdata
$usrdata = array(
	array(
		"usr" => "CNC",
		//"pwd" => '$2y$10$JgVlLOl76rgIX/5d9BO5nuKl/gWXu4Y/XLqnrjZpBYuiOP5zJ4Ddy' // password_hash('12345', PASSWORD_DEFAULT);
		"pwd" => "827ccb0eea8a706c4c34a16891f84e7b" // MD5-crypted form of 12345
	),
	array(
		"usr" => "Test",
		//"pwd" => '$2y$10$JgVlLOl76rgIX/5d9BO5nuKl/gWXu4Y/XLqnrjZpBYuiOP5zJ4Ddy' // password_hash('12345', PASSWORD_DEFAULT);
		"pwd" => "827ccb0eea8a706c4c34a16891f84e7b" // MD5-crypted form of 12345
	)
	/* ... */
);


header("Content-Type: text/html; charset=utf-8"); // Charset


// start PHP-Session and query actual State
session_start();
$_SESSION['logged_in'] = (isset($_SESSION['logged_in']) && $_SESSION['logged_in'] === true) ? true : false;
$_SESSION['usr'] = (isset($_SESSION['usr'])) ? $_SESSION['usr'] : '';

$error = array();
if(!isset($_POST['login'])){
	header('Location: '.LOGIN_FORM_URL);
}else{
	$usr = (!empty($_POST['user']) && trim($_POST['user']) != '') ? $_POST['user'] : false;
	$pwd = (!empty($_POST['password']) && trim($_POST['password']) != '') ? $_POST['password'] : false;
	
	if(!$usr || !$pwd){
		if(count($error) == 0)
			$error[] = "Please type in Username and Password";
	}else{
		$pwd = (MD5_ENCRYPT === true) ? md5($pwd) : $pwd; // hashing Password-Input, if Option set
		echo $pwd;
		foreach($usrdata as $ud){ // Compare User-List with Form-Data
			if($usr != $ud['usr'] || $pwd != $ud['pwd']){
				if(count($error) == 0)
					$error[] = "Username and/or Password not correct.";
			}else{
				$_SESSION['logged_in'] = true;
				$_SESSION['usr'] = $usr;
				header('Location: '.SUCCESS_URL);
			}
		}
	}
}

?>
<!doctype html>
<html>
	<head>
		<meta name="content-type" content="text/html; charset=utf-8" />
		<title>Login-Error</title>
	</head>
	<body>
		<ul>
		<?php
		foreach($error as $out){
			?>
			<li><?php echo $out; ?></li>
			<?php
		}
		?>
		</ul>
		<p><a href="<?php echo LOGIN_FORM_URL; ?>">To Login-Page</a></p>
	</body>
</html>
		