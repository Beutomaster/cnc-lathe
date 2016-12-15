/*
URL: window.location.origin
*/

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
  var i;
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
} 

//Show CNC Code from cnc_code.xml
function cnc_code_table(xml) { //loadDoc('xml/cnc_code.xml', cnc_code_table);
  var i;
  var xmlDoc = xml.responseXML;
  var table="<tr><th>Block-Nr.</th><th>G/M-Code</th><th>X</th><th>Z</th><th>F</th><th>H</th></tr>";
  var x = xmlDoc.getElementsByTagName("block");
  for (i = 0; i <x.length; i++) {
    table += "<tr><td>" +
    x[i].getElementsByTagName("blockno")[0].childNodes[0].nodeValue +
    "</td><td>" +
	x[i].getElementsByTagName("gmcode")[0].childNodes[0].nodeValue +
    "</td><td>" +
	x[i].getElementsByTagName("xvalue")[0].childNodes[0].nodeValue +
    "</td><td>" +
	x[i].getElementsByTagName("zvalue")[0].childNodes[0].nodeValue +
    "</td><td>" +
	x[i].getElementsByTagName("fvalue")[0].childNodes[0].nodeValue +
    "</td><td>" +
    x[i].getElementsByTagName("hvalue")[0].childNodes[0].nodeValue +
    "</td></tr>";
  }
  document.getElementById("code").innerHTML = table;
} 