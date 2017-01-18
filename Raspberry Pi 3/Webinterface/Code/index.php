<?php
session_start();
if(!$_SESSION['logged_in'])
	header("Location: /login.html");
?>
<!-- Login muss noch auf https eingeschraenkt werden!!!-->

<!doctype html>

<!-- Development-Version 0.2 -->

<html manifest="cnc_lathe.appcache" lang="en-US" class="no-js">
<head>
    <title>CNC-Lathe-Control</title>
    <meta charset="UTF-8" />
    <meta name="description" content="Web-GUI of an new Arduino MEGA and Raspberry Pi 3 driven CNC-Control for an old EMCO Compact 5 CNC Lathe" />
    <meta name="keywords" content="CNC-Control, EMCO Compact 5 CNC, Lathe, Arduino MEGA, Raspberry Pi 3" />
    <meta name="author" content="Hannes Beuter" />
	<link rel="shortcut icon" href="images/favicon.ico" type="image/vnd.microsoft.icon" />
    <script src="/js/jquery-3.1.1.min.js"></script>
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

<!-- <body onload="{loadDoc('/xml/cnc_code.xml', cnc_code_table);}"> -->
<body>

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
		
		<!-- <div class="clearfix solo"> -->
			<div class="left">
				<form action="/php/send_command.php" method="post"> <!--  some js needed -->
					<fieldset>
						<legend>Scale:</legend>
						<input type="hidden" name="command" value="SetMetricOrInch" />
						<label><input type="radio" name="metric_inch" id="metric" value="metric" checked="checked" />Metric</label>
						<br />
						<label><input type="radio" name="metric_inch" id="inch" value="inch" />Inch</label>
					</fieldset>
				</form>
			</div>
				
			<div class="left">
			<form action="/php/send_command.php" method="post">
				<fieldset>
					<legend>X-Origin-Offset:</legend>
					<input type="hidden" name="command" value="SetXOffset" />
					<label><span class="smallfont">(+-5999 mm/100)</span><br />
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
					<input type="hidden" name="command" value="SetZOffset" />
					<label><span class="smallfont">(+-32760 mm/100)</span><br />
					<input type="number" name="zoffset" min="-32760" max="32760" value="0" autocomplete="off" required />
					</label>
					<br />
					<input type="submit" class="button" name="SetZOffset" value="Set Z-Offset" />
				</fieldset>
			</form>
			</div>
		<!-- </div> -->
		
		<div class="left">
		<form action="/php/send_command.php" method="post">
			<fieldset>
				<legend>Spindle:</legend>
				<input type="hidden" name="command" value="SpindleSetRPM" />
				Rotation<br />
				<label><input type="radio" name="spindle_direction" value="0" checked="checked" />right handed</label>
				<br />
				<label><input type="radio" name="spindle_direction" value="1" />left handed</label>
				<br />
				<label>RPM <span class="smallfont">(460 to 3220 U/min)</span>:<br />
				<input type="number" name="rpm" min="460" max="3220" value="460" autocomplete="off" required />
				</label>
				<br />
				<input type="submit" id="SpindleOn" class="button" name="SpindleSetRPM" value="Spindle ON" />
				<br />
				<input type="button" id="SpindleOff" class="button button_red" name="SpindleOff" value="Spindle OFF" />
			</fieldset>
		</form>
		</div>
		
		<div class="left">
		<!-- <form action="php/set_feed.php" method="post"> -->
		<!-- for SetStepperFeed a js-function is needed to set a client-variable, because no message is send -->
		<form action="/php/send_command.php" method="post">
			<fieldset>
				<legend>Stepper:</legend>
				<label>Feed <span class="smallfont">(2 to 499 mm/min)</span>:<br />
				<input type="number" id="feed" name="feed" min="2" max="499" value="50" autocomplete="off" required />
				</label>
				<br />
				<input type="button" id="StepperOn" class="button" name="StepperOn" value="Stepper ON" />
				<br />
				<input type="button" id="StepperOff" class="button button_red" name="StepperOff" value="Stepper OFF" />
			</fieldset>
		</form>
		</div>
		
		<div class="left">
		<form id="tool" action="/php/send_command.php" method="post">
			<fieldset>
				<legend>Tool:</legend>
				<input type="hidden" name="command" value="SetTool" />
				<label>Tool <span class="smallfont">(1 to 6)</span>:<br />
				<input type="number" id="toolnumber" name="tool" min="1" max="6" value="0" autocomplete="off" required />
				</label>
				<br />
				<label>X-Correction <span class="smallfont">(+-5999 mm/100)</span>:<br />
				<input type="number" name="tool_x-correction" min="-5999" max="5999" value="0" autocomplete="off" required />
				</label>
				<br />
				<label>Z-Correction <span class="smallfont">(+-32760 mm/100)</span>:<br />
				<input type="number" name="tool_z-correction" min="-32760" max="32760" value="0" autocomplete="off" required />
				</label>
				<br />
				<input type="submit" id="SetTool" class="button" name="SetTool" value="Set Tool" />
			</fieldset>
		</form>
		</div>
		
		<form class="solo" id="controls">
			<fieldset>
				<legend>Manual Stepper Controls:</legend>
				<input id="XStepperNegativ" type="button" value="-X" />
				<br />
				<input id="ZStepperNegativ" type="button" value="-Z" />
				<input id="ZStepperPositiv" type="button" value="+Z" />
				<br />
				<input id="XStepperPositiv" type="button" value="+X" />
			</fieldset>
		</form>
		
		<div id="manual_responses"></div>
    </article>
    
    <article class="cnc clearfix">
        <h2>CNC-Control</h2>
        
