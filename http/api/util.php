<?php

/*******************************************************/

function read_config($file) {

	$fh = fopen($file, 'r');

	$data = trim(fread($fh, filesize($file)));
	fclose($fh);

	$my_array = explode("\n", $data);

	$assoc_array = array();
	foreach($my_array as $line) {
		$line = trim($line);
		/* Thank The Hammer */
		if (substr($line,0,1) == '#')
			continue;
		if (preg_match('/^(\w+)\s+(.*)$/', $line, $matches)) {
			$assoc_array[$matches[1]] = $matches[2];
		}       
	}


	return $assoc_array;
}


/*******************************************************/

function read_yaml($file) {

	$array = yaml_parse(file_get_contents($file));

	return $array;
}

/*******************************************************/

function get_client_time($file) {

	$array = read_yaml($file);

	if (isset($array['timestamp']))
		$ret = $array['timestamp'];
	else
		$ret = 0;

	return $ret;

}

/*******************************************************/

function site_deflect($site) {
	$ret = 0;
	$file = AUTODEFLECT_ROOT . "/clients.yml";
	if (file_exists($file)) {
		$array = @read_yaml($file);
		if (isset($array['remap'][$site])) 
			$ret = 1;

	}

	return (int)$ret;
}


/*******************************************************/

?>
