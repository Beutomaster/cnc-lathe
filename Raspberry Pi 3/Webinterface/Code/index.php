<?php
session_start();
if(!$_SESSION['logged_in'])
	header("Location: /login.html");
?>
<!-- Login muss noch auf https eingeschraenkt werden!!!-->

<!doctype html>

<!-- Development-Version 0.1 -->

<html manifest="cnc_lathe.appcache" lang="en-US" class="no-js">
<head>
    <title>CNC-Lathe-Control</title>
    <meta charset="UTF-8" />
    <meta name="description" content="Web-GUI of an new Arduino MEGA and Raspberry Pi 3 driven CNC-Control for an old EMCO Compact 5 CNC Lathe" />
    <meta name="keywords" content="CNC-Control, EMCO Compact 5 CNC, Lathe, Arduino MEGA, Raspberry Pi 3" />
    <meta name="author" content="Hannes Beuter" />
	<link rel="shortcut icon" href="images/favicon.ico" type="image/vnd.microsoft.icon" />
    <script src="/js/jquery-3.0.0.min.js"></script>
    <script src="/js/jquery_functions.js"></script>
	<script src="/js/style.js"></script>
	<script src="/js/ajax_com.js"></script>
	<link rel="stylesheet" href="/css/styles.css" />
    <!--[if lt IE 9]>
        <script src="https://cdnjs.cloudflare.com/ajax/libs/html5shiv/3.7.3/html5shiv.js"></script>
    <![endif]-->
		
	<!-- no-js class replacement to js, remove this if you use Modernizr -->
    <script>(function(e,t,n){var r=e.querySelectorAll("html")[0];r.className=r.className.replace(/(^|\s)no-js(\s|$)/,"$1js$2")})(document,window,0);</script>
</head>

<body onload="{loadDoc('/xml/cnc_code.xml', cnc_code_table);}">

<header class="clearfix">
    <h1>CNC-Lathe-Control</h1>
</header>

<nav class="clearfix">
	<ul class="top_nav">
		<li id="ManButton" class="clearfix">Manual Control</li>
		<li id="CncButton" class="clearfix">CNC Control</li>
		<li id="EmcoButton" class="emco clearfix">EMCO Control</li>
		<li id="HelpButton" class="clearfix">Help</li>
	</ul>
    <ul class="top_login">
		<li id="LogoutButton">Logout User: <?php echo $_SESSION['usr']; ?></li>
	</ul>
</nav>

