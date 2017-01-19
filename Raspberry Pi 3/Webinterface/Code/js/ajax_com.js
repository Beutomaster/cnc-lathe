/*
URL: window.location.origin
*/

var active_last=0;

//XMLHttpRequests
function loadDoc(url, cfunc) { //loadDoc('xml/machine_state.xml', machine_state)
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
        if (xhttp.readyState == 4 && xhttp.status == 200) {
            cfunc(xhttp);
        }
    };
xhttp.open("GET", url, true);
xhttp.send();
}

//Show Machine State from machine_state.xml
function machine_state(xml) { //loadDoc('xml/machine_state.xml', machine_state);
	var i, debug=0;
	var xmlDoc = xml.responseXML;
	var active=xmlDoc.getElementsByTagName("active")[0].textContent;
	var init=xmlDoc.getElementsByTagName("init")[0].textContent;
	var manual=xmlDoc.getElementsByTagName("manual")[0].textContent;
	var pause=xmlDoc.getElementsByTagName("pause")[0].textContent;
	var inch=xmlDoc.getElementsByTagName("inch")[0].textContent;
	var spindle_on=xmlDoc.getElementsByTagName("spindle_on")[0].textContent;
	var spindle_direction=xmlDoc.getElementsByTagName("spindle_direction")[0].textContent;
	var stepper_on=xmlDoc.getElementsByTagName("stepper_on")[0].textContent;
	var command_time=xmlDoc.getElementsByTagName("command_time")[0].textContent;
	var xstepper_running=xmlDoc.getElementsByTagName("xstepper_running")[0].textContent;
	var zstepper_running=xmlDoc.getElementsByTagName("zstepper_running")[0].textContent;
	var toolchanger_running=xmlDoc.getElementsByTagName("toolchanger_running")[0].textContent;
	var cnc_code_needed=xmlDoc.getElementsByTagName("cnc_code_needed")[0].textContent;
	var rpm_measure=xmlDoc.getElementsByTagName("rpm_measure")[0].textContent;
	var x_actual=xmlDoc.getElementsByTagName("x_actual")[0].textContent;
	var z_actual=xmlDoc.getElementsByTagName("z_actual")[0].textContent;
	var f_actual=xmlDoc.getElementsByTagName("f_actual")[0].textContent;
	var h_actual=xmlDoc.getElementsByTagName("h_actual")[0].textContent;
	var t_actual=xmlDoc.getElementsByTagName("t_actual")[0].textContent;
	var spi_error=xmlDoc.getElementsByTagName("spi_error")[0].textContent;
	var cnc_code_error=xmlDoc.getElementsByTagName("cnc_code_error")[0].textContent;
	var spindle_error=xmlDoc.getElementsByTagName("spindle_error")[0].textContent;
	var n_actual=xmlDoc.getElementsByTagName("n_actual")[0].textContent;
	var mtime=xmlDoc.getElementsByTagName("mtime")[0].textContent;
	/*
	document.getElementById("active").setAttribute("value", active);
	document.getElementById("init").setAttribute("value", init);
	document.getElementById("manual").setAttribute("value", manual);
	document.getElementById("pause").setAttribute("value", pause);
	document.getElementById("inch").setAttribute("value", inch);
	document.getElementById("spindle_on").setAttribute("value", spindle_on);
	document.getElementById("spindle_direction").setAttribute("value", spindle_direction);
	document.getElementById("stepper_on").setAttribute("value", stepper_on);
	document.getElementById("command_time").setAttribute("value", command_time);
	document.getElementById("xstepper_running").setAttribute("value", xstepper_running);
	document.getElementById("xstepper_running").setAttribute("value", zstepper_running);
	document.getElementById("toolchanger_running").setAttribute("value", toolchanger_running);
	document.getElementById("cnc_code_needed").setAttribute("value", cnc_code_needed);
	document.getElementById("rpm_measure").setAttribute("value", rpm_measure);
	document.getElementById("x_actual").setAttribute("value", x_actual);
	document.getElementById("z_actual").setAttribute("value", z_actual);
	document.getElementById("f_actual").setAttribute("value", f_actual);
	document.getElementById("h_actual").setAttribute("value", h_actual);
	document.getElementById("t_actual").setAttribute("value", t_actual);
	document.getElementById("spi_error").setAttribute("value", spi_error);
	document.getElementById("cnc_code_error").setAttribute("value", cnc_code_error);
	document.getElementById("spindle_error").setAttribute("value", spindle_error);
	document.getElementById("n_actual").setAttribute("value", n_actual);
	*/
	
	document.getElementById("NDisplaybox").innerHTML = n_actual;
	document.getElementById("RPMDisplaybox").innerHTML = rpm_measure;
	document.getElementById("XDisplaybox").innerHTML = x_actual;
	document.getElementById("ZDisplaybox").innerHTML = z_actual;
	document.getElementById("FeedDisplaybox").innerHTML = f_actual;
	document.getElementById("HDisplaybox").innerHTML = h_actual;
	document.getElementById("ToolDisplaybox").innerHTML = t_actual;

	if (active == 1) {
	  document.getElementById("activeLED").className = "led-red";
	  if (!debug) HideClass("emco");
	  ShowId("CncButton");
	  if (manual == 1) {
		  //if (!debug) HideClass("cnc"); //should be shown while pause
		  ShowId("ManButton");
		  if (active_last != active) {
			  if (!debug) HideClass("cnc");
			  ShowClass("manual");
		  }
	  }
	  else {
		  if (!pause && !debug) {
			  HideClass("manual");
			  HideId("ManButton"); //should not be shown while program is running
		  }
		  else {
			  ShowId("ManButton"); //should be shown while pause
		  }
		  if (active_last != active) {
			  if (!debug) HideClass("manual");
			  ShowClass("cnc");
		  }
	  }  
	}
	else {
	  document.getElementById("activeLED").className = "led-grey";
	  if (!debug) {
		  HideClass("manual");
		  HideClass("cnc");
		  HideId("ManButton");
		  HideId("CncButton");
	  }
	  ShowClass("emco");
	}

	active_last = active;
  
	if (init == 1) {
		document.getElementById("SetTool").value = "Set Tool";
		document.getElementById("initLED").className = "led-red";
		document.getElementById("LoadOldParameter").style.display='none';
	} 
	else {
		document.getElementById("SetTool").value = "Init Tool";
		document.getElementById("initLED").className = "led-grey";
		document.getElementById("LoadOldParameter").style.display='';
	}
	
	if (manual == 1) {
		document.getElementById("manualLED").className = "led-red";
	} 
	else {
		document.getElementById("manualLED").className = "led-grey";
	}
	
	if (pause == 1) {
		document.getElementById("pauseLED").className = "led-red";
		document.getElementById("ProgramPause").style.display='none';
		document.getElementById("ProgramStart").style.display='';
		//document.getElementById("ProgramStartPause").value = "Start";
	} 
	else {
		document.getElementById("pauseLED").className = "led-grey";
		document.getElementById("ProgramPause").style.display='';
		document.getElementById("ProgramStart").style.display='none';
		//document.getElementById("ProgramStartPause").value = "Pause";
	}
	
	if (inch == 1) {
		document.getElementById("inchLED").className = "led-red";
	} 
	else {
		document.getElementById("inchLED").className = "led-grey";
	}
	
	if (spindle_on == 1) {
		document.getElementById("SpindleOff").style.display='';
		document.getElementById("SpindleOffEMCO").style.display='';
		//document.getElementById("SpindleOff").disabled = false;
		document.getElementById("SpindleOn").value = "Set RPM";
		document.getElementById("SpindleOnEMCO").value = "Change Direction";
		document.getElementById("SpindleOnLED").className = "led-red";
	} 
	else {
		document.getElementById("SpindleOff").style.display='none';
		document.getElementById("SpindleOffEMCO").style.display='none';
		//document.getElementById("SpindleOff").disabled = true;
		document.getElementById("SpindleOn").value = "Spindle ON";
		document.getElementById("SpindleOnEMCO").value = "Spindle ON";
		document.getElementById("SpindleOnLED").className = "led-grey";
	}
	
	if (spindle_direction == 1) {
		document.getElementById("SpindleDirectionLED").className = "led-red";
	} 
	else {
		document.getElementById("SpindleDirectionLED").className = "led-grey";
	}
	
	if (stepper_on == 1) {
		document.getElementById("StepperOff").style.display='';
		//document.getElementById("StepperOff").disabled = false;
		document.getElementById("StepperOn").value = "Set Feed";
		document.getElementById("StepperOnLED").className = "led-red";
	} 
	else {
		document.getElementById("StepperOff").style.display='none';
		//document.getElementById("StepperOff").disabled = true;
		document.getElementById("StepperOn").value = "Stepper ON";
		document.getElementById("StepperOnLED").className = "led-grey";
	}
	
	if (command_time == 1) {
		document.getElementById("CommandTimeLED").className = "led-red";
	} 
	else {
		document.getElementById("CommandTimeLED").className = "led-grey";
	}
	
	if (xstepper_running == 1) {
		document.getElementById("XStepperMovingLED").className = "led-red";
	} 
	else {
		document.getElementById("XStepperMovingLED").className = "led-grey";
	}
	
	if (zstepper_running == 1) {
		document.getElementById("ZStepperMovingLED").className = "led-red";
	} 
	else {
		document.getElementById("ZStepperMovingLED").className = "led-grey";
	}
	
	if (toolchanger_running == 1) {
		document.getElementById("ToolchangerMovingLED").className = "led-red";
	} 
	else {
		document.getElementById("ToolchangerMovingLED").className = "led-grey";
	}
	
	if (cnc_code_needed == 1) {
		document.getElementById("CNCCodeNeededLED").className = "led-red";
	} 
	else {
		document.getElementById("CNCCodeNeededLED").className = "led-grey";
	}
	
	if (spi_error == 1) {
		document.getElementById("SpiErrorLED").className = "led-red";
	} 
	else {
		document.getElementById("SpiErrorLED").className = "led-grey";
	}
	
	if (cnc_code_error == 1) {
		document.getElementById("CNCErrorLED").className = "led-red";
	} 
	else {
		document.getElementById("CNCErrorLED").className = "led-grey";
	}
	
	if (spindle_error == 1) {
		document.getElementById("SpindleErrorLED").className = "led-red";
	} 
	else {
		document.getElementById("SpindleErrorLED").className = "led-grey";
	}
	
	if (mtime_WaitForUpdate == 0) {
		if (mtime_last == -1) mtime_last=mtime;
		if (mtime != mtime_last) {
			//cnc-code-file on server changed
			mtime_last=mtime;
			document.getElementById("responses").innerHTML = "CNC-Code-File on Server changed! You can reload it with Reset Changes";
			alert("CNC-Code-File on Server changed! You can reload it with Reset Changes");
		}
	}
	
	if (t_actual != t_last) {
		t_last = t_actual;
		document.getElementById("toolnumber").value = t_actual;
	}
	
	if (n_actual != n_last) {
		n_last = n_actual;
		document.getElementById("block").value = n_actual;
	}
}

