<?php
require 'util.php';

$out = array();
$edgelist_glob = $config['edgemanage_root'] . '/edges/*';

// Set $extra_ip_array if file config/api_extra_accesslist exists
if (file_exists($config['autodeflect_root'] . '/config/api_extra_accesslist')) {
	$api_extra_accesslist = $config['autodeflect_root'] . '/config/api_extra_accesslist';
	$fh = fopen($api_extra_accesslist, 'r');
	$data = trim(fread($fh, filesize($api_extra_accesslist)));
	fclose($fh);

	$data_array = explode("\n", $data);
	$extra_ip_array = array();
	foreach($data_array as $line) {
		$line = trim($line);
		if (substr($line,0,1) == '#')
			continue;
		if (filter_var($line, FILTER_VALIDATE_IP))
			array_push($extra_ip_array,$line);
	}
}

foreach (glob("$edgelist_glob") as $dnet) {
	$fh = fopen($dnet, 'r');
	$data = trim(fread($fh, filesize($dnet)));
	fclose($fh);

	$data_array = explode("\n", $data);
	$edge_ip_array = array();
	foreach($data_array as $line) {
		$line = trim($line);
		if (substr($line,0,1) == '#')
			continue;
		$ip = gethostbyname($line);
		if (filter_var($ip, FILTER_VALIDATE_IP))
			array_push($edge_ip_array,$ip);
	}
}

if (!empty($edge_ip_array))
	array_push($out,$edge_ip_array);
if (!empty($extra_ip_array))
	array_push($out,$extra_ip_array);

// Sort low to high
ksort($out);

if (isset($_GET["pretty"]))
	print json_encode($out[0],JSON_PRETTY_PRINT);
else
	print json_encode($out[0]);
?>
