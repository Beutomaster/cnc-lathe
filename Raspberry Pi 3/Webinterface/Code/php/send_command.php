<?php
	//Logged in?
	session_start();
	if(!$_SESSION['logged_in']) header("Location: /login.html");

	// define variables and set to empty values
	$Input_N = $Input_GM = $Input_GM_NO = $Input_XI = $Input_ZK = $Input_FTLK = $Input_HS = $Input_RPM = $Input_DIRECTION = $Input_INCH = $msg_pid = "";
	
	$success=false;
	
	$msg = session_id() . "\n";
	echo "session_id: " . session_id() . " <br />";
	
	function test_input($data) {
		$data = trim($data);
		$data = stripslashes($data);
		$data = htmlspecialchars($data);
		return $data;
	}
	
	function test_keys_exist($keyArray) {
		$success=true;
		foreach($keyArray as $key) {
			if (!array_key_exists($key, $_POST)) {
				$success=false;
				echo ("Key " . $key . " is missing!<br />");
			}
		}
		unset($key);
		return $success;
	}
	
	function test_value_range($value, $min, $max) {
		$success=true;
		//test if value has only numbers and a negativ sign needed
		if (!is_numeric($value) || is_float($value)) {
			$success=false;
			echo "Value of " . $name . " is not numeric or float!<br />";
		}
		//test range of value matches
		if ($value < $min || $value > $max) {
			$success=false;
			echo "Value of " . $name . " out of range!<br />";
		}
		return $success;
	}
	
	if ($_SERVER["REQUEST_METHOD"] == "POST") {
		foreach($_POST as $name => $value) { // Most people refer to $key => $value
			echo "HTML-Input name: $name, value: $value <br />";
		}
		
		$success=true;
		
		switch (test_input($_POST["command"])) {
			case "ProgramStart":
				$msg_pid = "2";
				$parameter = array("block");
				$success &= false_keys_exist($parameter) or exit(1);
				$Input_N = test_input($_POST["block"]);
				$success &= false_value_range($Input_N, 0, 9999);
				$msg .= $msg_pid . "\n" . $Input_N . "\n";
				break;
			case "ProgramStop":
				$msg_pid = "3";
				$msg .= $msg_pid . "\n";
				break;
			case "ProgramPause":
				$msg_pid = "4";
				$msg .= $msg_pid . "\n";
				break;
			case "SpindleSetRPM":
				$msg_pid = "5";
				$parameter = array("spindle_direction", "rpm");
				$success &= false_keys_exist($parameter) or exit(1);
				$Input_DIRECTION = test_input($_POST["spindle_direction"]);
				$success &= false_value_range($Input_DIRECTION, 0, 1);
				$Input_RPM = test_input($_POST["rpm"]);
				$success &= false_value_range($Input_RPM, 460, 3220);
				$msg .= $msg_pid . "\n" . $Input_RPM . "\n" . $Input_DIRECTION . "\n";
				break;
			case "SpindleOff":
				$msg_pid = "6";
				$msg .= $msg_pid . "\n";
				break;
			case "StepperOn":
				$msg_pid = "7";
				$msg .= $msg_pid . "\n";
				break;
			case "StepperOff":
				$msg_pid = "8";
				$msg .= $msg_pid . "\n";
				break;
			case "XStepper":
				$msg_pid = "9";
				$parameter = array("stepper_direction", "feed");
				$success &= false_keys_exist($parameter) or exit(1);
				$Input_DIRECTION = test_input($_POST["stepper_direction"]);
				$success &= false_value_range($Input_DIRECTION, 0, 1);
				$Input_FTLK = test_input($_POST["feed"]);
				$success &= false_value_range($Input_FTLK, 0, 499);
				$msg .= $msg_pid . "\n" . $Input_FTLK . "\n" . $Input_DIRECTION . "\n";
				break;
			case "ZStepper":
				$msg_pid = "10";
				$parameter = array("stepper_direction", "feed");
				$success &= false_keys_exist($parameter) or exit(1);
				$Input_DIRECTION = test_input($_POST["stepper_direction"]);
				$success &= false_value_range($Input_DIRECTION, 0, 1);
				$Input_FTLK = test_input($_POST["feed"]);
				$success &= false_value_range($Input_FTLK, 0, 499);
				$msg .= $msg_pid . "\n" . $Input_FTLK . "\n" . $Input_DIRECTION . "\n";
				break;
			case "SetTool":
				$msg_pid = "11";
				$parameter = array("tool_x-correction", "tool_z-correction", "tool");
				$success &= false_keys_exist($parameter) or exit(1);
				$Input_XI = test_input($_POST["tool_x-correction"]);
				$success &= false_value_range($Input_XI, -5999, 5999);
				$Input_ZK = test_input($_POST["tool_z-correction"]);
				$success &= false_value_range($Input_ZK, -32760, 32760);
				$Input_FTLK = test_input($_POST["tool"]);
				$success &= false_value_range($Input_FTLK, 1, 6);
				$msg .= $msg_pid . "\n" . $Input_XI . "\n" . $Input_ZK . "\n" . $Input_FTLK . "\n";
				break;
			case "SetXOffset":
				$msg_pid = "12";
				$parameter = array("xoffset");
				$success &= false_keys_exist($parameter) or exit(1);
				$Input_XI = test_input($_POST["xoffset"]);
				$success &= false_value_range($Input_XI, -5999, 5999);
				$msg .= $msg_pid . "\n" . $Input_XI . "\n";
				break;
			case "SetZOffset":
				$msg_pid = "13";
				$parameter = array("zoffset");
				$success &= false_keys_exist($parameter) or exit(1);
				$Input_ZK = test_input($_POST["zoffset"]);
				$success &= false_value_range($Input_ZK, "-32760", "32760");
				$msg .= $msg_pid . "\n" . $Input_ZK . "\n";
				break;
			case "SetMetricOrInch":
				$msg_pid = "14";
				$parameter = array("metric_inch");
				$success &= false_keys_exist($parameter) or exit(1);
				$Input_INCH = test_input($_POST["metric_inch"]);
				if ($Input_INCH == "metric") $Input_INCH = "0";
				elseif ($Input_INCH == "inch") $Input_INCH = "1";
				else {
					echo "Value of metric_inch out of range!";
					$success = false;
				}
				$msg .= $msg_pid . "\n" . $Input_INCH . "\n";
				break;
			case "LoadNewProgramm":
				$msg_pid = "15";
				$parameter = array("block", "metric_inch");
				$success &= false_keys_exist($parameter) or exit(1);
				$Input_N = test_input($_POST["block"]);
				$success &= false_value_range($Input_N, 0, 9999);
				$Input_INCH = test_input($_POST["metric_inch"]);
				if ($Input_INCH == "metric") $Input_INCH = "0";
				elseif ($Input_INCH == "inch") $Input_INCH = "1";
				else {
					echo "Value of metric_inch out of range!";
					$success = false;
				}
				$msg .= $msg_pid . "\n" . $Input_N . "\n" . $Input_INCH . "\n";
				break;
			case "NewProgrammBlock":
				$msg_pid = "16";
				$parameter = array("block", "gm_code", "gm_code_no", "cnc_xi", "cnc_zk", "cnc_ftlk", "cnc_hs");
				$success &= false_keys_exist($parameter) or exit(1);
				$Input_N = test_input($_POST["block"]);
				$success &= false_value_range($Input_N, 0, 9999);
				$Input_GM = test_input($_POST["gm_code"]);
				$Input_GM_NO = test_input($_POST["gm_code_no"]);
				if ($Input_GM == "G") {
					$success &= false_value_range($Input_GM_NO, 0, 196); //should be a check against an array of known codes!!!
				}
				elseif ($Input_GM == "M") {
					$success &= false_value_range($Input_GM_NO, 0, 99); //should be a check against an array of known codes!!!
				}
				else {
					echo "Value of gm_code out of range!";
					$success = false;
				}
				$Input_XI = test_input($_POST["cnc_xi"]);
				$success &= false_value_range($Input_XI, -5999, 5999);
				$Input_ZK = test_input($_POST["cnc_zk"]);
				$success &= false_value_range($Input_ZK, -32760, 32760);
				$Input_FTLK = test_input($_POST["cnc_ftlk"]);
				$success &= false_value_range($Input_FTLK, -32760, 32760); //not right!!! Many cases!!!
				$Input_HS = test_input($_POST["cnc_hs"]);
				$success &= false_value_range($Input_HS, -32760, 32760); //not right!!! Many cases!!!
				$msg .= $msg_pid . "\n" . $Input_N . "\n" . $Input_GM . "\n" . $Input_GM_NO . "\n" . $Input_XI . "\n" . $Input_ZK . "\n" . $Input_FTLK . "\n" . $Input_HS . "\n";
				break;
			case "Shutdown":
				$msg_pid = "17";
				$msg .= $msg_pid . "\n";
				break;
			case "LoadOldParameter":
				$msg_pid = "18";
				$msg .= $msg_pid . "\n";
				break;
			case "ResetErrors":
				$msg_pid = "19";
				$msg .= $msg_pid . "\n";
				break;
			default:
				echo ("Unknown Command!");
				$success=false;
		}
	}
	
	if (!$success) exit(1);
	
	ignore_user_abort(true);

	//create client-pipe
	//$success = posix_mkfifo ("/home/pi/spi_com/client_session_pipe.".session_id(), int $mode ) or exit ("Could not create client-pipe!");
	
	//open server pipe writeonly (shorter timeout needed!!!)
	$server_pipe = fopen("/home/pi/spi_com/arduino_pipe.tx", "w") or exit("Unable to open server-pipe!");
	//stream_set_blocking($server_pipe, false); // prevent fread / fwrite blocking
	stream_set_timeout($server_pipe, 2); //wait 2s for pipe (does not work)
	fwrite($server_pipe, $msg);
	$info = stream_get_meta_data($server_pipe);
	fclose($server_pipe);
	
	if ($info['timed_out']) {
        echo 'Pipe-Write timed out!';
		exit(1);
    } else {
        exit(0);
    }
	
	//non-blocking
	//$fh=fopen($fifo, "r+"); // ensures at least one writer (us) so will be non-blocking
	//stream_set_blocking($fh, false); // prevent fread / fwrite blocking
	
	//open client pipe writeonly
	//$answersize=87;
	//$client_pipe = fopen("/home/pi/spi_com/arduino_pipe.tx", "r") or exit("Unable to open client-pipe!");
	//fread($client_pipe, $answersize);
	//fclose($client_pipe);
?>
