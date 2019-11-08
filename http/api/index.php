<?php

require 'auth.php';

switch ($name) {

	case 'status':
		require 'status.php';
		break;

	case 'read-conf':
		require 'read-conf.php';
		break;

	case 'read-site-yml':
		require 'read-site-yml.php';
		break;

	case 'approval':
		require 'approval.php';
		break;

	case 'access-list':
		require 'access-list.php';
		break;

	case 'edges':
		require 'edges.php';
		break;

	case 'php-info':
		require 'php-info.php';
		break;

	default:
		header("$_SERVER['SERVER_PROTOCOL'] 404 Not Found");
		header("Content-Type: application/json");
		echo "{ http_code: 404 }\n";
		die();
}

?>
