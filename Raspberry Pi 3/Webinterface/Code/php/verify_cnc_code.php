<?php
	//Input Parameter Ranges
	define("CNC_CODE_NMIN", 0);
	define("CNC_CODE_NMAX", 9999); //Attention!!! Range, not supported in Arduino-Code yet (only up to 499)
	define("GM_CODE_MIN", 0);
	define("G_CODE_MAX", 196);
	define("M_CODE_MAX", 99);
	define("X_MIN_MAX_CNC", 5999);
	define("Z_MIN_MAX_CNC", 32760);
	define("XZ_MIN_MAX_HAND", 89999); //not supported in Arduino-Code yet
	define("X_DWELL_MIN_MAX_CNC", 5999);
	define("F_MIN", 2);
	define("F_MAX", 499);
	define("IK_MIN", 0);
	define("I_MAX", 5999);
	define("K_MAX", 5999);
	define("K_THREAD_PITCH_MAX", 499);
	define("L_MIN", 0);
	define("L_MAX", 499);
	define("T_MIN", 1);
	define("T_MAX", 6);
	define("H_MIN", 0);
	define("H_G86_MIN", 10);
	define("H_MAX", 999);
	define("REVOLUTIONS_MIN", 460); //rpm
	define("REVOLUTIONS_MAX", 3220); //rpm
	
	function test_input($data) {
		//information about changes needed!!!
		$data = trim($data);
		$data = stripslashes($data);
		$data = htmlspecialchars($data);
		return $data;
	}
	
	function test_keys_exist($keyArray) {
		$success=1;
		foreach($keyArray as $key) {
			if (!array_key_exists($key, $_POST)) {
				$success=0;
				echo ("Key " . $key . " is missing!<br />");
			}
		}
		unset($key);
		return $success;
	}
	
	function test_value_range($value, $min, $max) {
		$success=1;
		//test if value has only numbers and a negativ sign needed
		if (!is_numeric($value) || is_float($value)) {
			$success=0;
			echo "Value of " . $name . " is not numeric or float!<br />";
		}
		//test range of value matches
		if ($value < $min || $value > $max) {
			$success=0;
			echo "Value of " . $name . " out of range!<br />";
		}
		return $success;
	}
	
	function test_value_range_cnc_code($line, $name, $value, $min, $max) {
		$success = 1;
		
		//test if value has only numbers and a negativ sign needed
		if (!is_numeric($value) || is_float($value)) {
			$success = 0;
			echo "Line $line: $name not numeric or float<br />";
		}
		//test range of value matches
		if ($value < $min || $value > $max) {
			$success = 0;
			echo "Line $line: $name out of Range<br />";
		}
		return $success;
	}
	
	function get_next_cnc_code_parameter(&$RestParameterString, $line, $name, &$var, $optional, $min, $max) {
		$success = 1;
		//check String for Parameter
		if (preg_match('/^(['.$name.'])(-?[0-9]{1,5})[ ]?(.*$)/', $RestParameterString, $RestParameterStringArray)) {
			$var=$RestParameterStringArray[2];
			//Return Rest-String
			$RestParameterString = $RestParameterStringArray[3];
			//test range of value matches
			$success &= test_value_range_cnc_code($line, $name, $var, $min, $max);
		}
		else {
			$var=0; //set Default Value
			if (!$optional) {
				echo "Line $line: no $name-Parameter or incorrect format<br />";
				$success = 0;
			}
			else echo "Line $line: no optional $name-Parameter or incorrect format. $name = 0 is used<br />";
		}
		return $success;
	}
	
	function verify_cnc_code(&$cnc_code_reference, $send_cnc_code, $startblock) {
		$success = 1;
		//debug
		//var_dump($cnc_code_reference);
		
		//get lines
		$lines = count($cnc_code_reference);
		echo "File has $lines lines.<br />";
		
		//get CNC-Code-Start- and -Stop-Line
		$code_start_line ="";
		$code_stop_line = $lines+1; //if no Stop-Sign is found
		$i=0;
		foreach ($cnc_code_reference as $line => $linevalue) {
			if (preg_match('/^%/', $linevalue)) {
				if ($i == 0) {
					$code_start_line = $line+1;
					echo "Line $code_start_line: CNC-Code-Start<br />";
				}
				elseif ($i == 1) {
					$code_stop_line = $line+1;
					echo "Line $code_stop_line: CNC-Code-Stop<br />";
				}
				$i++;
			}
		}
		unset($line);
		
		//$code_start_line = array_search("%\r\n", $cnc_code_reference);
		if (!$code_start_line) {
			echo "CNC-Code-Error: No CNC-Code-Startline % found!<br />";
			$success = 0;
		}
		
		//Parse Code-Lines
		$N=0;
		$N_last=0;
		$M30=""; //should be an array
		for ($line = $code_start_line+1; $line < $code_stop_line; $line++) {
			//echo "Line $line: ". $cnc_code_reference[$line-1]."<br />";
			//check if every Code-line has a Blocknumber at the beginning and that they are increasing per line (N>N_last)
			if (preg_match('/^([N])([0-9]{4})[ ]?(.*$)/', $cnc_code_reference[$line-1], $code_line)) {
				$N = $code_line[2];
				if (test_value_range_cnc_code($line, "N", $N, 0, 9999)) { //CNC_CODE_NMIN, CNC_CODE_NMAX)) {
					if ($N<$N_last) {
						echo "Line $line: N not greater as a previous one<br />";
						$success = 0;
					}
					else {
						$N_last=$N;
					}
				}
				else {
					$success = 0;
				}
				if (preg_match('/^([GM])([0-9]{1,3})[ ]?(.*$)/', $code_line[3], $code)) {
					//Check if every Code-line has a correct G- or M-Code as next Parameter
					//Check if the following Parameters are matching and are in range
					//Check for extra characters
					$GM = $code[1];
					$GM_Code = $code[2];
					$Parameter = $code[3];
					$ret = 1;
					if ($GM == 'G') {
						switch ($GM_Code) {
							case 0:
									$ret &= get_next_cnc_code_parameter($Parameter, $line, "X", $X, 1, -X_MIN_MAX_CNC, X_MIN_MAX_CNC);
									$ret &= get_next_cnc_code_parameter($Parameter, $line, "Z", $Z, 1, -Z_MIN_MAX_CNC, Z_MIN_MAX_CNC);
									if (!ret) {
										echo "Line $line: no X- or Z-Parameter or incorrect format<br />";
										$success = 0;
									}
									break;
							case 1:
							case 2:
							case 3:
									$ret &= get_next_cnc_code_parameter($Parameter, $line, "X", $X, 1, -X_MIN_MAX_CNC, X_MIN_MAX_CNC);
									$ret &= get_next_cnc_code_parameter($Parameter, $line, "Z", $Z, 1, -Z_MIN_MAX_CNC, Z_MIN_MAX_CNC);
									if (!ret) {
										echo "Line $line: no X- or Z-Parameter or incorrect format<br />";
										$success = 0;
									}
									$success &= get_next_cnc_code_parameter($Parameter, $line, "F", $F, 0, F_MIN, F_MAX); //optional?
									break;
							case 4:
									$success &= get_next_cnc_code_parameter($Parameter, $line, "X", $X, 0, 0, X_DWELL_MIN_MAX_CNC); //Ranges correct?
									break;
							case 20:
							case 21:
									break;
							case 22: $M30="$line"; //last End of Programm for check of jump instructions
									break;
							case 24:
									break;
							case 25: //G25(L);
									//test if Block L exists needed
									$success &= get_next_cnc_code_parameter($Parameter, $line, "L", $L, 0, CNC_CODE_NMIN, CNC_CODE_NMAX);
									break;
							case 26: //G26(X,Z,T);
									$success &= get_next_cnc_code_parameter($Parameter, $line, "X", $X, 1, -X_MIN_MAX_CNC, X_MIN_MAX_CNC);
									$success &= get_next_cnc_code_parameter($Parameter, $line, "Z", $Z, 1, -Z_MIN_MAX_CNC, Z_MIN_MAX_CNC);
									$success &= get_next_cnc_code_parameter($Parameter, $line, "T", $T, 0, T_MIN, T_MAX);
									break;
							case 27: //G27(L);
									//test if Block L exists needed
									$success &= get_next_cnc_code_parameter($Parameter, $line, "L", $L, 0, CNC_CODE_NMIN, CNC_CODE_NMAX);
									break;
							case 33: //G33(Z,K);
									$success &= get_next_cnc_code_parameter($Parameter, $line, "Z", $Z, 0, -Z_MIN_MAX_CNC, Z_MIN_MAX_CNC); //optional?
									$success &= get_next_cnc_code_parameter($Parameter, $line, "K", $K, 0, IK_MIN, K_MAX); //Ranges correct?
									break;
							case 64:
									break;
							case 73: //G73(Z,F);
									$success &= get_next_cnc_code_parameter($Parameter, $line, "Z", $Z, 0, -Z_MIN_MAX_CNC, Z_MIN_MAX_CNC); //optional?
									$success &= get_next_cnc_code_parameter($Parameter, $line, "F", $F, 0, F_MIN, F_MAX); //optional?
									break;
							case 78: //G78(X,Z,K,H);
									$success &= get_next_cnc_code_parameter($Parameter, $line, "Z", $Z, 0, -Z_MIN_MAX_CNC, Z_MIN_MAX_CNC); //optional?
									break;
							case 81: //G81(Z,F);
							case 82: //G82(Z,F);
							case 83: //G83(Z,F);
									$success &= get_next_cnc_code_parameter($Parameter, $line, "Z", $Z, 0, -Z_MIN_MAX_CNC, Z_MIN_MAX_CNC); //optional?
									$success &= get_next_cnc_code_parameter($Parameter, $line, "F", $F, 0, F_MIN, F_MAX); //optional?
									//Ranges correct?
									break;
							case 84: //G84(X,Z,F,H);
									break;
							case 85: //G85(Z,F);
									$success &= get_next_cnc_code_parameter($Parameter, $line, "Z", $Z, 0, -Z_MIN_MAX_CNC, Z_MIN_MAX_CNC); //optional?
									$success &= get_next_cnc_code_parameter($Parameter, $line, "F", $F, 0, F_MIN, F_MAX); //optional?
									//Ranges correct?
									break;
							case 86: //G86(X,Z,F,H);
									$ret &= get_next_cnc_code_parameter($Parameter, $line, "X", $X, 1, -X_MIN_MAX_CNC, X_MIN_MAX_CNC); //optional?
									$ret &= get_next_cnc_code_parameter($Parameter, $line, "Z", $Z, 1, -Z_MIN_MAX_CNC, Z_MIN_MAX_CNC); //optional?
									if (!ret) {
										echo "Line $line: no X- or Z-Parameter or incorrect format<br />";
										$success = 0;
									}
									$success &= get_next_cnc_code_parameter($Parameter, $line, "F", $F, 0, F_MIN, F_MAX); //optional?
									$success &= get_next_cnc_code_parameter($Parameter, $line, "H", $H, 0, H_G86_MIN, H_MAX); //optional?
									//Ranges correct?
									break;
							case 88: //G88(X,Z,F,H);
									$ret &= get_next_cnc_code_parameter($Parameter, $line, "X", $X, 1, -X_MIN_MAX_CNC, X_MIN_MAX_CNC); //optional?
									$ret &= get_next_cnc_code_parameter($Parameter, $line, "Z", $Z, 1, -Z_MIN_MAX_CNC, Z_MIN_MAX_CNC); //optional?
									if (!ret) {
										echo "Line $line: no X- or Z-Parameter or incorrect format<br />";
										$success = 0;
									}
									$success &= get_next_cnc_code_parameter($Parameter, $line, "F", $F, 0, F_MIN, F_MAX); //optional?
									$success &= get_next_cnc_code_parameter($Parameter, $line, "H", $H, 0, H_MIN, H_MAX); //optional?
									//Ranges correct?
									break;
							case 89: //G89(Z,F);
									$success &= get_next_cnc_code_parameter($Parameter, $line, "Z", $Z, 0, -Z_MIN_MAX_CNC, Z_MIN_MAX_CNC); //optional?
									$success &= get_next_cnc_code_parameter($Parameter, $line, "F", $F, 0, F_MIN, F_MAX); //optional?
									//Ranges correct?
									break;
							case 90:
							case 91:
									break;
							case 92: //G92(X,Z);
									$ret &= get_next_cnc_code_parameter($Parameter, $line, "X", $X, 1, -X_MIN_MAX_CNC, X_MIN_MAX_CNC); //optional?
									$ret &= get_next_cnc_code_parameter($Parameter, $line, "Z", $Z, 1, -Z_MIN_MAX_CNC, Z_MIN_MAX_CNC); //optional?
									if (!ret) {
										echo "Line $line: no X- or Z-Parameter or incorrect format<br />";
										$success = 0;
									}
									break;
							case 94:
							case 95:
							case 96:
									break;
							case 97: //G97(S);
							case 196: //G196(S);
									$success &= get_next_cnc_code_parameter($Parameter, $line, "S", $S, 0, REVOLUTIONS_MIN, REVOLUTIONS_MAX);
									break;
							default:
									echo "Line $line: G-Code is not supported<br />";
									$success = 0;
						}
					}
					else {
						switch ($GM_Code) {
							case 0:
							case 3:
							case 4:
							case 5:
									break;
							case 6: //M06(X,Z,T);
									$success &= get_next_cnc_code_parameter($Parameter, $line, "X", $X, 1, -X_MIN_MAX_CNC, X_MIN_MAX_CNC);
									$success &= get_next_cnc_code_parameter($Parameter, $line, "Z", $Z, 1, -Z_MIN_MAX_CNC, Z_MIN_MAX_CNC);
									$success &= get_next_cnc_code_parameter($Parameter, $line, "T", $T, 0, T_MIN, T_MAX);
									break;
							case 17:
							case 30:
									$M30="$line"; //last End of Programm for check of jump instructions
									/*
									if ($M30=="") $M30="$line";
									else "Line $line: extra End of Programm<br />"; //maybe usefull with jump instructions
									*/
									break;
							case 98: //M98(X,Z);
									$ret &= get_next_cnc_code_parameter($Parameter, $line, "X", $X, 1, -X_MIN_MAX_CNC, X_MIN_MAX_CNC); //optional?
									$ret &= get_next_cnc_code_parameter($Parameter, $line, "Z", $Z, 1, -Z_MIN_MAX_CNC, Z_MIN_MAX_CNC); //optional?
									if (!ret) {
										echo "Line $line: no X- or Z-Parameter or incorrect format<br />";
										$success = 0;
									}
									break;
							case 99: //M99(I,K);
									$success &= get_next_cnc_code_parameter($Parameter, $line, "I", $I, 0, IK_MIN, I_MAX); //Ranges correct?, optional?
									$success &= get_next_cnc_code_parameter($Parameter, $line, "K", $K, 0, IK_MIN, K_MAX); //Ranges correct?, optional?
									break;
							default:
									echo "Line $line: M-Code is not supported<br />";
									$success = 0;
						}
					}
					if ($Parameter != "") echo "Line $line: extra characters<br />";
					//Check for metric-/inch-commands
					//Programm Stop (needed?)
					//Check if Jump-Instructions are ending (No jump back before a jump instruction, when there is no programm end between. No jump after the last programm end?)
				}
				else {
					echo "Line $line: no G- or M-Code or incorrect format<br />";
				}
			}
			else {
				echo "Line $line: no Blocknumber or incorrect format<br />";
			}
		}
		return $success;
	}
?>