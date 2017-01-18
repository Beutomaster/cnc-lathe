var mtime_last=-1;
var mtime_WaitForUpdate=0;
var t_last=0;
var n_last=0;

$(document).ready(function(){

    // jQuery methods go here...
	
	$.ajaxSetup ({
		// Disable caching of AJAX responses
		cache: false
	});
	
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
	
	$("#HelpButton").click(function(){
		$(".manual").hide();
		$(".cnc").hide();
		$(".emco").hide();
		$(".help").show();
	});
	
	$("#LogoutButton").click(function(){
	   location.href = href="/php/logout.php";
	});
	
		
	function UpdateAndResizeTextarea() {
		$.ajax({
			url: "/uploads/cnc_code.txt",
			success: function(result){
				$("#CncCodeTxt").val(result);
				$("#responses").append("<br />Textarea Updated!");
				var txt = $("#CncCodeTxt").val();
				//console.log(txt);
				var lines = txt.split(/\r|\r\n|\n/);
				var count = lines.length;
				//console.log(count);
				document.getElementById("CncCodeTxt").rows=count;
			},
			error: function(xhr){
				//alert("An error occured: " + xhr.status + " " + xhr.statusText);
				$("#responses").append("<br />Error updating Textarea: " + xhr.status + ": " + xhr.statusText);
			}
		}).always(function() {
			mtime_WaitForUpdate=0;
		});
	}
	
	//load last CNC-Code-File from Server
	$("#responses").html("CNC-Code from previous uploaded File.");
	UpdateAndResizeTextarea();
	
	//Reload CNC-Code-File from Server
	$("#ResetChanges").click(function(){
		$("#responses").html("CNC-Code resetted to previous uploaded File.");
		UpdateAndResizeTextarea();
	}); 
	
	//Upload Changes to CNC-Code-File on Server
	$("#UploadChanges").click(function(){
		mtime_WaitForUpdate=1;
		$.ajax({
			type:'POST',
			url: '/php/update_cam-file.php',
			data:$('#CncCode').serialize(),
			success: function(response) {
				mtime_last=-1;
				mtime_WaitForUpdate=0;
				$("#responses").html("Response:<br />" +  JSON.stringify(response));
				//Load new CNC-Code-File from Server (for Security)
				//UpdateAndResizeTextarea(); //Error-Handling needed!!!
			},
			error: function(xhr){
				//alert("An error occured: " + xhr.status + " " + xhr.statusText);
				$("#responses").append("Request-Error: Upload failed!, " + xhr.status + ": " + xhr.statusText);
			}
		});
	}); 
	
	function sendCommand(data) {
		// AJAX-Call
		$.ajax({
			url : '/php/send_command.php',
			type : 'POST',
			data : data,
			success: function(response) {
				$("#manual_responses").html("Response:<br />" +  JSON.stringify(response));
			},
			error: function(xhr){
				//alert("An error occured: " + xhr.status + " " + xhr.statusText);
				$("#manual_responses").html("Request-Error: Submitting Command failed!<br />" + xhr.status + ": " + xhr.statusText);
			}
		});
	}
	
	$("#ProgramStart").click(function(){
		var StartBlock = document.getElementById("block").value;
		var data = {command: "ProgramStart", block: StartBlock, FileParserOverride: "0"};
		sendCommand(data);
	});
	
	$("#ProgramStop").click(function(){
		var data = {command: "ProgramStop"};
		sendCommand(data);
	});
	
	$("#ProgramPause").click(function(){
		var data = {command: "ProgramPause"};
		sendCommand(data);
	});
	
	$("#SpindleOff").click(function(){
		var data = {command: "SpindleOff"};
		sendCommand(data);
	});
	
	$("#StepperOn").click(function(){
		var data = {command: "StepperOn"};
		sendCommand(data);
	});
	
	$("#StepperOff").click(function(){
		var data = {command: "StepperOff"};
		sendCommand(data);
	});
	
	//Polling Stepper while Button is pressed
	var StepperTimeout = 400; //400 ms
	var StepperButtonPressed=false;

	function StepperPoll() {
		var StepperButtonPressed_local = StepperButtonPressed;
		setTimeout(function() {
			var feedvalue = document.getElementById("feed").value;
			var stepper_direction_local = "0";
			if (StepperButtonPressed_local == "XStepperNegativ" || StepperButtonPressed_local == "ZStepperNegativ") stepper_direction_local = "1";
			var stepper = "XStepper";
			if (StepperButtonPressed_local == "ZStepperPositiv" || StepperButtonPressed_local == "ZStepperNegativ") stepper = "ZStepper";
			// AJAX-Call
			$.ajax({
				url : '/php/send_command.php',
				type : 'POST',
				data : {command: stepper, stepper_direction: stepper_direction_local, feed: feedvalue},
				success: function(response) {
					$("#manual_responses").html("Response:<br />" +  JSON.stringify(response));
				},
				error: function(xhr){
					//alert("An error occured: " + xhr.status + " " + xhr.statusText);
					$("#manual_responses").html("Request-Error: Submitting Command failed!<br />" + xhr.status + ": " + xhr.statusText);
				},
				complete: function () {
					if (StepperButtonPressed != StepperButtonPressed_local) {
						clearTimeout(StepperPoll);
					} else {
						StepperPoll();
					}
				}
			});
		}, StepperTimeout);
	}
	
	$("#XStepperPositiv").on({
		mousedown: function(event) {
			/*
			console.log( "XStepperPositiv pressed" );
			console.log( "event object:" );
			console.dir( event );
			console.log( "event.target.id" );
			console.dir( event.target.id );
			*/
			StepperButtonPressed = "XStepperPositiv";
			StepperPoll();
		},
		"mouseleave mouseup": function(event) {
			/*
			console.log( "XStepperPositiv released" );
			console.log( "event object:" );
			console.dir( event );
			console.log( "event.target.id" );
			console.dir( event.target.id );
			*/
			StepperButtonPressed = false;
		}
	});
	
	$("#ZStepperPositiv").on({
		mousedown: function() {
			StepperButtonPressed = "ZStepperPositiv";
			StepperPoll();
		},
		"mouseleave mouseup": function() {
			StepperButtonPressed = false;
		}
	});
	
	$("#XStepperNegativ").on({
		mousedown: function() {
			StepperButtonPressed = "XStepperNegativ";
			StepperPoll();
		},
		"mouseleave mouseup": function() {
			StepperButtonPressed = false;
		}
	});
	
	$("#ZStepperNegativ").on({
		mousedown: function() {
			StepperButtonPressed = "ZStepperNegativ";
			StepperPoll();
		},
		"mouseleave mouseup": function() {
			StepperButtonPressed = false;
		}
	});
	
	/*
	$("#XStepperPositiv").click(function(){ //needs to be sent every 400ms, while button is pressed
		var feedvalue = document.getElementById("feed").value;
		var data = {command: "XStepper", stepper_direction: "0", feed: feedvalue};
		sendCommand(data);
	});
	
	$("#ZStepperPositiv").click(function(){ //needs to be sent every 400ms, while button is pressed
		var feedvalue = document.getElementById("feed").value;
		var data = {command: "ZStepper", stepper_direction: "0", feed: feedvalue};
		sendCommand(data);
	});
	
	$("#XStepperNegativ").click(function(){ //needs to be sent every 400ms, while button is pressed
		var feedvalue = document.getElementById("feed").value;
		var data = {command: "XStepper", stepper_direction: "1", feed: feedvalue};
		sendCommand(data);
	});
	
	$("#ZStepperNegativ").click(function(){ //needs to be sent every 400ms, while button is pressed
		var feedvalue = document.getElementById("feed").value;
		var data = {command: "ZStepper", stepper_direction: "1", feed: feedvalue};
		sendCommand(data);
	});
	*/
	
	$("#ResetErrors").click(function(){
		var error_reset_mask=0;
		if (document.getElementById("SpiError").checked) error_reset_mask += Number($("#SpiError").val());
		if (document.getElementById("CNCError").checked) error_reset_mask += Number($("#CNCError").val());
		if (document.getElementById("SpindleError").checked) error_reset_mask += Number($("#SpindleError").val());
		var data = {command: "ResetErrors", error_reset_mask: error_reset_mask};
		sendCommand(data);
	});
	
	$("#LoadOldParameter").click(function(){
		var data = {command: "LoadOldParameter"};
		sendCommand(data);
	});
	
	$("#shutdown").click(function(){
		var data = {command: "Shutdown"};
		sendCommand(data);
		alert('Saving parameter and shutting down Pi!');
	});
	
	//$('body').on('change', '#metric_inch', function(){
	$( "input[name='metric_inch']" ).change(function() {
		var scale = $("input[name='metric_inch']:checked").val();
		var data = {command: "SetMetricOrInch", metric_inch: scale};
		sendCommand(data);
	});
	
	//Ajax Form Submit
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
			data : data,
			success: function(response) {
				$("#manual_responses").html("Response:<br />" +  JSON.stringify(response));
			},
			error: function(xhr){
				//alert("An error occured: " + xhr.status + " " + xhr.statusText);
				$("#manual_responses").html("Request-Error: Submitting Command failed!<br />" + xhr.status + ": " + xhr.statusText);
			}
		/*
		}).done(function (data) {
			// Bei Erfolg
			alert("Erfolgreich:" + data);
		}).fail(function() {
			// Bei Fehler
			alert("Fehler!");
		}).always(function() {
			// Immer
			alert("Beendet!");
		*/
		});
	});
	
	/*
	//Ajax Form Submit with name and value of Submit-Button (does not work)
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
	
	// Wir registrieren einen EventHandler für unser Input-Element (#file-1)
	// wenn es sich ändert
	$('body').on('change', '#file-1', function() {
		mtime_WaitForUpdate=1;
		var data = new FormData(); // das ist unser Daten-Objekt ...
		data.append('file-1', this.files[0]); // ... an die wir unsere Datei anhängen
		$.ajax({
			url: '/php/upload_cam-file.php', // Wohin soll die Datei geschickt werden?
			data: data,          // Das ist unser Datenobjekt.
			type: 'POST',         // HTTP-Methode, hier: POST
			processData: false,
			//contentType : 'multipart/form-data',
			contentType: false,
			success: function(response) {
				mtime_last=-1;
				$("#responses").html("Response:<br />" +  JSON.stringify(response));
			},
			error: function(xhr){
				//alert("An error occured: " + xhr.status + " " + xhr.statusText);
				$("#responses").append("Request-Error: Upload failed!, " + xhr.status + ": " + xhr.statusText);
			}
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
			/*
		}).done(function (data) {
			// Bei Erfolg
			//alert("Erfolgreich:" + data);
			$("#responses").html("Response: " +  JSON.stringify(data));
		}).fail(function() {
			// Bei Fehler
			$("#responses").html("Request-Error: Upload failed!");
			//alert("Fehler!");
			*/
		}).always(function() {
			// Immer
			//$("#responses").html("Finished unexpected: " + JSON.stringify(data));
			//alert("Beendet!");
			//Load new CNC-Code-File from Server (for Security)
			//$("#CncCodeTxt").load("/uploads/cnc_code.txt");
			UpdateAndResizeTextarea();
		});
	})
}); 