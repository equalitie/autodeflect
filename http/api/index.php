<?php

require 'auth.php';

switch ($name) {

	case 'status':
		require 'status.php';
		break;

	case 'read-conf':
		require 'read-conf.php';
		break;

	case 'read-site-yml';
		require 'read-site-yml.php';
		break;

	default:
		header('HTTP/1.0 404 Not Found');
		header("Content-Type: application/json");
		echo "{ http_code: 404 }\n";
		die();
}

?>