<section class="clearfix">
	<noscript>
		<h1>In Ihrem Browser ist JavaScript deaktiviert.</h1>
		<p>
			Ohne JavaScript funktioniert diese Seite nicht.
			<br />
			Im SELFHTML-Wiki erfahren Sie, <a href="https://wiki.selfhtml.org/wiki/JavaScript/Tutorials/JavaScript_aktivieren"> wie Sie JavaScript in Ihrem Browser aktivieren können. </a>
		</p>
	</noscript>

    <article class="manual clearfix">
        <h2>Manual Control</h2>
		
		<div class="clearfix solo">
			<div class="left">
			<form action="/php/send_command.php" method="post">
				<fieldset>
					<legend>X-Origin-Offset:</legend>
					<label>X-Offset (+-5999):<br />
					<input type="number" name="xoffset" min="-5999" max="5999" value="0" autocomplete="off" required />
					</label>
					<br />
					<input type="submit" class="button" name="SetXOffset" value="Set X-Offset" />
				</fieldset>
			</form>
			</div>
			
			<div class="left">
			<form action="/php/send_command.php" method="post">
				<fieldset>
					<legend>Z-Origin-Offset:</legend>
					<label>Z-Offset (+-32760):<br />
					<input type="number" name="zoffset" min="-32760" max="32760" value="0" autocomplete="off" required />
					</label>
					<br />
					<input type="submit" class="button" name="SetZOffset" value="Set Z-Offset" />
				</fieldset>
			</form>
			</div>
			
			<div class="clearfix">
			<div class="right">
			<form action="/php/send_command.php" method="post"> <!--  some js needed -->
				<fieldset>
					<label><input type="radio" name="metric_inch" value="metric" checked="checked" />Metric</label>
					<br />
					<label><input type="radio" name="metric_inch" value="inch" />Inch</label>
				</fieldset>
			</form>
			</div>
			</div>
		</div>
		
		<div class="left">
		<form action="/php/send_command.php" method="post">
			<fieldset>
				<legend>Spindle:</legend>
				<label><input type="radio" name="spindle_direction" value="right" checked="checked" />Rotation right handed</label>
				<br />
				<label><input type="radio" name="spindle_direction" value="left" />Rotation left handed</label>
				<br />
				<label>RPM (460 to 3220):<br />
				<input type="number" name="rpm" min="460" max="3220" value="460" autocomplete="off" />
				</label>
				<br />
				<input type="submit" id="SpindleOn" class="button" name="SpindleSetRPM" value="Spindle ON" />
				<br />
				<input type="button" id="SpindleOff" class="button" name="SpindleOff" value="Spindle OFF" />
			</fieldset>
		</form>
		</div>
		
		<div class="left">
		<!-- <form action="php/set_feed.php" method="post"> -->
		<!-- for SetStepperFeed a js-function is needed to set a client-variable, because no message is send -->
		<form action="/php/send_command.php" method="post">
			<fieldset>
				<legend>Stepper:</legend>
				<label>Feed (2 to 499):<br />
				<input type="number" name="feed" min="2" max="499" value="50" autocomplete="off" />
				</label>
				<br />
				<input type="submit" id="StepperOn" class="button" name="StepperOn" value="Stepper ON" />
				<br />
				<input type="button" id="StepperOff" class="button" name="StepperOff" value="Stepper OFF" />
			</fieldset>
		</form>
		</div>
		
		<div class="left">
		<form id="tool" action="/php/send_command.php" method="post">
			<fieldset>
				<legend>Tool:</legend>
				<label>Tool (1 to 6):<br />
				<input type="number" name="tool" min="1" max="6" value="0" autocomplete="off" />
				</label>
				<br />
				<label>X-Correction (+-5999):<br />
				<input type="number" name="tool_x-correction" min="-5999" max="5999" value="0" autocomplete="off" />
				</label>
				<br />
				<label>Z-Correction (+-32760):<br />
				<input type="number" name="tool_z-correction" min="-32760" max="32760" value="0" autocomplete="off" />
				</label>
				<br />
				<input type="submit" id="SetTool" class="button" name="SetTool" value="Set Tool" />
			</fieldset>
		</form>
		</div>
		
		<form class="solo" id="controls">
			<fieldset>
				<legend>Manual Stepper Controls:</legend>
				<input type="button" onclick="alert('-X')" value="-X" />
				<br />
				<input type="button" onclick="alert('-Z')" value="-Z" />
				<input type="button" onclick="alert('+Z')" value="+Z" />
				<br />
				<input type="button" onclick="alert('+X')" value="+X" />
			</fieldset>
		</form>
    </article>
    
    <article class="cnc clearfix">
        <h2>CNC-Control</h2>
        
		<!--
        <form enctype="multipart/form-data" method="POST">
		-->
		<form>
            <fieldset>
				<legend>Select CAM-File:</legend>
				<input type="hidden" name="MAX_FILE_SIZE" value="500000" />
				<input type="file" name="file-1" id="file-1" class="inputfile inputfile-1" data-multiple-caption="{count} files selected" />
				<label for="file-1"><span></span> <strong><svg xmlns="http://www.w3.org/2000/svg" width="20" height="17" viewBox="0 0 20 17"><path d="M10 0l-5.2 4.9h3.3v5.1h3.8v-5.1h3.3l-5.2-4.9zm9.3 11.5l-3.2-2.1h-2l3.4 2.6h-3.5c-.1 0-.2.1-.2.1l-.8 2.3h-6l-.8-2.2c-.1-.1-.1-.2-.2-.2h-3.6l3.4-2.6h-2l-3.2 2.1c-.4.3-.7 1-.6 1.5l.6 3.1c.1.5.7.9 1.2.9h16.3c.6 0 1.1-.4 1.3-.9l.6-3.1c.1-.5-.2-1.2-.7-1.5z"/></svg> Upload File</strong></label>
				<input type="button" class="button" value="Upload Changes" />
				<input type="button" class="button" value="Reset Changes" />
				<input type="button" class="button" value="Save File" />
				<div id="responses"></div>
			</fieldset>
        </form>
		
		<form>
			<fieldset>
				<legend>Programm Control:</legend>
				<input type="button" id="ProgramStartPause" class="button" onclick="/php/send_command.php" name="ProgramStartPause" value="Start" /> <!-- Set to Stop with js at Programmstart -->
				<input type="button" id="ProgramStop" class="button" onclick="/php/send_command.php" name="ProgramStop" value="Stop" />
			</fieldset>
		</form>
		
        <table id="code"><tr><th>N</th><th>G/M</th><th>G/M-Code</th><th>X/I</th><th>Z/K</th><th>F/T/L/K</th><th>H/S</th></tr></table>
    </article>
	
	<article class="emco clearfix">
        <h2>EMCO-Control</h2>
		<!-- Stream Video of old Emco Control
        <iframe src="http://cnc-lathe:8081/?action=stream" height="640" width="480" frameborder="0"></iframe>
		-->
    </article>
	
	<article class="help clearfix">
        <h2>Help</h2>
		<iframe id="helpwiki" src="/dokuwiki/index.php"></iframe> 
    </article>
</section>

