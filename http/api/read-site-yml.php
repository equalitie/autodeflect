<?php

require 'util.php';

$out = read_yaml($config['autodeflect_root'] . '/site.yml');

if (isset($_GET["pretty"]))
	echo json_encode($out,JSON_PRETTY_PRINT);
else
	echo json_encode($out);


/*******************************************************/

?>
