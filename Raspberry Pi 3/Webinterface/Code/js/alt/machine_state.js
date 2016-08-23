function load_machine_state() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (xhttp.readyState == 4 && xhttp.status == 200) {
    machine_state(xhttp);
    }
  };
  xhttp.open("GET", "xml/machine_state.xml", true);
  xhttp.send();
}

function machine_state(xml) {
  var i;
  var xmlDoc = xml.responseXML;
  var rpm_measure=xmlDoc.getElementsByTagName("rpm_measure")[0].textContent;
  var x_actual=xmlDoc.getElementsByTagName("x_actual")[0].textContent;
  var z_actual=xmlDoc.getElementsByTagName("z_actual")[0].textContent;
  var f_actual=xmlDoc.getElementsByTagName("f_actual")[0].textContent;
  var errorno=xmlDoc.getElementsByTagName("errorno")[0].textContent;
  document.getElementById("rpm_measure").setAttribute("value", rpm_measure);
  document.getElementById("x_actual").setAttribute("value", x_actual);
  document.getElementById("z_actual").setAttribute("value", z_actual);
  document.getElementById("f_actual").setAttribute("value", f_actual);
  document.getElementById("error_actual").setAttribute("value", errorno);
} 