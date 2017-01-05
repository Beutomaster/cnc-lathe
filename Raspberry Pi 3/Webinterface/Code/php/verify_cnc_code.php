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
		$i=0;
		foreach ($cnc_code_reference as $line => $linevalue) {
			if (preg_match('/^%/', $linevalue)) {
				if ($i == 0) {
					$code_start_line = $line;
					echo "CNC-Code-Start-Line: $code_start_line <br />";
				}
				elseif ($i == 1) {
					$code_stop_line = $line;
					echo "CNC-Code-Stop-Line: $code_stop_line <br />";
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
		//N>N_last, G/M-Codes with Parameter, Ranges
		//Programm Start/Stop ...
		
		return $success;
	}
?>