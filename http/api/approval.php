<?php


require 'util.php';

$conf = read_config(AUTODEFLECT_ROOT . AUTODAEMON_CONF);

$out = array();

// Check and make sure a site is passed
if ( isset($_GET['site']) && site_deflect(htmlentities($_GET['site'])) != 0 ) {
	$out['site'] = htmlentities($_GET['site']);

} else {
	$out['site'] = 0;
}

if (isset($_GET["pretty"]))
	echo json_encode($out,JSON_PRETTY_PRINT);
else
	echo json_encode($out);

?>