<aside>
	<h2>Machine State</h2>
	<div class="state clearfix">
				<div id="activeLED" class="led-grey"></div>
				<p>Control active</p>
	</div>
	<div class="state clearfix">
				<div id="initLED" class="led-grey"></div>
				<p>Init</p>
	</div>
	<div class="state clearfix">
				<div id="manualLED" class="led-grey"></div>
				<p>Manual</p>
	</div>
	<div class="state clearfix">
				<div id="pauseLED" class="led-grey"></div>
				<p>Pause</p>
	</div>
	<div class="state clearfix">
				<div id="inchLED" class="led-grey"></div>
				<p>inch</p>
	</div>
	<div class="state clearfix">
				<div id="SpindelOnLED" class="led-grey"></div>
				<p>Spindel on</p>
	</div>
	<div class="state clearfix">
				<div id="SpindelDirectionLED" class="led-grey"></div>
				<p>Spindel-Direction</p>
	</div>
	<div class="state clearfix">
				<div id="StepperOnLED" class="led-grey"></div>
				<p>Stepper on</p>
	</div>
	<div class="state clearfix">
				<p>RPM:</p>
				<div id="RPMDisplaybox" class="Displaybox"></div>	
	</div>
	<div class="state clearfix">
				<p>X:</p>
				<div id="XDisplaybox" class="Displaybox"></div>	
	</div>
	<div class="state clearfix">
				<p>Z:</p>
				<div id="ZDisplaybox" class="Displaybox"></div>	
	</div>
	<div class="state clearfix">
				<p>Feed:</p>
				<div id="FeedDisplaybox" class="Displaybox"></div>	
	</div>
	<div class="state clearfix">
				<p>H:</p>
				<div id="HDisplaybox" class="Displaybox"></div>	
	</div>
	<div class="state clearfix">
				<p>Tool:</p>
				<div id="ToolDisplaybox" class="Displaybox"></div>	
	</div>
	<div class="state clearfix">
				<div id="SpiErrorLED" class="led-grey"></div>
				<p>SPI-Error</p>
	</div>
	<div class="state clearfix">
				<div id="CNCErrorLED" class="led-grey"></div>
				<p>CNC-Code-Error</p>
	</div>
	<div class="state clearfix">
				<div id="SpindelErrorLED" class="led-grey"></div>
				<p>Spindel-Error</p>
	</div>
	
<!--
	<form>
			<label>Control active:<br />
			<input type="text" name="active" id="active" />
			</label>
			<br />
			<label>Init:<br />
			<input type="text" name="init" id="init" />
			</label>
			<br />
			<label>Manual:<br />
			<input type="text" name="manual" id="manual" />
			</label>
			<br />
			<label>Pause:<br />
			<input type="text" name="pause" id="pause" />
			</label>
			<br />
			<label>inch:<br />
			<input type="text" name="inch" id="inch" />
			</label>
			<br />
			<label>Spindel on:<br />
			<input type="text" name="spindel_on" id="spindel_on" />
			</label>
			<br />
			<label>Spindel-Direction:<br />
			<input type="text" name="init" id="spindel_direction" />
			</label>
			<br />
			<label>Stepper on:<br />
			<input type="text" name="stepper_on" id="stepper_on" />
			</label>
			<br /><br />
			<label>RPM:<br />
			<input type="text" name="rpm_measure" id="rpm_measure" />
			</label>
			<br />
			<label>X:<br />
			<input type="text" name="x_actual" id="x_actual" />
			</label>
			<br />
			<label>Z:<br />
			<input type="text" name="z_actual" id="z_actual" />
			</label>
			<br />
			<label>Feed:<br />
			<input type="text" name="f_actual" id="f_actual" />
			</label>
			<br />
			<label>H:<br />
			<input type="text" name="h_actual" id="h_actual" />
			</label>
			<br />
			<label>Tool:<br />
			<input type="text" name="t_actual" id="t_actual" />
			</label>
			<br /><br />
			<label>SPI-Error:<br />
			<input type="text" name="spi_error" id="spi_error" />
			</label>
			<br />
			<label>CNC-Code-Error:<br />
			<input type="text" name="cnc_code_error" id="cnc_code_error" />
			</label>
			<br />
			<label>Spindel-Error:<br />
			<input type="text" name="spindel_error" id="spindel_error" />
			</label>
			<br />
	</form>
-->
	
	<form>
	<input type="button" name="ResetErrors" id="ResetErrors" class="button" onclick="alert('Reset Errors')" value="Reset Errors" />
	</form>
	
	<form>
	<input type="button" name="shutdown" id="shutdown" class="button" onclick="alert('Shutdown Pi')" value="Shutdown Pi" />
	</form>
</aside>

<footer>
    <p>
        Semesterproject SS2016: CNC-Drehbank <br />
        by Hannes Beuter, Hannes Schuhmacher &amp; Niko Ramdorf <br />
        at the University of Applied Sciences Kiel, Germany
	</p>
	<p><a href="/impressum.html">Impressum</a></p>
	<p id="credits">Upload-Icon made by <a href="http://www.flaticon.com/authors/daniel-bruce" title="Daniel Bruce">Daniel Bruce</a> from <a href="http://www.flaticon.com" title="Flaticon">www.flaticon.com</a> is licensed by <a href="http://creativecommons.org/licenses/by/3.0/" title="Creative Commons BY 3.0" target="_blank">CC 3.0 BY</a></p>
</footer>

<script src="/js/jquery.custom-file-input.js"></script>

</body>
</html>