<?php
	function test_input($data) {
		//information about changes needed!!!
		$data = trim($data);
		$data = stripslashes($data);
		$data = htmlspecialchars($data);
		return $data;
	}
	
	function test_value_range_cnc_code($line, $name, $value, $min, $max) {
		$success=true;
		//test if value has only numbers and a negativ sign needed
		if (!is_numeric($value) || is_float($value)) {
			$success=false;
			echo "Line $line: $name not numeric or float<br />";
		}
		//test range of value matches
		if ($value < $min || $value > $max) {
			$success=false;
			echo "Line $line: $name out of Range<br />";
		}
		return $success;
	}
	
	function verify_cnc_code(&$cnc_code_reference, $send_cnc_code, $startblock) {
		$success = true;
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
			$success = false;
		}
		
		//Parse Code-Lines
		$N=0;
		$N_last=0;
		$M30=""; //should be an array
		for ($line = $code_start_line+1; $line < $code_stop_line; $line++) {
			//check if every Code-line has a Blocknumber at the beginning and that they are increasing per line (N>N_last)
			if (preg_match('/^([N])([0-9]{4})[ ]?(.*$)/', $cnc_code_reference[$line], $code_line)) {
				$N = $code_line[2];
				if (test_value_range_cnc_code($line, "N", $N, 0, 9999)) {
					if ($N<$N_last) {
						echo "Line $line: N not greater as a previous one<br />";
						$success = false;
					}
					else {
						$N_last=$N;
					}
				}
				else {
					$success = false;
				}
				if (preg_match('/^([GM])([0-9]{1,3})[ ]?(.*$)/', $code_line[3], $code)) {
					//Check if every Code-line has a correct G- or M-Code as next Parameter
					$GM = $code[1];
					$GM_Code = $code[2];
					$Parameter1 = $code[3];
					if ($GM == 'G') {
						switch ($GM_Code) {
							case 0:
									if (preg_match('/^([XZ])(-?[0-9]{1,5})[ ]?(.*$)/', $Parameter1, $Parameter2)) {
										if ($Parameter2[1] == "X") {
											$X = $Parameter2[2];
											$success &= test_value_range_cnc_code($line, "X", $X, -5999, 5999);
											if (preg_match('/^([Z])(-?[0-9]{1,5})(.*$)/', $Parameter2[3], $Parameter3)) {
												$Z = $Parameter3[2];
												$success &= test_value_range_cnc_code($line, "Z", $Z, -32760, 32760);
											}
											else {
												$Z=0;
												if ($Parameter2[3] != "") echo "Line $line: extra characters<br />";
											}
										}
										else {
											$Z = $Parameter2[2];
											$success &= test_value_range_cnc_code($line, "Z", $Z, -32760, 32760);
											$X=0;
										}
									}
									else {
										echo "Line $line: no X- or Z-Parameter or incorrect format<br />";
										$success = false;
										if ($Parameter1 != "") echo "Line $line: extra characters<br />";
									}
									break;
							case 1:
							case 2:
							case 3:
									if (preg_match('/^([XZ])(-?[0-9]{1,5})[ ]?(.*$)/', $Parameter1, $Parameter2)) {
										if ($Parameter2[1] == "X") {
											$X = $Parameter2[2];
											$success &= test_value_range_cnc_code($line, "X", $X, -5999, 5999);
											if (preg_match('/^([ZF])(-?[0-9]{1,5})[ ]?(.*$)/', $Parameter2[3], $Parameter3)) {
												if ($Parameter3[1] == "Z") {
													$Z = $Parameter3[2];
													$success &= test_value_range_cnc_code($line, "Z", $Z, -32760, 32760);
													if (preg_match('/^([F])(-?[0-9]{1,5})(.*$)/', $Parameter3[3], $Parameter4)) {
														$F = $Parameter4[2];
														$success &= test_value_range_cnc_code($line, "F", $F, 2, 499);
														if ($Parameter4[3] != "") echo "Line $line: extra characters<br />";
													}
													else {
														echo "Line $line: no Feed-Parameter or incorrect format<br />";
														$success = false;
														if ($Parameter3[3] != "") echo "Line $line: extra characters<br />";
													}
												}
												else {
													$F = $Parameter3[2];
													$success &= test_value_range_cnc_code($line, "F", $F, 2, 499);
													$Z=0;
												}
											}
											else {
												echo "Line $line: no Feed-Parameter or incorrect format<br />";
												$success = false;
												if ($Parameter2[3] != "") echo "Line $line: extra characters<br />";
											}
										}
										else {
											$Z = $Parameter2[2];
											$success &= test_value_range_cnc_code($line, "Z", $Z, -32760, 32760);
											if (preg_match('/^([F])(-?[0-9]{1,5})[ ]?(.*$)/', $Parameter2[3], $Parameter3)) {
												$F = $Parameter3[2];
												$success &= test_value_range_cnc_code($line, "F", $F, 2, 499);
												if ($Parameter3[3] != "") echo "Line $line: extra characters<br />";
												$X=0;
											}
											else {
												echo "Line $line: no Feed-Parameter or incorrect format<br />";
												$success = false;
												if ($Parameter2[3] != "") echo "Line $line: extra characters<br />";
											}
										}
									}
									else {
										echo "Line $line: no X-,Z- or Feed-Parameter or incorrect format<br />";
										$success = false;
										if ($Parameter1 != "") echo "Line $line: extra characters<br />";
									}
									break;
							case 4:
									if (preg_match('/^([X])(-?[0-9]{1,5})(.*$)/', $Parameter1, $Parameter2)) {
										$X = $Parameter2[2];
										$success &= test_value_range_cnc_code($line, "X", $X, 0, 5999); //Dwell Limit?
										if ($Parameter2[3] != "") echo "Line $line: extra characters<br />";
									}
									else {
										echo "Line $line: no X-Parameter<br />";
										$success = false;
										if ($Parameter1 != "") echo "Line $line: extra characters<br />";
									}
									break;
							case 20:
							case 21:
									if ($Parameter1 != "") echo "Line $line: extra characters<br />";
									break;
							case 22: $M30="$line"; //last End of Programm for check of jump instructions
									if ($Parameter1 != "") echo "Line $line: extra characters<br />";
									break;
							case 24:
									if ($Parameter1 != "") echo "Line $line: extra characters<br />";
									break;
							case 25: //G25(L);
									//test if L exists needed
									if (preg_match('/^([L])(-?[0-9]{4})(.*$)/', $Parameter1, $Parameter2)) {
										$L = $Parameter2[2];
										$success &= test_value_range_cnc_code($line, "L", $L, 0, 9999);
										if ($Parameter2[3] != "") echo "Line $line: extra characters<br />";
									}
									else {
										echo "Line $line: no L-Parameter or incorrect format<br />";
										$success = false;
										if ($Parameter1 != "") echo "Line $line: extra characters<br />";
									}
									break;
							case 26: //G26(X,Z,T);
									if (preg_match('/^([XZ])(-?[0-9]{1,5})[ ]?(.*$)/', $Parameter1, $Parameter2)) {
										if ($Parameter2[1] == "X") {
											$X = $Parameter2[2];
											$success &= test_value_range_cnc_code($line, "X", $X, -5999, 5999);
											if (preg_match('/^([ZT])(-?[0-9]{1,5})[ ]?(.*$)/', $Parameter2[3], $Parameter3)) {
												if ($Parameter3[1] == "Z") {
													$Z = $Parameter3[2];
													$success &= test_value_range_cnc_code($line, "Z", $Z, -32760, 32760);
													if (preg_match('/^([T])([0-9]{1,5})(.*$)/', $Parameter3[3], $Parameter4)) {
														$T = $Parameter4[2];
														$success &= test_value_range_cnc_code($line, "T", $T, 1, 6);
														if ($Parameter4[3] != "") echo "Line $line: extra characters<br />";
													}
													else {
														echo "Line $line: no T-Parameter or incorrect format<br />";
														$success = false;
														if ($Parameter3[3] != "") echo "Line $line: extra characters<br />";
													}
												}
												else {
													$T = $Parameter3[2];
													$success &= test_value_range_cnc_code($line, "T", $T, 1, 6);
													$Z=0;
												}
											}
											else {
												echo "Line $line: no T-Parameter or incorrect format<br />";
												$success = false;
												if ($Parameter2[3] != "") echo "Line $line: extra characters<br />";
											}
										}
										else {
											$Z = $Parameter2[2];
											$success &= test_value_range_cnc_code($line, "Z", $Z, -32760, 32760);
											$X=0;
										}
									}
									elseif (preg_match('/^([T])([0-9]{1,5})(.*$)/', $Parameter1, $Parameter2)) {
										$T = $Parameter2[2];
										$success &= test_value_range_cnc_code($line, "T", $T, 1, 6);
										if ($Parameter2[3] != "") echo "Line $line: extra characters<br />";
										$X=0;
										$Z=0;
									}
									else {
										echo "Line $line: no T-Parameter or incorrect format<br />";
										$success = false;
										if ($Parameter1 != "") echo "Line $line: extra characters<br />";
									}
									break;
							case 27: //G27(L);
									//test if L exists needed
									if (preg_match('/^([L])(-?[0-9]{4})(.*$)/', $Parameter1, $Parameter2)) {
										$L = $Parameter2[2];
										$success &= test_value_range_cnc_code($line, "L", $L, 0, 9999);
										if ($Parameter2[3] != "") echo "Line $line: extra characters<br />";
									}
									else {
										echo "Line $line: no L-Parameter or incorrect format<br />";
										$success = false;
										if ($Parameter1 != "") echo "Line $line: extra characters<br />";
									}
									break;
							case 33: //G33(Z,K);
									break;
							case 64:
									if ($Parameter1 != "") echo "Line $line: extra characters<br />";
									break;
							case 73: //G73(Z,F);
									break;
							case 78: //G78(X,Z,K,H);
									break;
							case 81: //G81(Z,F);
							case 82: //G82(Z,F);
							case 83: //G83(Z,F);
									break;
							case 84: //G84(X,Z,F,H);
									break;
							case 85: //G85(Z,F);
									break;
							case 86: //G86(X,Z,F,H);
									break;
							case 88: //G88(X,Z,F,H);
									break;
							case 89: //G89(Z,F);
									break;
							case 90:
							case 91:
									if ($Parameter1 != "") echo "Line $line: extra characters<br />";
									break;
							case 92: //G92(X,Z);
									break;
							case 94:
							case 95:
							case 96:
									if ($Parameter1 != "") echo "Line $line: extra characters<br />";
									break;
							case 97: //G97(S);
							case 196: //G196(S);
									if (preg_match('/^([S])([0-9]{1,5})(.*$)/', $Parameter1, $Parameter2)) {
										$S = $Parameter2[2];
										$success &= test_value_range_cnc_code($line, "S", $S, 460, 3220);
										if ($Parameter2[3] != "") echo "Line $line: extra characters<br />";
									}
									else {
										echo "Line $line: no S-Parameter or incorrect format<br />";
										$success = false;
										if ($Parameter1 != "") echo "Line $line: extra characters<br />";
									}
									break;
							default:
									echo "Line $line: G-Code is not supported<br />";
									$success = false;
						}
					}
					else {
						switch ($GM_Code) {
							case 0:
							case 3:
							case 4:
							case 5:
									if ($Parameter1 != "") echo "Line $line: extra characters<br />";
									break;
							case 6: //M06(X,Z,T);
									if (preg_match('/^([XZ])(-?[0-9]{1,5})[ ]?(.*$)/', $Parameter1, $Parameter2)) {
										if ($Parameter2[1] == "X") {
											$X = $Parameter2[2];
											$success &= test_value_range_cnc_code($line, "X", $X, -5999, 5999);
											if (preg_match('/^([ZT])(-?[0-9]{1,5})[ ]?(.*$)/', $Parameter2[3], $Parameter3)) {
												if ($Parameter3[1] == "Z") {
													$Z = $Parameter3[2];
													$success &= test_value_range_cnc_code($line, "Z", $Z, -32760, 32760);
													if (preg_match('/^([T])([0-9]{1,5})(.*$)/', $Parameter3[3], $Parameter4)) {
														$T = $Parameter4[2];
														$success &= test_value_range_cnc_code($line, "T", $T, 1, 6);
														if ($Parameter4[3] != "") echo "Line $line: extra characters<br />";
													}
													else {
														echo "Line $line: no T-Parameter or incorrect format<br />";
														$success = false;
														if ($Parameter3[3] != "") echo "Line $line: extra characters<br />";
													}
												}
												else {
													$T = $Parameter3[2];
													$success &= test_value_range_cnc_code($line, "T", $T, 1, 6);
													$Z=0;
												}
											}
											else {
												echo "Line $line: no T-Parameter or incorrect format<br />";
												$success = false;
												if ($Parameter2[3] != "") echo "Line $line: extra characters<br />";
											}
										}
										else {
											$Z = $Parameter2[2];
											$success &= test_value_range_cnc_code($line, "Z", $Z, -32760, 32760);
											$X=0;
										}
									}
									elseif (preg_match('/^([T])([0-9]{1,5})(.*$)/', $Parameter1, $Parameter2)) {
										$T = $Parameter2[2];
										$success &= test_value_range_cnc_code($line, "T", $T, 1, 6);
										if ($Parameter2[3] != "") echo "Line $line: extra characters<br />";
										$X=0;
										$Z=0;
									}
									else {
										echo "Line $line: no T-Parameter or incorrect format<br />";
										$success = false;
										if ($Parameter1 != "") echo "Line $line: extra characters<br />";
									}
									break;
							case 17:
							case 30:
									$M30="$line"; //last End of Programm for check of jump instructions
									/*
									if ($M30=="") $M30="$line";
									else "Line $line: extra End of Programm<br />"; //maybe usefull with jump instructions
									*/
									if ($Parameter1 != "") echo "Line $line: extra characters<br />";
									break;
							case 98: //M98(X,Z);
									break;
							case 99: //M99(I,K);
									break;
							default:
									echo "Line $line: M-Code is not supported<br />";
									$success = false;
						}
					}
					//Check for metric-/inch-commands
					//Check if the following Parameters are matching and are in range
					//Programm Stop (needed?)
					//Check if Jump-Instructions are ending (No jump back before a jump instruction, when there is no programm end between. No jump after the last programm end?)
					//Check for extra characters
				}
				else {
					echo "Line $line: no G- or M-Code or incorrect format<br />";
				}
			}
			else {
				echo "Line $line: no Blocknumber or incorrect format<br />";
			}
		}
		echo "success: $success";
		return $success;
	}
?>