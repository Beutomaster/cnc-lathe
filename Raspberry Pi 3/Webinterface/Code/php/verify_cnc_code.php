<?php
	function test_input($data) {
		//information about changes needed!!!
		$data = trim($data);
		$data = stripslashes($data);
		$data = htmlspecialchars($data);
		return $data;
	}
	
	function verify_cnc_code(&$cnc_code_reference) {
		$success = true;
		//debug
		//var_dump($cnc_code_reference);
		
		//get lines
		$lines = count($cnc_code_reference);
		echo "File has $lines lines.<br />";
		
		//get CNC-Code-Start- and -Stop-Line
		$code_stop_line = $lines+1; //if no Stop-Sign is found
		$i=0;
		foreach ($cnc_code_reference as $line => $linevalue) {
			if (preg_match('/^%/', $linevalue)) {
				if ($i == 0) {
					$code_start_line = $line;
					echo "Line $line: CNC-Code-Start<br />";
				}
				elseif ($i == 1) {
					$code_stop_line = $line;
					echo "Line $line: CNC-Code-Stop<br />";
				}
				$i++;
			}
		}
		//$code_start_line = array_search("%\r\n", $cnc_code_reference);
		if (!$code_start_line) {
			echo "CNC-Code-Error: No CNC-Code-Startline % found!<br />";
			$success = false;
		}
		
		//Parse Code-Lines
		$N=0;
		$N_last=0;
		for ($line = $code_start_line+1; $line < $code_stop_line; $line++) {
			//check if every Code-line has a Blocknumber at the beginning and that they are increasing per line (N>N_last)
			if (preg_match('/^([N])([0-9]{4})[ ](.*$)/', $cnc_code_reference[$line], $code_line)) {
				$N = $code_line[2];
				if ($N<$N_last) {
					echo "Line $line: Blocknumber not greater<br />";
				}
				else {
					$N_last=$N;
				}
				if (preg_match('/^([GM])([0-9]{1,3})[ ]?(.*$)/', $code_line[3], $code)) {
					//Check if every Code-line has a correct G- or M-Code as next Parameter
					$GM = $code[1];
					$GM_Code = $code[2];
					$Parameter1 = $code[3];
					if ($GM == 'G') {
						switch ($GM_Code) {
							case 0:
									if (preg_match('/^([XZ])(-?[0-9]{1,5})[ ](.*$)/', $Parameter1, $Parameter2)) {
										if ($Parameter2[1] == "X") {
											if (preg_match('/^([Z])(-?[0-9]{1,5})[ ](.*$)/', $Parameter2[3], $Parameter3)) {
												
											}
											else {
												if ($Parameter2[3] != "") echo "Line $line: has extra characters<br />";
											}
										}
										else if ($Parameter2[1] != "") echo "Line $line: has extra characters<br />";
									}
									else {
										echo "Line $line: has no X- or Z-Parameter<br />";
										if ($Parameter1 != "") echo "Line $line: has extra characters<br />";
									}
									break;
							case 1:
							case 2:
							case 3:
									if (preg_match('/^([XZ])(-?[0-9]{1,5})[ ](.*$)/', $Parameter1, $Parameter2)) {
										if (preg_match('/^([ZF])(-?[0-9]{1,5})[ ](.*$)/', $Parameter2[3], $Parameter3)) {
											if ($Parameter3[1] == "Z") {
												if (preg_match('/^([F])(-?[0-9]{1,5})[ ](.*$)/', $Parameter3[3], $Parameter4)) {
													if ($Parameter4[3] != "") echo "Line $line: has extra characters<br />";
												}
												else {
													echo "Line $line: has no Feed-Parameter<br />";
													if ($Parameter3[3] != "") echo "Line $line: has extra characters<br />";
												}
											}
										}
										else {
											echo "Line $line: has no Feed-Parameter<br />";
											if ($Parameter2[3] != "") echo "Line $line: has extra characters<br />";
										}
									}
									else {
										echo "Line $line: has no X-,Z- or Feed-Parameter<br />";
										if ($Parameter1 != "") echo "Line $line: has extra characters<br />";
									}
									break;
							case 4:
									if (preg_match('/^([X])(-?[0-9]{1,5})[ ](.*$)/', $Parameter1[3], $Parameter2)) {
										if ($Parameter2[3] != "") echo "Line $line: has extra characters<br />";
									}
									else {
										echo "Line $line: has no X-Parameter<br />";
										if ($Parameter1 != "") echo "Line $line: has extra characters<br />";
									}
									break;
							case 20:
							case 21:
							case 22:
							case 24:
									if ($Parameter1 != "") echo "Line $line: has extra characters<br />";
									break;
							case 25: //G25(cnc_code[STATE_N].FTLK);
									break;
							case 26: //G26(cnc_code[STATE_N].XI, cnc_code[STATE_N].ZK, cnc_code[STATE_N].FTLK);
									break;
							case 27: //G27(cnc_code[STATE_N].FTLK);
									break;
							case 33: //G33(cnc_code[STATE_N].ZK, cnc_code[STATE_N].FTLK);
									break;
							case 64:
									if ($Parameter1 != "") echo "Line $line: has extra characters<br />";
									break;
							case 73: //G73(cnc_code[STATE_N].ZK, cnc_code[STATE_N].FTLK);
									break;
							case 78: //G78(cnc_code[STATE_N].XI, cnc_code[STATE_N].ZK, cnc_code[STATE_N].FTLK, cnc_code[STATE_N].HS);
									break;
							case 81: //G81(cnc_code[STATE_N].ZK, cnc_code[STATE_N].FTLK);
									break;
							case 82: //G82(cnc_code[STATE_N].ZK, cnc_code[STATE_N].FTLK);
									break;
							case 83: //G83(cnc_code[STATE_N].ZK, cnc_code[STATE_N].FTLK);
									break;
							case 84: //G84(cnc_code[STATE_N].XI, cnc_code[STATE_N].ZK, cnc_code[STATE_N].FTLK, cnc_code[STATE_N].HS);
									break;
							case 85: //G85(cnc_code[STATE_N].ZK, cnc_code[STATE_N].FTLK);
									break;
							case 86: //G86(cnc_code[STATE_N].XI, cnc_code[STATE_N].ZK, cnc_code[STATE_N].FTLK, cnc_code[STATE_N].HS);
									break;
							case 88: //G88(cnc_code[STATE_N].XI, cnc_code[STATE_N].ZK, cnc_code[STATE_N].FTLK, cnc_code[STATE_N].HS);
									break;
							case 89: //G89(cnc_code[STATE_N].ZK, cnc_code[STATE_N].FTLK);
									break;
							case 90:
							case 91:
									if ($Parameter1 != "") echo "Line $line: has extra characters<br />";
									break;
							case 92: //G92(cnc_code[STATE_N].XI, cnc_code[STATE_N].ZK);
									break;
							case 94:
							case 95:
							case 96:
									if ($Parameter1 != "") echo "Line $line: has extra characters<br />";
									break;
							case 97: //G97(cnc_code[STATE_N].HS);
									break;
							case 196: //G196(cnc_code[STATE_N].HS);
									break;
							default: echo "Line $line: G-Code is not supported<br />";
						}
					}
					else {
						switch ($GM_Code) {
							case 0:
							case 3:
							case 4:
							case 5:
									if ($Parameter1 != "") echo "Line $line: has extra characters<br />";
									break;
							case 6: //M06(cnc_code[STATE_N].XI, cnc_code[STATE_N].ZK, cnc_code[STATE_N].FTLK);
									break;
							case 17:
							case 30:
									if ($Parameter1 != "") echo "Line $line: has extra characters<br />";
									break;
							case 98: //M98(cnc_code[STATE_N].XI, cnc_code[STATE_N].ZK);
									break;
							case 99: //M99(cnc_code[STATE_N].XI, cnc_code[STATE_N].ZK);
									break;
							default: echo "Line $line: M-Code is not supported<br />";
						}
					}
					//Check for metric-/inch-commands
					//Check if the following Parameters are matching and are in range
					//Programm Start/Stop
					//Check if Jump-Instructions are ending
					//Check for extra characters
				}
				else {
					echo "Line $line: has no G- or M-Code<br />";
				}
			}
			else {
				echo "Line $line: has no Blocknumber<br />";
			}
		}
		return $success;
	}
?>