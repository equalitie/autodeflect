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
		status(404);
}

?>
