<?php

require 'util.php';

$out = read_config($config['autodeflect_root'] . $config['autodaemon_conf']);

if (isset($_GET["pretty"]))
	echo json_encode($out,JSON_PRETTY_PRINT);
else
	echo json_encode($out);


/*******************************************************/

