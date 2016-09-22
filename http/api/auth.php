<?php

if(!isset($_SERVER['PHP_AUTH_USER']) || !isset($_SERVER['PHP_AUTH_PW'])) {
 
	header("WWW-Authenticate: Basic realm=\"Secure Page\"");
	header("HTTP\ 1.1 401 Unauthorized");
	header("Content-Type: application/json");
	echo '{ auth: 0 }';
	exit;
}

if (simple_auth($_SERVER['PHP_AUTH_USER'], $_SERVER['PHP_AUTH_PW'])) {
	// Do nothing
	;;
} else {
	header("WWW-Authenticate: Basic realm=\"Secure Page\"");
	header("HTTP\ 1.1 401 Unauthorized");
	header("Content-Type: application/json");
	echo '{ auth: 0 }';
	exit;
}

/********************************************/

function simple_auth($user, $pass) {

	require_once '../config.php';
	if (htmlentities($user) == htmlentities(API_USER) && htmlentities($pass) == htmlentities(API_PASS)) 
		return 1;
	else
		return 0;
}
?>
