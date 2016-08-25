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
  var rpm_measure=xmlDoc.getElementsByTagName("rpm_measure")[0].textContent;
  var x_actual=xmlDoc.getElementsByTagName("x_actual")[0].textContent;
  var z_actual=xmlDoc.getElementsByTagName("z_actual")[0].textContent;
  var f_actual=xmlDoc.getElementsByTagName("f_actual")[0].textContent;
  var t_actual=xmlDoc.getElementsByTagName("t_actual")[0].textContent;
  var errorno=xmlDoc.getElementsByTagName("errorno")[0].textContent;
  document.getElementById("rpm_measure").setAttribute("value", rpm_measure);
  document.getElementById("x_actual").setAttribute("value", x_actual);
  document.getElementById("z_actual").setAttribute("value", z_actual);
  document.getElementById("f_actual").setAttribute("value", f_actual);
  document.getElementById("t_actual").setAttribute("value", t_actual);
  document.getElementById("error_actual").setAttribute("value", errorno);
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