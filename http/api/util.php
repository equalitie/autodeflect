<?php

use Symfony\Component\Yaml\Yaml;

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

function read_site_yaml($file) {

	$yaml = $value = Yaml::parse(file_get_contents($file));

	return $yaml;
}

/*******************************************************/

?>
