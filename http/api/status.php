<?php

require 'util.php';

$conf = read_config(AUTODEFLECT_ROOT . AUTODAEMON_CONF);

$out = array();

if (file_exists(AUTODEFLECT_ROOT . CLIENT_LAST_FILE)) {
	$out['client_last_success'] = get_client_time(AUTODEFLECT_ROOT . CLIENT_LAST_FILE);
} else {
	$out['client_last_success'] = 0;
}

if (file_exists($conf['directory_script'] . "/" . $conf['process_file'] . $conf['pid_suffix'])) {
	$timestamp = file_get_contents($conf['directory_script'] . "/" . $conf['process_file'] . $conf['pid_suffix']); 
	if (file_exists(AUTODEFLECT_ROOT . "/clients.yml")) {
		$client_current = get_client_time(AUTODEFLECT_ROOT . "/clients.yml");
		if (intval($out['client_last_success']) <= intval($client_current)) {
			$out['client_last_running'] = intval($client_current);
		} else {
			$out['client_last_running'] = 0;
		}
	}
	$out['running'] = intval($timestamp);

} else {
	if ((!isset($out['client_last_running'])) || ($out['client_last_running'] == 0) )
		$out['client_last_running'] = 0;

	$out['running'] = 0;
}

if (file_exists(AUTODEFLECT_ROOT . "/config/rundata/.completed_timestamp")) {
	$timestamp = file_get_contents(AUTODEFLECT_ROOT . "/config/rundata/.completed_timestamp");
	$out['last_success'] = intval(trim($timestamp) * 1000);
} else {
	$out['last_success'] = 0;
}

if (file_exists(AUTODEFLECT_ROOT . "/config/rundata/.aw_lastrun")) {
	$timestamp = file_get_contents(AUTODEFLECT_ROOT . "/config/rundata/.aw_lastrun");
	$out['last_awstats'] = intval(trim($timestamp) * 1000);
} else {
	$out['last_awstats'] = 0;
}

if (file_exists($conf['directory_run'] . "/ad2p" . $conf['pid_suffix'] . ".pid")) {
	$out['autodeflect_process'] = 1;
} else {
	$out['autodeflect_process'] = 0;
}

if (file_exists($conf['directory_run'] . "/ad2ssh" . $conf['pid_suffix'] . ".pid")) {
	$out['autodeflect_sshkey'] = 1;
} else {
	$out['autodeflect_sshkey'] = 0;
}

if (file_exists($conf['directory_run'] . "/ad2runner" . $conf['pid_suffix'] . ".pid")) {
	$out['autodeflect_runner'] = 1;
} else {
	$out['autodeflect_runner'] = 0;
}

$maint_mode = glob(AUTODEFLECT_ROOT . "/config/rundata/.maint_lock-*");
if (count($maint_mode) > 0) {
	$out['maintenance'] = 1;
} else {
	$out['maintenance'] = 0;
}

if (isset($_GET["pretty"]))
	echo json_encode($out,JSON_PRETTY_PRINT);
else
	echo json_encode($out);

?>
