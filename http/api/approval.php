<?php

require 'util.php';

$conf = read_config($config['autodeflect_root'] . $config['autodaemon_conf']);

$out = array();

// Check and make sure a site is passed
if ( isset($_GET['site']) && site_deflect($config['autodeflect_root'] . 'clients.yml',htmlentities($_GET['site'])) != 0 ) {

	// set site to site name that was passed since we found it in clients.yml
	$out['site'] = htmlentities($_GET['site']);


} else {
// At this point we just want to print out
// json { "site": 0 } 
	$out['site'] = 0;

}

if (isset($_GET["pretty"]))
	echo json_encode($out,JSON_PRETTY_PRINT);
else
	echo json_encode($out);

