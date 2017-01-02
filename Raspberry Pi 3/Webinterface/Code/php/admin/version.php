<?php
// gibt z.B. 'Die aktuelle PHP-Version ist 4.1.1' aus
echo 'Die aktuelle PHP Version ist ' . phpversion();

// Gibt z.B. '2.0' aus oder nichts, falls die Extension nicht aktiviert ist
echo phpversion('tidy');
?>