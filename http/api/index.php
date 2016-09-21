<?php

switch ($name) {

	case 'status':
		require 'status.php';
		break;

	default:
		status(404);
}

?>
