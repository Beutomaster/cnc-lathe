<?php
   if(is_writable(".") && isset($_FILES['file'])) {
      move_uploaded_file($_FILES['file']['name'], ".");
   } or die ("Upload Error!");
?>