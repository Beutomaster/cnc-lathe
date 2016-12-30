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
	
	//Toggle Visibility of Manual-, CNC-, Emco-Control and Help
	$(".help").hide();
	
    $("#ManButton").click(function(){
	   $(".cnc").hide();
	   //$(".emco").hide();
	   $(".help").hide();
	   $(".manual").show();
	});
	
	$("#CncButton").click(function(){
	   $(".manual").hide();
	   //$(".emco").hide();
	   $(".help").hide();
	   $(".cnc").show();
	});
	
	/*
	$("#EmcoButton").click(function(){
	   $(".manual").hide();
	   $(".cnc").hide();
	   $(".help").hide();
	   $(".emco").show();
	});
	*/
	
	$("#HelpButton").click(function(){
		$(".manual").hide();
		$(".cnc").hide();
		$(".emco").hide();
		$(".help").show();
	});
	
	$("#LogoutButton").click(function(){
	   location.href = href="/php/logout.php";
	});
	
	//load last CNC-Code-File from Server
	$("#CncCodeTxt").load("/uploads/cnc_code.txt");
	
	//Reload CNC-Code-File from Server
	$("#ResetChanges").click(function(){
		$("#CncCodeTxt").load("/uploads/cnc_code.txt"); //does not work ... why???
		//$("#CncCode").reset();
		alert( "Reset does not work yet!" );
	}); 
	
	//Upload Changes to CNC-Code-File on Server
	$("#UploadChanges").click(function(){
		$.ajax({type:'POST', url: '/php/update_cam-file.php', data:$('#CncCode').serialize(), success: function(response) {
				//$('#ContactForm').find('.form_result').html(response);
				//$("#responses").html("Response: " +  JSON.stringify(data));
			}
		}).done(function (data) {
			// Bei Erfolg
			//alert("Erfolgreich:" + data);
			$("#responses").html("Response: " +  JSON.stringify(data));
		}).fail(function() {
			// Bei Fehler
			$("#responses").html("Request-Error: Upload failed!");
			//alert("Fehler!");
		}).always(function() {
			// Immer
			//$("#responses").html("Finished unexpected: " + JSON.stringify(data));
			//alert("Beendet!");
			//Load new CNC-Code-File from Server (for Security)
			$("#CncCodeTxt").load("/uploads/cnc_code.txt"); //does not work ... why???
		});
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
	
	// Wir registrieren einen EventHandler für unser Input-Element (#file-1)
	// wenn es sich ändert
	$('body').on('change', '#file-1', function() {
	   var data = new FormData(); // das ist unser Daten-Objekt ...
	   data.append('file-1', this.files[0]); // ... an die wir unsere Datei anhängen
	   $.ajax({
			url: '/php/upload_cam-file.php', // Wohin soll die Datei geschickt werden?
			data: data,          // Das ist unser Datenobjekt.
			type: 'POST',         // HTTP-Methode, hier: POST
			processData: false,
			//contentType : 'multipart/form-data',
			contentType: false,
			// und wenn alles erfolgreich verlaufen ist, schreibe eine Meldung
			// in das Response-Div
			//success: function() { $("#responses").html("File successfully uploaded!");}
			//success: function() { $("#responses").html("Success: " +  JSON.stringify(data));},
			//error: function( jqXhr, textStatus, errorThrown ){console.log( errorThrown );}
			/*
			success: function(response) {
				console.log(response);
			},
			error: function(errResponse) {
				console.log(errResponse);
			}
			*/
		}).done(function (data) {
			// Bei Erfolg
			//alert("Erfolgreich:" + data);
			$("#responses").html("Response: " +  JSON.stringify(data));
		}).fail(function() {
			// Bei Fehler
			$("#responses").html("Request-Error: Upload failed!");
			//alert("Fehler!");
		}).always(function() {
			// Immer
			//$("#responses").html("Finished unexpected: " + JSON.stringify(data));
			//alert("Beendet!");
			//Load new CNC-Code-File from Server (for Security)
			$("#CncCodeTxt").load("/uploads/cnc_code.txt");
		});
	})

}); 