$(document).ready(function(){

    // jQuery methods go here...
	
	//Polling Machine State
	var pollTimeout = 1000; //1000 = every second
	
	(function poll() {
		setTimeout(function() {
			$.ajax({
				url: 'xml/machine_state.xml',
				success: function(data, textStatus, jqXHR) {
					machine_state(jqXHR);
				}, dataType: "xml", complete: poll });
		}, pollTimeout);
	})();
	
	//Toggle Visibility of Manual and CNC Control
    $("#manbutton").click(function(){
	   $(".cnc").hide();
	   $(".emco").hide();
	   $(".manual").show();
	});
	
	$("#cncbutton").click(function(){
	   $(".manual").hide();
	   $(".emco").hide();
	   $(".cnc").show();
	});
	
	$("#emcobutton").click(function(){
	   $(".manual").hide();
	   $(".cnc").hide();
	   $(".emco").show();
	});

	/*
	$(function() {
    //hang on event of form with id=myform
		$("#tool").submit(function(e) {

			//prevent Default functionality
			e.preventDefault();

			//get the action-url of the form
			var actionurl = e.currentTarget.action;

			//do your own request and handle the results
			$.ajax({
					url: actionurl,
					type: 'post',
					dataType: 'json',
					data: $("#tool").serialize(),
					success: function(data) {
						//... do something with the data...
					}
			});

		});

	});
	*/
	
	/*
	var form = $("form");
	$(":submit",form).click(function(){
			if($(this).attr('name')) {
				$(form).append(
					$("<input type='hidden'>").attr( { 
						name: $(this).attr('name'), 
						value: $(this).attr('value') })
				);
			}
		});

	$(form).submit(function(){
	 console.log($(this).serializeArray());
	});
	*/
	
	/*
	$("form").submit(function(event) {
		// Das eigentliche Absenden verhindern
		event.preventDefault();
		
		// Das sendende Formular und die Metadaten bestimmen
		var form = $(this); // Dieser Zeiger $(this) oder $("form"), falls die ID form im HTML exisitiert, klappt übrigens auch ohne jQuery ;)
		var action = form.attr("action"), // attr() kann enweder den aktuellen Inhalt des gennanten Attributs auslesen, oder setzt ein neuen Wert, falls ein zweiter Parameter gegeben ist
			method = form.attr("method"),
			data   = form.serialize(); // baut die Daten zu einem String nach dem Muster vorname=max&nachname=Müller&alter=42 ... zusammen
			
		// Der eigentliche AJAX Aufruf
		$.ajax({
			url : action,
			type : method,
			data : data
		}).done(function (data) {
			// Bei Erfolg
			alert("Erfolgreich:" + data);
		}).fail(function() {
			// Bei Fehler
			alert("Fehler!");
		}).always(function() {
			// Immer
			alert("Beendet!");
		});
	});
	*/

}); 