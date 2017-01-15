<?php
	//Logged in?
	session_start();
	if(!$_SESSION['logged_in']) header("Location: /login.html");
	
	include 'verify_cnc_code.php';

	// define variables and set to empty values
	$Input_N = $Input_N_MAX = $Input_N_OFFSET = $Input_GM = $Input_GM_NO = $Input_XI = $Input_ZK = $Input_FTLK = $Input_HS = $Input_RPM = $Input_DIRECTION = $Input_INCH = $msg_pid = "";
	$G_Code_Numbers = array(0, 1, 2, 3, 4, 20, 21 ,22, 24, 25, 26, 27, 33, 64, 73, 78, 81, 82, 83, 84, 85, 86, 88, 89, 90, 91, 92, 94, 95, 96, 97, 196);
	$M_Code_Numbers = array(0, 3, 4, 5, 6, 17, 30, 98, 99);
	
	$success=0;
	
	$msg = session_id() . " ";
	echo "session_id: " . session_id() . " <br />";
	
	if ($_SERVER["REQUEST_METHOD"] == "POST") {
		foreach($_POST as $name => $value) { // Most people refer to $key => $value
			echo "HTML-Input name: $name, value: $value <br />";
		}
		
		$success=1;
		
		switch (test_input($_POST["command"])) {
			case "ProgramStart":
				$msg_pid = "2";
				$parameter = array("block", "FileParserOverride");
				$success &= test_keys_exist($parameter) or exit(1);
				$Input_N = test_input($_POST["block"]);
				$success &= test_value_range($Input_N, CNC_CODE_NMIN, CNC_CODE_FILE_PARSER_NMAX);
				$FileParserOverride = test_input($_POST["FileParserOverride"]);
				$success &= test_value_range($FileParserOverride, 0, 1);
				$msg .= $msg_pid . " " . $Input_N . " " . $FileParserOverride . "\n";
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
				$success &= test_keys_exist($parameter) or exit(1);
				$Input_DIRECTION = test_input($_POST["spindle_direction"]);
				$success &= test_value_range($Input_DIRECTION, 0, 1);
				$Input_RPM = test_input($_POST["rpm"]);
				$success &= test_value_range($Input_RPM, REVOLUTIONS_MIN, REVOLUTIONS_MAX);
				$msg .= $msg_pid . " " . $Input_RPM . " " . $Input_DIRECTION . "\n";
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
				$success &= test_keys_exist($parameter) or exit(1);
				$Input_DIRECTION = test_input($_POST["stepper_direction"]);
				$success &= test_value_range($Input_DIRECTION, 0, 1);
				$Input_FTLK = test_input($_POST["feed"]);
				$success &= test_value_range($Input_FTLK, F_MIN, F_MAX);
				$msg .= $msg_pid . " " . $Input_FTLK . " " . $Input_DIRECTION . "\n";
				break;
			case "ZStepper":
				$msg_pid = "10";
				$parameter = array("stepper_direction", "feed");
				$success &= test_keys_exist($parameter) or exit(1);
				$Input_DIRECTION = test_input($_POST["stepper_direction"]);
				$success &= test_value_range($Input_DIRECTION, 0, 1);
				$Input_FTLK = test_input($_POST["feed"]);
				$success &= test_value_range($Input_FTLK, F_MIN, F_MAX);
				$msg .= $msg_pid . " " . $Input_FTLK . " " . $Input_DIRECTION . "\n";
				break;
			case "SetTool":
				$msg_pid = "11";
				$parameter = array("tool_x-correction", "tool_z-correction", "tool");
				$success &= test_keys_exist($parameter) or exit(1);
				$Input_XI = test_input($_POST["tool_x-correction"]);
				$success &= test_value_range($Input_XI, -X_MIN_MAX_CNC, X_MIN_MAX_CNC); //XZ_MIN_MAX_HAND should be used, when Arduino-Code supports it
				$Input_ZK = test_input($_POST["tool_z-correction"]);
				$success &= test_value_range($Input_ZK, -Z_MIN_MAX_CNC, Z_MIN_MAX_CNC);
				$Input_FTLK = test_input($_POST["tool"]);
				$success &= test_value_range($Input_FTLK, 1, 6);
				$msg .= $msg_pid . " " . $Input_XI . " " . $Input_ZK . " " . $Input_FTLK . "\n";
				break;
			case "SetXOffset":
				$msg_pid = "12";
				$parameter = array("xoffset");
				$success &= test_keys_exist($parameter) or exit(1);
				$Input_XI = test_input($_POST["xoffset"]);
				$success &= test_value_range($Input_XI, -X_MIN_MAX_CNC, X_MIN_MAX_CNC); //XZ_MIN_MAX_HAND should be used, when Arduino-Code supports it
				$msg .= $msg_pid . " " . $Input_XI . "\n";
				break;
			case "SetZOffset":
				$msg_pid = "13";
				$parameter = array("zoffset");
				$success &= test_keys_exist($parameter) or exit(1);
				$Input_ZK = test_input($_POST["zoffset"]);
				$success &= test_value_range($Input_ZK, -Z_MIN_MAX_CNC, Z_MIN_MAX_CNC); //XZ_MIN_MAX_HAND should be used, when Arduino-Code supports it
				$msg .= $msg_pid . " " . $Input_ZK . "\n";
				break;
			case "SetMetricOrInch":
				$msg_pid = "14";
				$parameter = array("metric_inch");
				$success &= test_keys_exist($parameter) or exit(1);
				$Input_INCH = test_input($_POST["metric_inch"]);
				if ($Input_INCH == "metric") $Input_INCH = "0";
				elseif ($Input_INCH == "inch") $Input_INCH = "1";
				else {
					echo "Value of metric_inch out of range!";
					$success = 0;
				}
				$msg .= $msg_pid . " " . $Input_INCH . "\n";
				break;
			case "LoadNewProgramm": //maybe not used, instead Code is automatically uploaded at Programm Start
				$msg_pid = "15";
				$parameter = array("n_offset", "n_max", "metric_inch");
				$success &= test_keys_exist($parameter) or exit(1);
				$Input_N_OFFSET = test_input($_POST["n_offset"]);
				$success &= test_value_range($Input_N_OFFSET, CNC_CODE_NMIN, CNC_CODE_FILE_PARSER_NMAX); //value ignored by Backend, instead value from file-parser is used
				$Input_N_MAX = test_input($_POST["n_max"]);
				$success &= test_value_range($Input_N_MAX, CNC_CODE_NMIN, CNC_CODE_FILE_PARSER_NMAX); //value ignored by Backend, instead value from file-parser is used
				$Input_INCH = test_input($_POST["metric_inch"]);
				if ($Input_INCH == "metric") $Input_INCH = "0";
				elseif ($Input_INCH == "inch") $Input_INCH = "1";
				else {
					echo "Value of metric_inch out of range!";
					$success = 0;
				}
				$msg .= $msg_pid . " " . $Input_N_OFFSET . " " . $Input_N_MAX . " " . $Input_INCH . "\n";
				break;
			case "NewProgrammBlock": //maybe not used, instead Textarea is uploaded
				$msg_pid = "16";
				$parameter = array("block", "gm_code", "gm_code_no", "cnc_xi", "cnc_zk", "cnc_ftlk", "cnc_hs");
				$success &= test_keys_exist($parameter) or exit(1);
				$Input_N = test_input($_POST["block"]);
				$success &= test_value_range($Input_N, CNC_CODE_NMIN, CNC_CODE_NMAX); //block-numbers should be flattened
				$Input_GM = test_input($_POST["gm_code"]);
				$Input_GM_NO = test_input($_POST["gm_code_no"]);
				if ($Input_GM == "G") {
					//$success &= test_value_range($Input_GM_NO, 0, 196); //should be a check against an array of known codes!!!
					if (!in_array($Input_GM_NO, $G_Code_Numbers)) {
						$success = 0;
						echo "G-Code out of range!<br />";
					}
				}
				elseif ($Input_GM == "M") {
					//$success &= test_value_range($Input_GM_NO, 0, 99); //should be a check against an array of known codes!!!
					if (!in_array($Input_GM_NO, $M_Code_Numbers)) {
						$success = 0;
						echo "M-Code out of range!<br />";
					}
				}
				else {
					echo "Value of gm_code out of range!";
					$success = 0;
				}
				$Input_XI = test_input($_POST["cnc_xi"]);
				$success &= test_value_range($Input_XI, -X_MIN_MAX_CNC, X_MIN_MAX_CNC); //not right!!! Many cases!!!
				$Input_ZK = test_input($_POST["cnc_zk"]);
				$success &= test_value_range($Input_ZK, -Z_MIN_MAX_CNC, Z_MIN_MAX_CNC); //not right!!! Many cases!!!
				$Input_FTLK = test_input($_POST["cnc_ftlk"]);
				$success &= test_value_range($Input_FTLK, -CNC_CODE_FILE_PARSER_NMAX, CNC_CODE_FILE_PARSER_NMAX); //not right!!! Many cases!!!
				$Input_HS = test_input($_POST["cnc_hs"]);
				$success &= test_value_range($Input_HS, H_MIN, REVOLUTIONS_MAX); //not right!!! Many cases!!!
				$msg .= $msg_pid . " " . $Input_N . " " . $Input_GM . " " . $Input_GM_NO . " " . $Input_XI . " " . $Input_ZK . " " . $Input_FTLK . " " . $Input_HS . "\n";
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
				$success=0;
		}
	}
	
	if (!$success) exit(1);
	
	ignore_user_abort(1);

	//create client-pipe
	//$success = posix_mkfifo ("/home/pi/spi_com/client_session_pipe.".session_id(), int $mode ) or exit ("Could not create client-pipe!");
	
	//open server pipe writeonly (shorter timeout needed!!!)
	$server_pipe = fopen("/home/pi/spi_com/arduino_pipe.tx", "w") or exit("Unable to open server-pipe!");
	//stream_set_blocking($server_pipe, 0); // prevent fread / fwrite blocking
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
	//stream_set_blocking($fh, 0); // prevent fread / fwrite blocking
	
	//open client pipe writeonly
	//$answersize=87;
	//$client_pipe = fopen("/home/pi/spi_com/arduino_pipe.tx", "r") or exit("Unable to open client-pipe!");
	//fread($client_pipe, $answersize);
	//fclose($client_pipe);
?>