<!--		
        <form enctype="multipart/form-data" method="POST" action="/php/upload_cam-file.php">
-->
		<form>
            <fieldset>
				<legend>Select CAM-File:</legend>
				<input type="hidden" name="MAX_FILE_SIZE" value="500000" />
				<input type="file" name="file-1" id="file-1" class="inputfile inputfile-1" data-multiple-caption="{count} files selected" />
				<label for="file-1"><span></span> <strong><svg xmlns="http://www.w3.org/2000/svg" width="20" height="17" viewBox="0 0 20 17"><path d="M10 0l-5.2 4.9h3.3v5.1h3.8v-5.1h3.3l-5.2-4.9zm9.3 11.5l-3.2-2.1h-2l3.4 2.6h-3.5c-.1 0-.2.1-.2.1l-.8 2.3h-6l-.8-2.2c-.1-.1-.1-.2-.2-.2h-3.6l3.4-2.6h-2l-3.2 2.1c-.4.3-.7 1-.6 1.5l.6 3.1c.1.5.7.9 1.2.9h16.3c.6 0 1.1-.4 1.3-.9l.6-3.1c.1-.5-.2-1.2-.7-1.5z"/></svg> Upload File</strong></label>
				<!--	<input type="submit" class="button" value="Upload File" />	-->
				<input id="UploadChanges" type="button" class="button" value="Upload Changes" />
				<input id="ResetChanges" type="button" class="button button_red" value="Reset Changes" />
				<input id="SaveFile" type="button" class="button" onclick="saveTextAsFile()" value="Save File" />
				<div id="responses"></div>
			</fieldset>
        </form>
		
		<form>
			<fieldset>
				<legend>Programm Control:</legend>
				<input type="hidden" name="command" value="ProgramStart" />
				<label>Start-Block:
				<input id="block" type="number" name="block" min="0" max="9999" value="0" autocomplete="off" required />
				</label>
				<input type="submit" id="ProgramStart" class="button" name="ProgramStart" value="Start" />
				<input type="button" id="ProgramPause" class="button" name="ProgramPause" value="Pause" />
				<input type="button" id="ProgramStop" class="button button_red" name="ProgramStop" value="Stop" />
			</fieldset>
		</form>
		
		<!--
        <table id="code"><thead><tr><th>N</th><th>G/M</th><th>G/M-Code</th><th>X/I</th><th>Z/K</th><th>F/T/L/K</th><th>H/S</th></tr></thead><tbody><tr><td></td><td></td><td></td><td></td><td></td><td></td><td></td></tr></tbody></table>
		
		<p></p>
		-->
		
		<form id="CncCode">
			<label for="CncCodeTxt">CNC-Code:</label><br />
			<textarea id="CncCodeTxt" name="CncCodeTxt" cols="120" rows="50" wrap="soft"></textarea>
		</form>
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
				<div id="SpindleOnLED" class="led-grey"></div>
				<p>Spindle on</p>
	</div>
	<div class="state clearfix">
				<div id="SpindleDirectionLED" class="led-grey"></div>
				<p>Spindle-Direction</p>
	</div>
	<div class="state clearfix">
				<div id="StepperOnLED" class="led-grey"></div>
				<p>Stepper on</p>
	</div>
	<div class="state clearfix">
				<div id="CommandTimeLED" class="led-grey"></div>
				<p>Command-Time</p>
	</div>
	<div class="state clearfix">
				<div id="XStepperMovingLED" class="led-grey"></div>
				<p>X-Stepper moving</p>
	</div>
	<div class="state clearfix">
				<div id="ZStepperMovingLED" class="led-grey"></div>
				<p>Z-Stepper moving</p>
	</div>
	<div class="state clearfix">
				<div id="ToolchangerMovingLED" class="led-grey"></div>
				<p>Toolchanger moving</p>
	</div>
	<div class="state clearfix">
				<div id="CNCCodeNeededLED" class="led-grey"></div>
				<p>CNC-Code needed</p>
	</div>
	<div class="state clearfix">
				<p>RPM <span class="smallfont">[U/min]</span>:</p>
				<div id="RPMDisplaybox" class="Displaybox"></div>	
	</div>
	<div class="state clearfix">
				<p>X <span class="smallfont">[mm/100]</span>:</p>
				<div id="XDisplaybox" class="Displaybox"></div>	
	</div>
	<div class="state clearfix">
				<p>Z <span class="smallfont">[mm/100]</span>:</p>
				<div id="ZDisplaybox" class="Displaybox"></div>	
	</div>
	<div class="state clearfix">
				<p>Feed <span class="smallfont">[mm/min]</span>:</p>
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
				<p>N:</p>
				<div id="NDisplaybox" class="Displaybox"></div>	
	</div>
	<form>
		<input type="button" name="LoadOldParameter" id="LoadOldParameter" class="button" value="Load Parameter" />
	</form>
	<div class="state clearfix">
				<div id="SpiErrorLED" class="led-grey"></div>
				<p>SPI-Error</p>
				<form class="errorcheckboxes">
					<input type="checkbox" name="SpiError" id="SpiError" value="1" />
				</form>
	</div>
	<div class="state clearfix">
				<div id="CNCErrorLED" class="led-grey"></div>
				<p>CNC-Code-Error</p>
				<form class="errorcheckboxes">
					<input type="checkbox" name="CNCError" id="CNCError" value="2" />
				</form>
	</div>
	<div class="state clearfix">
				<div id="SpindleErrorLED" class="led-grey"></div>
				<p>Spindle-Error</p>
				<form class="errorcheckboxes">
					<input type="checkbox" name="SpindleError" id="SpindleError" value="4" />
				</form>
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
			<label>Spindle on:<br />
			<input type="text" name="spindle_on" id="spindle_on" />
			</label>
			<br />
			<label>Spindle-Direction:<br />
			<input type="text" name="init" id="spindle_direction" />
			</label>
			<br />
			<label>Stepper on:<br />
			<input type="text" name="stepper_on" id="stepper_on" />
			</label>
			<br />
			<label>Command-Time:<br />
			<input type="text" name="command_time" id="command_time" />
			</label>
			<br />
			<label>X-Stepper moving:<br />
			<input type="text" name="xstepper_running" id="xstepper_running" />
			</label>
			<br />
			<label>Z-Stepper moving:<br />
			<input type="text" name="zstepper_running" id="zstepper_running" />
			</label>
			<br />
			<label>Toolchanger moving:<br />
			<input type="text" name="toolchanger_running" id="toolchanger_running" />
			</label>
			<br />
			<label>CNC-Code needed:<br />
			<input type="text" name="cnc_code_needed" id="cnc_code_needed" />
			</label>
			<br /><br />
			<label>RPM [U/min]:<br />
			<input type="text" name="rpm_measure" id="rpm_measure" />
			</label>
			<br />
			<label>X [mm/100]:<br />
			<input type="text" name="x_actual" id="x_actual" />
			</label>
			<br />
			<label>Z [mm/100]:<br />
			<input type="text" name="z_actual" id="z_actual" />
			</label>
			<br />
			<label>Feed [mm/min]:<br />
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
			<br />
			<label>N:<br />
			<input type="text" name="n_actual" id="n_actual" />
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
			<label>Spindle-Error:<br />
			<input type="text" name="spindle_error" id="spindle_error" />
			</label>
			<br />
	</form>
-->	
	<form>
		<input type="button" name="ResetErrors" id="ResetErrors" class="button" value="Reset Errors" />
	</form>
	
	<form>
		<input type="button" name="shutdown" id="shutdown" class="button button_red" value="Shutdown Pi" />
	</form>
</aside>

<footer>
    <p>
        Semesterproject SS2016: CNC-Drehbank <br />
        by Hannes Beuter, Hannes Schuhmacher &amp; Niko Ramdorf <br />
        at the University of Applied Sciences Kiel, Germany
	</p>
	<p><a href="/impressum.html">Impressum</a></p>
</footer>

<script src="/js/jquery.custom-file-input.js"></script>

</body>
</html>