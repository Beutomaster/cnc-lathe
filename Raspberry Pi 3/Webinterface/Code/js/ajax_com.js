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
	var spindel_on=xmlDoc.getElementsByTagName("spindel_on")[0].textContent;
	var spindel_direction=xmlDoc.getElementsByTagName("spindel_direction")[0].textContent;
	var stepper_on=xmlDoc.getElementsByTagName("stepper_on")[0].textContent;
	var rpm_measure=xmlDoc.getElementsByTagName("rpm_measure")[0].textContent;
	var x_actual=xmlDoc.getElementsByTagName("x_actual")[0].textContent;
	var z_actual=xmlDoc.getElementsByTagName("z_actual")[0].textContent;
	var f_actual=xmlDoc.getElementsByTagName("f_actual")[0].textContent;
	var h_actual=xmlDoc.getElementsByTagName("h_actual")[0].textContent;
	var t_actual=xmlDoc.getElementsByTagName("t_actual")[0].textContent;
	var spi_error=xmlDoc.getElementsByTagName("spi_error")[0].textContent;
	var cnc_code_error=xmlDoc.getElementsByTagName("cnc_code_error")[0].textContent;
	var spindel_error=xmlDoc.getElementsByTagName("spindel_error")[0].textContent;

	/*
	document.getElementById("active").setAttribute("value", active);
	document.getElementById("init").setAttribute("value", init);
	document.getElementById("manual").setAttribute("value", manual);
	document.getElementById("pause").setAttribute("value", pause);
	document.getElementById("inch").setAttribute("value", inch);
	document.getElementById("spindel_on").setAttribute("value", spindel_on);
	document.getElementById("spindel_direction").setAttribute("value", spindel_direction);
	document.getElementById("stepper_on").setAttribute("value", stepper_on);
	document.getElementById("rpm_measure").setAttribute("value", rpm_measure);
	document.getElementById("x_actual").setAttribute("value", x_actual);
	document.getElementById("z_actual").setAttribute("value", z_actual);
	document.getElementById("f_actual").setAttribute("value", f_actual);
	document.getElementById("h_actual").setAttribute("value", h_actual);
	document.getElementById("t_actual").setAttribute("value", t_actual);
	document.getElementById("spi_error").setAttribute("value", spi_error);
	document.getElementById("cnc_code_error").setAttribute("value", cnc_code_error);
	document.getElementById("spindel_error").setAttribute("value", spindel_error);
	*/

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
		  if (spindel_on == 1) {
			  document.getElementById("SpindleOff").style.display='';
			  //document.getElementById("SpindleOff").disabled = false;
			  document.getElementById("SpindleOn").value = "Set RPM";
		  }
		  else {
			  document.getElementById("SpindleOff").style.display='none';
			  //document.getElementById("SpindleOff").disabled = true;
			  document.getElementById("SpindleOn").value = "Spindle ON";
		  }
		  if (stepper_on == 1) {
			  document.getElementById("StepperOff").style.display='';
			  //document.getElementById("StepperOff").disabled = false;
			  document.getElementById("StepperOn").value = "Set Feed";
		  }
		  else {
			  document.getElementById("StepperOff").style.display='none';
			  //document.getElementById("StepperOff").disabled = true;
			  document.getElementById("StepperOn").value = "Stepper ON";
		  }
		  if (init == 0) document.getElementById("SetTool").value = "Init Tool";
		  else document.getElementById("SetTool").value = "Set Tool";
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
		document.getElementById("initLED").className = "led-red";
	} 
	else {
		document.getElementById("initLED").className = "led-grey";
	}
	
	if (manual == 1) {
		document.getElementById("manualLED").className = "led-red";
	} 
	else {
		document.getElementById("manualLED").className = "led-grey";
	}
	
	if (pause == 1) {
		document.getElementById("pauseLED").className = "led-red";
	} 
	else {
		document.getElementById("pauseLED").className = "led-grey";
	}
	
	if (inch == 1) {
		document.getElementById("inchLED").className = "led-red";
	} 
	else {
		document.getElementById("inchLED").className = "led-grey";
	}
	
	if (spindel_on == 1) {
		document.getElementById("SpindelOnLED").className = "led-red";
	} 
	else {
		document.getElementById("SpindelOnLED").className = "led-grey";
	}
	
	if (spindel_direction == 1) {
		document.getElementById("SpindelDirectionLED").className = "led-red";
	} 
	else {
		document.getElementById("SpindelDirectionLED").className = "led-grey";
	}
	
	if (stepper_on == 1) {
		document.getElementById("StepperOnLED").className = "led-red";
	} 
	else {
		document.getElementById("StepperOnLED").className = "led-grey";
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
	
	if (spindel_error == 1) {
		document.getElementById("SpindelErrorLED").className = "led-red";
	} 
	else {
		document.getElementById("SpindelErrorLED").className = "led-grey";
	}
}

//send command
function sendCommand(str) {
    if (str.length == 0) {
        document.getElementById("debug").innerHTML = "";
        return;
    } else {
        var xmlhttp = new XMLHttpRequest();
        xmlhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
                document.getElementById("debug").innerHTML = this.responseText;
            }
        };
        xmlhttp.open("POST", "gethint.php?q=" + str, true); //has to be changed
        xmlhttp.send();
    }
}

function testekennwortqualitaet(inhalt)
{
    if (inhalt=="")
    {
        document.getElementById("sicherheitshinweise").innerHTML="keine Eingabe da";
        return;
    }
    if (window.XMLHttpRequest)
    {
        // AJAX nutzen mit IE7+, Chrome, Firefox, Safari, Opera
        xmlhttp=new XMLHttpRequest();
    }
    else
    {
        // AJAX mit IE6, IE5
        xmlhttp=new ActiveXObject("Microsoft.XMLHTTP");
    }
    xmlhttp.onreadystatechange=function()
    {
        if (xmlhttp.readyState==4 && xmlhttp.status==200)
        {
            document.getElementById("sicherheitshinweise").innerHTML=xmlhttp.responseText;
        }
    }
    xmlhttp.open("GET","kennworttesten.php?q="+inhalt,true);
    xmlhttp.send();
}

//Show CNC Code from cnc_code.xml
function cnc_code_table(xml) { //loadDoc('xml/cnc_code.xml', cnc_code_table);
  var i;
  var xmlDoc = xml.responseXML;
  var table="<tr><th>N</th><th>G/M</th><th>G/M-Code</th><th>X/I</th><th>Z/K</th><th>F/T/L/K</th><th>H/S</th></tr>";
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
  document.getElementById("code").innerHTML = table;
} 