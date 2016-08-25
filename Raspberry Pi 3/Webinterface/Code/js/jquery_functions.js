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
	   $(".manual").show();
	});
	
	$("#cncbutton").click(function(){
	   $(".manual").hide();
	   $(".cnc").show();
	});

}); 