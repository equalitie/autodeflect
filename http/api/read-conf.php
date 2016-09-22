<?php

require 'util.php';

$out = read_config(AUTODEFLECT_ROOT . AUTODAEMON_CONF);

if (isset($_GET["pretty"]))
	echo json_encode($out,JSON_PRETTY_PRINT);
else
	echo json_encode($out);


/*******************************************************/

