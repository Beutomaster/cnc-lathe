<?php
 
$encrypted_password = password_hash('12345', PASSWORD_DEFAULT);
echo "blowfish-hash: ";
echo $encrypted_password;
echo "<br />";
$encrypted_password = md5('12345');
echo "MD5_ENCRYPT: ";
echo $encrypted_password;
?>