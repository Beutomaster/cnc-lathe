<?php
	//Logged in?
	session_start();
	if(!$_SESSION['logged_in']) header("Location: /login.html");

	// define variables and set to empty values
	$Input_N = $Input_GM = $Input_GM_NO = $Input_XI = $Input_ZK = $Input_FTLK = $Input_HS = $Input_RPM = $Input_DIRECTION = $Input_INCH = $msg_pid = "";

	$msg = session_id() . "\n";
	echo "session_id: " . session_id() . " <br>";
	
	function test_input($data) {
		$data = trim($data);
		$data = stripslashes($data);
		$data = htmlspecialchars($data);
		return $data;
	}
	
	if ($_SERVER["REQUEST_METHOD"] == "POST") {
		foreach($_POST as $name => $value) { // Most people refer to $key => $value
			echo "HTML name: $name <br>";
			echo "value of it: $value <br>";
			
			$value = test_input($_POST[$name]);
			
			//test if value has only numbers and a negativ sign needed
			
			switch ($name) {
				case "xoffset":
				case "tool_x-correction": //maybe a higher range is needed
					if ($value >= -5999 && $value <= 5999) $Input_XI = $value;
					else die ("Value of " . $name . " out of range!");
					break;
				case "zoffset":
				case "tool_z-correction": //maybe a higher range is needed
					if ($value >= -32760 && $value <= 32760) $Input_ZK = $value;
					else die ("Value of " . $name . " out of range!");
					break;
				case "metric_inch":
					if ($value == "metric") $Input_INCH = "0";
					elseif ($value == "inch") $Input_INCH = "1";
					else die ("Value of " . $name . " out of range!");
					break;
				case "spindle_direction":
					if ($value == "right") $Input_DIRECTION = "0";
					elseif ($value == "left") $Input_DIRECTION = "1";
					else die ("Value of " . $name . " out of range!");
					break;
				case "rpm":
					if ($value >= 460 && $value <= 3220) $Input_RPM = $value;
					else die ("Value of " . $name . " out of range!");
					break;
				case "feed":
					if ($value >= 2 && $value <= 499) $Input_FTLK = $value;
					else die ("Value of " . $name . " out of range!");
					break;
				case "tool":
					if ($value >= 1 && $value <= 6) $Input_FTLK = $value;
					else die ("Value of " . $name . " out of range!");
					break;
				case "ProgramStartStop":
					if ($value == "Start") {
						if (empty($Input_N)) die ("Minimum one field for message " . $name . " is missing!");
						$msg_pid = "1";
						$msg .= $msg_pid . "\n" . $Input_N . "\n";
					}
					elseif ($value == "Stop") {
						$msg_pid = "2";
						$msg .= $msg_pid . "\n";
					}
					else die ("Value of " . $name . " out of range!");
					break;
				case "ProgramPause":
					$msg_pid = "3";
					$msg .= $msg_pid . "\n";
					break;
				case "SpindleSetRPM":
					if (empty($Input_DIRECTION)) die ("Minimum one field for message " . $name . " is missing!");
					if (empty($Input_RPM)) die ("Minimum one field for message " . $name . " is missing!");
					$msg_pid = "4";
					$msg .= $msg_pid . "\n" . $Input_RPM . "\n" . $Input_DIRECTION . "\n";
					break;
				case "SpindleOff":
					$msg_pid = "5";
					$msg .= $msg_pid . "\n";
					break;
				case "StepperOn":
					$msg_pid = "6";
					$msg .= $msg_pid . "\n";
					break;
				case "StepperOff":
					$msg_pid = "7";
					$msg .= $msg_pid . "\n";
					break;
				case "XStepper":
					if (empty($Input_FTLK)) die ("Minimum one field for message " . $name . " is missing!");
					if (empty($Input_DIRECTION)) die ("Minimum one field for message " . $name . " is missing!");
					$msg_pid = "8";
					$msg .= $msg_pid . "\n" . $Input_FTLK . "\n" . $Input_DIRECTION . "\n";
					break;
				case "ZStepper":
					if (empty($Input_FTLK)) die ("Minimum one field for message " . $name . " is missing!");
					if (empty($Input_DIRECTION)) die ("Minimum one field for message " . $name . " is missing!");
					$msg_pid = "9";
					$msg .= $msg_pid . "\n" . $Input_FTLK . "\n" . $Input_DIRECTION . "\n";
					break;
				case "SetTool":
					if (empty($Input_XI)) die ("Minimum one field for message " . $name . " is missing!");
					if (empty($Input_ZK)) die ("Minimum one field for message " . $name . " is missing!");
					if (empty($Input_FTLK)) die ("Minimum one field for message " . $name . " is missing!");
					$msg_pid = "10";
					$msg .= $msg_pid . "\n" . $Input_XI . "\n" . $Input_ZK . "\n" . $Input_FTLK . "\n";
					break;
				case "SetXOffset":
					if (empty($Input_XI)) die ("Minimum one field for message " . $name . " is missing!");
					$msg_pid = "11";
					$msg .= $msg_pid . "\n" . $Input_XI . "\n";
					break;
				case "SetZOffset":
					if (empty($Input_ZK)) die ("Minimum one field for message " . $name . " is missing!");
					$msg_pid = "12";
					$msg .= $msg_pid . "\n" . $Input_ZK . "\n";
					break;
				case "SetMetricOrInch": //some js needed
					if (empty($Input_INCH)) die ("Minimum one field for message " . $name . " is missing!");
					$msg_pid = "13";
					$msg .= $msg_pid . "\n" . $Input_INCH . "\n";
					break;
				case "LoadNewProgramm":
					if (empty($Input_N)) die ("Minimum one field for message " . $name . " is missing!");
					if (empty($Input_INCH)) die ("Minimum one field for message " . $name . " is missing!");
					$msg_pid = "14";
					$msg .= $msg_pid . "\n" . $Input_N . "\n" . $Input_INCH . "\n";
					break;
				case "NewProgrammBlock":
					if (empty($Input_N)) die ("Minimum one field for message " . $name . " is missing!");
					if (empty($Input_GM)) die ("Minimum one field for message " . $name . " is missing!");
					if (empty($Input_GM_NO)) die ("Minimum one field for message " . $name . " is missing!");
					if (empty($Input_XI)) die ("Minimum one field for message " . $name . " is missing!");
					if (empty($Input_ZK)) die ("Minimum one field for message " . $name . " is missing!");
					if (empty($Input_FTLK)) die ("Minimum one field for message " . $name . " is missing!");
					if (empty($Input_HS)) die ("Minimum one field for message " . $name . " is missing!");
					$msg_pid = "15";
					$msg .= $msg_pid . "\n" . $Input_N . "\n" . $Input_GM . "\n" . $Input_GM_NO . "\n" . $Input_XI . "\n" . $Input_ZK . "\n" . $Input_FTLK . "\n" . $Input_HS . "\n";
					break;
				case "Shutdown":
					$msg_pid = "16";
					$msg .= $msg_pid . "\n";
					break;
				case "ResetErrors":
					$msg_pid = "17";
					$msg .= $msg_pid . "\n";
					break;
				default:
					die ("Unknown field-name" . $name . " !");
			}
		}
	}
	
	if (empty($msg_pid)) die ("Unknown message-pid!");

	//create client-pipe
	//posix_mkfifo ("/home/pi/spi_com/client_session_pipe.".session_id(), int $mode ) or die ("Could not create client-pipe!");
	
	//open server pipe writeonly
	$server_pipe = fopen("/home/pi/spi_com/arduino_pipe.tx", "w") or die("Unable to open server-pipe!");
	fwrite($server_pipe, $msg);
	fclose($server_pipe);
	
	//non-blocking
	//$fh=fopen($fifo, "r+"); // ensures at least one writer (us) so will be non-blocking
	//stream_set_blocking($fh, false); // prevent fread / fwrite blocking
	
	//open client pipe writeonly
	//$answersize=87;
	//$client_pipe = fopen("/home/pi/spi_com/arduino_pipe.tx", "r") or die("Unable to open client-pipe!");
	//fread($client_pipe, $answersize);
	//fclose($client_pipe);
?>
