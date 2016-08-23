function load_cnc_code_table() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (xhttp.readyState == 4 && xhttp.status == 200) {
    cnc_code_table(xhttp);
    }
  };
  xhttp.open("GET", "xml/cnc_code.xml", true);
  xhttp.send();
}

function cnc_code_table(xml) {
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