//Show CNC Code from cnc_code.xml
function cnc_code_table(xml) { //loadDoc('xml/cnc_code.xml', cnc_code_table);
  var i;
  var xmlDoc = xml.responseXML;
  var table="<thead><tr><th>N</th><th>G/M</th><th>G/M-Code</th><th>X/I</th><th>Z/K</th><th>F/T/L/K</th><th>H/S</th></tr></thead><tbody>";
  var x = xmlDoc.getElementsByTagName("block");
  for (i = 0; i <x.length; i++) {
    table += "<tr><td>" +
    x[i].getElementsByTagName("NValue")[0].childNodes[0].nodeValue +
    "</td><td>" +
	x[i].getElementsByTagName("GMCode")[0].childNodes[0].nodeValue +
    "</td><td>" +
	x[i].getElementsByTagName("GMValue")[0].childNodes[0].nodeValue +
    "</td><td>" +
	x[i].getElementsByTagName("XIValue")[0].childNodes[0].nodeValue +
    "</td><td>" +
	x[i].getElementsByTagName("ZKValue")[0].childNodes[0].nodeValue +
    "</td><td>" +
	x[i].getElementsByTagName("FTLKValue")[0].childNodes[0].nodeValue +
    "</td><td>" +
    x[i].getElementsByTagName("HSValue")[0].childNodes[0].nodeValue +
    "</td></tr>";
  }
  table += "</tbody>";
  document.getElementById("code").innerHTML = table;
} 

//Save as File
function saveTextAsFile()
{
    var textToSave = document.getElementById("CncCodeTxt").value;
    var textToSaveAsBlob = new Blob([textToSave], {type:"text/plain"});
    var textToSaveAsURL = window.URL.createObjectURL(textToSaveAsBlob);
    var fileNameToSaveAs = "cnc_code.txt"; //document.getElementById("inputFileNameToSaveAs").value;
 
    var downloadLink = document.createElement("a");
    downloadLink.download = fileNameToSaveAs;
    downloadLink.innerHTML = "Download File";
    downloadLink.href = textToSaveAsURL;
	//downloadLink.target="_blank";
	downloadLink.setAttribute("target", "_blank");
    downloadLink.onclick = destroyClickedElement;
    downloadLink.style.display = "none";
    document.body.appendChild(downloadLink);
 
    downloadLink.click();
}

function destroyClickedElement(event)
{
    document.body.removeChild(event.target);
}