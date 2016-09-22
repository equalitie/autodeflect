<?php

require 'util.php';

$out = read_site_yaml(AUTODEFLECT_ROOT . '/site.yml');

if (isset($_GET["pretty"]))
	echo json_encode($out,JSON_PRETTY_PRINT);
else
	echo json_encode($out);


/*******************************************************/

?>
